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

//-------------
// Actions
//-------------

static Portion *GSM_Actions(Portion **param)
{
  if( param[0]->Spec().Type == porNULL )
    return new ListValPortion;

  Infoset *s = ((InfosetPortion *) param[0])->Value();
  EFSupport* sup = ((EfSupportPortion*) param[1])->Value();

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

  support->AddAction(action);

  return param[0]->RefCopy();
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
  Action *action = ((ActionPortion *) param[0])->Value();
  Infoset *infoset = action->BelongsTo();
  if (!infoset->GetPlayer()->IsChance()) 
    return new ErrorPortion("Action must belong to the chance player");
  Efg *efg = infoset->Game();

  return new NumberPortion(efg->GetChanceProb(infoset, action->GetNumber()));
}

//---------------
// Children
//---------------

static Portion *GSM_Children(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  int child;
  Portion* por = new ListValPortion();
  for(child=1; child <= n->NumChildren(); child++)
    ((ListPortion*) por)->Append(new NodePortion(n->GetChild(child)));
  return por;
}

//---------------
// CompressEfg
//---------------

Efg *CompressEfg(const Efg &, const EFSupport &);

static Portion *GSM_CompressEfg(Portion **param)
{
  EFSupport *S = ((EfSupportPortion *) param[0])->Value();

  return new EfgValPortion(CompressEfg(S->Game(), *S));
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
  Action *action = ((ActionPortion *) param[0])->Value();
  Infoset *infoset = action->BelongsTo();

  if (infoset->NumActions() == 1)
    return new ErrorPortion("Cannot delete the only action at an infoset.");

  infoset->Game()->DeleteAction(infoset, action);

  _gsm->UnAssignGameElement(infoset->Game(), true,
			    porBEHAV | porEFSUPPORT);
  _gsm->UnAssignEfgElement(infoset->Game(), porACTION, action);

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
    return new ErrorPortion("keep is not a child of node");

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


extern EFSupport *ComputeDominated(EFSupport &S, bool strong, 
				   const gArray<int> &players,
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
    return new ErrorPortion("Elimination by mixed strategies not implemented");

  gWatch watch;
  gBlock<int> players(S->Game().NumPlayers());
  int i;
  for (i = 1; i <= players.Length(); i++)   players[i] = i;

  EFSupport *T = ComputeDominated(*S, strong, players,
				  ((OutputPortion *) param[4])->Value(),
				  gstatus);

  ((NumberPortion *) param[3])->Value() = watch.Elapsed();
  
  Portion *por = (T) ? new EfSupportPortion(T) : new EfSupportPortion(new EFSupport(*S));

  return por;
}


//----------
// Game
//----------

Portion* GSM_Game_EfgElements(Portion** param)
{
  if (param[0]->Game())  {
    assert(param[0]->GameIsEfg());
    return new EfgValPortion((Efg*) param[0]->Game());
  }
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

  Action *a = ((ActionPortion *) param[0])->Value();

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

  return new ActionPortion(s->Game()->InsertAction(s));
}

static Portion *GSM_InsertActionAt(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  Action *a = ((ActionPortion *) param[1])->Value();

  _gsm->UnAssignGameElement(s->Game(), true, porBEHAV | porEFSUPPORT);

  return new ActionPortion(s->Game()->InsertAction(s, a));
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

static Portion *GSM_IsConstSumEfg(Portion **param)
{
  Efg &E = *((EfgPortion*) param[0])->Value();
  return new BoolPortion(E.IsConstSum());
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
  gString file = ((TextPortion *) param[0])->Value();
  
  gFileInput f(file);

  if (f.IsValid())   {
    Efg *E = 0;
    ReadEfgFile((gInput &) f, E);
	
    if (E)
      return new EfgValPortion(E);
    else
      return new ErrorPortion("Not a valid .efg file");
  }
  else
    return new ErrorPortion("Unable to open file for reading");
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
    return new ListValPortion();

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
  if( param[0]->Spec().Type == porNULL )
  {
    return new TextPortion( "" );
  }

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
      assert(0);
      return 0;
  }
}


//------------
// NewEfg
//------------

static Portion *GSM_NewEfg(Portion **param)
{
  gSpace *space = new gSpace;
  ORD_PTR ord = &lex;
  Efg *E = new Efg(space, new term_order(space, ord));
  ListPortion *players = (ListPortion *) param[0];
  for (int i = 1; i <= players->Length(); i++)
    E->NewPlayer()->SetName(((TextPortion *) (*players)[i])->Value());
  return new EfgValPortion(E);
}


//--------------
// NewInfoset
//--------------

static Portion *GSM_NewInfoset(Portion **param)
{
  EFPlayer *p = ((EfPlayerPortion *) param[0])->Value();
  int n = ((IntPortion *) param[1])->Value();

  if (n <= 0)
    return new ErrorPortion("Information sets must have at least one action");

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
  int child = ((IntPortion *) param[1])->Value();
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
  EFOutcome *c = ((EfOutcomePortion *) param[0])->Value();
  EFPlayer *player = ((EfPlayerPortion *) param[1])->Value();
  Efg *efg = player->Game();

  gArray<gNumber> values(efg->Parameters()->Dmnsn());
  for (int i = 1; i <= values.Length(); values[i++] = gNumber(0));
  return new NumberPortion(efg->Payoff(c, player->GetNumber()).Evaluate(values));
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

static Portion *GSM_Players_Efg(Portion **param)
{
  Efg &E = *((EfgPortion*) param[0])->Value();

  Portion* por = ArrayToList(E.Players());
  return por;
}

//--------------
// PriorAction
//--------------

static Portion *GSM_PriorAction(Portion** param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  Action* a = LastAction(n);
  if(a == 0)
    return new NullPortion(porACTION);

  return new ActionPortion(a);
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

//-------------
// RandomEfg
//-------------

static Portion *GSM_RandomEfg(Portion **param)
{
  Efg &E = * ((EfgPortion *) param[0])->Value();
  
  RandomEfg(E);
  return param[0]->ValCopy();
}

static Portion *GSM_RandomEfg_Seed(Portion **param)
{
  Efg &E = * ((EfgPortion *) param[0])->Value();
  int seed = ((IntPortion *) param[1])->Value();

  SetSeed(seed);
  RandomEfg(E);
  return param[0]->ValCopy();
}



//-----------------
// RemoveAction
//-----------------

static Portion *GSM_RemoveAction(Portion **param)
{  
  EFSupport *support = ((EfSupportPortion *) param[0])->Value();
  Action *action = ((ActionPortion *) param[1])->Value();

  support->RemoveAction(action);

  return param[0]->RefCopy();
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
  gString text = ((TextPortion *) param[1])->Value();
  gFileOutput f(text);

  if (!f.IsValid())
    return new ErrorPortion("Cannot open file for writing");

  E->WriteEfgFile(f);

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
    return new ErrorPortion
      ("Information set does not belong to chance player");
  
  for (int i = 1; i <= p->Length(); i++)
    efg->SetChanceProb(s, i, ((NumberPortion *) (*p)[i])->Value());
  _gsm->InvalidateGameProfile(s->Game(), true);

  return new InfosetPortion(s);
}

//--------------
// SetName
//--------------

static Portion *GSM_SetName_EfgElements(Portion **param)
{
  gString name(((TextPortion *) param[1])->Value());
  
  switch (param[0]->Spec().Type)   {
    case porACTION:
      ((ActionPortion *) param[0])->Value()->SetName(name);
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
      assert(0);
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

  efg->SetPayoff(c, player->GetNumber(),
                 gPoly<gNumber>(efg->Parameters(), value, efg->ParamOrder()));

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

static Portion *GSM_Support_Efg(Portion **param)
{
  Efg &E = * ((EfgPortion *) param[0])->Value();
  Portion *por = new EfSupportPortion(new EFSupport(E));
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
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("Actions", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Actions, 
				       PortionSpec(porACTION, 1), 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("infoset", 
					    PortionSpec(porINFOSET, 0, 
							porNULLSPEC )));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("support", porEFSUPPORT)); 
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("AddAction", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_AddAction, 
				       porEFSUPPORT, 2, 
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porEFSUPPORT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("action", porACTION));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("AddMove", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_AddMove, porNODE, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("infoset", porINFOSET));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Chance", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Chance, porEFPLAYER, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("ChanceProb", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ChanceProb, porNUMBER, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("action", porACTION));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Children", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Children, 
				       PortionSpec(porNODE, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("CompressEfg", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_CompressEfg,
				       porEFG, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porEFSUPPORT));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("CopyTree", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_CopyTree, porNODE, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("from", porNODE));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("to", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("DeleteAction", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_DeleteAction, porINFOSET, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("action", porACTION));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("DeleteEmptyInfoset", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_DeleteEmptyInfoset, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("infoset", porINFOSET));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("DeleteMove", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_DeleteMove, porNODE, 2, 
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("keep", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("DeleteOutcome", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_DeleteOutcome,
				       PortionSpec(porNODE, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("outcome", porEFOUTCOME));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("DeleteTree", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_DeleteTree, porNODE, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("ElimDom", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ElimDom_Efg, 
				       porEFSUPPORT, 6));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porEFSUPPORT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("strong", porBOOL,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("mixed", porBOOL,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(0, 3, ParamInfoType("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 4, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(0, 5, ParamInfoType("traceLevel", porINTEGER,
					    new IntPortion(0)));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Game", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Game_EfgElements, porEFG, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("player", porEFPLAYER));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Game_EfgElements, porEFG, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("node", porNODE));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Game_EfgElements, porEFG, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("outcome", porEFOUTCOME));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Game_EfgElements, porEFG, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("infoset", porINFOSET));
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("Infoset", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Infoset_Node, porINFOSET, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", 
			      PortionSpec(porNODE, 0, porNULLSPEC)));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Infoset_Action, porINFOSET, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("action", 
			      PortionSpec(porACTION, 0, porNULLSPEC)));
  gsm->AddFunction(FuncObj);
  
  
  FuncObj = new FuncDescObj("Infosets", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Infosets, 
				       PortionSpec(porINFOSET, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("player", porEFPLAYER));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("InsertAction", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_InsertAction, porACTION, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("infoset", porINFOSET));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_InsertActionAt, porACTION, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("infoset", porINFOSET));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("at", porACTION));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("InsertMove", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_InsertMove, porNODE, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("infoset", porINFOSET));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsConstSum", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsConstSumEfg, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG));
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("IsPerfectRecall", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsPerfectRecall, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsPredecessor", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsPredecessor, porBOOL, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("of", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsSuccessor", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsSuccessor, porBOOL, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("from", porNODE));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LoadEfg", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_LoadEfg, porEFG, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("file", porTEXT));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("MarkSubgame", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_MarkSubgame, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("MarkedSubgame", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_MarkedSubgame, porBOOL, 1)); 
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Members", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Members, 
				       PortionSpec(porNODE, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("infoset", 
                              PortionSpec(porINFOSET, 0, porNULLSPEC) ));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("MergeInfosets", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_MergeInfosets, porINFOSET, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("infoset1", porINFOSET));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("infoset2", porINFOSET));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("MoveToInfoset", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_MoveToInfoset, porNODE, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("infoset", porINFOSET));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("MoveTree", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_MoveTree, porNODE, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("from", porNODE));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("to", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Name", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Name_EfgElements, porTEXT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", 
                              PortionSpec(porACTION | porINFOSET | 
					  porNODE | porEFOUTCOME | 
					  porEFPLAYER | porEFG, 
                                          0, 
                                          porNULLSPEC) ));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NewEfg", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NewEfg, porEFG, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("players", PortionSpec(porTEXT,1),
					    new ListValPortion));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NewInfoset", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NewInfoset, porINFOSET, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("player", porEFPLAYER));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("actions", porINTEGER));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NewOutcome", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NewOutcome, porEFOUTCOME, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NewPlayer", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NewPlayer, porEFPLAYER, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NextSibling", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NextSibling, porNODE, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);

  
  FuncObj = new FuncDescObj("Nodes", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Nodes,
				       PortionSpec(porNODE, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NthChild", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NthChild, porNODE, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("n", porINTEGER));
  gsm->AddFunction(FuncObj);
  
  
  FuncObj = new FuncDescObj("Outcome", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Outcome, porEFOUTCOME, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", 
                              PortionSpec(porNODE, 0, porNULLSPEC) ));
  gsm->AddFunction(FuncObj);
  
  
  FuncObj = new FuncDescObj("Outcomes", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Outcomes, 
				       PortionSpec(porEFOUTCOME, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG));
  gsm->AddFunction(FuncObj);
  
  
  FuncObj = new FuncDescObj("Parent", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Parent, porNODE, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", 
                              PortionSpec(porNODE, 0, porNULLSPEC) ));
  gsm->AddFunction(FuncObj);
  
  
  FuncObj = new FuncDescObj("Payoff", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Payoff,
				       porNUMBER, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("outcome", porEFOUTCOME));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("player", porEFPLAYER));
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("Player", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Player, porEFPLAYER, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("infoset", 
			      PortionSpec(porINFOSET, 0, porNULLSPEC) ));
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("Players", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Players_Efg, 
				       PortionSpec(porEFPLAYER, 1), 1));  
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG));
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("PriorAction", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_PriorAction, porACTION, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("PriorSibling", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_PriorSibling, porNODE, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Randomize", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_RandomEfg, porEFG, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porEFG));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_RandomEfg_Seed,
				       porEFG, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porEFG));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("seed", porINTEGER));

  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("RemoveAction", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_RemoveAction, porEFSUPPORT, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porEFSUPPORT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("action", porACTION));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Reveal", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Reveal, porINFOSET, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("infoset", porINFOSET));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("who", 
					    PortionSpec(porEFPLAYER,1)));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RootNode", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_RootNode, porNODE, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG));
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("SaveEfg", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SaveEfg, porEFG, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("file", porTEXT));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetChanceProbs", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetChanceProbs, porINFOSET, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("infoset", porINFOSET));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("probs",
					    PortionSpec(porNUMBER, 1)));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetName", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetName_EfgElements, 
				       porACTION | porINFOSET | porNODE | 
				       porEFOUTCOME | porEFPLAYER |
				       porEFG, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porACTION | porINFOSET | 
					    porNODE | porEFOUTCOME | 
					    porEFPLAYER | porEFG));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("name", porTEXT));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetOutcome", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetOutcome, porEFOUTCOME, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("outcome", 
			      PortionSpec(porEFOUTCOME, 0, porNULLSPEC) ));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("SetPayoff", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetPayoff, porEFOUTCOME, 3,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("outcome", porEFOUTCOME));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("player", porEFPLAYER));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("payoff", porNUMBER));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Subgames", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Subgames,
				       PortionSpec(porNODE, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Support", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Support_Efg, porEFSUPPORT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("UnmarkSubgame", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_UnmarkSubgame, porNODE, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);
}



