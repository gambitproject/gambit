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
    int efg_no;
    gString name;
    NodeSet nodes;
    PlayerSet &players;

    void AddPlayer(int);
    int CreateInfoset(int, int, int);
    Node DeleteSubtree(Node);
    Node DeleteTerminalNode(const Node &);

  public:
	// CONSTRUCTORS AND DESTRUCTOR
    ExtForm(int number, PlayerSet &p) : 
      efg_no(number), players(p), nodes(number)   { }
    ExtForm(const ExtForm &ef) : nodes(ef.nodes), players(ef.players)  { }
    ~ExtForm()  { }

	// OPERATOR OVERLOADING
    ExtForm &operator=(const ExtForm &);

	// HIGH-LEVEL OPERATIONS
    Node AddNode(const Node &n, int player, int child_count);
    void SetNodeLabel(const Node &n, const gString &s)
      { if (nodes.IsMember(n))  nodes.SetNodeName(n, s); }
    Node InsertNode(const Node &n, int player, int child_count);
    Node DeleteNode(const Node &n, int keep);

    Node JoinInfoset(const Node &new_node, const Node &to_iset);
    Node LeaveInfoset(const Node &n);
    Node MergeInfoset(const Node &from, const Node &into);
    void LabelInfoset(const Node &n, const gString &label)
      { players.SetInfosetName(n[1], n[0], n[2], label); }

    void InsertBranch(const Node &n, int where, int number);
    Node DeleteBranch(const Node &n, int which);
    void LabelBranch(const Node &n, int br, const gString &label)  { }
    gVector<double> GetBranchProbs(const Node &n) const;
    double GetBranchProb(const Node &n, int br) const;
    void SetBranchProbs(const Node &n, const gVector<double> &probs);

    void SetOutcome(const Node &n, int outcome)
      { if (nodes.IsMember(n))  nodes.SetOutcome(n, outcome); }
    void ExpungeOutcome(int outcome)
      { nodes.ExpungeOutcome(outcome); }

    void SetNextGame(const Node &n, int game)
      { if (nodes.IsMember(n))  nodes.SetNextGame(n, game); }
    int GetNextGame(const Node &n) const
      { if (!nodes.IsMember(n))   return 0;
	return nodes.GetNextGame(n); }
    void ExpungeGame(int game)
      { nodes.ExpungeGame(game); }

    Node MoveTree(Node from, Node dest);
    Node CopyTree(Node from, Node dest);
    Node DeleteTree(const Node &n);

    void SetTreeName(const gString &s)   { name = s; }
    gString GetTreeName(void) const   { return name; }

	// OPERATIONS ON NODES
    Node RootNode(void) const
      { return nodes.RootNode(); }
    Node GetParent(const Node &n) const
      { return nodes.GetParent(n); }
    int NumChildren(const Node &n) const
      { return nodes.NumChildren(n); }
    Node GetChildNumber(const Node &n, int number) const
      { return nodes.GetChildNumber(n, number); }
    gSet<Node> GetChildren(const Node &n) const
      { return nodes.GetChildren(n); }
    Node GetPriorSibling(const Node &n) const
      { return nodes.GetPriorSibling(n); }
    Node GetNextSibling(const Node &n) const
      { return nodes.GetNextSibling(n); }
    int HasSuccessorGame(const Node &n) const
      { return nodes.HasSuccessorGame(n); }

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

	// FILE OPERATIONS
    void WriteToFile(output &f) const;
};


#endif   // EXTFORM_H

