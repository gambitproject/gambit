//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of extensive form editing functions
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "base/base.h"

#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "base/gstatus.h"
#include "game/efg.h"
#include "game/efgutils.h"
#include "game/efbasis.h"
#include "game/sfg.h"
#include "game/actiter.h"

//
// Implementations of these are provided as necessary in gsmutils.cc
//
template <class T> Portion *ArrayToList(const gArray<T> &);
// these added to get around g++ 2.7.2 not properly completing type
// unification when gVector passed to the parameter...
extern Portion *ArrayToList(const gArray<double> &);
extern Portion *ArrayToList(const gArray<gRational> &);
template <class T> Portion *ArrayToList(const gList<T> &);
template <class T> Portion *gDPVectorToList(const gDPVector<T> &);


//------------
// ActionNumber
//------------

static Portion *GSM_ActionNumber(GSM &, Portion **param)
{
  gbtEfgAction a = AsEfgAction(param[0]);
  EFSupport *support = ((EfSupportPortion *) param[1])->Value();

  return new NumberPortion(support->GetIndex(a));
}

static Portion *GSM_BasisActionNumber(GSM &, Portion **param)
{
  gbtEfgAction a = AsEfgAction(param[0]);
  EFBasis *basis = ((EfBasisPortion *) param[1])->Value();

  return new NumberPortion(basis->EFSupport::GetIndex(a));
}

//-------------
// Actions
//-------------

static Portion *GSM_Actions(GSM &, Portion **param)
{
  if (param[0]->Spec().Type == porNULL)
    return new ListPortion;

  Infoset *infoset = ((InfosetPortion *) param[0])->Value();
  EFSupport *support = ((EfSupportPortion *) param[1])->Value();

  ListPortion *ret = new ListPortion;
  for (gbtActionIterator action(*support, infoset); !action.End(); action++) {
    ret->Append(new ActionPortion(*action));
  }
  return ret;
}

static Portion *GSM_BasisActions(GSM &, Portion **param)
{
  if( param[0]->Spec().Type == porNULL )
    return new ListPortion;

  Infoset *s = ((InfosetPortion *) param[0])->Value();
  EFBasis* sup = ((EfBasisPortion*) param[1])->Value();

  ListPortion *ret = new ListPortion;
  if (s->IsChanceInfoset()) {
    for (int act = 1; act <= s->NumActions(); act++) {
      ret->Append(new ActionPortion(s->GetAction(act)));
    }
  }
  else {
    for (int act = 1; act <= sup->NumActions(s); act++) {
      ret->Append(new ActionPortion(sup->GetAction(s, act)));
    }
  }
  return ret;
}

//--------------
// AddAction
//--------------

static Portion *GSM_AddAction(GSM &, Portion **param)
{  
  EFSupport *support = ((EfSupportPortion *) param[0])->Value();
  gbtEfgAction action = AsEfgAction(param[1]);

  EFSupport *S = new EFSupport(*support);
  S->AddAction(action);

  return new EfSupportPortion(S);
}

static Portion *GSM_AddBasisAction(GSM &, Portion **param)
{  
  EFBasis *basis = ((EfBasisPortion *) param[0])->Value();
  gbtEfgAction action = AsEfgAction(param[1]);

  EFBasis *S = new EFBasis(*basis);
  S->AddAction(action);

  return new EfBasisPortion(S);
}

//-------------
// Nodes
//-------------

static Portion *GSM_BasisNodeNumber(GSM &, Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  EFBasis *basis = ((EfBasisPortion *) param[1])->Value();

  return new NumberPortion(basis->Find(n));
}

static Portion *GSM_BasisNodes(GSM &, Portion **param)
{
  if( param[0]->Spec().Type == porNULL )
    return new ListPortion;

  Infoset *s = ((InfosetPortion *) param[0])->Value();
  EFBasis* sup = ((EfBasisPortion*) param[1])->Value();

  ListPortion *ret = new ListPortion;
  if (s->IsChanceInfoset()) {
    for (int i = 1; i <= s->NumMembers(); i++) {
      ret->Append(new NodePortion(s->GetMember(i)));
    }
  }
  else {
    for (int i = 1; i <= sup->NumNodes(s); i++) {
      ret->Append(new NodePortion(sup->GetNode(s, i)));
    }
  }

  return ret;
}

static Portion *GSM_AddBasisNode(GSM &, Portion **param)
{  
  EFBasis *basis = ((EfBasisPortion *) param[0])->Value();
  Node *node = ((NodePortion *) param[1])->Value();

  EFBasis *S = new EFBasis(*basis);
  S->AddNode(node);

  return new EfBasisPortion(S);
}

//------------------
// AddMove
//------------------

static Portion *GSM_AddMove(GSM &gsm, Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  Node *n = ((NodePortion *) param[1])->Value();

  n->Game()->AppendNode(n, s);

  gsm.UnAssignGameElement(n->Game(), true, porBEHAV | porEFSUPPORT);

  return new NodePortion(n->GetChild(1));
}

//--------------
// Chance
//--------------

static Portion *GSM_Chance(GSM &, Portion **param)
{
  efgGame &E = *((EfgPortion*) param[0])->Value();

  return new EfPlayerPortion(E.GetChance());
}

//----------------
// ChanceProb
//----------------

static Portion *GSM_ChanceProb(GSM &, Portion **param)
{
  gbtEfgAction action = AsEfgAction(param[0]);
  Infoset *infoset = action.GetInfoset();
  if (!infoset->GetPlayer().IsChance()) { 
    throw gclRuntimeError("Action must belong to the chance player");
  }
  efgGame *efg = infoset->Game();

  return new NumberPortion(efg->GetChanceProb(infoset, action.GetId()));
}

//---------------
// Children
//---------------

static Portion *GSM_Children(GSM &, Portion **param)
{
  Node *node = ((NodePortion *) param[0])->Value();

  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= node->NumChildren(); i++) {
    ret->Append(new NodePortion(node->GetChild(i)));
  }
  return ret;
}

