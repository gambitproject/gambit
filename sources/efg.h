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
#include "game.h"
#include "display.h"

class ExtFormIter;

class ExtForm : public Handled  {

  private:
    gString *_title;
    gSet<Outcome> *_outcomes;
    gSet<Infoset> *_infosets;
    bool _modified, _trivial;
    ExtFormIter *_iterator;
    gHandle<Node> _root;

    void AddOutcome(uint outcome_number);
    void AddPlayer(uint player_number);
    void MoveTree(gHandle<Node> &to, gHandle<Node> &from);

  public:
    gSet<Player> *_players;
    gSet<Node> *_nodes;

    ExtForm(void);
    ~ExtForm();

    int AddNode(gHandle<Node> &n, uint player, uint branch_count);
    int LabelNode(gHandle<Node> &n, gString &label);
    int InsertNode(gHandle<Node> &n, uint player, uint branch_count);
    int SetOutcome(gHandle<Node> &n, uint outcome_number);
    int GetOutcome(gHandle<Node> &n);
    int DeleteNode(gHandle<Node> &n, gHandle<Node> &keep);

    int InfosetMember(gHandle<Node> &new_member, gHandle<Node> &to_iset);
    int BreakInfoset(gHandle<Node> &breakpoint);
    int JoinInfoset(gHandle<Node> &new_members, gHandle<Node> &to_iset);

    int LabelBranch(gHandle<Node> &n, gString &label);
    int InsertBranch(gHandle<Node> &n, uint branch_number);
    int DeleteBranch(gHandle<Node> &n, gHandle<Node> &remove);
//  gVector<double> BranchProbs(gHandle<Node> &n) const;
//  int SetBranchProbs(gHandle<Node> &n, gVector<double> &v);

    int DeleteTree(gHandle<Node> &n);
    int CopyTree(gHandle<Node> &n, gHandle<Node> &to);
    int LabelTree(const gString &label);
    gString TreeLabel(void) const;

    gHandle<Node> RootNode(void) const;

    bool IsTrivial(void) const;
    bool IsModified(void) const;

    int NumNodes(void) const;

    void RegisterIterator(ExtFormIter *iter);
    void UnregisterIterator(ExtFormIter *iter);

};


class ExtFormIter   {
#ifdef EXTFORM_C
  private:
    ExtForm *_extform;
    gHandle<Node> _cursor;
#endif    // EXTFORM_C

  public:
    ExtFormIter(ExtForm *extform);
    ~ExtFormIter();

    gHandle<Node> Cursor(void) const;
    void SetCursor(const gHandle<Node> &n);
    void GoParent(void);
		void GoFirstChild(void);

    void GoPriorSibling(void);
    void GoNextSibling(void);

    void Invalidate(void);
};


#endif   // EXTFORM_H

