//
// FILE: extform.h -- Declaration of extensive form data type
//
// $Id$
//

#ifndef EXTFORM_H
#define EXTFORM_H

#include "node.h"
#include "player.h"


class ExtForm    {
  private:
    NodeSet nodes;
    PlayerSet &players;

    void AddPlayer(int);
    int CreateInfoset(int, int);

  public:
	// CONSTRUCTORS AND DESTRUCTOR
    ExtForm(PlayerSet &p) : players(p)   { }
    ExtForm(const ExtForm &ef) : nodes(ef.nodes), players(ef.players)  { }
    ~ExtForm()  { }

	// OPERATOR OVERLOADING
    ExtForm &operator=(const ExtForm &);

	// HIGH-LEVEL OPERATIONS
    Node AddNode(const Node &n, int player, int child_count);
    void SetNodeLabel(const Node &n, const gString &s)
      { if (nodes.IsMember(n))  nodes.SetNodeName(n, s); }
    Node InsertNode(const Node &n, int player, int child_count);
    Node DeleteNode(const Node &n);

    Node JoinInfoset(const Node &new_node, const Node &to_iset);
    Node LeaveInfoset(const Node &n);
    Node MergeInfoset(const Node &from, const Node &into);
    void LabelInfoset(const Node &n, const gString &label)
      { players.SetInfosetName(n[1], n[2], label); }

    void InsertBranch(const Node &n, int where, int number);
    void DeleteBranch(const Node &n, int which);
    void LabelBranch(const Node &n, int br, const gString &label)  { }

    void SetOutcome(const Node &n, int outcome)
      { if (nodes.IsMember(n))  nodes.SetOutcome(n, outcome); }

    Node MoveTree(const Node &from, const Node &dest);
    Node CopyTree(const Node &from, const Node &dest);
    Node DeleteTree(const Node &n);

	// GENERAL INFORMATION
    int NumNodes(void) const
      { return nodes.NumNodes(); }
    int NumNodes(int pl) const
      { return nodes.NumNodes(pl); }
    int NumNodes(int pl, int iset) const
      { return nodes.NumNodes(pl, iset); }

    gString GetNodeLabel(const Node &n) const
      { return nodes.GetNodeName(n); }
    int GetOutcome(const Node &n) const
      { return nodes.GetOutcome(n); }

};


#endif   // EXTFORM_H