//------------
// Comment
//------------

static Portion *GSM_Comment(GSM &, Portion **param)
{
  efgGame *efg = ((EfgPortion *) param[0])->Value();
  return new TextPortion(efg->GetComment());
}

//---------------
// CompressEfg
//---------------

static Portion *GSM_CompressEfg(GSM &, Portion **param)
{
  EFSupport *S = ((EfSupportPortion *) param[0])->Value();

  return new EfgPortion(CompressEfg((efgGame &) S->GetGame(), *S));
}

//---------------
// CopyTree
//---------------

static Portion *GSM_CopyTree(GSM &gsm, Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();

  gsm.UnAssignGameElement(n1->Game(), true, porBEHAV | porEFSUPPORT);

  return new NodePortion(n1->Game()->CopyTree(n1, n2));
}

//-----------------
// DeleteAction
//-----------------

static Portion *GSM_DeleteAction(GSM &gsm, Portion **param)
{
  gbtEfgAction action = AsEfgAction(param[0]);
  Infoset *infoset = action.GetInfoset();

  if (infoset->NumActions() == 1)
    throw gclRuntimeError("Cannot delete the only action at an infoset.");

  infoset->Game()->DeleteAction(infoset, action);

  gsm.UnAssignGameElement(infoset->Game(), true, porBEHAV | porEFSUPPORT);

  return new InfosetPortion(infoset);
}

//----------------------
// DeleteEmptyInfoset
//----------------------

static Portion *GSM_DeleteEmptyInfoset(GSM &gsm, Portion **param)
{
  Infoset *infoset = ((InfosetPortion *) param[0])->Value();

  gsm.UnAssignGameElement(infoset->Game(), true, porBEHAV | porEFSUPPORT);
  gsm.UnAssignEfgElement(infoset->Game(), porINFOSET, infoset);

  return new BoolPortion(infoset->Game()->DeleteEmptyInfoset(infoset));
}

//----------------
// DeleteMove
//----------------

static Portion *GSM_DeleteMove(GSM &gsm, Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  Node *keep = ((NodePortion *) param[1])->Value();

  if (keep->GetParent() != n)
    throw gclRuntimeError("keep is not a child of node");

  gsm.UnAssignGameElement(n->Game(), true, porBEHAV | porEFSUPPORT);
  for (int i = 1; i <= n->NumChildren(); i++) 
    if (n->GetChild(i) != keep)
      gsm.UnAssignEfgSubTree(n->Game(), n->GetChild(i));
  gsm.UnAssignEfgElement(n->Game(), porNODE, n);

  return new NodePortion(n->Game()->DeleteNode(n, keep));
}

//-----------------
// DeleteOutcome
//-----------------

static Portion *GSM_DeleteOutcome(GSM &gsm, Portion **param)
{
  gbtEfgOutcome outcome = ((EfOutcomePortion *) param[0])->Value();

  efgGame *efg = outcome.GetGame();

  gList<Node *> nodes;
  Nodes(*efg, nodes);
  for (int i = 1; i <= nodes.Length(); ) {
    if (efg->GetOutcome(nodes[i]) != outcome) {
      nodes.Remove(i);
    }
    else {
      i++;
    }
  }
    
  gsm.InvalidateGameProfile(outcome.GetGame(), true);
  gsm.UnAssignEfgOutcome(outcome.GetGame(), outcome);

  efg->DeleteOutcome(outcome);

  return ArrayToList(nodes);
}

//----------------
// DeleteTree
//----------------

static Portion *GSM_DeleteTree(GSM &gsm, Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  gsm.UnAssignGameElement(n->Game(), true, porBEHAV | porEFSUPPORT);
  gsm.UnAssignEfgSubTree(n->Game(), n);

  n->Game()->DeleteTree(n);

  return new NodePortion(n);
}


//--------------
// IsDominated
//--------------

static Portion *GSM_IsDominated_Efg(GSM &, Portion **param)
{
  gbtEfgAction act = AsEfgAction(param[0]);
  EFSupport *S = ((EfSupportPortion *) param[1])->Value();
  bool strong = ((BoolPortion *) param[2])->Value();
  bool conditional = ((BoolPortion *) param[3])->Value();
  gWatch watch;
  bool ret;
  
  ret = S->IsDominated(act, strong, conditional);
  
  ((NumberPortion *) param[4])->SetValue(watch.Elapsed());
  
  return new BoolPortion(ret);
}


//----------
// Game
//----------

Portion* GSM_Game_EfgElements(GSM &, Portion** param)
{
  if (param[0]->Game())
    return new EfgPortion((efgGame*) param[0]->Game());
  else
    return 0;
}


//-------------
// Infoset
//-------------

static Portion *GSM_Infoset_Node(GSM &, Portion **param)
{
  if( param[0]->Spec().Type == porNULL )
    return new NullPortion( porINFOSET );

  Node *n = ((NodePortion *) param[0])->Value();

  if (!n->GetInfoset())
    return new NullPortion(porINFOSET);

  return new InfosetPortion(n->GetInfoset());
}


static Portion *GSM_Infoset_Action(GSM &, Portion **param)
{
  if( param[0]->Spec().Type == porNULL )
  {
    return new NullPortion( porINFOSET );
  }

  gbtEfgAction a = AsEfgAction(param[0]);

  if (!a.GetInfoset())
    return new NullPortion(porINFOSET);

  return new InfosetPortion(a.GetInfoset());
}

//-------------
// Infosets
//-------------

static Portion *GSM_Infosets(GSM &, Portion **param)
{
  gbtEfgPlayer player = AsEfgPlayer(param[0]);

  ListPortion *ret = new ListPortion;
  for (gbtEfgInfosetIterator infoset(player); !infoset.End(); infoset++) {
    ret->Append(new InfosetPortion(*infoset));
  }

  return ret;
}

