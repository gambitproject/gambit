//
// FILE: funcefg.cc -- Efg functions for GEL
//
// $Id$
//

#include "gmisc.h"
#include "exprtree.h"
#include "funcmisc.h"
#include "glist.h"
#include "tristate.h"

#include "efg.h"
#include "efgutils.h"


//------------
// AddAction
//------------

DECLARE_BINARY(gelfuncAddAction, EFSupport *, Action *, EFSupport *)

EFSupport *gelfuncAddAction::EvalItem(EFSupport *S, Action *a) const
{
  EFSupport *T = new EFSupport(*S);
  T->AddAction(a);
  return T;
}


//-----------
// AddMove
//-----------

DECLARE_BINARY(gelfuncAddMove, Infoset *, Node *, Node *)

Node *gelfuncAddMove::EvalItem(Infoset *s, Node *n) const
{
  n->Game()->AppendNode(n, s);
  return n->GetChild(1);
}

//-----------
// Chance
//-----------

DECLARE_UNARY(gelfuncChance, Efg *, EFPlayer *)

EFPlayer *gelfuncChance::EvalItem(Efg *E) const
{
  return E->GetChance();
}

//-------------
// ChanceProb
//-------------

DECLARE_UNARY(gelfuncChanceProb, Action *, gNumber)

gNumber gelfuncChanceProb::EvalItem(Action *a) const
{
  Infoset *infoset = a->BelongsTo();
  if (!infoset->GetPlayer()->IsChance())  return 0;
  return infoset->Game()->GetChanceProb(infoset, a->GetNumber());
}

//------------
// CopyTree
//------------

DECLARE_BINARY(gelfuncCopyTree, Node *, Node *, Node *)

Node *gelfuncCopyTree::EvalItem(Node *n1, Node *n2) const
{
  return n1->Game()->CopyTree(n1, n2);
}

//----------------
// DeleteAction
//----------------

DECLARE_UNARY(gelfuncDeleteAction, Action *, Infoset *)

Infoset *gelfuncDeleteAction::EvalItem(Action *a) const
{
  Infoset *infoset = a->BelongsTo();

  if (infoset->NumActions() == 1)   return infoset;
  infoset->Game()->DeleteAction(infoset, a);
  return infoset;
}

//----------------------
// DeleteEmptyInfoset
//----------------------

DECLARE_UNARY(gelfuncDeleteEmptyInfoset, Infoset *, gTriState)

gTriState gelfuncDeleteEmptyInfoset::EvalItem(Infoset *s) const
{
  return (s->Game()->DeleteEmptyInfoset(s)) ? triTRUE : triFALSE;
}

//-------------
// DeleteMove
//-------------

DECLARE_BINARY(gelfuncDeleteMove, Node *, Node *, Node *)

Node *gelfuncDeleteMove::EvalItem(Node *n, Node *keep) const
{
  if (keep->GetParent() != n)  return n;

  return n->Game()->DeleteNode(n, keep);
}

//--------------
// DeleteTree
//--------------

DECLARE_UNARY(gelfuncDeleteTree, Node *, Node *)

Node *gelfuncDeleteTree::EvalItem(Node *n) const
{
  n->Game()->DeleteTree(n);
  return n;
}

//----------
// Game
//----------

DECLARE_UNARY(gelfuncGameInfoset, Infoset *, Efg *)

Efg *gelfuncGameInfoset::EvalItem(Infoset *s) const
{
  return s->Game();
}

DECLARE_UNARY(gelfuncGameNode, Node *, Efg *)

Efg *gelfuncGameNode::EvalItem(Node *n) const
{
  return n->Game();
}

DECLARE_UNARY(gelfuncGameEFPlayer, EFPlayer *, Efg *)

Efg *gelfuncGameEFPlayer::EvalItem(EFPlayer *p) const
{
  return p->Game();
}

DECLARE_UNARY(gelfuncGameEFOutcome, EFOutcome *, Efg *)

Efg *gelfuncGameEFOutcome::EvalItem(EFOutcome *c) const
{
  return c->BelongsTo();
}

//------------
// Infoset
//------------

