//
// FILE: efg.h -- Declaration of extensive form data type
//
// $Id$
//

#ifndef EFG_H
#define EFG_H

#include "gstring.h"
#include "gblock.h"
#include "glist.h"

#include "gpvector.h"


class Outcome;
class EFPlayer;
class Infoset;
class Node;
class Action;
class Lexicon;
class BaseNfg;
class EFSupport;

class BaseEfg     {
  
private:
  // this is used to track memory leakage; #define MEMCHECK to use it
  static int _NumObj;
  
friend class EfgFileReader;
friend class BaseNfg;  

protected:
    bool sortisets;
    gString title;
    gBlock<EFPlayer *> players;
    gBlock<Outcome *> outcomes;
    Node *root;
    EFPlayer *chance;
    Lexicon *lexicon;
    BaseNfg *afg;

    gBlock<Node *> dead_nodes;
    gBlock<Infoset *> dead_infosets;
    gBlock<Outcome *> dead_outcomes;

       //# PROTECTED CONSTRUCTORS -- FOR DERIVED CLASS USE ONLY
    BaseEfg(void);
    BaseEfg(const BaseEfg &);

    void CopySubtree(Node *, Node *, Node *);
    void MarkSubtree(Node *);
    void UnmarkSubtree(Node *);

    void DisplayTree(gOutput &, Node *) const;
    void WriteEfgFile(gOutput &, Node *) const;

    virtual Infoset *CreateInfoset(int n, EFPlayer *pl, int br) = 0;
    virtual Node *CreateNode(Node *parent) = 0;

    void ScrapNode(Node *);
    void ScrapInfoset(Infoset *);
    void ScrapOutcome(Outcome *);

    void SortInfosets(void);

  
// These are auxiliary functions used by the .efg file reader code
    Infoset *GetInfosetByIndex(EFPlayer *p, int index) const;
    Infoset *CreateInfosetByIndex(EFPlayer *p, int index, int br);
    Outcome *GetOutcomeByIndex(int index) const;
    virtual Outcome *CreateOutcomeByIndex(int index) = 0;
    void Reindex(void);
    
    void DeleteLexicon(void);

// These are used in identification of subgames
    void MarkTree(Node *, Node *);
    bool CheckTree(Node *, Node *);
    void MarkSubgame(Node *, Node *);

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
    virtual bool IsConstSum(void) const = 0;

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
    bool DeleteEmptyInfoset(Infoset *);
    Infoset *JoinInfoset(Infoset *s, Node *n);
    Infoset *LeaveInfoset(Node *n);
    Infoset *SplitInfoset(Node *n);
    Infoset *MergeInfoset(Infoset *to, Infoset *from);

    Infoset *SwitchPlayer(Infoset *s, EFPlayer *p);

    Node *CopyTree(Node *src, Node *dest);
    Node *MoveTree(Node *src, Node *dest);
    Node *DeleteTree(Node *n);

    Action *InsertAction(Infoset *s);
    Action *InsertAction(Infoset *s, Action *at);
    Infoset *DeleteAction(Infoset *s, Action *a);

    void Reveal(Infoset *, const gArray<EFPlayer *> &);

    // This function put in to facilitate error-detection in MixedToBehav[]
    friend BaseNfg *AssociatedNfg(BaseEfg *E);
    friend BaseNfg *AssociatedAfg(BaseEfg *E);

    // Unmarks all subgames in the subtree rooted at n
    void UnmarkSubgames(Node *n);
    bool IsLegalSubgame(Node *n);

    // Mark all the (legal) subgames in the list
    void MarkSubgames(const gList<Node *> &list);
    bool DefineSubgame(Node *n);
    void RemoveSubgame(Node *n);


    int ProfileLength(void) const;
    gArray<int> PureDimensionality(void) const;
    gPVector<int> Dimensionality(void) const;
    gPVector<int> BeliefDimensionality(void) const;
};


template <class T> class OutcomeVector;
#include "behav.h"

template <class T> class Efg;
template <class T> class BehavProfile;
template <class T> class Nfg;
template <class T> class MixedProfile;

template <class T> class Efg : public BaseEfg   {
  private:
    Efg<T> &operator=(const Efg<T> &);

    void Payoff(Node *n, T, const gPVector<int> &, gVector<T> &) const;
    void Payoff(Node *n, T, const gArray<gArray<int> *> &, gVector<T> &) const;


    Infoset *CreateInfoset(int n, EFPlayer *pl, int br);
    Node *CreateNode(Node *parent);
    Outcome *CreateOutcomeByIndex(int index);


    // this is for use with the copy constructor
    void CopySubtree(Node *, Node *);

  public:
	//# CONSTRUCTORS AND DESTRUCTOR
    Efg(void);
    Efg(const BaseEfg &);
    Efg(const Efg<T> &);
    Efg(const Efg<T> &, Node *);
    virtual ~Efg(); 

	//# DATA ACCESS -- GENERAL INFORMATION
    DataType Type(void) const;
    bool IsConstSum(void) const;
    T MinPayoff(int pl = 0) const;
    T MaxPayoff(int pl = 0) const;

        //# DATA ACCESS -- OUTCOMES
    OutcomeVector<T> *NewOutcome(void);

    Infoset *CreateInfoset(EFPlayer *pl, int br);

        //# COMPUTING VALUES OF PROFILES

    void Payoff(const gPVector<int> &profile, gVector<T> &payoff) const;
    void Payoff(const gArray<gArray<int> *> &profile, 
		gVector<T> &payoff) const;

    friend Nfg<T> *MakeReducedNfg(Efg<T> &, const EFSupport &);
    friend Nfg<T> *MakeAfg(Efg<T> &);
    friend void MixedToBehav(const Nfg<T> &N, const MixedProfile<T> &mp,
		             const Efg<T> &E, BehavProfile<T> &bp);
    friend void BehavToMixed(const Efg<T> &, const BehavProfile<T> &,
			     const Nfg<T> &, MixedProfile<T> &);
    
    // defined in efgutils.cc
    friend void RandomEfg(Efg<T> &);
};

#include "efplayer.h"
#include "infoset.h"
#include "node.h"
#include "outcome.h"

// These functions are provided in readefg.y/readefg.cc
template <class T> int ReadEfgFile(gInput &, Efg<T> *&);
void EfgFileType(gInput &f, bool &valid, DataType &type);

#endif   //# EFG_H