//----------------
// InsertAction
//----------------

static Portion *GSM_InsertAction(GSM &gsm, Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();

  gsm.UnAssignGameElement(s->Game(), true, porBEHAV | porEFSUPPORT);

  return new ActionPortion(s->Game()->InsertAction(s));
}

static Portion *GSM_InsertActionAt(GSM &gsm, Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  gbtEfgAction a = AsEfgAction(param[1]);

  gsm.UnAssignGameElement(s->Game(), true, porBEHAV | porEFSUPPORT);

  return new ActionPortion(s->Game()->InsertAction(s, a));
}

//--------------
// InsertMove
//--------------

static Portion *GSM_InsertMove(GSM &gsm, Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  Node *n = ((NodePortion *) param[1])->Value();

  n->Game()->InsertNode(n, s);

  gsm.UnAssignGameElement(s->Game(), true, porBEHAV | porEFSUPPORT);

  return new NodePortion(n->GetParent());
}

//---------------
// IsConstSum
//---------------

static Portion *GSM_IsConstSum(GSM &, Portion **param)
{
  efgGame &E = *((EfgPortion*) param[0])->Value();
  return new BoolPortion(E.IsConstSum());
}

//---------------
// IsConsistent
//---------------

static Portion *GSM_IsBasisConsistent(GSM &, Portion **param)
{
  EFBasis *basis = ((EfBasisPortion *) param[0])->Value();
  return new BoolPortion(basis->IsConsistent());
}

//----------------
// IsPredecessor
//----------------

static Portion *GSM_IsPredecessor(GSM &, Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();
  return new BoolPortion(n1->Game()->IsPredecessor(n1, n2));
}

//---------------
// IsSuccessor
//---------------

static Portion *GSM_IsSuccessor(GSM &, Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();
  return new BoolPortion(n1->Game()->IsSuccessor(n1, n2));
}


//-----------
// LoadEfg
//-----------

static Portion *GSM_LoadEfg(GSM &, Portion **param)
{
  gText file = ((TextPortion *) param[0])->Value();
  
  try  {
    gFileInput f(file);
    efgGame *efg = ReadEfgFile(f);
    
    if (efg)
      return new EfgPortion(efg);
    else
      throw gclRuntimeError(file + " is not a valid .efg file");
  }
  catch (gFileInput::OpenFailed &)  {
    throw gclRuntimeError("Unable to open file " + file + " for reading");
  }
}

//-------------------
// IsPerfectRecall
//-------------------

static Portion *GSM_IsPerfectRecall(GSM &, Portion **param)
{
  Infoset *s1, *s2;
  efgGame &efg = * ((EfgPortion *) param[0])->Value();
  return new BoolPortion(IsPerfectRecall(efg, s1, s2));
}

//-----------------
// MarkSubgame
//-----------------

static Portion *GSM_MarkSubgame(GSM &, Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  return new BoolPortion(n->Game()->MarkSubgame(n));
}

//------------------
// MarkedSubgame
//------------------

static Portion *GSM_MarkedSubgame(GSM &, Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  return new BoolPortion(n->GetSubgameRoot() == n);
}

//------------
// Members
//------------

static Portion *GSM_Members(GSM &, Portion **param)
{
  if (param[0]->Spec().Type == porNULL)
    return new ListPortion();

  Infoset *infoset = ((InfosetPortion *) param[0])->Value();

  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= infoset->NumMembers(); i++) {
    ret->Append(new NodePortion(infoset->GetMember(i)));
  }
  return ret;
}

//----------------
// MergeInfosets
//----------------

static Portion *GSM_MergeInfosets(GSM &gsm, Portion **param)
{
  Infoset *s1 = ((InfosetPortion *) param[0])->Value();
  Infoset *s2 = ((InfosetPortion *) param[1])->Value();
  
  s1->Game()->MergeInfoset(s1, s2);
  
  gsm.UnAssignGameElement(s1->Game(), true, porBEHAV | porEFSUPPORT);

  return new InfosetPortion(s1);
}

//----------------
// MoveToInfoset
//----------------

static Portion *GSM_MoveToInfoset(GSM &gsm, Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  Infoset *s = ((InfosetPortion *) param[1])->Value();
  
  s->Game()->JoinInfoset(s, n);
  
  gsm.UnAssignGameElement(s->Game(), true, porBEHAV | porEFSUPPORT);

  return new NodePortion(n);
}

//-------------
// MoveTree
//-------------

static Portion *GSM_MoveTree(GSM &gsm, Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();

  gsm.UnAssignGameElement(n1->Game(), true, porBEHAV | porEFSUPPORT);

  return new NodePortion(n1->Game()->MoveTree(n1, n2));
}

//----------
// Name
//----------

static Portion *GSM_Name(GSM &, Portion **param)
{
  if (param[0]->Spec().Type == porNULL)
    return new TextPortion("");

  switch (param[0]->Spec().Type)  {
  case porACTION:
    return new TextPortion(AsEfgAction(param[0]).GetLabel());
  case porINFOSET:
    return new TextPortion(((InfosetPortion *) param[0])->Value()->
			   GetName());
  case porNODE:
    return new TextPortion(((NodePortion *) param[0])->Value()->
			   GetName());
  case porEFOUTCOME:
    return new TextPortion(((EfOutcomePortion *) param[0])->Value().GetLabel());
  case porEFPLAYER:
    return new TextPortion(AsEfgPlayer(param[0]).GetLabel());
  case porEFSUPPORT:
    return new TextPortion(((EfSupportPortion *) param[0])->Value()->GetName());
  case porEFG:
    return new TextPortion(((EfgPortion *) param[0])->Value()->
			   GetTitle());
  default:
    throw gclRuntimeError("Unknown type passed to Name[]");
  }
}


//------------
// NewEfg
//------------