DECLARE_UNARY(gelfuncInfosetAction, Action *, Infoset *)

Infoset *gelfuncInfosetAction::EvalItem(Action *a) const
{
  return a->BelongsTo();
}

DECLARE_UNARY(gelfuncInfosetNode, Node *, Infoset *)

Infoset *gelfuncInfosetNode::EvalItem(Node *n) const
{
  return n->GetInfoset();
}

//----------------
// InsertAction
//----------------

DECLARE_UNARY(gelfuncInsertAction, Infoset *, Action *)

Action *gelfuncInsertAction::EvalItem(Infoset *s) const
{
  return s->Game()->InsertAction(s);
}

DECLARE_BINARY(gelfuncInsertActionAt, Infoset *, Action *, Action *)

Action *gelfuncInsertActionAt::EvalItem(Infoset *s, Action *a) const
{
  return s->Game()->InsertAction(s, a);
}

//--------------
// InsertMove
//--------------

DECLARE_BINARY(gelfuncInsertMove, Infoset *, Node *, Node *)

Node *gelfuncInsertMove::EvalItem(Infoset *s, Node *n) const
{
  n->Game()->InsertNode(n, s);
  return n->GetParent();
}

//--------------
// IsConstSum
//--------------

DECLARE_UNARY(gelfuncIsConstSumEfg, Efg *, gTriState)

gTriState gelfuncIsConstSumEfg::EvalItem(Efg *E) const
{
  return (E->IsConstSum()) ? triTRUE : triFALSE;
}

//------------------
// IsPerfectRecall
//------------------

DECLARE_UNARY(gelfuncIsPerfectRecall, Efg *, gTriState)

gTriState gelfuncIsPerfectRecall::EvalItem(Efg *E) const
{
  Infoset *s1, *s2;
  return (IsPerfectRecall(*E, s1, s2)) ? triTRUE : triFALSE;
}

//-----------------
// IsPredecessor
//-----------------

DECLARE_BINARY(gelfuncIsPredecessor, Node *, Node *, gTriState)

gTriState gelfuncIsPredecessor::EvalItem(Node *n1, Node *n2) const
{
  return (n1->Game()->IsPredecessor(n1, n2)) ? triTRUE : triFALSE;
}

//---------------
// IsSuccessor
//---------------

DECLARE_BINARY(gelfuncIsSuccessor, Node *, Node *, gTriState)

gTriState gelfuncIsSuccessor::EvalItem(Node *n1, Node *n2) const
{
  return (n1->Game()->IsSuccessor(n1, n2)) ? triTRUE : triFALSE;
}

//------------
// LoadEfg
//------------

DECLARE_UNARY(gelfuncLoadEfg, gText, Efg *)

Efg *gelfuncLoadEfg::EvalItem(gText filename) const
{
  gFileInput f(filename);
  Efg *E = 0;
  ReadEfgFile(f, E);
  return E;
}

//-----------------
// MarkSubgame
//-----------------

DECLARE_UNARY(gelfuncMarkSubgame, Node *, gTriState)

gTriState gelfuncMarkSubgame::EvalItem(Node *n) const
{
  return (n->Game()->DefineSubgame(n)) ? triTRUE : triFALSE;
}

//-----------------
// MarkedSubgame
//-----------------

DECLARE_UNARY(gelfuncMarkedSubgame, Node *, gTriState)

gTriState gelfuncMarkedSubgame::EvalItem(Node *n) const
{
  return (n->GetSubgameRoot() == n) ? triTRUE : triFALSE;
}

//----------------
// MergeInfosets
//----------------

DECLARE_BINARY(gelfuncMergeInfosets, Infoset *, Infoset *, Infoset *)

Infoset *gelfuncMergeInfosets::EvalItem(Infoset *s1, Infoset *s2) const
{
  s1->Game()->MergeInfoset(s1, s2);
  return s1;
}

//-----------------
// MoveToInfoset
//-----------------

DECLARE_BINARY(gelfuncMoveToInfoset, Node *, Infoset *, Infoset *)

Infoset *gelfuncMoveToInfoset::EvalItem(Node *n, Infoset *s) const
{
  s->Game()->JoinInfoset(s, n);
  return s;
}

