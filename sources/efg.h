//#
//# FILE: extform.h -- Declaration of extensive form data type
//#
//# $Id$
//#

#ifndef EXTFORM_H
#define EXTFORM_H

#include "gstring.h"
#include "gnumber.h"
#include "gtuple.h"
#include "gmap.h"
#include "noderep.h"

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
class ExtForm    {
  private:
    gString title;
    gTuple<gString> players;
    gSparseSet<NodeSet *> nodes;
    gSparseSet<Outcome *> outcomes;

    void AddPlayer(int p);
    int CreateInfoset(int p, int game, int iset);

    Node DeleteSubtree(Node);
    Node DeleteTerminalNode(const Node &);

//
// These are being defined privately for now so they are not accidentally
// used.  They will be implemented later.
//+grp
    ExtForm(const ExtForm &);
    ExtForm &operator=(const ExtForm &);
//-grp

    int EfgYaccer(void);

  public:
	//# CONSTRUCTORS AND DESTRUCTOR
    ExtForm(void);
    ExtForm(gInput &f);  
    ~ExtForm(); 

        //# TITLE ACCESS AND MANIPULATION
    void SetTitle(const gString &s);
    const gString &GetTitle(void) const;

        //# READING AND WRITING DATA FILES
    void ReadEfgFile(gInput &f);
    void WriteEfgFile(gOutput &f) const;

	//# DATA ACCESS -- GENERAL INFORMATION
    int NumNodes(void) const;
    int NumNodes(int game) const;
    int NumNodes(int game, int pl) const;
    int NumNodes(int game, int pl, int iset) const;
    int NumPlayers(void) const;
    int NumInfosets(int game, int pl) const;
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

        //# TREE MODIFICATION ROUTINES -- SUBGAMES
    int CreateSubgame(void);
    int CreateSubgame(int game, int from_file = 0);
    void RemoveSubgame(int game);
    gString GetSubgameLabel(int game) const;
    void LabelSubgame(int game, const gString &name);
    int IsSubgameDefined(int game) const;

	//# TREE MODIFICATION ROUTINES -- NODES
    Node AddNode(const Node &n, int player, int child_count);
    void LabelNode(const Node &n, const gString &s);
    Node InsertNode(const Node &n, int player, int child_count);
    Node DeleteNode(const Node &n, int keep);
    void SetOutcome(const Node &n, int outcome);
    void SetNextGame(const Node &n, int game);

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
    void LabelAction(int game, int pl, int iset, int act,
		     const gString &label);
    gString GetActionLabel(const Node &n, int act) const;
    gTuple<gNumber> GetActionProbs(const Node &n) const;
    gNumber GetActionProb(const Node &n, int br) const;
    void SetActionProbs(const Node &n, const gTuple<gNumber> &probs);
    void SetActionProbs(int game, int iset, const gTuple<gNumber> &probs);

        //# TREE MODIFICATION ROUTINES -- SUBTREES
    Node MoveTree(Node from, Node dest);
    Node CopyTree(Node from, Node dest);
    Node DeleteTree(const Node &n);

        //# TREE MODIFICATION ROUTINES -- OUTCOMES
    int CreateOutcome(void);
    int CreateOutcome(int outc);
    void RemoveOutcome(int outc);
    gString GetOutcomeLabel(int outc) const;
    void LabelOutcome(int outc, const gString &name);
    int IsOutcomeDefined(int outc) const;
    gTuple<gNumber> GetOutcomeValues(int outc) const;
    void SetOutcomeValues(int outc, const gTuple<gNumber> &vals);
    void SetOutcomeValue(int outc, int pl, gNumber value);

        //# MANAGEMENT OF VARIABLES
    int IsVariableDefined(const gString &name) const;
    Node GetNodeVariable(const gString &name) const;
    int SetNodeVariable(const gString &name, const Node &node);
    void RemoveNodeVariable(const gString &name);
};


#endif   //# EXTFORM_H