static Portion *GSM_NewEfg(GSM &, Portion **param)
{
  efgGame *E = new efgGame;
  ListPortion *players = (ListPortion *) param[0];
  for (int i = 1; i <= players->Length(); i++) {
    E->NewPlayer().SetLabel(((TextPortion *) (*players)[i])->Value());
  }
  return new EfgPortion(E);
}


//--------------
// NewInfoset
//--------------

static Portion *GSM_NewInfoset(GSM &gsm, Portion **param)
{
  gbtEfgPlayer player = AsEfgPlayer(param[0]);
  int n = ((NumberPortion *) param[1])->Value();

  if (n <= 0) {
    throw gclRuntimeError("Information sets must have at least one action");
  }

  Infoset *s = player.GetGame()->CreateInfoset(player, n);
  gsm.UnAssignGameElement(player.GetGame(), true, porBEHAV | porEFSUPPORT);
  return new InfosetPortion(s);
}
 
//--------------
// NewOutcome
//--------------

static Portion *GSM_NewOutcome(GSM &, Portion **param)
{
  efgGame *efg = ((EfgPortion *) param[0])->Value();
  gbtEfgOutcome outcome = efg->NewOutcome();

  return new EfOutcomePortion(outcome);
}

//---------------
// NewPlayer
//---------------

static Portion *GSM_NewPlayer(GSM &gsm, Portion **param)
{
  efgGame &E = *((EfgPortion*) param[0])->Value();
  gbtEfgPlayer player = E.NewPlayer();

  gsm.UnAssignGameElement(&E, true, porBEHAV | porEFSUPPORT);

  return new EfPlayerPortion(player);
}

//----------------
// NextSibling
//----------------

static Portion *GSM_NextSibling(GSM &, Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value()->NextSibling();
  if (!n)
    return new NullPortion(porNODE);
  
  return new NodePortion(n);
}

//----------
// Nodes
//----------

static Portion *GSM_Nodes(GSM &, Portion **param)
{
  efgGame *efg = ((EfgPortion *) param[0])->Value();

  gList<Node *> nodes;
  Nodes(*efg, nodes);

  Portion *por = ArrayToList(nodes);
  return por;
}

//---------------
// NthChild
//---------------

static Portion *GSM_NthChild(GSM &, Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  int child = ((NumberPortion *) param[1])->Value();
  if (child < 1 || child > n->NumChildren())  
    return new NullPortion(porNODE);

  return new NodePortion(n->GetChild(child));
}

//------------
// Outcome
//------------

static Portion *GSM_Outcome(GSM &, Portion **param)
{
  if (param[0]->Spec().Type == porNULL) {
    return new NullPortion(porEFOUTCOME);
  }

  Node *n = ((NodePortion *) param[0])->Value();
  gbtEfgOutcome outcome = n->Game()->GetOutcome(n);
  if (outcome.IsNull()) {
    return new NullPortion(porEFOUTCOME);
  }
  else {
    return new EfOutcomePortion(outcome);
  }
}

//------------
// Outcomes
//------------

static Portion *GSM_Outcomes(GSM &, Portion **param)
{
  efgGame *efg = ((EfgPortion*) param[0])->Value();

  ListPortion *ret = new ListPortion;
  for (int outc = 1; outc <= efg->NumOutcomes(); outc++) {
    ret->Append(new EfOutcomePortion(efg->GetOutcome(outc)));
  }
  
  return ret;
}

//------------
// Parent
//------------

static Portion *GSM_Parent(GSM &, Portion **param)
{
  if( param[0]->Spec().Type == porNULL )
  {
    return new NullPortion(porNODE);
  }

  Node *n = ((NodePortion *) param[0])->Value();
  if (!n->GetParent())
    return new NullPortion(porNODE);

  return new NodePortion(n->GetParent());
}

//-----------
// Payoff
//-----------

static Portion *GSM_Payoff(GSM &, Portion **param)
{
  if (param[0]->Spec().Type == porNULL)
    return new NumberPortion(0);
  gbtEfgOutcome outcome = ((EfOutcomePortion *) param[0])->Value();
  gbtEfgPlayer player = AsEfgPlayer(param[1]);
  efgGame *efg = player.GetGame();

  return new NumberPortion(efg->Payoff(outcome, player));
}

//----------
// Player
//----------

static Portion *GSM_Player(GSM &, Portion **param)
{
  if( param[0]->Spec().Type == porNULL )
  {
    return new NullPortion( porEFPLAYER );
  }

  Infoset *s = ((InfosetPortion *) param[0])->Value();

  return new EfPlayerPortion(s->GetPlayer());
}

//------------
// Players
//------------

static Portion *GSM_Players(GSM &, Portion **param)
{
  efgGame &E = *((EfgPortion*) param[0])->Value();

  ListPortion *ret = new ListPortion;
  for (int pl = 1; pl <= E.NumPlayers(); pl++) {
    ret->Append(new EfPlayerPortion(E.GetPlayer(pl)));
  }

  return ret;
}

//------------------------
// PossibleNashSupports
//------------------------

#include "game/efgensup.h"

static Portion *GSM_PossibleNashSupports(GSM &gsm, Portion **param)
{
  efgGame &E = *((EfgPortion*) param[0])->Value();

  gList<const EFSupport> list =
    PossibleNashSubsupports(EFSupport(E), gsm.GetStatusMonitor());

  return ArrayToList(list);
}

//--------------
// PriorAction
//--------------

static Portion *GSM_PriorAction(GSM &, Portion** param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  efgGame *e = ((efgGame*) param[0]->Game());
  gbtEfgAction a = LastAction(*e,n);
  if (a.IsNull()) {
    return new NullPortion(porACTION);
  }
  return new ActionPortion(a);
}

//----------------
// PriorSibling
//----------------

static Portion *GSM_PriorSibling(GSM &, Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value()->PriorSibling();
  if (!n)
    return new NullPortion(porNODE);
  
  return new NodePortion(n);
}

//-----------------
// RemoveAction
//-----------------