//-------------
// MoveTree
//-------------

DECLARE_BINARY(gelfuncMoveTree, Node *, Node *, Node *)

Node *gelfuncMoveTree::EvalItem(Node *n1, Node *n2) const
{
  return n1->Game()->MoveTree(n1, n2);
}

//--------
// Name
//--------

DECLARE_UNARY(gelfuncNameAction, Action *, gText)

gText gelfuncNameAction::EvalItem(Action *a) const
{
  return a->GetName();
}

DECLARE_UNARY(gelfuncNameInfoset, Infoset *, gText)

gText gelfuncNameInfoset::EvalItem(Infoset *s) const
{
  return s->GetName();
}

DECLARE_UNARY(gelfuncNameNode, Node *, gText)

gText gelfuncNameNode::EvalItem(Node *n) const
{
  return n->GetName();
}

DECLARE_UNARY(gelfuncNameEFOutcome, EFOutcome *, gText)

gText gelfuncNameEFOutcome::EvalItem(EFOutcome *c) const
{
  return c->GetName();
}

DECLARE_UNARY(gelfuncNameEFPlayer, EFPlayer *, gText)

gText gelfuncNameEFPlayer::EvalItem(EFPlayer *p) const
{
  return p->GetName();
}

DECLARE_UNARY(gelfuncNameEfg, Efg *, gText)

gText gelfuncNameEfg::EvalItem(Efg *E) const
{
  return E->GetTitle();
}


//-----------
// NewEfg
//-----------

DECLARE_UNARY(gelfuncNewEfg, gNumber, Efg *)

Efg *gelfuncNewEfg::EvalItem(gNumber n) const
{
  Efg *E = new Efg;
  int m = n;
  for (int i = 1; i <= m; i++)
    E->NewPlayer();
  return E;
}

//-------------
// NewInfoset
//-------------

DECLARE_BINARY(gelfuncNewInfoset, EFPlayer *, gNumber, Infoset *)

Infoset *gelfuncNewInfoset::EvalItem(EFPlayer *p, gNumber n) const
{
  return p->Game()->CreateInfoset(p, n);
}

//-------------
// NewOutcome
//-------------

DECLARE_UNARY(gelfuncNewOutcomeEfg, Efg *, EFOutcome *)

EFOutcome *gelfuncNewOutcomeEfg::EvalItem(Efg *E) const
{
  return E->NewOutcome();
}

//-------------
// NewPlayer
//-------------

DECLARE_UNARY(gelfuncNewPlayer, Efg *, EFPlayer *)

EFPlayer *gelfuncNewPlayer::EvalItem(Efg *E) const
{
  return E->NewPlayer();
}

//--------------
// NextSibling
//--------------

DECLARE_UNARY(gelfuncNextSibling, Node *, Node *)

Node *gelfuncNextSibling::EvalItem(Node *n) const
{
  return n->NextSibling();
}

//------------
// NthChild
//------------

DECLARE_BINARY(gelfuncNthChild, Node *, gNumber, Node *)

Node *gelfuncNthChild::EvalItem(Node *n, gNumber i) const
{
  return n->GetChild(i);
}

//-----------
// Outcome
//-----------

DECLARE_UNARY(gelfuncOutcome, Node *, EFOutcome *)

EFOutcome *gelfuncOutcome::EvalItem(Node *n) const
{
  return n->GetOutcome();
}

//-----------
// Parent
//-----------

DECLARE_UNARY(gelfuncParent, Node *, Node *)

Node *gelfuncParent::EvalItem(Node *n) const
{
  return n->GetParent();
}

//-----------
// Payoff
//-----------

DECLARE_BINARY(gelfuncPayoffEFOutcome, EFOutcome *, EFPlayer *, gNumber)

gNumber gelfuncPayoffEFOutcome::EvalItem(EFOutcome *c, EFPlayer *p) const
{
  return p->Game()->Payoff(c, p->GetNumber());
}

//----------
// Player
//----------

DECLARE_UNARY(gelfuncPlayer, Infoset *, EFPlayer *)

