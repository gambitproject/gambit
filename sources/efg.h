//#
//# FILE: extform.h -- Declaration of extensive form data type
//#
//# $Id$
//#

#ifndef EXTFORM_H
#define EXTFORM_H

#include "gstring.h"
#include "gtuple.h"
#include "gmap.h"
#include "noderep.h"
#include "gpvector.h"
#include "glist.h"

template <class T> class gDPVector;
class Outcome;
class NodeSet;
//
// The extensive form class contains all the functionality necessary for
// convenient construction and manipulation of extensive form games.
// Support is available for multiple subgames, thus allowing for the
// construction of repeated and stochastic games.
//
// The subgames of an extensive form are assigned arbitrary positive
// integers.  There is always a subgame number 1 defined; this game is
// considered the "root" subgame and the root node of this subgame is the
// root node of the whole extensive form.  Other subgames may be assigned
// any number; they need not be consecutive.
//
class BaseExtForm     {
  friend class EfgFileReader;
  protected:
    gString title;
    gTuple<gString> players;
    gSparseSet<NodeSet *> nodes;
    gList<Outcome *> outcomes;
  
    void AddPlayer(int p);
    int CreateInfoset(int p, int game, int iset);

    Node DeleteSubtree(Node);
    Node DeleteTerminalNode(const Node &);

  public:
       //# CONSTRUCTORS AND DESTRUCTOR
    BaseExtForm(void);
    virtual ~BaseExtForm();

       //# TITLE ACCESS AND MANIPULATION
    void SetTitle(const gString &s);
    const gString &GetTitle(void) const;

       //# WRITING DATA FILES
    virtual void WriteEfgFile(gOutput &f) const = 0;
    virtual void DisplayTree(gOutput &, int game = 1) const = 0;

       //# DATA ACCESS -- GENERAL INFORMATION
    virtual DataType Type(void) const = 0;

    int NumGames(void) const;
    int NumNodes(void) const;
    int NumNodes(int game) const;
    int NumNodes(int game, int pl) const;
    int NumNodes(int game, int pl, int iset) const;
    int NumPlayers(void) const;
    int NumInfosets(int game, int pl) const;
    int NumActions(int game, int pl, int iset) const;
    int NumOutcomes(void) const;

	//# DATA ACCESS -- NODES
    Node RootNode(int game = 1) const;
    Node GetParent(const Node &n) const;
    int NumChildren(const Node &n) const;
    Node GetChildNumber(const Node &n, int number) const;
    gBlock<Node> GetChildren(const Node &n) const;
    Node GetPriorSibling(const Node &n) const;
    Node GetNextSibling(const Node &n) const;
    int HasSuccessorGame(const Node &n) const;
    Node GetSuccessorGameRoot(const Node &n) const;
    int IsSuccessor(const Node &n, const Node &from) const;
    int IsPredecessor(const Node &n, const Node &of) const;
    gString GetNodeLabel(const Node &n) const;  
    int GetOutcome(const Node &n) const;
    int GetNextGame(const Node &n) const;
    void SetNextGame(const Node &n, int game);

       //# TREE MODIFICATION ROUTINES -- PLAYERS
    gString GetPlayerLabel(int) const;
    void LabelPlayer(int, const gString &);

        //# TREE MODIFICATION ROUTINES -- GAME ELEMENTS
    virtual int CreateGame(void) = 0;
    virtual int CreateGame(int game, int from_file = 0) = 0;
    virtual void RemoveGame(int game) = 0;
    gString GetGameLabel(int game) const;
    void LabelGame(int game, const gString &name);
    int IsGameDefined(int game) const;

	//# TREE MODIFICATION ROUTINES -- NODES
    Node AddNode(const Node &n, int player, int child_count);
    void LabelNode(const Node &n, const gString &s);
    Node InsertNode(const Node &n, int player, int child_count);
    Node DeleteNode(const Node &n, int keep);
    void SetOutcome(const Node &n, int outcome);