static Portion *GSM_RemoveAction(GSM &, Portion **param)
{  
  EFSupport *support = ((EfSupportPortion *) param[0])->Value();
  gbtEfgAction action = AsEfgAction(param[1]);

  EFSupport *S = new EFSupport(*support);
  S->RemoveAction(action);

  return new EfSupportPortion(S);
}

static Portion *GSM_RemoveBasisAction(GSM &, Portion **param)
{  
  EFBasis *support = ((EfBasisPortion *) param[0])->Value();
  gbtEfgAction action = AsEfgAction(param[1]);

  EFBasis *S = new EFBasis(*support);
  S->RemoveAction(action);

  return new EfBasisPortion(S);
}


//-----------------
// RemoveNode
//-----------------

static Portion *GSM_RemoveBasisNode(GSM &, Portion **param)
{  
  EFBasis *basis = ((EfBasisPortion *) param[0])->Value();
  Node *node = ((NodePortion *) param[1])->Value();

  EFBasis *S = new EFBasis(*basis);
  S->RemoveNode(node);

  return new EfBasisPortion(S);
}


//-------------
// Reveal
//-------------

static Portion *GSM_Reveal(GSM &gsm, Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  ListPortion *players = (ListPortion *) param[1];

  for (int i = 1; i <= players->Length(); i++) {
    s->Game()->Reveal(s, AsEfgPlayer((*players)[i]));
  }

  gsm.UnAssignGameElement(s->Game(), true, porBEHAV | porEFSUPPORT);

  return new InfosetPortion(s);
}

//-------------
// RootNode
//-------------

static Portion *GSM_RootNode(GSM &, Portion **param)
{
  efgGame &E = *((EfgPortion*) param[0])->Value();
  return new NodePortion(E.RootNode());
}

//-------------
// SaveEfg
//-------------

extern NumberPortion _WriteGameDecimals;

static Portion *GSM_SaveEfg(GSM &, Portion **param)
{
  efgGame* E = ((EfgPortion *) param[0])->Value();
  gText text = ((TextPortion *) param[1])->Value();

  try { 
    gFileOutput f(text);
    E->WriteEfgFile(f, _WriteGameDecimals.Value());
  }
  catch (gFileOutput::OpenFailed &)  {
    throw gclRuntimeError("Cannot open file " + text + " for writing");
  }

  return param[0]->ValCopy();
}

//-------------
// WriteSequenceForm
//-------------

static Portion *GSM_WriteSfg(GSM &, Portion **param)
{
  gOutput &out = ((OutputPortion*) param[0])->Value();
  const efgGame *efg = ((EfgPortion*) param[1])->Value();
  if (!IsPerfectRecall(*efg)) 
    throw gclRuntimeError("Sequence form not defined for game of imperfect recall");

  EFSupport efs(*efg);
  Sfg sfg(efs);
  sfg.Dump(out);
  return param[0]->ValCopy();
}

//-------------
// SequenceForm
//-------------

void Recurse_Sfg(ListPortion *por, int pl, const Sfg &sfg, gIndexOdometer &index)
{
  if(pl >=1 ) 
    for(int j=1;j<=sfg.NumSequences(pl);j++) {
      ListPortion *p = new ListPortion;
      Recurse_Sfg(p,pl-1,sfg,index);
      por->Append(p);
    }
  else {
    index.Turn();
    for (int i = 1; i <= sfg.NumPlayers(); i++) 
      por->Append(new NumberPortion(sfg.Payoff(index.CurrentIndices(),i)));
  }
}

static Portion *GSM_Sfg(GSM &, Portion **param)
{
  const efgGame *efg = ((EfgPortion*) param[0])->Value();
  if (!IsPerfectRecall(*efg)) 
    throw gclRuntimeError("Sequence form not defined for game of imperfect recall");

  EFSupport efs(*efg);
  Sfg sfg(efs);

  ListPortion *por = new ListPortion;
  gIndexOdometer index(sfg.NumSequences());

  Recurse_Sfg(por,sfg.NumPlayers(),sfg,index);

  return por;
}

static Portion *GSM_SfgStrats(GSM &, Portion **param)
{
  const efgGame *efg = ((EfgPortion*) param[0])->Value();
  if (!IsPerfectRecall(*efg)) 
    throw gclRuntimeError("Sequence form not defined for game of imperfect recall");
  gbtEfgPlayer player = AsEfgPlayer(param[1]);
  int p = player.GetId();
  EFSupport efs(*efg);
  Sfg sfg(efs);

  ListPortion *por = new ListPortion;
  for (int i=1;i<=sfg.NumSequences(p);i++) {
    gList<gbtEfgAction> h((sfg.GetSequence(p,i))->History());
    ListPortion *por1 = new ListPortion;
    for(int j=1;j<=h.Length();j++) {
      por1->Append(new ActionPortion(h[j]));
    }
    por->Append(por1);
  }  
  return por;
}

static Portion *GSM_SfgConstraints(GSM &, Portion **param)
{
  const efgGame *efg = ((EfgPortion*) param[0])->Value();
  if (!IsPerfectRecall(*efg)) 
    throw gclRuntimeError("Sequence form not defined for game of imperfect recall");
  gbtEfgPlayer player = AsEfgPlayer(param[1]);
  int p = player.GetId();
  EFSupport efs(*efg);
  Sfg sfg(efs);

  gRectArray<gNumber> A(sfg.Constraints(p));

  ListPortion *por = new ListPortion;
  for(int i=A.MinRow();i<=A.MaxRow();i++) {
    ListPortion *p1 = new ListPortion;
    for(int j=A.MinCol();j<=A.MaxCol();j++)
      p1->Append(new NumberPortion(A(i,j)));
    por->Append(p1);
  }
  
  return por;
}
/*
  #include "seqform.h"
  #include "seqform.imp"
  
  static Portion *GSM_SfgTableau(GSM &, Portion **param)
  {
  EFSupport *efs = ((EfSupportPortion*) param[0])->Value();
  SeqFormParams prm;
  SeqFormModule<gNumber> sfm(*efs,prm);
  
  gMatrix<gNumber> A(sfm.GetA());
  ListPortion *por = new ListPortion;
  for(int i=A.MinRow();i<=A.MaxRow();i++) {
  ListPortion *p1 = new ListPortion;
  for(int j=A.MinCol();j<=A.MaxCol();j++)
  p1->Append(new NumberPortion(A(i,j)));
  por->Append(p1);
  }
  
  return por;
  }
*/

