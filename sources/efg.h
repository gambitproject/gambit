//#
//# FILE: extform.h -- Declaration of extensive form data type
//#
//# $Id$
//#

#ifndef EXTFORM_H
#define EXTFORM_H

#include "node.h"

class ExtForm    {
  private:
    int efg_no;
    gString name;
    NodeSet nodes;

    Node DeleteSubtree(Node);
    Node DeleteTerminalNode(const Node &);

  public:
	//# CONSTRUCTORS AND DESTRUCTOR
    ExtForm(int number, int numPlayers, int from_file = 0) : 
    efg_no(number), nodes(number, numPlayers, from_file)   { }
//# numPlayers above may need +/- 2
//#    ExtForm(const ExtForm &ef) : nodes(ef.nodes)   { }
    ~ExtForm()  { }

	//# OPERATOR OVERLOADING
//#    ExtForm &operator=(const ExtForm &);

//#    int yyparse(void);

	//# HIGH-LEVEL OPERATIONS
    Node AddNode(const Node &n, int player, int child_count);
    void SetNodeLabel(const Node &n, const gString &s)
      { if (nodes.IsMember(n))  nodes.SetNodeName(n, s); }
    Node InsertNode(const Node &n, int player, int child_count);
    Node DeleteNode(const Node &n, int keep);

    Node JoinInfoset(const Node &new_node, const Node &to_iset);
    Node LeaveInfoset(const Node &n);
    Node MergeInfoset(const Node &from, const Node &into);
    void LabelInfoset(const Node &n, const gString &label)
      { if (!nodes.IsMember(n)) return;
	nodes.SetInfosetName(n[1], n[2], label); }
    void LabelInfoset(int pl, int iset, const gString &label)
      { nodes.SetInfosetName(pl, iset, label); }
//#old code      { players.SetInfosetName(n[1], n[0], n[2], label); }
    gString GetInfosetName(const Node &n)
      { if (!nodes.IsMember(n)) return "Not defined";
	return nodes.GetInfosetName(n[1], n[2]); }

    void AppendAction(int pl, int iset)
      { nodes.AppendAction(pl, iset); }
    void InsertAction(const Node &n, int where, int number);
    Node DeleteAction(const Node &n, int which);
    void LabelAction(const Node &n, int act, const gString &label)
      { if (!nodes.IsMember(n)) return;
	nodes.SetActionName(n[1], n[2], act, label); }
    void LabelAction(int pl, int iset, int act, const gString &label)
      { nodes.SetActionName(pl, iset, act, label); }
    gString GetActionLabel(const Node &n, int act)
      { if (!nodes.IsMember(n)) return "Not defined";
	nodes.GetActionName(n.GetPlayer(), n.GetInfoset(), act); }
    gTuple<gNumber> GetActionProbs(const Node &n) const;
    gNumber GetActionProb(const Node &n, int br) const;
    void SetActionProbs(const Node &n, const gTuple<gNumber> &probs);
    void SetActionProbs(int pl, int iset, const gTuple<gNumber> &probs);

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

	//# OPERATIONS ON NODES
    Node RootNode(void) const
      { return nodes.RootNode(); }
    Node GetParent(const Node &n) const
      { if (!nodes.IsMember(n)) return Node();
	return nodes.GetParent(n); }
    int NumChildren(const Node &n) const
      { if (!nodes.IsMember(n)) return 0;
	return nodes.NumChildren(n); }
    Node GetChildNumber(const Node &n, int number) const
      { if (!nodes.IsMember(n)) return Node();
	return nodes.GetChildNumber(n, number); }
    gBlock<Node> GetChildren(const Node &n) const
      { if (!nodes.IsMember(n)) return gBlock<Node>();
	return nodes.GetChildren(n); }
    Node GetPriorSibling(const Node &n) const
      { if (!nodes.IsMember(n)) return Node();
	return nodes.GetPriorSibling(n); }
    Node GetNextSibling(const Node &n) const
      { if (!nodes.IsMember(n)) return Node();
	return nodes.GetNextSibling(n); }
    int HasSuccessorGame(const Node &n) const
      { if (!nodes.IsMember(n)) return 0;
	return nodes.HasSuccessorGame(n); }
    int IsSuccessor(const Node &n, const Node &from) const
      { if (!nodes.IsMember(n) || !nodes.IsMember(from)) return 0;
	return nodes.IsPredecessor(from, n); }
    int IsPredecessor(const Node &n, const Node &of) const
      { if (!nodes.IsMember(n) || !nodes.IsMember(of)) return 0;
	return nodes.IsPredecessor(n, of); }

	//# GENERAL INFORMATION
    int NumNodes(void) const
      { return nodes.NumNodes(); }
    int NumNodes(int pl) const
      { return nodes.NumNodes(pl); }
    int NumNodes(int pl, int iset) const
      { return nodes.NumNodes(pl, iset); }
    int NumPlayers(void) const
      { return nodes.NumPlayers(); }
    int NumInfosets(int pl) const
      { return nodes.NumInfosets(pl); }

    gString GetNodeLabel(const Node &n) const
      { if (!nodes.IsMember(n)) return "Not defined";
	return nodes.GetNodeName(n); }
    int GetOutcome(const Node &n) const
      { if (!nodes.IsMember(n)) return 0;
	return nodes.GetOutcome(n); }

    void AddPlayer(int);
    int CreateInfoset(int, int, int);

	//# FILE OPERATIONS
    int InputFromFile(const gBlock<struct nodeinfo *> &b)
      { return nodes.InputFromFile(b); }
    void WriteToFile(gOutput &f) const;
};


#endif   //# EXTFORM_H