        //# TREE MODIFICATION ROUTINES -- INFORMATION SETS
    Node JoinInfoset(const Node &new_node, const Node &to_iset);
    Node LeaveInfoset(const Node &n);
    Node MergeInfoset(const Node &from, const Node &into);
    void LabelInfoset(const Node &n, const gString &label);
    void LabelInfoset(int game, int pl, int iset, const gString &label);
    gString GetInfosetLabel(const Node &n) const;

        //# TREE MODIFICATION ROUTINES -- ACTIONS
    void AppendAction(int game, int pl, int iset);
    void InsertAction(const Node &n, int where, int number);
    Node DeleteAction(const Node &n, int which);
    void LabelAction(const Node &n, int act, const gString &label);
    void LabelAction(int game, int pl, int iset, int act, const gString &s);
    gString GetActionLabel(const Node &n, int act) const;

        //# TREE MODIFICATION ROUTINES -- SUBTREES
    Node MoveTree(Node from, Node dest);
    Node CopyTree(Node from, Node dest);
    Node DeleteTree(const Node &n);

        //# TREE MODIFICATION ROUTINES -- OUTCOMES
    virtual int CreateOutcome(void) = 0;
    virtual int CreateOutcome(int outc) = 0;
    void RemoveOutcome(int outc);
    gString GetOutcomeLabel(int outc) const;
    void LabelOutcome(int outc, const gString &name);

        //# MANAGEMENT OF VARIABLES
    gString GetUniqueVariable(void) const;
    int IsVariableDefined(const gString &name) const;
    Node GetNodeVariable(const gString &name) const;
    int SetNodeVariable(const gString &name, const Node &node);
    void RemoveNodeVariable(const gString &name);

};


template <class T> class ExtForm : public BaseExtForm   {
  friend class EfgFileReader;
  private:
//
// These are being defined privately for now so they are not accidentally
// used.  They will be implemented later.
//+grp
    ExtForm(const ExtForm<T> &);
    ExtForm<T> &operator=(const ExtForm<T> &);
//-grp

    ExtForm(int);

  public:
	//# CONSTRUCTORS AND DESTRUCTOR
    ExtForm(void);
    ~ExtForm(); 

        //# WRITING DATA FILES
    void WriteEfgFile(gOutput &f) const;
    void DisplayTree(gOutput &, int game = 1) const;

	//# DATA ACCESS -- GENERAL INFORMATION
    DataType Type(void) const;

        //# TREE MODIFICATION ROUTINES -- GAME ELEMENTS
    int CreateGame(void);
    int CreateGame(int game, int from_file = 0);
    void RemoveGame(int game);

        //# TREE MODIFICATION ROUTINES -- ACTIONS
    gTuple<T> GetActionProbs(const Node &n) const;
    T GetActionProb(const Node &n, int br) const;
    void SetActionProbs(const Node &n, const gTuple<T> &probs);
    void SetActionProbs(int game, int iset, const gTuple<T> &probs);

        //# TREE MODIFICATION ROUTINES -- OUTCOMES
    int CreateOutcome(void);
    int CreateOutcome(int outc);
    gVector<T> GetOutcomeValues(int outc) const;
    void SetOutcomeValues(int outc, const gVector<T> &vals);
    void SetOutcomeValue(int outc, int pl, T value);

        //# PAYOFF INFORMATION
    int ProfileLength(void) const;
    gPVector<int> Dimensionality(void) const;
    void Payoff(const gPVector<int> &, gVector<T> &payoff) const;
    T Payoff(int pl, const gDPVector<T> &) const;
    void CondPayoff(const gDPVector<T> &profile, gDPVector<T> &value) const;
    void CondPayoff(const gDPVector<T> &profile, gDPVector<T> &value,
		    gPVector<T> &probs) const;
};


#endif   //# EXTFORM_H