EFPlayer *gelfuncPlayer::EvalItem(Infoset *s) const
{
  return s->GetPlayer();
}

//----------------
// PriorSibling
//----------------

DECLARE_UNARY(gelfuncPriorSibling, Node *, Node *)

Node *gelfuncPriorSibling::EvalItem(Node *n) const
{
  return n->PriorSibling();
}

//----------------
// RemoveAction
//----------------

DECLARE_BINARY(gelfuncRemoveAction, EFSupport *, Action *, EFSupport *)

EFSupport *gelfuncRemoveAction::EvalItem(EFSupport *S, Action *a) const
{
  EFSupport *T = new EFSupport(*S);
  T->RemoveAction(a);
  return T;
}

//------------
// RootNode
//------------

DECLARE_UNARY(gelfuncRootNode, Efg *, Node *)

Node *gelfuncRootNode::EvalItem(Efg *E) const
{
  return E->RootNode();
}

//------------
// SaveEfg
//------------

DECLARE_BINARY(gelfuncSaveEfg, Efg *, gText, Efg *)

Efg *gelfuncSaveEfg::EvalItem(Efg *E, gText filename) const
{
  gFileOutput f(filename);
  E->WriteEfgFile(f);
  return E;
}

//------------
// SetName
//------------

DECLARE_BINARY(gelfuncSetNameAction, Action *, gText, Action *)

Action *gelfuncSetNameAction::EvalItem(Action *a, gText name) const
{
  a->SetName(name);
  return a;
}

DECLARE_BINARY(gelfuncSetNameInfoset, Infoset *, gText, Infoset *)

Infoset *gelfuncSetNameInfoset::EvalItem(Infoset *s, gText name) const
{
  s->SetName(name);
  return s;
}

DECLARE_BINARY(gelfuncSetNameNode, Node *, gText, Node *)

Node *gelfuncSetNameNode::EvalItem(Node *n, gText name) const
{
  n->SetName(name);
  return n;
}

DECLARE_BINARY(gelfuncSetNameEFOutcome, EFOutcome *, gText, EFOutcome *)

EFOutcome *gelfuncSetNameEFOutcome::EvalItem(EFOutcome *c, gText name) const
{
  c->SetName(name);
  return c;
}

DECLARE_BINARY(gelfuncSetNameEFPlayer, EFPlayer *, gText, EFPlayer *)

EFPlayer *gelfuncSetNameEFPlayer::EvalItem(EFPlayer *p, gText name) const
{
  p->SetName(name);
  return p;
}

DECLARE_BINARY(gelfuncSetNameEfg, Efg *, gText, Efg *)

Efg *gelfuncSetNameEfg::EvalItem(Efg *E, gText name) const
{
  E->SetTitle(name);
  return E;
}

//--------------
// SetOutcome
//--------------

DECLARE_BINARY(gelfuncSetOutcome, Node *, EFOutcome *, EFOutcome *)

EFOutcome *gelfuncSetOutcome::EvalItem(Node *n, EFOutcome *c) const
{
  n->SetOutcome(c);
  return c;
}

//------------
// Support
//------------

DECLARE_UNARY(gelfuncSupportEfg, Efg *, EFSupport *)

EFSupport *gelfuncSupportEfg::EvalItem(Efg *E) const
{
  return new EFSupport(*E);
}


//-----------------
// UnmarkSubgame
//-----------------

DECLARE_UNARY(gelfuncUnmarkSubgame, Node *, Node *)

Node *gelfuncUnmarkSubgame::EvalItem(Node *n) const
{
  n->Game()->RemoveSubgame(n);
  return n;
}


gelExpr *GEL_AddAction(const gArray<gelExpr *> &params)
{
  return new gelfuncAddAction((gelExpression<EFSupport *> *) params[1],
                              (gelExpression<Action *> *) params[2]);
}

gelExpr *GEL_AddMove(const gArray<gelExpr *> &params)
{
  return new gelfuncAddMove((gelExpression<Infoset *> *) params[1],
                            (gelExpression<Node *> *) params[2]);
}

gelExpr *GEL_Chance(const gArray<gelExpr *> &params)
{
  return new gelfuncChance((gelExpression<Efg *> *) params[1]);
}

