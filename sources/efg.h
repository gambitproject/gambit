//
// FILE: extform.h -- Declaration of extensive form data type
//
// $Id$
//
#ifndef EXTFORM_H
#define EXTFORM_H

#include "basic.h"
#include "gstring.h"
#include "gset.h"

#include "outcome.h"
#include "player.h"
#include "node.h"
#include "infoset.h"
#include "display.h"

class ExtFormIter;

class ExtForm   {

  private:
    gString _title;
    gSet<Outcome> _outcomes;
    gSet<Infoset> _infosets;
    bool _modified, _trivial;
    ExtFormIter *_iterator;
    Node _root;
    // dummy player for terminal information sets
    Player _dummy, _chance;

    void AddOutcome(uint outcome_number);
    void AddPlayer(uint player_number);
    void MoveTree(Node &to, Node &from);

  public:
    gSet<Player> _players;
    gSet<Node> _nodes;

	// CONSTRUCTORS AND DESTRUCTOR
	// initialize the trivial extensive form
    ExtForm(void);
	// destruct an extensive form, including substructures
    ~ExtForm();

	// OPERATIONS ON NODES
    int AddNode(Node &n, uint player, uint branch_count);
    int LabelNode(Node &n, gString &label);
    int InsertNode(Node &n, uint player, uint branch_count);
    int SetOutcome(Node &n, uint outcome_number);
    int GetOutcome(Node &n) const;
    int DeleteNode(Node &n, Node &keep);

	// OPERATIONS ON INFORMATION SETS
    int JoinInfoset(Node &new_member, Node &to_iset);
    int LeaveInfoset(Node &departer);

    int LabelBranch(Node &n, gString &label);
    int InsertBranch(Node &n, uint branch_number);
    int DeleteBranch(Node &n, Node &remove);
//  gVector<double> BranchProbs(Node &n) const;
//  int SetBranchProbs(Node &n, gVector<double> &v);

	// OPERATIONS ON TREES
    int DeleteTree(Node &n);
    int CopyTree(Node &n, Node &to);
    int LabelTree(const gString &label);
    gString TreeLabel(void) const;

    Node RootNode(void);

    bool IsTrivial(void) const;
    bool IsModified(void) const;
    
    int NumNodes(void) const   { return _nodes.Length(); }

    void RegisterIterator(ExtFormIter *iter);
    void UnregisterIterator(ExtFormIter *iter);

};


class ExtFormIter   {
  private:
    ExtForm *_extform;
    Node _cursor;

  public:
    ExtFormIter(ExtForm *extform);
    ~ExtFormIter();

    Node Cursor(void);
    void SetCursor(const Node &n);
    void GoParent(void);
    void GoFirstChild(void);

    void GoPriorSibling(void);
    void GoNextSibling(void);

    void Invalidate(void);
};


#endif   // EXTFORM_H

