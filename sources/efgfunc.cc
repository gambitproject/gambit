//
// FILE: efgfunc.cc -- Extensive form editing builtins
//
// $Id$
//


#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "gwatch.h"
#include "gstatus.h"

#include "efg.h"
#include "efgutils.h"

#include "efbasis.h"

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


extern GSM *_gsm;

//------------
// ActionNumber
//------------

static Portion *GSM_ActionNumber(Portion **param)
{
  Action *a = ((ActionPortion *) param[0])->Value();
  EFSupport *support = ((EfSupportPortion *) param[1])->Value();

  return new NumberPortion(support->Find(a));
}

static Portion *GSM_BasisActionNumber(Portion **param)
{
  Action *a = ((ActionPortion *) param[0])->Value();
  EFBasis *basis = ((EfBasisPortion *) param[1])->Value();

  return new NumberPortion(basis->EFSupport::Find(a));
}

//-------------
// Actions
//-------------

static Portion *GSM_Actions(Portion **param)
{
  if (param[0]->Spec().Type == porNULL)
    return new ListPortion;

  Infoset *infoset = ((InfosetPortion *) param[0])->Value();
  EFSupport *support = ((EfSupportPortion *) param[1])->Value();
  return ArrayToList(support->Actions(infoset));
}

static Portion *GSM_BasisActions(Portion **param)
{
  if( param[0]->Spec().Type == porNULL )
    return new ListPortion;

  Infoset *s = ((InfosetPortion *) param[0])->Value();
  EFBasis* sup = ((EfBasisPortion*) param[1])->Value();

  Portion *por = (s->IsChanceInfoset()) ? ArrayToList(s->Actions()) :
                ArrayToList(sup->Actions(s->GetPlayer()->GetNumber(),
					 s->GetNumber()));
  return por;
}

//--------------
// AddAction
//--------------

static Portion *GSM_AddAction(Portion **param)
{  
  EFSupport *support = ((EfSupportPortion *) param[0])->Value();
  Action *action = ((ActionPortion *) param[1])->Value();

  EFSupport *S = new EFSupport(*support);
  S->AddAction(action);

  return new EfSupportPortion(S);
}

static Portion *GSM_AddBasisAction(Portion **param)
{  
  EFBasis *basis = ((EfBasisPortion *) param[0])->Value();
  Action *action = ((ActionPortion *) param[1])->Value();

  EFBasis *S = new EFBasis(*basis);
  S->AddAction(action);

  return new EfBasisPortion(S);
}

//-------------
// Nodes
//-------------

static Portion *GSM_BasisNodeNumber(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  EFBasis *basis = ((EfBasisPortion *) param[1])->Value();

  return new NumberPortion(basis->Find(n));
}

static Portion *GSM_BasisNodes(Portion **param)
{
  if( param[0]->Spec().Type == porNULL )
    return new ListPortion;

  Infoset *s = ((InfosetPortion *) param[0])->Value();
  EFBasis* sup = ((EfBasisPortion*) param[1])->Value();

  Portion *por = (s->IsChanceInfoset()) ? ArrayToList(s->Members()) :
                ArrayToList(sup->Nodes(s->GetPlayer()->GetNumber(),
					 s->GetNumber()));
  return por;
}

static Portion *GSM_AddBasisNode(Portion **param)
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

static Portion *GSM_AddMove(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  Node *n = ((NodePortion *) param[1])->Value();

  n->Game()->AppendNode(n, s);

  _gsm->UnAssignGameElement(n->Game(), true, porBEHAV | porEFSUPPORT);

  return new NodePortion(n->GetChild(1));
}

//--------------
// Chance
//--------------

static Portion *GSM_Chance(Portion **param)
{
  Efg &E = *((EfgPortion*) param[0])->Value();

  return new EfPlayerPortion(E.GetChance());
}

//----------------
// ChanceProb
//----------------

static Portion *GSM_ChanceProb(Portion **param)
{
  const Action *action = ((ActionPortion *) param[0])->Value();
  Infoset *infoset = action->BelongsTo();
  if (!infoset->GetPlayer()->IsChance()) 
    throw gclRuntimeError("Action must belong to the chance player");
  Efg *efg = infoset->Game();

  return new NumberPortion(efg->GetChanceProb(infoset, action->GetNumber()));
}