gelExpr *GEL_ChanceProb(const gArray<gelExpr *> &params)
{
  return new gelfuncChanceProb((gelExpression<Action *> *) params[1]);
}

gelExpr *GEL_CopyTree(const gArray<gelExpr *> &params)
{
  return new gelfuncCopyTree((gelExpression<Node *> *) params[1],
                             (gelExpression<Node *> *) params[2]);
}

gelExpr *GEL_DeleteAction(const gArray<gelExpr *> &params)
{
  return new gelfuncDeleteAction((gelExpression<Action *> *) params[1]);
}

gelExpr *GEL_DeleteEmptyInfoset(const gArray<gelExpr *> &params)
{
  return new gelfuncDeleteEmptyInfoset((gelExpression<Infoset *> *) params[1]);
}

gelExpr *GEL_DeleteMove(const gArray<gelExpr *> &params)
{
  return new gelfuncDeleteMove((gelExpression<Node *> *) params[1],
                               (gelExpression<Node *> *) params[2]);
}

gelExpr *GEL_DeleteTree(const gArray<gelExpr *> &params)
{
  return new gelfuncDeleteTree((gelExpression<Node *> *) params[1]);
}

gelExpr *GEL_GameInfoset(const gArray<gelExpr *> &params)
{
  return new gelfuncGameInfoset((gelExpression<Infoset *> *) params[1]);
}

gelExpr *GEL_GameNode(const gArray<gelExpr *> &params)
{
  return new gelfuncGameNode((gelExpression<Node *> *) params[1]);
}

gelExpr *GEL_GameEFPlayer(const gArray<gelExpr *> &params)
{
  return new gelfuncGameEFPlayer((gelExpression<EFPlayer *> *) params[1]);
}

gelExpr *GEL_GameEFOutcome(const gArray<gelExpr *> &params)
{
  return new gelfuncGameEFOutcome((gelExpression<EFOutcome *> *) params[1]);
}

gelExpr *GEL_InfosetAction(const gArray<gelExpr *> &params)
{
  return new gelfuncInfosetAction((gelExpression<Action *> *) params[1]);
}

gelExpr *GEL_InfosetNode(const gArray<gelExpr *> &params)
{
  return new gelfuncInfosetNode((gelExpression<Node *> *) params[1]);
}

gelExpr *GEL_InsertAction(const gArray<gelExpr *> &params)
{
  return new gelfuncInsertAction((gelExpression<Infoset *> *) params[1]);
}

gelExpr *GEL_InsertActionAt(const gArray<gelExpr *> &params)
{
  return new gelfuncInsertActionAt((gelExpression<Infoset *> *) params[1],
                                   (gelExpression<Action *> *) params[2]);
}

gelExpr *GEL_InsertMove(const gArray<gelExpr *> &params)
{
  return new gelfuncInsertMove((gelExpression<Infoset *> *) params[1],
                               (gelExpression<Node *> *) params[2]);
}

gelExpr *GEL_IsConstSumEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncIsConstSumEfg((gelExpression<Efg *> *) params[1]);
}

gelExpr *GEL_IsPerfectRecall(const gArray<gelExpr *> &params)
{
  return new gelfuncIsPerfectRecall((gelExpression<Efg *> *) params[1]);
}

gelExpr *GEL_IsPredecessor(const gArray<gelExpr *> &params)
{
  return new gelfuncIsPredecessor((gelExpression<Node *> *) params[1],
                                  (gelExpression<Node *> *) params[2]);
}

gelExpr *GEL_IsSuccessor(const gArray<gelExpr *> &params)
{
  return new gelfuncIsSuccessor((gelExpression<Node *> *) params[1],
                                (gelExpression<Node *> *) params[2]);
}

gelExpr *GEL_LoadEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncLoadEfg((gelExpression<gText> *) params[1]);
}

gelExpr *GEL_MarkSubgame(const gArray<gelExpr *> &params)
{
  return new gelfuncMarkSubgame((gelExpression<Node *> *) params[1]);
}

