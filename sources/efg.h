//#
//# FILE: efg.h -- Declaration of extensive form data type
//#
//# $Id$
//#

#ifndef EFG_H
#define EFG_H

#include "gstring.h"
#include "gblock.h"

class Outcome;
class EFPlayer;
class Infoset;
class Node;
class Action;
class Lexicon;

class BaseEfg     {
  
  private:
    // this is used to track memory leakage; #define MEMCHECK to use it
    static int _NumObj;
  
  friend class EfgFileReader;
  protected:
    gString title;
    gBlock<EFPlayer *> players;
    gBlock<Outcome *> outcomes;
    Node *root;
    EFPlayer *chance;
    Lexicon *lexicon;

    gBlock<Node *> dead_nodes;
    gBlock<Infoset *> dead_infosets;
    gBlock<Outcome *> dead_outcomes;

       //# PROTECTED CONSTRUCTORS -- FOR DERIVED CLASS USE ONLY
    BaseEfg(void);
    BaseEfg(const BaseEfg &);

    void CopySubtree(Node *, Node *, Node *);

    void DisplayTree(gOutput &, Node *) const;
    void WriteEfgFile(gOutput &, Node *) const;

    virtual Infoset *CreateInfoset(int n, EFPlayer *pl, int br) = 0;
    virtual Node *CreateNode(Node *parent) = 0;

    void ScrapNode(Node *);
    void ScrapInfoset(Infoset *);
    void ScrapOutcome(Outcome *);

// These are auxiliary functions used by the .efg file reader code
    Infoset *GetInfosetByIndex(EFPlayer *p, int index) const;
    Infoset *CreateInfosetByIndex(EFPlayer *p, int index, int br);
    Outcome *GetOutcomeByIndex(int index) const;
    virtual Outcome *CreateOutcomeByIndex(int index) = 0;
    void Reindex(void);
    
    void DeleteLexicon(void);

  public:
       //# DESTRUCTOR
    virtual ~BaseEfg();

       //# TITLE ACCESS AND MANIPULATION
    void SetTitle(const gString &s);
    const gString &GetTitle(void) const;

       //# WRITING DATA FILES
    void DisplayTree(gOutput &) const;
    void WriteEfgFile(gOutput &f) const;

       //# DATA ACCESS -- GENERAL INFORMATION
    virtual DataType Type(void) const = 0;

    Node *RootNode(void) const;
    bool IsSuccessor(const Node *n, const Node *from) const;
    bool IsPredecessor(const Node *n, const Node *of) const;

       //# DATA ACCESS -- PLAYERS
    int NumPlayers(void) const;

    EFPlayer *GetChance(void) const;
    EFPlayer *NewPlayer(void);
    const gArray<EFPlayer *> &PlayerList(void) const  { return players; }

       //# DATA ACCESS -- OUTCOMES
    int NumOutcomes(void) const;
    const gArray<Outcome *> &OutcomeList(void) const  { return outcomes; }
    void DeleteOutcome(Outcome *c);  
 
       //# EDITING OPERATIONS
    Infoset *AppendNode(Node *n, EFPlayer *p, int br);
    Infoset *AppendNode(Node *n, Infoset *s);
    Node *DeleteNode(Node *n, Node *keep);
    Infoset *InsertNode(Node *n, EFPlayer *p, int br);
    Infoset *InsertNode(Node *n, Infoset *s);

    virtual Infoset *CreateInfoset(EFPlayer *pl, int br) = 0;
    Infoset *JoinInfoset(Infoset *s, Node *n);
    Infoset *LeaveInfoset(Node *n);
    Infoset *MergeInfoset(Infoset *to, Infoset *from);

    Infoset *SwitchPlayer(Infoset *s, EFPlayer *p);

    Node *CopyTree(Node *src, Node *dest);
    Node *MoveTree(Node *src, Node *dest);
    Node *DeleteTree(Node *n);

    Infoset *AppendAction(Infoset *s);
    Infoset *InsertAction(Infoset *s, Action *a);
    Infoset *DeleteAction(Infoset *s, Action *a);
};


template <class T> class OutcomeVector;
#include "behav.h"
template <class T> class Nfg;
template <class T> class MixedProfile;

template <class T> class Efg : public BaseEfg   {
  private:
    Efg<T> &operator=(const Efg<T> &);

    void Payoff(Node *n, T, const gPVector<int> &, gVector<T> &) const;
    void Payoff(Node *n, T, const gArray<gArray<int> *> &, gVector<T> &) const;
    void Payoff(Node *n, T prob, int pl, T &value,
		const BehavProfile<T> &profile) const;
    void NodeValues(Node *n, T prob, int pl, T&value, gArray<T> &valarray,
		    const BehavProfile<T> &profile, int &index) const;
    void CondPayoff(Node *n, T prob, const BehavProfile<T> &,
		    gPVector<T> &, gDPVector<T> &) const;
    void NodeRealizProbs(Node *n, T prob, const BehavProfile<T> &profile,
			 int &index, gArray<T> &NRProbs);
    void Beliefs(Node *n, T prob, const BehavProfile<T> &profile,
		 int &index, gArray<T> &BProbs, gPVector<T> &gpv);

    Infoset *CreateInfoset(int n, EFPlayer *pl, int br);
    Node *CreateNode(Node *parent);
    Outcome *CreateOutcomeByIndex(int index);


    // this is for use with the copy constructor
    void CopySubtree(Node *, Node *);

  public:
	//# CONSTRUCTORS AND DESTRUCTOR
    Efg(void);
    Efg(const Efg<T> &);
    virtual ~Efg(); 

	//# DATA ACCESS -- GENERAL INFORMATION
    DataType Type(void) const;

        //# DATA ACCESS -- OUTCOMES
    OutcomeVector<T> *NewOutcome(void);

    Infoset *CreateInfoset(EFPlayer *pl, int br);

        //# COMPUTING VALUES OF PROFILES
//    gDPVector<T> *NewBehavProfile(void) const;


    int ProfileLength(bool trunc = false) const;
    gPVector<int> Dimensionality(bool trunc = false) const;
    void Centroid(BehavProfile<T> &profile) const;

    void Payoff(const gPVector<int> &profile, gVector<T> &payoff) const;
    void Payoff(const gArray<gArray<int> *> &profile, gVector<T> &payoff) const;
    T Payoff(int pl, const BehavProfile<T> &) const;
    gArray<T> NodeValues(int pl, const BehavProfile<T> &) const;
    void CondPayoff(const BehavProfile<T> &profile, gDPVector<T> &value,
		    gPVector<T> &probs) const;

    gArray<T> NodeRealizProbs(const BehavProfile<T> &);
    gArray<T> Beliefs(const BehavProfile<T> &);

    friend Nfg<T> *MakeReducedNfg(Efg<T> &);
    friend void MixedToBehav(const Nfg<T> &N, const MixedProfile<T> &mp,
		             const Efg<T> &E, BehavProfile<T> &bp);

    // defined in efgutils.cc
    friend void Random(Efg<T> &, int seed = 0);
};

#include "efplayer.h"
#include "infoset.h"
#include "node.h"
#include "outcome.h"

// These functions are provided in readefg.y/readefg.cc
template <class T> int ReadEfgFile(gInput &, Efg<T> *&);
void EfgFileType(gInput &f, bool &valid, DataType &type);

#endif   //# EFG_H