//---------------
// Children
//---------------

static Portion *GSM_Children(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  return ArrayToList(n->Children());
}

//------------
// Comment
//------------

static Portion *GSM_Comment(Portion **param)
{
  Efg *efg = ((EfgPortion *) param[0])->Value();
  return new TextPortion(efg->GetComment());
}

//---------------
// CompressEfg
//---------------

Efg *CompressEfg(const Efg &, const EFSupport &);

static Portion *GSM_CompressEfg(Portion **param)
{
  EFSupport *S = ((EfSupportPortion *) param[0])->Value();

  return new EfgPortion(CompressEfg(S->Game(), *S));
}

//---------------
// CopyTree
//---------------

static Portion *GSM_CopyTree(Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();

  _gsm->UnAssignGameElement(n1->Game(), true, porBEHAV | porEFSUPPORT);

  return new NodePortion(n1->Game()->CopyTree(n1, n2));
}

//-----------------
// DeleteAction
//-----------------

static Portion *GSM_DeleteAction(Portion **param)
{
  const Action *action = ((ActionPortion *) param[0])->Value();
  Infoset *infoset = action->BelongsTo();

  if (infoset->NumActions() == 1)
    throw gclRuntimeError("Cannot delete the only action at an infoset.");

  infoset->Game()->DeleteAction(infoset, action);

  _gsm->UnAssignGameElement(infoset->Game(), true,
			    porBEHAV | porEFSUPPORT);
  _gsm->UnAssignEfgElement(infoset->Game(), porACTION, (Action *)action);

  return new InfosetPortion(infoset);
}

//----------------------
// DeleteEmptyInfoset
//----------------------

static Portion *GSM_DeleteEmptyInfoset(Portion **param)
{
  Infoset *infoset = ((InfosetPortion *) param[0])->Value();

  _gsm->UnAssignGameElement(infoset->Game(), true,
			    porBEHAV | porEFSUPPORT);
  _gsm->UnAssignEfgElement(infoset->Game(), porINFOSET, infoset);

  return new BoolPortion(infoset->Game()->DeleteEmptyInfoset(infoset));
}

//----------------
// DeleteMove
//----------------

static Portion *GSM_DeleteMove(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  Node *keep = ((NodePortion *) param[1])->Value();

  if (keep->GetParent() != n)
    throw gclRuntimeError("keep is not a child of node");

  _gsm->UnAssignGameElement(n->Game(), true, porBEHAV | porEFSUPPORT);
  for (int i = 1; i <= n->NumChildren(); i++) 
    if (n->GetChild(i) != keep)
      _gsm->UnAssignEfgSubTree(n->Game(), n->GetChild(i));
  _gsm->UnAssignEfgElement(n->Game(), porNODE, n);

  return new NodePortion(n->Game()->DeleteNode(n, keep));
}

//-----------------
// DeleteOutcome
//-----------------

static Portion *GSM_DeleteOutcome(Portion **param)
{
  EFOutcome *outc = ((EfOutcomePortion *) param[0])->Value();

  gList<Node *> nodes;
  Nodes(*outc->BelongsTo(), nodes);
  for (int i = 1; i <= nodes.Length(); )
    if (nodes[i]->GetOutcome() != outc) 
      nodes.Remove(i);
    else
      i++;
    
  _gsm->InvalidateGameProfile(outc->BelongsTo(), true);
  _gsm->UnAssignEfgElement(outc->BelongsTo(), porEFOUTCOME, outc);

  outc->BelongsTo()->DeleteOutcome(outc);

  Portion *por = ArrayToList(nodes);
  return por;
}

//----------------
// DeleteTree
//----------------

static Portion *GSM_DeleteTree(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  _gsm->UnAssignGameElement(n->Game(), true, porBEHAV | porEFSUPPORT);
  _gsm->UnAssignEfgSubTree(n->Game(), n);

  n->Game()->DeleteTree(n);

  return new NodePortion(n);
}