gelExpr *GEL_MarkedSubgame(const gArray<gelExpr *> &params)
{
  return new gelfuncMarkedSubgame((gelExpression<Node *> *) params[1]);
}

gelExpr *GEL_MergeInfosets(const gArray<gelExpr *> &params)
{
  return new gelfuncMergeInfosets((gelExpression<Infoset *> *) params[1],
                                  (gelExpression<Infoset *> *) params[2]);
}

gelExpr *GEL_MoveToInfoset(const gArray<gelExpr *> &params)
{
  return new gelfuncMoveToInfoset((gelExpression<Node *> *) params[1],
                                  (gelExpression<Infoset *> *) params[2]);
}

gelExpr *GEL_MoveTree(const gArray<gelExpr *> &params)
{
  return new gelfuncMoveTree((gelExpression<Node *> *) params[1],
                             (gelExpression<Node *> *) params[2]);
}

gelExpr *GEL_NameAction(const gArray<gelExpr *> &params)
{
  return new gelfuncNameAction((gelExpression<Action *> *) params[1]);
}

gelExpr *GEL_NameInfoset(const gArray<gelExpr *> &params)
{
  return new gelfuncNameInfoset((gelExpression<Infoset *> *) params[1]);
}

gelExpr *GEL_NameNode(const gArray<gelExpr *> &params)
{
  return new gelfuncNameNode((gelExpression<Node *> *) params[1]);
}

gelExpr *GEL_NameEFOutcome(const gArray<gelExpr *> &params)
{
  return new gelfuncNameEFOutcome((gelExpression<EFOutcome *> *) params[1]);
}

gelExpr *GEL_NameEFPlayer(const gArray<gelExpr *> &params)
{
  return new gelfuncNameEFPlayer((gelExpression<EFPlayer *> *) params[1]);
}

gelExpr *GEL_NameEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncNameEfg((gelExpression<Efg *> *) params[1]);
}

gelExpr *GEL_NewEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncNewEfg((gelExpression<gNumber> *) params[1]);
}

gelExpr *GEL_NewInfoset(const gArray<gelExpr *> &params)
{
  return new gelfuncNewInfoset((gelExpression<EFPlayer *> *) params[1],
                               (gelExpression<gNumber> *) params[2]);
}

gelExpr *GEL_NewOutcome(const gArray<gelExpr *> &params)
{
  return new gelfuncNewOutcomeEfg((gelExpression<Efg *> *) params[1]);
}

gelExpr *GEL_NewPlayer(const gArray<gelExpr *> &params)
{
  return new gelfuncNewPlayer((gelExpression<Efg *> *) params[1]);
}

gelExpr *GEL_NextSibling(const gArray<gelExpr *> &params)
{
  return new gelfuncNextSibling((gelExpression<Node *> *) params[1]);
}

gelExpr *GEL_NthChild(const gArray<gelExpr *> &params)
{
  return new gelfuncNthChild((gelExpression<Node *> *) params[1],
                             (gelExpression<gNumber> *) params[2]);
}

gelExpr *GEL_Outcome(const gArray<gelExpr *> &params)
{
  return new gelfuncOutcome((gelExpression<Node *> *) params[1]);
}

gelExpr *GEL_Parent(const gArray<gelExpr *> &params)
{
  return new gelfuncParent((gelExpression<Node *> *) params[1]);
}

gelExpr *GEL_PayoffEFOutcome(const gArray<gelExpr *> &params)
{
  return new gelfuncPayoffEFOutcome((gelExpression<EFOutcome *> *) params[1],
                                    (gelExpression<EFPlayer *> *) params[2]);
}

gelExpr *GEL_Player(const gArray<gelExpr *> &params)
{
  return new gelfuncPlayer((gelExpression<Infoset *> *) params[1]);
}

gelExpr *GEL_PriorSibling(const gArray<gelExpr *> &params)
{
  return new gelfuncPriorSibling((gelExpression<Node *> *) params[1]);
}

gelExpr *GEL_RemoveAction(const gArray<gelExpr *> &params)
{
  return new gelfuncRemoveAction((gelExpression<EFSupport *> *) params[1],
                                 (gelExpression<Action *> *) params[2]);
}