//-------------------
// SetChanceProbs
//-------------------

static Portion *GSM_SetChanceProbs(GSM &gsm, Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  ListPortion *p = (ListPortion *) param[1];
  efgGame *efg = s->Game();

  if (!s->GetPlayer().IsChance()) {
    throw gclRuntimeError("Information set does not belong to chance player");
  }
  
  for (int i = 1; i <= p->Length(); i++)
    efg->SetChanceProb(s, i, ((NumberPortion *) (*p)[i])->Value());
  gsm.InvalidateGameProfile(s->Game(), true);

  return new InfosetPortion(s);
}

//-------------
// SetComment
//-------------

static Portion *GSM_SetComment(GSM &, Portion **param)
{
  efgGame *efg = ((EfgPortion *) param[0])->Value();
  gText comment = ((TextPortion *) param[1])->Value();
  efg->SetComment(comment);
  return param[0]->ValCopy();
}

//--------------
// SetName
//--------------

static Portion *GSM_SetName(GSM &, Portion **param)
{
  gText name(((TextPortion *) param[1])->Value());
  
  switch (param[0]->Spec().Type)   {
  case porACTION:
    AsEfgAction(param[0]).SetLabel(name);
    break;
  case porINFOSET:
    ((InfosetPortion *) param[0])->Value()->SetName(name);
    break;
  case porNODE:
    ((NodePortion *) param[0])->Value()->SetName(name);
    break;
  case porEFOUTCOME: {
    gbtEfgOutcome outcome = ((EfOutcomePortion *) param[0])->Value();
    outcome.GetGame()->SetLabel(outcome, name);
    break;
  }
  case porEFPLAYER:
    AsEfgPlayer(param[0]).SetLabel(name);
    break;
  case porEFSUPPORT:
    ((EfSupportPortion *) param[0])->Value()->SetName(name);
    break;
  case porEFG:
    ((EfgPortion *) param[0])->Value()->SetTitle(name);
    break;
  default:
    throw gclRuntimeError("Bad type passed to SetName[]");
  }

  return param[0]->ValCopy();
}


//----------------
// SetOutcome
//----------------

static Portion *GSM_SetOutcome(GSM &gsm, Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  gbtEfgOutcome outcome = ((EfOutcomePortion *) param[1])->Value();

  if (!outcome.IsNull()) {
    outcome.GetGame()->SetOutcome(n, outcome);
    gsm.InvalidateGameProfile(n->Game(), true);
    return new EfOutcomePortion(outcome);
  }
  else {
    return new NullPortion(porEFOUTCOME);
  }
}

//----------------
// SetPayoff
//----------------

static Portion *GSM_SetPayoff(GSM &gsm, Portion **param)
{
  gbtEfgOutcome outcome = ((EfOutcomePortion *) param[0])->Value();
  gbtEfgPlayer player = AsEfgPlayer(param[1]);
  efgGame *efg = player.GetGame();
  gNumber value = ((NumberPortion *) param[2])->Value();

  efg->SetPayoff(outcome, player.GetId(), value);

  gsm.InvalidateGameProfile(outcome.GetGame(), true);

  return param[0]->ValCopy();
}


//----------------
// Subgames
//----------------

static Portion *GSM_Subgames(GSM &, Portion **param)
{
  efgGame &E = *((EfgPortion*) param[0])->Value();
  gList<Node *> list;
  LegalSubgameRoots(E, list);

  Portion *por = ArrayToList(list);
  return por;  
}  

//--------------
// Support
//--------------

static Portion *GSM_Support(GSM &, Portion **param)
{
  efgGame &E = * ((EfgPortion *) param[0])->Value();
  Portion *por = new EfSupportPortion(new EFSupport(E));
  return por;
}

//--------------
// EFBasis
//--------------

static Portion *GSM_Basis(GSM &, Portion **param)
{
  efgGame &E = * ((EfgPortion *) param[0])->Value();
  Portion *por = new EfBasisPortion(new EFBasis(E));
  return por;
}

//--------------
// UnDominated
//--------------

static Portion *GSM_UnDominated(GSM &gsm, Portion **param)
{
  EFSupport *S = ((EfSupportPortion *) param[0])->Value();
  bool strong = ((BoolPortion *) param[1])->Value();
  bool conditional = ((BoolPortion *) param[2])->Value();
  gWatch watch;

  gBlock<int> players(S->GetGame().NumPlayers());
  for (int i = 1; i <= players.Length(); i++)   players[i] = i;

  EFSupport T(S->Undominated(strong, conditional, players,
			     ((OutputPortion *) param[4])->Value(),
			     gsm.GetStatusMonitor()));

  ((NumberPortion *) param[3])->SetValue(watch.Elapsed());
  
  return new EfSupportPortion(new EFSupport(T));
}

//-----------------
// UnMarkSubgame
//-----------------

static Portion *GSM_UnMarkSubgame(GSM &, Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  n->Game()->UnmarkSubgame(n);
  
  return new NodePortion(n);
}