extern EFSupport *DominanceTruncatedSupport(const EFSupport &S, 
					    const bool strong, 
					    const bool conditional,
					          gOutput &tracefile,
					          gStatus &status);

//--------------
// ElimDom
//--------------

static Portion *GSM_ElimDom_Efg(Portion **param)
{
  EFSupport *S = ((EfSupportPortion *) param[0])->Value();
  bool strong = ((BoolPortion *) param[1])->Value();
  bool mixed = ((BoolPortion *) param[2])->Value();

  if (mixed)
    throw gclRuntimeError("Elimination by mixed strategies not implemented");

  gWatch watch;

  // The following sets conditional = false.  Could be more general.

  EFSupport *T = DominanceTruncatedSupport(*S, strong, false,
				  ((OutputPortion *) param[4])->Value(),
				  gstatus);

  ((NumberPortion *) param[3])->SetValue(watch.Elapsed());
  
  Portion *por = (T) ? new EfSupportPortion(T) : new EfSupportPortion(new EFSupport(*S));

  return por;
}


//----------
// Game
//----------

Portion* GSM_Game_EfgElements(Portion** param)
{
  if (param[0]->Game())
    return new EfgPortion((Efg*) param[0]->Game());
  else
    return 0;
}


//-------------
// Infoset
//-------------

static Portion *GSM_Infoset_Node(Portion **param)
{
  if( param[0]->Spec().Type == porNULL )
    return new NullPortion( porINFOSET );

  Node *n = ((NodePortion *) param[0])->Value();

  if (!n->GetInfoset())
    return new NullPortion(porINFOSET);

  return new InfosetPortion(n->GetInfoset());
}


static Portion *GSM_Infoset_Action(Portion **param)
{
  if( param[0]->Spec().Type == porNULL )
  {
    return new NullPortion( porINFOSET );
  }

  const Action *a = ((ActionPortion *) param[0])->Value();

  if (!a->BelongsTo())
    return new NullPortion(porINFOSET);

  return new InfosetPortion(a->BelongsTo());
}

//-------------
// Infosets
//-------------

static Portion *GSM_Infosets(Portion **param)
{
  EFPlayer *p = ((EfPlayerPortion *) param[0])->Value();

  return ArrayToList(p->Infosets());
}

//----------------
// InsertAction
//----------------

static Portion *GSM_InsertAction(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();

  _gsm->UnAssignGameElement(s->Game(), true, porBEHAV | porEFSUPPORT);

  return new ActionPortion((Action *)s->Game()->InsertAction(s));
}

static Portion *GSM_InsertActionAt(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  const Action *a = ((ActionPortion *) param[1])->Value();

  _gsm->UnAssignGameElement(s->Game(), true, porBEHAV | porEFSUPPORT);

  return new ActionPortion((Action *)s->Game()->InsertAction(s, a));
}

//--------------
// InsertMove
//--------------

static Portion *GSM_InsertMove(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  Node *n = ((NodePortion *) param[1])->Value();

  n->Game()->InsertNode(n, s);

  _gsm->UnAssignGameElement(s->Game(), true, porBEHAV | porEFSUPPORT);

  return new NodePortion(n->GetParent());
}

//---------------
// IsConstSum
//---------------

static Portion *GSM_IsConstSum(Portion **param)
{
  Efg &E = *((EfgPortion*) param[0])->Value();
  return new BoolPortion(E.IsConstSum());
}

//---------------
// IsConsistent
//---------------

static Portion *GSM_IsBasisConsistent(Portion **param)
{
  EFBasis *basis = ((EfBasisPortion *) param[0])->Value();
  return new BoolPortion(basis->IsConsistent());
}

//----------------
// IsPredecessor
//----------------

static Portion *GSM_IsPredecessor(Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();
  return new BoolPortion(n1->Game()->IsPredecessor(n1, n2));
}

//---------------
// IsSuccessor
//---------------

static Portion *GSM_IsSuccessor(Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();
  return new BoolPortion(n1->Game()->IsSuccessor(n1, n2));
}


//-----------
// LoadEfg
//-----------