gelExpr *GEL_RootNode(const gArray<gelExpr *> &params)
{
  return new gelfuncRootNode((gelExpression<Efg *> *) params[1]);
}

gelExpr *GEL_SaveEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncSaveEfg((gelExpression<Efg *> *) params[1],
                            (gelExpression<gText> *) params[2]);
}

gelExpr *GEL_SetNameAction(const gArray<gelExpr *> &params)
{
  return new gelfuncSetNameAction((gelExpression<Action *> *) params[1],
                                  (gelExpression<gText> *) params[2]);
}

gelExpr *GEL_SetNameInfoset(const gArray<gelExpr *> &params)
{
  return new gelfuncSetNameInfoset((gelExpression<Infoset *> *) params[1],
                                   (gelExpression<gText> *) params[2]);
}

gelExpr *GEL_SetNameNode(const gArray<gelExpr *> &params)
{
  return new gelfuncSetNameNode((gelExpression<Node *> *) params[1],
                                (gelExpression<gText> *) params[2]);
}

gelExpr *GEL_SetNameEFOutcome(const gArray<gelExpr *> &params)
{
  return new gelfuncSetNameEFOutcome((gelExpression<EFOutcome *> *) params[1],
                                     (gelExpression<gText> *) params[2]);
}

gelExpr *GEL_SetNameEFPlayer(const gArray<gelExpr *> &params)
{
  return new gelfuncSetNameEFPlayer((gelExpression<EFPlayer *> *) params[1],
                                    (gelExpression<gText> *) params[2]);
}

gelExpr *GEL_SetNameEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncSetNameEfg((gelExpression<Efg *> *) params[1],
                               (gelExpression<gText> *) params[2]);
}

gelExpr *GEL_SetOutcome(const gArray<gelExpr *> &params)
{
  return new gelfuncSetOutcome((gelExpression<Node *> *) params[1],
                               (gelExpression<EFOutcome *> *) params[2]);
}

gelExpr *GEL_SupportEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncSupportEfg((gelExpression<Efg *> *) params[1]);
}

gelExpr *GEL_UnmarkSubgame(const gArray<gelExpr *> &params)
{
  return new gelfuncUnmarkSubgame((gelExpression<Node *> *) params[1]);
}


#include "match.h"

