//#
//# FILE: extform.h -- Declaration of extensive form data type
//#
//# $Id$
//#

#ifndef EXTFORM_H
#define EXTFORM_H

#include "gstring.h"
#include "gblock.h"

class Outcome;
class Player;
class Infoset;
class Node;
class Action;

class BaseExtForm     {
  friend class EfgFileReader;
  protected:
    gString title;
    gBlock<Player *> players;
    gBlock<Outcome *> outcomes;
    Node *root;
    Player *chance;

    gBlock<Node *> dead_nodes;
    gBlock<Infoset *> dead_infosets;
    gBlock<Outcome *> dead_outcomes;

       //# PROTECTED CONSTRUCTORS -- FOR DERIVED CLASS USE ONLY
    BaseExtForm(void);
    BaseExtForm(const BaseExtForm &);

    void CopySubtree(Node *, Node *, Node *);

    void DisplayTree(gOutput &, Node *) const;
    void WriteEfgFile(gOutput &, Node *) const;

    virtual Infoset *CreateInfoset(int n, Player *pl, int br) = 0;
    virtual Node *CreateNode(Node *parent) = 0;

    void ScrapNode(Node *);
    void ScrapInfoset(Infoset *);
    void ScrapOutcome(Outcome *);

// These are auxiliary functions used by the .efg file reader code
    Infoset *GetInfosetByIndex(Player *p, int index) const;
    Infoset *CreateInfosetByIndex(Player *p, int index, int br);
    Outcome *GetOutcomeByIndex(int index) const;
    virtual Outcome *CreateOutcomeByIndex(int index) = 0;
    void Reindex(void);

  public:
       //# DESTRUCTOR
    virtual ~BaseExtForm();

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

    Player *GetChance(void) const;
    Player *NewPlayer(void);
    const gArray<Player *> &PlayerList(void) const  { return players; }

       //# DATA ACCESS -- OUTCOMES
    int NumOutcomes(void) const;
    const gArray<Outcome *> &OutcomeList(void) const  { return outcomes; }
    void DeleteOutcome(Outcome *c);  
 
       //# EDITING OPERATIONS
    Infoset *AppendNode(Node *n, Player *p, int br);
    Infoset *AppendNode(Node *n, Infoset *s);
    Node *DeleteNode(Node *n, Node *keep);
    Infoset *InsertNode(Node *n, Player *p, int br);
    Infoset *InsertNode(Node *n, Infoset *s);

    virtual Infoset *CreateInfoset(Player *pl, int br) = 0;
    Infoset *JoinInfoset(Infoset *s, Node *n);
    Infoset *LeaveInfoset(Node *n);
    Infoset *MergeInfoset(Infoset *to, Infoset *from);

    Infoset *SwitchPlayer(Infoset *s, Player *p);

    Node *CopyTree(Node *src, Node *dest);
    Node *MoveTree(Node *src, Node *dest);
    Node *DeleteTree(Node *n);

    Infoset *AppendAction(Infoset *s);
    Infoset *InsertAction(Infoset *s, Action *a);
    Infoset *DeleteAction(Infoset *s, Action *a);
};


template <class T> class OutcomeVector;
#include "behav.h"

template <class T> class ExtForm : public BaseExtForm   {
  private:
    ExtForm<T> &operator=(const ExtForm<T> &);

    void Payoff(Node *n, T, const gPVector<int> &, gVector<T> &) const;
    void Payoff(Node *n, T prob, int pl, T &value,
		const BehavProfile<T> &profile) const;
    void CondPayoff(Node *n, T prob, const BehavProfile<T> &,
		    gPVector<T> &, gDPVector<T> &) const;

    Infoset *CreateInfoset(int n, Player *pl, int br);
    Node *CreateNode(Node *parent);
    Outcome *CreateOutcomeByIndex(int index);

    // this is for use with the copy constructor
    void CopySubtree(Node *, Node *);

  public:
	//# CONSTRUCTORS AND DESTRUCTOR
    ExtForm(void);
    ExtForm(const ExtForm<T> &);
    virtual ~ExtForm(); 

	//# DATA ACCESS -- GENERAL INFORMATION
    DataType Type(void) const;

        //# DATA ACCESS -- OUTCOMES
    OutcomeVector<T> *NewOutcome(void);

    Infoset *CreateInfoset(Player *pl, int br);

        //# COMPUTING VALUES OF PROFILES
//    gDPVector<T> *NewBehavProfile(void) const;

    int ProfileLength(bool trunc = false) const;
    gPVector<int> Dimensionality(bool trunc = false) const;
    void Centroid(BehavProfile<T> &profile) const;

    void Payoff(const gPVector<int> &profile, gVector<T> &payoff) const;
    T Payoff(int pl, const BehavProfile<T> &) const;
    void CondPayoff(const BehavProfile<T> &profile, gDPVector<T> &value,
		    gPVector<T> &probs) const;
};

#include "player.h"
#include "infoset.h"
#include "node.h"
#include "outcome.h"

// These functions are provided in readefg.y/readefg.cc
template <class T> int ReadEfgFile(gInput &, ExtForm<T> *&);
void EfgFileType(gInput &f, bool &valid, DataType &type);

#endif   //# EXTFORM_H


