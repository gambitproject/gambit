//
// File: efgfunc.cc -- Extensive form editing builtins
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
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
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

  n->BelongsTo()->AppendNode(n, s);

  _gsm->UnAssignGameElement(n->BelongsTo(), true, porBEHAV | porEFSUPPORT);

  Portion* por = new NodeValPortion(n->GetChild(1));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//--------------
// Chance
//--------------

static Portion *GSM_Chance(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();

  Portion* por = new EfPlayerValPortion(E.GetChance());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------
// ChanceProb
//----------------

static Portion *GSM_ChanceProb(Portion **param)
{
  Portion* por;
  Action *action = ((ActionPortion *) param[0])->Value();
  Infoset *infoset = action->BelongsTo();
  if (!infoset->GetPlayer()->IsChance()) 
    return new ErrorPortion("Action must belong to the chance player");

  switch (infoset->BelongsTo()->Type())   {
    case DOUBLE:
      por = new FloatValPortion(((ChanceInfoset<double> *) infoset)->GetActionProb(action->GetNumber()));
      break;
    case RATIONAL:
      por = new RationalValPortion(((ChanceInfoset<gRational> *) infoset)->GetActionProb(action->GetNumber()));
      break;
    default:
      assert(0);
      return 0;
  }

  return por;
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
    ((ListPortion*) por)->Append(new NodeValPortion(n->GetChild(child)));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//---------------
// CompressEfg
//---------------

template <class T> Efg<T> *CompressEfg(const Efg<T> &, const EFSupport &);

static Portion *GSM_CompressEfg(Portion **param)
{
  EFSupport *S = ((EfSupportPortion *) param[0])->Value();
  const BaseEfg &E = S->BelongsTo();
  
  if (E.Type() == DOUBLE)
    return new EfgValPortion(CompressEfg((const Efg<double> &) E, *S));
  else
    return new EfgValPortion(CompressEfg((const Efg<gRational> &) E, *S));
}

//---------------
// CopyTree
//---------------

static Portion *GSM_CopyTree(Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();

  _gsm->UnAssignGameElement(n1->BelongsTo(), true, porBEHAV | porEFSUPPORT);

  Portion* por = new NodeValPortion(n1->BelongsTo()->CopyTree(n1, n2));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
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

  infoset->BelongsTo()->DeleteAction(infoset, action);

  _gsm->UnAssignGameElement(infoset->BelongsTo(), true,
			    porBEHAV | porEFSUPPORT);
  _gsm->UnAssignEfgElement(infoset->BelongsTo(), porACTION, action);

  Portion* por = new InfosetValPortion(infoset);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------------
// DeleteEmptyInfoset
//----------------------

static Portion *GSM_DeleteEmptyInfoset(Portion **param)
{
  Infoset *infoset = ((InfosetPortion *) param[0])->Value();

  _gsm->UnAssignGameElement(infoset->BelongsTo(), true,
			    porBEHAV | porEFSUPPORT);
  _gsm->UnAssignEfgElement(infoset->BelongsTo(), porINFOSET, infoset);

  return new BoolValPortion(infoset->BelongsTo()->DeleteEmptyInfoset(infoset));
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

  _gsm->UnAssignGameElement(n->BelongsTo(), true, porBEHAV | porEFSUPPORT);
  for (int i = 1; i <= n->NumChildren(); i++) 
    if (n->GetChild(i) != keep)
      _gsm->UnAssignEfgSubTree(n->BelongsTo(), n->GetChild(i));
  _gsm->UnAssignEfgElement(n->BelongsTo(), porNODE, n);

  Portion* por = new NodeValPortion(n->BelongsTo()->DeleteNode(n, keep));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
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
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------
// DeleteTree
//----------------

static Portion *GSM_DeleteTree(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  _gsm->UnAssignGameElement(n->BelongsTo(), true, porBEHAV | porEFSUPPORT);
  _gsm->UnAssignEfgSubTree(n->BelongsTo(), n);

  n->BelongsTo()->DeleteTree(n);

  Portion* por = new NodeValPortion(n);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
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
  gBlock<int> players(S->BelongsTo().NumPlayers());
  int i;
  for (i = 1; i <= players.Length(); i++)   players[i] = i;

  EFSupport *T = ComputeDominated(*S, strong, players,
				  ((OutputPortion *) param[4])->Value(),
				  gstatus);

  ((FloatPortion *) param[3])->Value() = watch.Elapsed();
  
  Portion *por = (T) ? new EfSupportValPortion(T) : new EfSupportValPortion(new EFSupport(*S));

  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}


//------------
// Float
//------------

extern Efg<double> *ConvertEfg(const Efg<gRational> &);

static Portion *GSM_FloatEfg(Portion **param)
{
  Efg<gRational> &orig = * (Efg<gRational> *) ((EfgPortion *) param[0])->Value();
  Efg<double> *E = ConvertEfg(orig);

  if (E)
    return new EfgValPortion(E);
  else
    return new ErrorPortion("Conversion failed.");
}

//----------
// Game
//----------

Portion* GSM_Game_EfgElements(Portion** param)
{
  if (param[0]->Game())  {
    assert(param[0]->GameIsEfg());
    return new EfgValPortion((BaseEfg*) param[0]->Game());
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

  Portion* por = new InfosetValPortion(n->GetInfoset());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
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

  Portion* por = new InfosetValPortion(a->BelongsTo());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//-------------
// Infosets
//-------------

static Portion *GSM_Infosets(Portion **param)
{
  EFPlayer *p = ((EfPlayerPortion *) param[0])->Value();

  Portion* por = ArrayToList(p->Infosets());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------
// InsertAction
//----------------

static Portion *GSM_InsertAction(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();

  _gsm->UnAssignGameElement(s->BelongsTo(), true, porBEHAV | porEFSUPPORT);

  Portion *por = new ActionValPortion(s->BelongsTo()->InsertAction(s));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

static Portion *GSM_InsertActionAt(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  Action *a = ((ActionPortion *) param[1])->Value();

  _gsm->UnAssignGameElement(s->BelongsTo(), true, porBEHAV | porEFSUPPORT);

  Portion *por = new ActionValPortion(s->BelongsTo()->InsertAction(s, a));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//--------------
// InsertMove
//--------------

static Portion *GSM_InsertMove(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  Node *n = ((NodePortion *) param[1])->Value();

  n->BelongsTo()->InsertNode(n, s);

  _gsm->UnAssignGameElement(s->BelongsTo(), true, porBEHAV | porEFSUPPORT);

  Portion* por = new NodeValPortion(n->GetParent());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//---------------
// IsConstSum
//---------------

static Portion *GSM_IsConstSumEfg(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();
  return new BoolValPortion(E.IsConstSum());
}

//----------------
// IsPredecessor
//----------------

static Portion *GSM_IsPredecessor(Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();
  return new BoolValPortion(n1->BelongsTo()->IsPredecessor(n1, n2));
}

//---------------
// IsSuccessor
//---------------

static Portion *GSM_IsSuccessor(Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();
  return new BoolValPortion(n1->BelongsTo()->IsSuccessor(n1, n2));
}


//-----------
// LoadEfg
//-----------

static Portion *GSM_LoadEfg(Portion **param)
{
  gString file = ((TextPortion *) param[0])->Value();
  
  gFileInput f(file);

  if (f.IsValid())   {
    DataType type;
    bool valid;

    EfgFileType(f, valid, type);
    if (!valid)   return new ErrorPortion("Not a valid .efg file");
    
    switch (type)   {
      case DOUBLE:  {
	Efg<double> *E = 0;
	ReadEfgFile((gInput &) f, E);

	if (E)
	  return new EfgValPortion(E);
	else
	  return new ErrorPortion("Not a valid .efg file");
      }
      case RATIONAL:   {
	Efg<gRational> *E = 0;
	ReadEfgFile((gInput &) f, E);
	
	if (E)
	  return new EfgValPortion(E);
	else
	  return new ErrorPortion("Not a valid .efg file");
      }
      default:
	assert(0);
	return 0;
    }
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
  BaseEfg &efg = * ((EfgPortion *) param[0])->Value();
  return new BoolValPortion(IsPerfectRecall(efg, s1, s2));
}

//-----------------
// MarkSubgame
//-----------------

static Portion *GSM_MarkSubgame(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  return new BoolValPortion(n->BelongsTo()->DefineSubgame(n));
}

//------------------
// MarkedSubgame
//------------------

static Portion *GSM_MarkedSubgame(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  return new BoolValPortion(n->GetSubgameRoot() == n);
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
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------
// MergeInfosets
//----------------

static Portion *GSM_MergeInfosets(Portion **param)
{
  Infoset *s1 = ((InfosetPortion *) param[0])->Value();
  Infoset *s2 = ((InfosetPortion *) param[1])->Value();
  
  s1->BelongsTo()->MergeInfoset(s1, s2);
  
  _gsm->UnAssignGameElement(s1->BelongsTo(), true, porBEHAV | porEFSUPPORT);

  Portion* por = new InfosetValPortion(s1);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------
// MoveToInfoset
//----------------

static Portion *GSM_MoveToInfoset(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  Infoset *s = ((InfosetPortion *) param[1])->Value();
  
  s->BelongsTo()->JoinInfoset(s, n);
  

  _gsm->UnAssignGameElement(s->BelongsTo(), true, porBEHAV | porEFSUPPORT);

  Portion* por = new InfosetValPortion(s);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//-------------
// MoveTree
//-------------

static Portion *GSM_MoveTree(Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();

  _gsm->UnAssignGameElement(n1->BelongsTo(), true, porBEHAV | porEFSUPPORT);

  Portion* por = new NodeValPortion(n1->BelongsTo()->MoveTree(n1, n2));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------
// Name
//----------

static Portion *GSM_Name_EfgElements(Portion **param)
{
  if( param[0]->Spec().Type == porNULL )
  {
    return new TextValPortion( "" );
  }

  switch (param[0]->Spec().Type)  {
    case porACTION:
      return new TextValPortion(((ActionPortion *) param[0])->Value()->
				GetName());
    case porINFOSET:
    case porINFOSET_FLOAT:
    case porINFOSET_RATIONAL:
      return new TextValPortion(((InfosetPortion *) param[0])->Value()->
				GetName());
    case porNODE:
      return new TextValPortion(((NodePortion *) param[0])->Value()->
				GetName());
    case porEFOUTCOME:
    case porEFOUTCOME_FLOAT:
    case porEFOUTCOME_RATIONAL:
      return new TextValPortion(((EfOutcomePortion *) param[0])->Value()->
				GetName());
    case porEFPLAYER:
      return new TextValPortion(((EfPlayerPortion *) param[0])->Value()->
				GetName());
    case porEFG_FLOAT:
    case porEFG_RATIONAL:
      return new TextValPortion(((EfgPortion *) param[0])->Value()->
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
  bool rat = ((BoolPortion *) param[0])->Value();
  
  if (rat)   {
    Efg<gRational> *E = new Efg<gRational>;
    ListPortion *players = (ListPortion *) param[1];
    for (int i = 1; i <= players->Length(); i++)
      E->NewPlayer()->SetName(((TextPortion *) (*players)[i])->Value());
    return new EfgValPortion(E);
  }
  else  {
    Efg<double> *E = new Efg<double>;
    ListPortion *players = (ListPortion *) param[1];
    for (int i = 1; i <= players->Length(); i++)
      E->NewPlayer()->SetName(((TextPortion *) (*players)[i])->Value());
    return new EfgValPortion(E);
  }
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

  Infoset *s = p->BelongsTo()->CreateInfoset(p, n);

  _gsm->UnAssignGameElement(p->BelongsTo(), true, porBEHAV | porEFSUPPORT);

  Portion* por = new InfosetValPortion(s);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}
 
//--------------
// NewOutcome
//--------------

static Portion *GSM_NewOutcome(Portion **param)
{
  EFOutcome *c;

  if (param[0]->Spec().Type == porEFG_FLOAT)
    c = ((Efg<double> *) ((EfgPortion *) param[0])->Value())->NewOutcome();
  else
    c = ((Efg<gRational> *) ((EfgPortion *) param[0])->Value())->NewOutcome();

  Portion *por = new EfOutcomeValPortion(c);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//---------------
// NewPlayer
//---------------

static Portion *GSM_NewPlayer(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();
  EFPlayer *p = E.NewPlayer();

  _gsm->UnAssignGameElement(&E, true, porBEHAV | porEFSUPPORT);

  Portion* por = new EfPlayerValPortion(p);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------
// NextSibling
//----------------

static Portion *GSM_NextSibling(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value()->NextSibling();
  if (!n)
    return new NullPortion(porNODE);
  
  Portion* por = new NodeValPortion(n);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------
// Nodes
//----------

static Portion *GSM_Nodes(Portion **param)
{
  BaseEfg *efg = ((EfgPortion *) param[0])->Value();

  gList<Node *> nodes;
  Nodes(*efg, nodes);

  Portion *por = ArrayToList(nodes);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
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

  Portion* por = new NodeValPortion(n->GetChild(child));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
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
    switch(n->BelongsTo()->Type())
    {
    case DOUBLE:
      return new NullPortion(porEFOUTCOME_FLOAT);
    case RATIONAL:
      return new NullPortion(porEFOUTCOME_RATIONAL);
    default:
      assert(0);
    }

  Portion* por = new EfOutcomeValPortion(n->GetOutcome());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//------------
// Outcomes
//------------

static Portion *GSM_Outcomes(Portion **param)
{
  BaseEfg *E = ((EfgPortion*) param[0])->Value();
  
  Portion* por = ArrayToList(E->Outcomes());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
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

  Portion* por = new NodeValPortion(n->GetParent());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//-----------
// Payoff
//-----------

static Portion *GSM_Payoff_Float(Portion **param)
{
  EFOutcome *c = ((EfOutcomePortion *) param[0])->Value();
  EFPlayer *player = ((EfPlayerPortion *) param[1])->Value();
  Efg<double> *efg = (Efg<double> *) player->BelongsTo();

  Portion* por = new FloatValPortion(efg->Payoff(c, player->GetNumber()));
  return por;
}

static Portion *GSM_Payoff_Rational(Portion **param)
{
  EFOutcome *c = ((EfOutcomePortion *) param[0])->Value();
  EFPlayer *player = ((EfPlayerPortion *) param[1])->Value();
  Efg<gRational> *efg = (Efg<gRational> *) player->BelongsTo();

  Portion* por = new RationalValPortion(efg->Payoff(c, player->GetNumber()));
  return por;
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

  Portion* por = new EfPlayerValPortion(s->GetPlayer());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//------------
// Players
//------------

static Portion *GSM_Players_Efg(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();

  Portion* por = ArrayToList(E.Players());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
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

  Portion* por = new ActionValPortion(a);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------
// PriorSibling
//----------------

static Portion *GSM_PriorSibling(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value()->PriorSibling();
  if (!n)
    return new NullPortion(porNODE);
  
  Portion* por = new NodeValPortion(n);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//-------------
// RandomEfg
//-------------

static Portion *GSM_RandomEfg_Float(Portion **param)
{
  Efg<double> &E = * (Efg<double> *) ((EfgPortion *) param[0])->Value();
  
  RandomEfg(E);
  return param[0]->ValCopy();
}

static Portion *GSM_RandomEfg_Rational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational> *) ((EfgPortion *) param[0])->Value();
  
  RandomEfg(E);
  return param[0]->ValCopy();
}

static Portion *GSM_RandomEfg_SeedFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double> *) ((EfgPortion *) param[0])->Value();
  int seed = ((IntPortion *) param[1])->Value();

  SetSeed(seed);
  RandomEfg(E);
  return param[0]->ValCopy();
}

static Portion *GSM_RandomEfg_SeedRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational> *) ((EfgPortion *) param[0])->Value();
  int seed = ((IntPortion *) param[1])->Value();

  SetSeed(seed);
  RandomEfg(E);
  return param[0]->ValCopy();
}  

//-------------
// Rational
//-------------

extern Efg<gRational> *ConvertEfg(const Efg<double> &);

static Portion *GSM_Rational_Efg(Portion **param)
{
  Efg<double> &orig = * (Efg<double> *) ((EfgPortion *) param[0])->Value();
  Efg<gRational> *E = ConvertEfg(orig);

  if (E)
    return new EfgValPortion(E);
  else
    return new ErrorPortion("Conversion failed.");
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
  
  s->BelongsTo()->Reveal(s, player);

  _gsm->UnAssignGameElement(s->BelongsTo(), true, porBEHAV | porEFSUPPORT);

  Portion *por = new InfosetValPortion(s);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//-------------
// RootNode
//-------------

static Portion *GSM_RootNode(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();

  Portion* por = new NodeValPortion(E.RootNode());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//-------------
// SaveEfg
//-------------

static Portion *GSM_SaveEfg(Portion **param)
{
  BaseEfg* E = ((EfgPortion *) param[0])->Value();
  gString text = ((TextPortion *) param[1])->Value();
  gFileOutput f(text);

  if (!f.IsValid())
    return new ErrorPortion("Cannot open file for writing");

  if (E->Type() == DOUBLE)
    ((Efg<double> *) E)->WriteEfgFile(f);
  else
    ((Efg<gRational> *) E)->WriteEfgFile(f);

  return param[0]->ValCopy();
}

//-------------------
// SetChanceProbs
//-------------------

static Portion *GSM_SetChanceProbs(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  ListPortion *p = (ListPortion *) param[1];

  if (!s->GetPlayer()->IsChance())
    return new ErrorPortion
      ("Information set does not belong to chance player");
  
  if ((s->BelongsTo()->Type() == DOUBLE && p->Spec().Type != porFLOAT) ||
      (s->BelongsTo()->Type() == RATIONAL && p->Spec().Type != porRATIONAL))
    return new ErrorPortion("Probability list does not match game type");
  if (p->Length() != s->NumActions())  
    return new ErrorPortion("Wrong number of probabilities");

  int i;

  switch (p->Spec().Type)   {
    case porFLOAT:
      for (i = 1; i <= p->Length(); i++) 
	((ChanceInfoset<double> *) s)->SetActionProb
          (i, ((FloatPortion *) (*p)[i])->Value());
      break;
    case porRATIONAL:
      for (i = 1; i <= p->Length(); i++)
	((ChanceInfoset<gRational> *) s)->SetActionProb
   	  (i, ((RationalPortion *) (*p)[i])->Value());
      break;
  }

  _gsm->InvalidateGameProfile(s->BelongsTo(), true);

  Portion* por = new InfosetValPortion(s);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
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
    case porINFOSET_FLOAT:
    case porINFOSET_RATIONAL:
      ((InfosetPortion *) param[0])->Value()->SetName(name);
      break;
    case porNODE:
      ((NodePortion *) param[0])->Value()->SetName(name);
      break;
    case porEFOUTCOME:
    case porEFOUTCOME_FLOAT:
    case porEFOUTCOME_RATIONAL:
      ((EfOutcomePortion *) param[0])->Value()->SetName(name);
      break;
    case porEFPLAYER:
      ((EfPlayerPortion *) param[0])->Value()->SetName(name);
      break;
    case porEFG_FLOAT:
    case porEFG_RATIONAL:
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

  _gsm->InvalidateGameProfile(n->BelongsTo(), true);
  
  if (c)  {
    Portion* por = new EfOutcomeValPortion(c);
    por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
    return por;
  }
  else 
    return new NullPortion(porEFOUTCOME);
}

//----------------
// SetPayoff
//----------------

static Portion *GSM_SetPayoff_Float(Portion **param)
{
  EFOutcome *c = ((EfOutcomePortion *) param[0])->Value();
  EFPlayer *player = ((EfPlayerPortion *) param[1])->Value();
  Efg<double> *efg = (Efg<double> *) player->BelongsTo();
  double value = ((FloatPortion *) param[2])->Value();

  efg->SetPayoff(c, player->GetNumber(), value);

  _gsm->InvalidateGameProfile(c->BelongsTo(), true);

  Portion* por = new EfOutcomeValPortion(c);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

static Portion *GSM_SetPayoff_Rational(Portion **param)
{
  EFOutcome *c = ((EfOutcomePortion *) param[0])->Value();
  EFPlayer *player = ((EfPlayerPortion *) param[1])->Value();
  Efg<gRational> *efg = (Efg<gRational> *) player->BelongsTo();
  gRational value = ((RationalPortion *) param[2])->Value();

  efg->SetPayoff(c, player->GetNumber(), value);

  _gsm->InvalidateGameProfile(c->BelongsTo(), true);

  Portion* por = new EfOutcomeValPortion(c);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------
// Subgames
//----------------

static Portion *GSM_Subgames(Portion **param)
{
  BaseEfg& E = *((EfgPortion*) param[0])->Value();
  gList<Node *> list;
  LegalSubgameRoots(E, list);

  Portion *por = ArrayToList(list);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;  
}  

//--------------
// Support
//--------------

static Portion *GSM_Support_Efg(Portion **param)
{
  BaseEfg &E = * ((EfgPortion *) param[0])->Value();
  Portion *por = new EfSupportValPortion(new EFSupport(E));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//-----------------
// UnmarkSubgame
//-----------------

static Portion *GSM_UnmarkSubgame(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  n->BelongsTo()->RemoveSubgame(n);
  
  Portion *por = new NodeValPortion(n);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
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
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ChanceProb,
				       porFLOAT | porRATIONAL, 1));
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
					    new BoolValPortion(false)));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("mixed", porBOOL,
					    new BoolValPortion(false)));
  FuncObj->SetParamInfo(0, 3, ParamInfoType("time", porFLOAT,
					    new FloatValPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 4, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(0, 5, ParamInfoType("traceLevel", porINTEGER,
					    new IntValPortion(0)));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Float", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_FloatEfg, porEFG_FLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG_RATIONAL));
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
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NewEfg, porEFG, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("rational", porBOOL,
					    new BoolValPortion(false)));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("players", PortionSpec(porTEXT,1),
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
  
  
  FuncObj = new FuncDescObj("Payoff", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Payoff_Float, porFLOAT, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("outcome", porEFOUTCOME_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("player", porEFPLAYER));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Payoff_Rational, 
				       porRATIONAL, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("outcome", porEFOUTCOME_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("player", porEFPLAYER));
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

  FuncObj = new FuncDescObj("Randomize", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_RandomEfg_Float, porEFG_FLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porEFG_FLOAT, 
					    REQUIRED ));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_RandomEfg_Rational, 
				       porEFG_RATIONAL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porEFG_RATIONAL,
					    REQUIRED ));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_RandomEfg_SeedFloat, 
				       porEFG_FLOAT, 2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porEFG_FLOAT,
					    REQUIRED ));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("seed", porINTEGER));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_RandomEfg_SeedRational, 
				       porEFG_RATIONAL, 2));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", porEFG_RATIONAL,
					    REQUIRED ));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("seed", porINTEGER));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("Rational", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Rational_Efg, porEFG_RATIONAL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG_FLOAT));
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
					    PortionSpec(porFLOAT | 
							porRATIONAL,1)));
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


  FuncObj = new FuncDescObj("SetPayoff", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetPayoff_Float, porEFOUTCOME, 3,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("outcome", porEFOUTCOME_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("player", porEFPLAYER));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("payoff", porFLOAT));
 
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_SetPayoff_Rational, 
				       porEFOUTCOME, 3,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("outcome", porEFOUTCOME_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("player", porEFPLAYER));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("payoff", porRATIONAL));
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