void gelEfgInit(gelEnvironment *env)
{
  struct  { gelAdapter *func; char *sig; }  sigarray[] = {
    { GEL_AddAction, "AddAction[support->EFSUPPORT, action->ACTION] =: EFSUPPORT" },
    { GEL_AddMove, "AddMove[infoset->INFOSET, node->NODE] =: NODE" },
    { GEL_Chance, "Chance[efg->EFG] =: EFPLAYER" },
    { GEL_ChanceProb, "ChanceProb[action->ACTION] =: NUMBER" },
    { GEL_CopyTree, "CopyTree[dest->NODE, src->NODE] =: NODE" },
    { GEL_DeleteAction, "DeleteAction[action->ACTION] =: INFOSET" },
    { GEL_DeleteEmptyInfoset, "DeleteEmptyInfoset[infoset->INFOSET] =: BOOLEAN" },
    { GEL_DeleteMove, "DeleteMove[node->NODE, keep->NODE] =: NODE" },
    { GEL_DeleteTree, "DeleteTree[node->NODE] =: NODE" },
    { GEL_GameInfoset, "Game[infoset->INFOSET] =: EFG" },
    { GEL_GameNode, "Game[node->NODE] =: EFG" },
    { GEL_GameEFPlayer, "Game[player->EFPLAYER] =: EFG" },
    { GEL_GameEFOutcome, "Game[outcome->EFOUTCOME] =: EFG" },
    { GEL_InfosetAction, "Infoset[action->ACTION] =: INFOSET" },
    { GEL_InfosetNode, "Infoset[node->NODE] =: INFOSET" },
    { GEL_InsertMove, "InsertMove[infoset->INFOSET, node->NODE] =: NODE" },
    { GEL_IsConstSumEfg, "IsConstSum[efg->EFG] =: BOOLEAN" },
    { GEL_IsPerfectRecall, "IsPerfectRecall[efg->EFG] =: BOOLEAN" },
    { GEL_IsPredecessor, "IsPredecessor[node->NODE, of->NODE] =: BOOLEAN" },
    { GEL_IsSuccessor, "IsSuccessor[node->NODE, of->NODE] =: BOOLEAN" },
    { GEL_LoadEfg, "LoadEfg[file->TEXT] =: EFG" },
    { GEL_MarkSubgame, "MarkSubgame[node->NODE] =: BOOLEAN" },
    { GEL_MarkedSubgame, "MarkedSubgame[node->NODE] =: BOOLEAN" },
    { GEL_MergeInfosets,
          "MergeInfosets[infoset1->INFOSET, infoset2->INFOSET] =: INFOSET" },
    { GEL_MoveToInfoset, "MoveToInfoset[node->NODE, infoset->INFOSET] =: INFOSET" },
    { GEL_MoveTree, "MoveTree[src->NODE, dest->NODE] =: NODE" },
    { GEL_NameAction, "Name[action->ACTION] =: TEXT" },
    { GEL_NameInfoset, "Name[infoset->INFOSET] =: TEXT" },
    { GEL_NameNode, "Name[node->NODE] =: TEXT" },
    { GEL_NameEFOutcome, "Name[outcome->EFOUTCOME] =: TEXT" },
    { GEL_NameEFPlayer, "Name[player->EFPLAYER] =: TEXT" },
    { GEL_NameEfg, "Name[efg->EFG] =: TEXT" },
    { GEL_NewEfg, "NewEfg[players->NUMBER] =: EFG" },
    { GEL_NewInfoset, "NewInfoset[player->EFPLAYER, actions->NUMBER] =: INFOSET" },
    { GEL_NewOutcome, "NewOutcome[efg->EFG] =: EFOUTCOME" },
    { GEL_NewPlayer, "NewPlayer[efg->EFG] =: EFPLAYER" },
    { GEL_NextSibling, "NextSibling[node->NODE] =: NODE" },
    { GEL_NthChild, "NthChild[node->NODE, child->NUMBER] =: NODE" },
    { GEL_Outcome, "Outcome[node->NODE] =: EFOUTCOME" },
    { GEL_Parent, "Parent[node->NODE] =: NODE" },
    { GEL_PayoffEFOutcome, "Payoff[outcome->EFOUTCOME, player->EFPLAYER] =: NUMBER" },
    { GEL_Player, "Player[infoset->INFOSET] =: EFPLAYER" },
    { GEL_PriorSibling, "PriorSibling[node->NODE] =: NODE" },
    { GEL_RemoveAction, "RemoveAction[support->EFSUPPORT, action->ACTION] =: EFSUPPORT" },
    { GEL_RootNode, "RootNode[efg->EFG] =: NODE" },
    { GEL_SaveEfg, "SaveEfg[efg->EFG, file->TEXT] =: EFG" },
    { GEL_SetNameAction, "SetName[action->ACTION, name->TEXT] =: ACTION" },
    { GEL_SetNameInfoset, "SetName[infoset->INFOSET, name->TEXT] =: INFOSET" },
    { GEL_SetNameNode, "SetName[node->NODE, name->TEXT] =: NODE" },
    { GEL_SetNameEFOutcome, "SetName[outcome->EFOUTCOME, name->TEXT] =: EFOUTCOME" },
    { GEL_SetNameEFPlayer, "SetName[player->EFPLAYER, name->TEXT] =: EFPLAYER" },
    { GEL_SetNameEfg, "SetName[efg->EFG, name->TEXT] =: EFG" },
    { GEL_SetOutcome, "SetOutcome[node->NODE, outcome->EFOUTCOME] =: EFOUTCOME" },
    { GEL_SupportEfg, "Support[efg->EFG] =: EFSUPPORT" },
    { GEL_UnmarkSubgame, "UnmarkSubgame[node->NODE] =: NODE" },
    { 0, 0 } };


  for (int i = 0; sigarray[i].func; i++)
    env->Register(sigarray[i].func, sigarray[i].sig);
}












