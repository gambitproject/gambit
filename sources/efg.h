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
  protected:
    gString title;
    gBlock<Player *> players;
    gBlock<Outcome *> outcomes;
    Node *root;
    Player *chance;

       //# PROTECTED CONSTRUCTORS -- FOR DERIVED CLASS USE ONLY
    BaseExtForm(void);

    void DisplayTree(gOutput &, Node *) const;
    void WriteEfgFile(gOutput &, Node *) const;
    virtual Infoset *CreateInfoset(int n, Player *pl, int br) = 0;
    virtual Node *CreateNode(Node *parent) const = 0;

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

    Player *GetPlayer(const gString &name) const;
    Player *GetChance(void) const;
    Player *NewPlayer(void);
    const gArray<Player *> &PlayerList(void) const  { return players; }

       //# DATA ACCESS -- OUTCOMES
    int NumOutcomes(void) const;
    const gArray<Outcome *> &OutcomeList(void) const  { return outcomes; }
 
       //# EDITING OPERATIONS
    Infoset *AppendNode(Node *n, Player *p, int br);
    Infoset *AppendNode(Node *n, Infoset *s);
    Node *DeleteNode(Node *n, Node *keep);
    Infoset *InsertNode(Node *n, Player *p, int br);
    Infoset *InsertNode(Node *n, Infoset *s);

    Infoset *JoinInfoset(Infoset *s, Node *n);
    Infoset *LeaveInfoset(Node *n);
    Infoset *MergeInfoset(Infoset *to, Infoset *from);

    Infoset *SwitchPlayer(Infoset *s, Player *p);
    Infoset *SwitchPlayer(Node *n, Player *p);

    Node *CopyTree(Node *src, Node *dest);
    Node *MoveTree(Node *src, Node *dest);
    Node *DeleteTree(Node *n);

    Infoset *InsertAction(Infoset *s, Action *a, int count = 1);
    Infoset *DeleteAction(Infoset *s, Action *a);
};


#include "outcome.h"
#include "behav.h"

template <class T> class ExtForm : public BaseExtForm   {
  private:
//
// These are being defined privately for now so they are not accidentally
// used.  They will be implemented later.
//+grp
    ExtForm(const ExtForm<T> &);
    ExtForm<T> &operator=(const ExtForm<T> &);
//-grp

    void Payoff(Node *n, T, const gPVector<int> &, gVector<T> &) const;
    void Payoff(Node *n, T prob, int pl, T &value,
		const BehavProfile<T> &profile) const;
    gVector<T> CondPayoff(Node *n, T prob, const gVector<T> &,
			  const BehavProfile<T> &, gPVector<T> &,
			  gDPVector<T> &) const;

    Infoset *CreateInfoset(int n, Player *pl, int br);
    Node *CreateNode(Node *parent) const;

  public:
	//# CONSTRUCTORS AND DESTRUCTOR
    ExtForm(void);
    virtual ~ExtForm(); 

	//# DATA ACCESS -- GENERAL INFORMATION
    DataType Type(void) const;

        //# DATA ACCESS -- OUTCOMES
    OutcomeVector<T> *NewOutcome(void);
    OutcomeVector<T> *GetOutcome(const gString &) const;

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


// These functions are provided in readefg.y/readefg.cc
int ReadEfgFile(gInput &, BaseExtForm *&);
template <class T> int ReadEfgFile(gInput &, ExtForm<T> *&);

#endif   //# EXTFORM_H