void Init_efgfunc(GSM *gsm)
{
  gclFunction *FuncObj;

  static struct { char *sig; Portion *(*func)(GSM &, Portion **); } ftable[] =
    { { "Actions[infoset->INFOSET*, support->EFSUPPORT] =: LIST(ACTION)",
 	GSM_Actions },
      { "Actions[infoset->INFOSET*, basis->EFBASIS] =: LIST(ACTION)",
 	GSM_BasisActions },
      { "ActionNumber[action->ACTION, sup->EFSUPPORT] =: INTEGER", 
	GSM_ActionNumber },
      { "ActionNumber[action->ACTION, basis->EFBASIS] =: INTEGER", 
	GSM_BasisActionNumber },
      { "AddAction[support->EFSUPPORT, action->ACTION] =: EFSUPPORT",
	GSM_AddAction },
      { "AddAction[basis->EFBASIS, action->ACTION] =: EFBASIS",
	GSM_AddBasisAction },
      { "AddMove[infoset->INFOSET, node->NODE] =: NODE",
	GSM_AddMove },
      { "AddNode[basis->EFBASIS, node->NODE] =: EFBASIS",
	GSM_AddBasisNode },
      { "Chance[efg->EFG] =: EFPLAYER", GSM_Chance },
      { "ChanceProb[action->ACTION] =: NUMBER", GSM_ChanceProb },
      { "Children[node->NODE] =: LIST(NODE)", GSM_Children },
      { "Comment[efg->EFG] =: TEXT", GSM_Comment },
      { "CompressEfg[support->EFSUPPORT] =: EFG", GSM_CompressEfg },
      { "CopyTree[from->NODE, to->NODE] =: NODE", GSM_CopyTree },
      { "DeleteAction[action->ACTION] =: INFOSET", GSM_DeleteAction },
      { "DeleteEmptyInfoset[infoset->INFOSET] =: BOOLEAN", 
	GSM_DeleteEmptyInfoset },
      { "DeleteMove[node->NODE, keep->NODE] =: NODE", GSM_DeleteMove },
      { "DeleteOutcome[outcome->EFOUTCOME] =: LIST(NODE)",
	GSM_DeleteOutcome },
      { "DeleteTree[node->NODE] =: NODE", GSM_DeleteTree },
      { "Game[player->EFPLAYER] =: EFG", GSM_Game_EfgElements },
      { "Game[node->NODE] =: EFG", GSM_Game_EfgElements },
      { "Game[outcome->EFOUTCOME] =: EFG", GSM_Game_EfgElements },
      { "Game[infoset->INFOSET] =: EFG", GSM_Game_EfgElements },
      { "Infoset[node->NODE*] =: INFOSET", GSM_Infoset_Node },
      { "Infoset[action->ACTION*] =: INFOSET", GSM_Infoset_Action },
      { "Infosets[player->EFPLAYER] =: LIST(INFOSET)", GSM_Infosets },
      { "InsertAction[infoset->INFOSET] =: ACTION", GSM_InsertAction },
      { "InsertAction[infoset->INFOSET, at->ACTION] =: ACTION",
	GSM_InsertActionAt },
      { "InsertMove[infoset->INFOSET, node->NODE] =: NODE",
	GSM_InsertMove },
      { "IsConsistent[basis->EFBASIS] =: BOOLEAN", GSM_IsBasisConsistent },
      { "IsConstSum[efg->EFG] =: BOOLEAN", GSM_IsConstSum },
      { "IsPerfectRecall[efg->EFG] =: BOOLEAN", GSM_IsPerfectRecall },
      { "IsPredecessor[node->NODE, of->NODE] =: BOOLEAN",
	GSM_IsPredecessor },
      { "IsSuccessor[node->NODE, from->NODE] =: BOOLEAN",
	GSM_IsSuccessor },
      { "LoadEfg[file->TEXT] =: EFG", GSM_LoadEfg },
      { "MarkSubgame[node->NODE] =: BOOLEAN", GSM_MarkSubgame },
      { "MarkedSubgame[node->NODE] =: BOOLEAN", GSM_MarkedSubgame },
      { "Members[infoset->INFOSET*] =: LIST(NODE)", GSM_Members },
      { "MergeInfosets[to->INFOSET, from->INFOSET] =: INFOSET",
	GSM_MergeInfosets },
      { "MoveToInfoset[node->NODE, infoset->INFOSET] =: NODE",
	GSM_MoveToInfoset },
      { "MoveTree[from->NODE, to->NODE] =: NODE", GSM_MoveTree },
      { "Name[x->ACTION*] =: TEXT", GSM_Name },
      { "Name[x->INFOSET*] =: TEXT", GSM_Name },
      { "Name[x->NODE*] =: TEXT", GSM_Name },
      { "Name[x->EFOUTCOME*] =: TEXT", GSM_Name },
      { "Name[x->EFPLAYER*] =: TEXT", GSM_Name },
      { "Name[x->EFSUPPORT*] =: TEXT", GSM_Name },
      { "Name[x->EFG*] =: TEXT", GSM_Name },
      { "NewInfoset[player->EFPLAYER, actions->NUMBER] =: INFOSET",
	GSM_NewInfoset },
      { "NewOutcome[efg->EFG] =: EFOUTCOME", GSM_NewOutcome },
      { "NewPlayer[efg->EFG] =: EFPLAYER", GSM_NewPlayer },
      { "NextSibling[node->NODE] =: NODE", GSM_NextSibling },
      { "Nodes[efg->EFG] =: LIST(NODE)", GSM_Nodes },
      { "Nodes[infoset->INFOSET*, basis->EFBASIS] =: LIST(NODE)",
 	GSM_BasisNodes },
      { "NodeNumber[node->NODE, basis->EFBASIS] =: INTEGER", 
	GSM_BasisNodeNumber },
      { "NthChild[node->NODE, n->INTEGER] =: NODE", GSM_NthChild },
      { "Outcome[node->NODE*] =: EFOUTCOME", GSM_Outcome },
      { "Outcomes[efg->EFG] =: LIST(EFOUTCOME)", GSM_Outcomes },
      { "Parent[node->NODE*] =: NODE", GSM_Parent },
      { "Payoff[outcome->EFOUTCOME*, player->EFPLAYER] =: NUMBER",
	GSM_Payoff },
      { "Player[infoset->INFOSET*] =: EFPLAYER", GSM_Player },
      { "Players[efg->EFG] =: LIST(EFPLAYER)", GSM_Players },
      { "PriorAction[node->NODE] =: ACTION", GSM_PriorAction },
      { "PriorSibling[node->NODE] =: NODE", GSM_PriorSibling },
      { "RemoveAction[support->EFSUPPORT, action->ACTION] =: EFSUPPORT",
	GSM_RemoveAction },
      { "RemoveAction[support->EFBASIS, action->ACTION] =: EFBASIS",
	GSM_RemoveBasisAction },
      { "RemoveNode[support->EFBASIS, node->NODE] =: EFBASIS",
	GSM_RemoveBasisNode },
      { "Reveal[infoset->INFOSET, who->LIST(EFPLAYER)] =: INFOSET",
	GSM_Reveal },
      { "RootNode[efg->EFG] =: NODE", GSM_RootNode }, 
      { "SaveEfg[efg->EFG, file->TEXT] =: EFG", GSM_SaveEfg },
      { "SetChanceProbs[infoset->INFOSET, probs->LIST(NUMBER)] =: INFOSET",
	GSM_SetChanceProbs },
      { "SetComment[efg->EFG, comment->TEXT] =: EFG", GSM_SetComment },
      { "SetName[x->ACTION, name->TEXT] =: ACTION", GSM_SetName },
      { "SetName[x->INFOSET, name->TEXT] =: INFOSET", GSM_SetName },
      { "SetName[x->NODE, name->TEXT] =: NODE", GSM_SetName },
      { "SetName[x->EFOUTCOME, name->TEXT] =: EFOUTCOME", GSM_SetName },
      { "SetName[x->EFPLAYER, name->TEXT] =: EFPLAYER", GSM_SetName },
      { "SetName[x->EFSUPPORT, name->TEXT] =: EFSUPPORT", GSM_SetName },
      { "SetName[x->EFG, name->TEXT] =: EFG", GSM_SetName },
      { "SetOutcome[node->NODE, outcome->EFOUTCOME*] =: EFOUTCOME",
	GSM_SetOutcome },
      { "SetPayoff[outcome->EFOUTCOME, player->EFPLAYER, payoff->NUMBER] =: EFOUTCOME", GSM_SetPayoff },
      { "Subgames[efg->EFG] =: LIST(NODE)", GSM_Subgames },
      { "Support[efg->EFG] =: EFSUPPORT", GSM_Support },
      { "Basis[efg->EFG] =: EFBASIS", GSM_Basis },
      { "UnMarkSubgame[node->NODE] =: NODE", GSM_UnMarkSubgame },
      { "PossibleNashSupports[efg->EFG] =: LIST(EFSUPPORT)", GSM_PossibleNashSupports },
      { 0, 0 }
    };

  for (int i = 0; ftable[i].sig != 0; i++) {
    gsm->AddFunction(new gclFunction(*gsm, ftable[i].sig, ftable[i].func,
				     funcLISTABLE | funcGAMEMATCH));
  }

  FuncObj = new gclFunction(*gsm, "UnDominated", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_UnDominated, porEFSUPPORT, 6));
  FuncObj->SetParamInfo(0, 0, gclParameter("support", porEFSUPPORT));
  FuncObj->SetParamInfo(0, 1, gclParameter("strong", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(0, 2, gclParameter("conditional", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(0, 3, gclParameter("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 4, gclParameter("traceFile", porOUTPUT,
					    new OutputPortion(*new gNullOutput), 
					    BYREF));
  FuncObj->SetParamInfo(0, 5, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "IsDominated", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_IsDominated_Efg, porBOOLEAN, 7));
  FuncObj->SetParamInfo(0, 0, gclParameter("action", porACTION));
  FuncObj->SetParamInfo(0, 1, gclParameter("support", porEFSUPPORT));
  FuncObj->SetParamInfo(0, 2, gclParameter("strong", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(0, 3, gclParameter("conditional", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(0, 4, gclParameter("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 5, gclParameter("traceFile", porOUTPUT,
					    new OutputPortion(*new gNullOutput), 
					    BYREF));
  FuncObj->SetParamInfo(0, 6, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));
  gsm->AddFunction(FuncObj);

  // Adding NewEfg
  FuncObj = new gclFunction(*gsm, "NewEfg", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_NewEfg, porEFG, 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("players", PortionSpec(porTEXT,1),
					    new ListPortion));
  gsm->AddFunction(FuncObj);

  // Adding WriteSequenceForm
  FuncObj = new gclFunction(*gsm, "WriteSequenceForm", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_WriteSfg, porOUTPUT, 2, 0 , funcNONLISTABLE));
  FuncObj->SetParamInfo(0, 0, gclParameter("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, gclParameter("efg", porEFG));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "SequenceForm", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Sfg, PortionSpec(porNUMBER, NLIST), 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("efg", porEFG));
  gsm->AddFunction(FuncObj);
  FuncObj = new gclFunction(*gsm, "SequenceFormStrats", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_SfgStrats, PortionSpec(porACTION, 2), 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("efg", porEFG));
  FuncObj->SetParamInfo(0, 1, gclParameter("player", porEFPLAYER));
  gsm->AddFunction(FuncObj);
  FuncObj = new gclFunction(*gsm, "SequenceFormConstraints", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_SfgConstraints, PortionSpec(porNUMBER, 2), 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("efg", porEFG));
  FuncObj->SetParamInfo(0, 1, gclParameter("player", porEFPLAYER));
  gsm->AddFunction(FuncObj);
  /*
    FuncObj = new gclFunction(*gsm, "SequenceFormTableau", 1);
    FuncObj->SetFuncInfo(0, gclSignature(GSM_SfgTableau, PortionSpec(porNUMBER, 2), 1));
    FuncObj->SetParamInfo(0, 0, gclParameter("support", porEFSUPPORT));
    gsm->AddFunction(FuncObj);
  */
}