static Portion *GSM_LoadEfg(Portion **param)
{
  gText file = ((TextPortion *) param[0])->Value();
  
  try  {
    gFileInput f(file);
    Efg *E = 0;
    ReadEfgFile((gInput &) f, E);
    
    if (E)
      return new EfgPortion(E);
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

static Portion *GSM_IsPerfectRecall(Portion **param)
{
  Infoset *s1, *s2;
  Efg &efg = * ((EfgPortion *) param[0])->Value();
  return new BoolPortion(IsPerfectRecall(efg, s1, s2));
}

//-----------------
// MarkSubgame
//-----------------

static Portion *GSM_MarkSubgame(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  return new BoolPortion(n->Game()->DefineSubgame(n));
}

//------------------
// MarkedSubgame
//------------------

static Portion *GSM_MarkedSubgame(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  return new BoolPortion(n->GetSubgameRoot() == n);
}

//------------
// Members
//------------

static Portion *GSM_Members(Portion **param)
{
  if (param[0]->Spec().Type == porNULL)
    return new ListPortion();

  Infoset *s = ((InfosetPortion *) param[0])->Value();

  Portion* por = ArrayToList(s->Members());
  return por;
}

//----------------
// MergeInfosets
//----------------

static Portion *GSM_MergeInfosets(Portion **param)
{
  Infoset *s1 = ((InfosetPortion *) param[0])->Value();
  Infoset *s2 = ((InfosetPortion *) param[1])->Value();
  
  s1->Game()->MergeInfoset(s1, s2);
  
  _gsm->UnAssignGameElement(s1->Game(), true, porBEHAV | porEFSUPPORT);

  return new InfosetPortion(s1);
}

//----------------
// MoveToInfoset
//----------------

static Portion *GSM_MoveToInfoset(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  Infoset *s = ((InfosetPortion *) param[1])->Value();
  
  s->Game()->JoinInfoset(s, n);
  

  _gsm->UnAssignGameElement(s->Game(), true, porBEHAV | porEFSUPPORT);

  return new InfosetPortion(s);
}

//-------------
// MoveTree
//-------------

static Portion *GSM_MoveTree(Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();

  _gsm->UnAssignGameElement(n1->Game(), true, porBEHAV | porEFSUPPORT);

  return new NodePortion(n1->Game()->MoveTree(n1, n2));
}

//----------
// Name
//----------

static Portion *GSM_Name_EfgElements(Portion **param)
{
  if (param[0]->Spec().Type == porNULL)
    return new TextPortion("");

  switch (param[0]->Spec().Type)  {
    case porACTION:
      return new TextPortion(((ActionPortion *) param[0])->Value()->
				GetName());
    case porINFOSET:
      return new TextPortion(((InfosetPortion *) param[0])->Value()->
				GetName());
    case porNODE:
      return new TextPortion(((NodePortion *) param[0])->Value()->
				GetName());
    case porEFOUTCOME:
      return new TextPortion(((EfOutcomePortion *) param[0])->Value()->
				GetName());
    case porEFPLAYER:
      return new TextPortion(((EfPlayerPortion *) param[0])->Value()->
				GetName());
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

static Portion *GSM_NewEfg(Portion **param)
{
  Efg *E = new Efg;
  ListPortion *players = (ListPortion *) param[0];
  for (int i = 1; i <= players->Length(); i++)
    E->NewPlayer()->SetName(((TextPortion *) (*players)[i])->Value());
  return new EfgPortion(E);
}


//--------------
// NewInfoset
//--------------

static Portion *GSM_NewInfoset(Portion **param)
{
  EFPlayer *p = ((EfPlayerPortion *) param[0])->Value();
  int n = ((NumberPortion *) param[1])->Value();

  if (n <= 0)
    throw gclRuntimeError("Information sets must have at least one action");

  Infoset *s = p->Game()->CreateInfoset(p, n);

  _gsm->UnAssignGameElement(p->Game(), true, porBEHAV | porEFSUPPORT);

  return new InfosetPortion(s);
}
 
//--------------
// NewOutcome
//--------------

static Portion *GSM_NewOutcome(Portion **param)
{
  Efg *efg = ((EfgPortion *) param[0])->Value();
  EFOutcome *c = efg->NewOutcome();

  return new EfOutcomePortion(c);
}

//---------------
// NewPlayer
//---------------

static Portion *GSM_NewPlayer(Portion **param)
{
  Efg &E = *((EfgPortion*) param[0])->Value();
  EFPlayer *p = E.NewPlayer();

  _gsm->UnAssignGameElement(&E, true, porBEHAV | porEFSUPPORT);

  return new EfPlayerPortion(p);
}

//----------------
// NextSibling
//----------------

static Portion *GSM_NextSibling(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value()->NextSibling();
  if (!n)
    return new NullPortion(porNODE);
  
  return new NodePortion(n);
}

//----------
// Nodes
//----------

static Portion *GSM_Nodes(Portion **param)
{
  Efg *efg = ((EfgPortion *) param[0])->Value();

  gList<Node *> nodes;
  Nodes(*efg, nodes);

  Portion *por = ArrayToList(nodes);
  return por;
}

//---------------
// NthChild
//---------------

static Portion *GSM_NthChild(Portion **param)
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

static Portion *GSM_Outcome(Portion **param)
{
  if( param[0]->Spec().Type == porNULL )
  {
    return new NullPortion(porEFOUTCOME);
  }

  Node *n = ((NodePortion *) param[0])->Value();
  if (!n->GetOutcome())
    return new NullPortion(porEFOUTCOME);

  return new EfOutcomePortion(n->GetOutcome());
}

//------------
// Outcomes
//------------

static Portion *GSM_Outcomes(Portion **param)
{
  Efg *E = ((EfgPortion*) param[0])->Value();
  
  Portion* por = ArrayToList(E->Outcomes());
  return por;
}

//------------
// Parent
//------------

static Portion *GSM_Parent(Portion **param)
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

static Portion *GSM_Payoff(Portion **param)
{
  if (param[0]->Spec().Type == porNULL)
    return new NumberPortion(0);
  EFOutcome *c = ((EfOutcomePortion *) param[0])->Value();
  EFPlayer *player = ((EfPlayerPortion *) param[1])->Value();
  Efg *efg = player->Game();

  return new NumberPortion(efg->Payoff(c, player->GetNumber()));
}

//----------
// Player
//----------

static Portion *GSM_Player(Portion **param)
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

static Portion *GSM_Players(Portion **param)
{
  Efg &E = *((EfgPortion*) param[0])->Value();

  return ArrayToList(E.Players());
}

//--------------
// PriorAction
//--------------

static Portion *GSM_PriorAction(Portion** param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  const Action* a = LastAction(n);
  if(a == 0)
    return new NullPortion(porACTION);

  return new ActionPortion((Action *)a);
}

//----------------
// PriorSibling
//----------------

static Portion *GSM_PriorSibling(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value()->PriorSibling();
  if (!n)
    return new NullPortion(porNODE);
  
  return new NodePortion(n);
}

//-----------------
// RemoveAction
//-----------------

static Portion *GSM_RemoveAction(Portion **param)
{  
  EFSupport *support = ((EfSupportPortion *) param[0])->Value();
  Action *action = ((ActionPortion *) param[1])->Value();

  EFSupport *S = new EFSupport(*support);
  S->RemoveAction(action);

  return new EfSupportPortion(S);
}

static Portion *GSM_RemoveBasisAction(Portion **param)
{  
  EFBasis *support = ((EfBasisPortion *) param[0])->Value();
  Action *action = ((ActionPortion *) param[1])->Value();

  EFBasis *S = new EFBasis(*support);
  S->RemoveAction(action);

  return new EfBasisPortion(S);
}


//-----------------
// RemoveNode
//-----------------

static Portion *GSM_RemoveBasisNode(Portion **param)
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

static Portion *GSM_Reveal(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  ListPortion *players = (ListPortion *) param[1];

  gBlock<EFPlayer *> player(players->Length());
  for (int i = 1; i <= players->Length(); i++)
    player[i] = ((EfPlayerPortion *) (*players)[i])->Value();
  
  s->Game()->Reveal(s, player);

  _gsm->UnAssignGameElement(s->Game(), true, porBEHAV | porEFSUPPORT);

  return new InfosetPortion(s);
}

//-------------
// RootNode
//-------------

static Portion *GSM_RootNode(Portion **param)
{
  Efg &E = *((EfgPortion*) param[0])->Value();
  return new NodePortion(E.RootNode());
}

//-------------
// SaveEfg
//-------------

static Portion *GSM_SaveEfg(Portion **param)
{
  Efg* E = ((EfgPortion *) param[0])->Value();
  gText text = ((TextPortion *) param[1])->Value();

  try { 
    gFileOutput f(text);
    E->WriteEfgFile(f);
  }
  catch (gFileOutput::OpenFailed &)  {
    throw gclRuntimeError("Cannot open file " + text + " for writing");
  }

  return param[0]->ValCopy();
}

//-------------------
// SetChanceProbs
//-------------------

static Portion *GSM_SetChanceProbs(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  ListPortion *p = (ListPortion *) param[1];
  Efg *efg = s->Game();

  if (!s->GetPlayer()->IsChance())
    throw gclRuntimeError("Information set does not belong to chance player");
  
  for (int i = 1; i <= p->Length(); i++)
    efg->SetChanceProb(s, i, ((NumberPortion *) (*p)[i])->Value());
  _gsm->InvalidateGameProfile(s->Game(), true);

  return new InfosetPortion(s);
}

//-------------
// SetComment
//-------------

static Portion *GSM_SetComment(Portion **param)
{
  Efg *efg = ((EfgPortion *) param[0])->Value();
  gText comment = ((TextPortion *) param[1])->Value();
  efg->SetComment(comment);
  return param[0]->ValCopy();
}

//--------------
// SetName
//--------------

static Portion *GSM_SetName(Portion **param)
{
  gText name(((TextPortion *) param[1])->Value());
  
  switch (param[0]->Spec().Type)   {
    case porACTION:
      ((Action *)((ActionPortion *) param[0])->Value())->SetName(name);
      break;
    case porINFOSET:
      ((InfosetPortion *) param[0])->Value()->SetName(name);
      break;
    case porNODE:
      ((NodePortion *) param[0])->Value()->SetName(name);
      break;
    case porEFOUTCOME:
      ((EfOutcomePortion *) param[0])->Value()->SetName(name);
      break;
    case porEFPLAYER:
      ((EfPlayerPortion *) param[0])->Value()->SetName(name);
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

static Portion *GSM_SetOutcome(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  EFOutcome *c = ((EfOutcomePortion *) param[1])->Value();

  n->SetOutcome(c);

  _gsm->InvalidateGameProfile(n->Game(), true);
  
  if (c)  {
    Portion* por = new EfOutcomePortion(c);
    return por;
  }
  else 
    return new NullPortion(porEFOUTCOME);
}

//----------------
// SetPayoff
//----------------

static Portion *GSM_SetPayoff(Portion **param)
{
  EFOutcome *c = ((EfOutcomePortion *) param[0])->Value();
  EFPlayer *player = ((EfPlayerPortion *) param[1])->Value();
  Efg *efg = player->Game();
  gNumber value = ((NumberPortion *) param[2])->Value();

  efg->SetPayoff(c, player->GetNumber(), value);

  _gsm->InvalidateGameProfile(c->BelongsTo(), true);

  return param[0]->ValCopy();
}


//----------------
// Subgames
//----------------

static Portion *GSM_Subgames(Portion **param)
{
  Efg& E = *((EfgPortion*) param[0])->Value();
  gList<Node *> list;
  LegalSubgameRoots(E, list);

  Portion *por = ArrayToList(list);
  return por;  
}  

//--------------
// Support
//--------------

static Portion *GSM_Support(Portion **param)
{
  Efg &E = * ((EfgPortion *) param[0])->Value();
  Portion *por = new EfSupportPortion(new EFSupport(E));
  return por;
}

//--------------
// EFBasis
//--------------

static Portion *GSM_Basis(Portion **param)
{
  Efg &E = * ((EfgPortion *) param[0])->Value();
  Portion *por = new EfBasisPortion(new EFBasis(E));
  return por;
}

//-----------------
// UnmarkSubgame
//-----------------

static Portion *GSM_UnmarkSubgame(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  n->Game()->RemoveSubgame(n);
  
  return new NodePortion(n);
}



void Init_efgfunc(GSM *gsm)
{
  gclFunction *FuncObj;

  static struct { char *sig; Portion *(*func)(Portion **); } ftable[] =
    { { "Actions[infoset->INFOSET*, support->EFSUPPORT] =: LIST(ACTION)",
 	GSM_Actions },
      { "Actions[infoset->INFOSET*, basis->EFBASIS] =: LIST(ACTION)",
 	GSM_BasisActions },
      { "ActionNumber[action->ACTION, sup->EFSUPPORT] =: NUMBER", 
	GSM_ActionNumber },
      { "ActionNumber[action->ACTION, basis->EFBASIS] =: NUMBER", 
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
      { "MergeInfosets[infoset1->INFOSET, infoset2->INFOSET] =: INFOSET",
	GSM_MergeInfosets },
      { "MoveToInfoset[node->NODE, infoset->INFOSET] =: NODE",
	GSM_MoveToInfoset },
      { "MoveTree[from->NODE, to->NODE] =: NODE", GSM_MoveTree },
      { "Name[x->ACTION*] =: TEXT", GSM_Name_EfgElements },
      { "Name[x->INFOSET*] =: TEXT", GSM_Name_EfgElements },
      { "Name[x->NODE*] =: TEXT", GSM_Name_EfgElements },
      { "Name[x->EFOUTCOME*] =: TEXT", GSM_Name_EfgElements },
      { "Name[x->EFPLAYER*] =: TEXT", GSM_Name_EfgElements },
      { "Name[x->EFG*] =: TEXT", GSM_Name_EfgElements },
      { "NewInfoset[player->EFPLAYER, actions->NUMBER] =: INFOSET",
	GSM_NewInfoset },
      { "NewOutcome[efg->EFG] =: EFOUTCOME", GSM_NewOutcome },
      { "NewPlayer[efg->EFG] =: EFPLAYER", GSM_NewPlayer },
      { "NextSibling[node->NODE] =: NODE", GSM_NextSibling },
      { "Nodes[efg->EFG] =: LIST(NODE)", GSM_Nodes },
      { "Nodes[infoset->INFOSET*, basis->EFBASIS] =: LIST(NODE)",
 	GSM_BasisNodes },
      { "NodeNumber[node->NODE, basis->EFBASIS] =: NUMBER", 
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
      { "SetName[x->EFG, name->TEXT] =: EFG", GSM_SetName },
      { "SetOutcome[node->NODE, outcome->EFOUTCOME*] =: EFOUTCOME",
	GSM_SetOutcome },
      { "SetPayoff[outcome->EFOUTCOME, player->EFPLAYER, payoff->NUMBER] =: EFOUTCOME", GSM_SetPayoff },
      { "Subgames[efg->EFG] =: LIST(NODE)", GSM_Subgames },
      { "Support[efg->EFG] =: EFSUPPORT", GSM_Support },
      { "Basis[efg->EFG] =: EFBASIS", GSM_Basis },
      { "UnmarkSubgame[node->NODE] =: NODE", GSM_UnmarkSubgame },
      { 0, 0 }
    };


  for (int i = 0; ftable[i].sig != 0; i++) 
    gsm->AddFunction(new gclFunction(ftable[i].sig, ftable[i].func,
				     funcLISTABLE | funcGAMEMATCH));

  FuncObj = new gclFunction("ElimDom", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_ElimDom_Efg, porEFSUPPORT, 6));
  FuncObj->SetParamInfo(0, 0, gclParameter("support", porEFSUPPORT));
  FuncObj->SetParamInfo(0, 1, gclParameter("strong", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(0, 2, gclParameter("mixed", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(0, 3, gclParameter("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 4, gclParameter("traceFile", porOUTPUT,
					    new OutputPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(0, 5, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction("NewEfg", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_NewEfg, porEFG, 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("players", PortionSpec(porTEXT,1),
					    new ListPortion));
  gsm->AddFunction(FuncObj);
}



