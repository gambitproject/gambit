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


//-------------
// Actions
//-------------

Portion *GSM_Actions(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  EFSupport* sup = ((EfSupportPortion*) param[1])->Value();

  Portion *por = ArrayToList(sup->ActionList(s->GetPlayer()->GetNumber(),
					     s->GetNumber()));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//--------------
// AddAction
//--------------

Portion *GSM_AddAction(Portion **param)
{  
  EFSupport *support = ((EfSupportPortion *) param[0])->Value();
  Action *action = ((ActionPortion *) param[1])->Value();

  if (&support->BelongsTo() != action->BelongsTo()->BelongsTo())
    return new ErrorPortion("Support and action must be from same game");

  Infoset *infoset = action->BelongsTo();

  support->AddAction(infoset->GetPlayer()->GetNumber(),
		     infoset->GetNumber(), action);

  return param[0]->RefCopy();
}

//------------------
// AppendNode
//------------------

Portion *GSM_AppendNode(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  Infoset *s = ((InfosetPortion *) param[1])->Value();

  if (n->BelongsTo() != s->BelongsTo()) 
    return new ErrorPortion("Node and information set from different games");
  n->BelongsTo()->AppendNode(n, s);

  Portion* por = new NodeValPortion(n->GetChild(1));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------
// AttachOutcome
//----------------

Portion *GSM_AttachOutcome(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  Outcome *c = ((OutcomePortion *) param[1])->Value();

  if (n->BelongsTo() != c->BelongsTo())
    return new ErrorPortion("Node and outcome from different games");
  n->SetOutcome(c);
  
  Portion* por = new OutcomeValPortion(c);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//--------------
// Chance
//--------------

Portion *GSM_Chance(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();

  Portion* por = new EfPlayerValPortion(E.GetChance());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------
// ChanceProb
//----------------

Portion *GSM_ChanceProb(Portion **param)
{
  Portion* por;
  Action *action = ((ActionPortion *) param[0])->Value();
  Infoset *infoset = action->BelongsTo();
  if (!infoset->GetPlayer()->IsChance()) 
    return new NullPortion
      ((infoset->BelongsTo()->Type() == DOUBLE) ? porFLOAT : porRATIONAL);

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

Portion *GSM_Children(Portion **param)
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

Portion *GSM_CompressEfg(Portion **param)
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

Portion *GSM_CopyTree(Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();

  if (n1->BelongsTo() != n2->BelongsTo())
    return new ErrorPortion("n1 and n2 belong to different trees");

  Portion* por = new NodeValPortion(n1->BelongsTo()->CopyTree(n1, n2));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//-----------------
// DeleteAction
//-----------------

Portion *GSM_DeleteAction(Portion **param)
{
  Action *action = ((ActionPortion *) param[0])->Value();
  Infoset *infoset = action->BelongsTo();

  if (infoset->NumActions() == 1)
    return new ErrorPortion("Cannot delete the only action at an infoset.");

  infoset->BelongsTo()->DeleteAction(infoset, action);

  Portion* por = new InfosetValPortion(infoset);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------------
// DeleteEmptyInfoset
//----------------------

Portion *GSM_DeleteEmptyInfoset(Portion **param)
{
  Infoset *infoset = ((InfosetPortion *) param[0])->Value();

  return new BoolValPortion(infoset->BelongsTo()->DeleteEmptyInfoset(infoset));
}

//----------------
// DeleteNode
//----------------

Portion *GSM_DeleteNode(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  Node *keep = ((NodePortion *) param[1])->Value();

  if (n->BelongsTo() != keep->BelongsTo())
    return new ErrorPortion("The nodes are from different games");
  if (keep->GetParent() != n)
    return new ErrorPortion("keep is not a child of node");

  Portion* por = new NodeValPortion(n->BelongsTo()->DeleteNode(n, keep));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//-----------------
// DeleteOutcome
//-----------------

Portion *GSM_DeleteOutcome(Portion **param)
{
  Outcome *outc = ((OutcomePortion *) param[0])->Value();
  outc->BelongsTo()->DeleteOutcome(outc);
  return new BoolValPortion(true);
}

//----------------
// DeleteTree
//----------------

Portion *GSM_DeleteTree(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  n->BelongsTo()->DeleteTree(n);

  Portion* por = new NodeValPortion(n);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------
// DetachOutcome
//----------------

Portion *GSM_DetachOutcome(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  n->SetOutcome(0);

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

Portion *GSM_ElimDom_Efg(Portion **param)
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

Portion *GSM_FloatEfg(Portion **param)
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

Portion *GSM_Infoset_Node(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  if (!n->GetInfoset())
    return new NullPortion(porINFOSET);
  //return new ErrorPortion("Terminal nodes have no information set");

  Portion* por = new InfosetValPortion(n->GetInfoset());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}


Portion *GSM_Infoset_Action(Portion **param)
{
  Action *a = ((ActionPortion *) param[0])->Value();

  if (!a->BelongsTo())
    return new NullPortion(porINFOSET);
  //return new ErrorPortion("Terminal nodes have no information set");

  Portion* por = new InfosetValPortion(a->BelongsTo());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//-------------
// Infosets
//-------------

Portion *GSM_Infosets(Portion **param)
{
  EFPlayer *p = ((EfPlayerPortion *) param[0])->Value();

  Portion* por = ArrayToList(p->InfosetList());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------
// InsertAction
//----------------

Portion *GSM_InsertAction(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  Portion *por = new ActionValPortion(s->BelongsTo()->InsertAction(s));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

Portion *GSM_InsertActionAt(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  Action *a = ((ActionPortion *) param[1])->Value();
  Portion *por = new ActionValPortion(s->BelongsTo()->InsertAction(s, a));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//--------------
// InsertNode
//--------------

Portion *GSM_InsertNode(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  Infoset *s = ((InfosetPortion *) param[1])->Value();

  if (n->BelongsTo() != s->BelongsTo()) 
    return new ErrorPortion("Node and information set from different games");
  n->BelongsTo()->InsertNode(n, s);

  Portion* por = new NodeValPortion(n->GetParent());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//---------------
// IsConstSum
//---------------

Portion *GSM_IsConstSumEfg(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();
  return new BoolValPortion(E.IsConstSum());
}

//----------------
// IsPredecessor
//----------------

Portion *GSM_IsPredecessor(Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();
  return new BoolValPortion(n1->BelongsTo()->IsPredecessor(n1, n2));
}

//---------------
// IsSuccessor
//---------------

Portion *GSM_IsSuccessor(Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();
  return new BoolValPortion(n1->BelongsTo()->IsSuccessor(n1, n2));
}

//-----------
// LoadEfg
//-----------

Portion *GSM_LoadEfg(Portion **param)
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

//---------------
// MarkSubgames
//---------------

Portion *GSM_MarkSubgames_Efg(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();

  gList<Node *> roots;
  LegalSubgameRoots(E, roots);

  E.MarkSubgames(roots);

  MarkedSubgameRoots(E, roots);

  Portion *por = ArrayToList(roots);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

Portion *GSM_MarkSubgames_Node(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  gList<Node *> roots;
  LegalSubgameRoots(n, roots);

  n->BelongsTo()->MarkSubgames(roots);
  
  MarkedSubgameRoots(*n->BelongsTo(), roots);
  
  Portion *por = ArrayToList(roots);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//-------------------
// IsPerfectRecall
//-------------------

Portion *GSM_IsPerfectRecall(Portion **param)
{
  Infoset *s1, *s2;
  BaseEfg &efg = * ((EfgPortion *) param[0])->Value();
  return new BoolValPortion(IsPerfectRecall(efg, s1, s2));
}

//--------------------
// MarkThisSubgame
//--------------------

Portion *GSM_MarkThisSubgame(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  return new BoolValPortion(n->BelongsTo()->DefineSubgame(n));
}

//------------
// Members
//------------

Portion *GSM_Members(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();

  Portion* por = ArrayToList(s->GetMemberList());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------
// MergeInfosets
//----------------

Portion *GSM_MergeInfosets(Portion **param)
{
  Infoset *s1 = ((InfosetPortion *) param[0])->Value();
  Infoset *s2 = ((InfosetPortion *) param[1])->Value();
  
  if (s1->BelongsTo() != s2->BelongsTo())
    return new ErrorPortion("Information sets from different games");
  s1->BelongsTo()->MergeInfoset(s1, s2);
  
  Portion* por = new InfosetValPortion(s1);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------
// MoveToInfoset
//----------------

Portion *GSM_MoveToInfoset(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  Infoset *s = ((InfosetPortion *) param[1])->Value();
  
  if (s->BelongsTo() != n->BelongsTo())
    return new ErrorPortion("Information set and node from different games");
  s->BelongsTo()->JoinInfoset(s, n);
  
  Portion* por = new InfosetValPortion(s);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//-------------
// MoveTree
//-------------

Portion *GSM_MoveTree(Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();

  if (n1->BelongsTo() != n2->BelongsTo())
    return new ErrorPortion("n1 and n2 belong to different trees");

  Portion* por = new NodeValPortion(n1->BelongsTo()->MoveTree(n1, n2));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------
// Name
//----------

Portion *GSM_Name_EfgElements(Portion **param)
{
  switch (param[0]->Spec().Type)  {
    case porACTION:
      return new TextValPortion(((ActionPortion *) param[0])->Value()->GetName());
    case porINFOSET:
      return new TextValPortion(((InfosetPortion *) param[0])->Value()->GetName());
    case porNODE:
      return new TextValPortion(((NodePortion *) param[0])->Value()->GetName());
    case porOUTCOME:
      return new TextValPortion(((OutcomePortion *) param[0])->Value()->GetName());
    case porPLAYER_EFG:
      return new TextValPortion(((EfPlayerPortion *) param[0])->Value()->GetName());
    case porEFG:
      return new TextValPortion(((EfgPortion *) param[0])->Value()->GetTitle());
    default:
      assert(0);
      return 0;
  }
}


//------------
// NewEfg
//------------

Portion *GSM_NewEfg(Portion **param)
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

Portion *GSM_NewInfoset(Portion **param)
{
  EFPlayer *p = ((EfPlayerPortion *) param[0])->Value();
  int n = ((IntPortion *) param[1])->Value();
  gString name = ((TextPortion *) param[2])->Value();

  if (n <= 0)
    return new ErrorPortion("Information sets must have at least one action");

  Infoset *s = p->BelongsTo()->CreateInfoset(p, n);
  s->SetName(name);

  Portion* por = new InfosetValPortion(s);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}
 
//--------------
// NewOutcome
//--------------

Portion *GSM_NewOutcome(Portion **param)
{
  Outcome *c;
  gString name = ((TextPortion *) param[1])->Value();

  if (param[0]->Spec().Type == porEFG_FLOAT)
    c = ((Efg<double> *) ((EfgPortion *) param[0])->Value())->NewOutcome();
  else
    c = ((Efg<gRational> *) ((EfgPortion *) param[0])->Value())->NewOutcome();

  c->SetName(name);

  Portion *por = new OutcomeValPortion(c);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//---------------
// NewPlayer
//---------------

Portion *GSM_NewPlayer(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();

  EFPlayer *p = E.NewPlayer();
  p->SetName(name);

  Portion* por = new EfPlayerValPortion(p);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------
// NextSibling
//----------------

Portion *GSM_NextSibling(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value()->NextSibling();
  if (!n)
    return new NullPortion(porNODE);
  //return new ErrorPortion("Node is the last sibling");
  
  Portion* por = new NodeValPortion(n);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//---------------
// NthChild
//---------------

Portion *GSM_NthChild(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  int child = ((IntPortion *) param[1])->Value();
  if (child < 1 || child > n->NumChildren())  
    return new NullPortion(porNODE);
  //return new ErrorPortion("Child number out of range");

  Portion* por = new NodeValPortion(n->GetChild(child));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//------------
// Outcome
//------------

Portion *GSM_Outcome(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  if (!n->GetOutcome())
    switch(n->BelongsTo()->Type())
    {
    case DOUBLE:
      return new NullPortion(porOUTCOME_FLOAT);
    case RATIONAL:
      return new NullPortion(porOUTCOME_RATIONAL);
    default:
      assert(0);
    }
  //return new ErrorPortion("No outcome attached to node");

  Portion* por = new OutcomeValPortion(n->GetOutcome());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//------------
// Outcomes
//------------

Portion *GSM_Outcomes(Portion **param)
{
  BaseEfg *E = ((EfgPortion*) param[0])->Value();
  
  Portion* por = ArrayToList(E->OutcomeList());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//------------
// Parent
//------------

Portion *GSM_Parent(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  if (!n->GetParent())
    return new NullPortion(porNODE);
  //return new ErrorPortion("Node has no parent");

  Portion* por = new NodeValPortion(n->GetParent());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//-----------
// Payoff
//-----------

Portion *GSM_Payoff_Float(Portion **param)
{
  OutcomeVector<double> *c = 
    (OutcomeVector<double> *) ((OutcomePortion *) param[0])->Value();
  EFPlayer *player = ((EfPlayerPortion *) param[1])->Value();

  Portion* por = new FloatValPortion((*c)[player->GetNumber()]);
  return por;
}

Portion *GSM_Payoff_Rational(Portion **param)
{
  OutcomeVector<gRational> *c = 
    (OutcomeVector<gRational> *) ((OutcomePortion *) param[0])->Value();
  EFPlayer *player = ((EfPlayerPortion *) param[1])->Value();

  Portion* por = new RationalValPortion((*c)[player->GetNumber()]);
  return por;
}

//----------
// Player
//----------

Portion *GSM_Player(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();

  Portion* por = new EfPlayerValPortion(s->GetPlayer());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//------------
// Players
//------------

Portion *GSM_Players_Efg(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();

  Portion* por = ArrayToList(E.PlayerList());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//--------------
// PriorAction
//--------------

Portion *GSM_PriorAction(Portion** param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  Action* a = LastAction(n);
  if(a == 0)
    return new NullPortion(porACTION);
  //return new ErrorPortion("called on a root node");

  Portion* por = new ActionValPortion(a);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------
// PriorSibling
//----------------

Portion *GSM_PriorSibling(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value()->PriorSibling();
  if (!n)
    return new NullPortion(porNODE);
  //return new ErrorPortion("Node is the first sibling");
  
  Portion* por = new NodeValPortion(n);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//-------------
// RandomEfg
//-------------

Portion *GSM_RandomEfg_Float(Portion **param)
{
  Efg<double> &E = * (Efg<double> *) ((EfgPortion *) param[0])->Value();
  
  RandomEfg(E);
  return param[0]->RefCopy();
}

Portion *GSM_RandomEfg_Rational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational> *) ((EfgPortion *) param[0])->Value();
  
  RandomEfg(E);
  return param[0]->RefCopy();
}

Portion *GSM_RandomEfg_SeedFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double> *) ((EfgPortion *) param[0])->Value();
  int seed = ((IntPortion *) param[1])->Value();

  SetSeed(seed);
  RandomEfg(E);
  return param[0]->RefCopy();
}

Portion *GSM_RandomEfg_SeedRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational> *) ((EfgPortion *) param[0])->Value();
  int seed = ((IntPortion *) param[1])->Value();

  SetSeed(seed);
  RandomEfg(E);
  return param[0]->RefCopy();
}  

//-------------
// Rational
//-------------

extern Efg<gRational> *ConvertEfg(const Efg<double> &);

Portion *GSM_Rational_Efg(Portion **param)
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

Portion *GSM_RemoveAction(Portion **param)
{  
  EFSupport *support = ((EfSupportPortion *) param[0])->Value();
  Action *action = ((ActionPortion *) param[1])->Value();
  Infoset *infoset = action->BelongsTo();

  if (&support->BelongsTo() != infoset->BelongsTo())
    return new ErrorPortion("Support and action must be from same game");

  support->RemoveAction(infoset->GetPlayer()->GetNumber(),
			infoset->GetNumber(), action);

  return param[0]->RefCopy();
}


//-------------
// Reveal
//-------------

Portion *GSM_Reveal(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  ListPortion *players = (ListPortion *) param[1];

  gBlock<EFPlayer *> player(players->Length());
  for (int i = 1; i <= players->Length(); i++)
    player[i] = ((EfPlayerPortion *) (*players)[i])->Value();
  
  s->BelongsTo()->Reveal(s, player);

  Portion *por = new InfosetValPortion(s);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//-------------
// RootNode
//-------------

Portion *GSM_RootNode(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();

  Portion* por = new NodeValPortion(E.RootNode());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//-------------
// SaveEfg
//-------------

Portion *GSM_SaveEfg(Portion **param)
{
  BaseEfg* E = ((EfgPortion *) param[0])->Value();
  gString text = ((TextPortion *) param[1])->Value();
  gFileOutput f(text);

  if (!f.IsValid())
    return new ErrorPortion("Cannot open file for writing");

  E->WriteEfgFile(f);

  return param[0]->RefCopy();
}

//-------------------
// SetChanceProbs
//-------------------

Portion *GSM_SetChanceProbs(Portion **param)
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

  Portion* por = new InfosetValPortion(s);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//--------------
// SetName
//--------------

Portion *GSM_SetName_EfgElements(Portion **param)
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
    case porOUTCOME:
      ((OutcomePortion *) param[0])->Value()->SetName(name);
      break;
    case porPLAYER_EFG:
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
// SetPayoff
//----------------

Portion *GSM_SetPayoff_Float(Portion **param)
{
  OutcomeVector<double> *c = 
    (OutcomeVector<double> *) ((OutcomePortion *) param[0])->Value();
  EFPlayer *player = ((EfPlayerPortion *) param[1])->Value();
  double value = ((FloatPortion *) param[2])->Value();

  (*c)[player->GetNumber()] = value;

  Portion* por = new OutcomeValPortion(c);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

Portion *GSM_SetPayoff_Rational(Portion **param)
{
  OutcomeVector<gRational> *c = 
    (OutcomeVector<gRational> *) ((OutcomePortion *) param[0])->Value();
  EFPlayer *player = ((EfPlayerPortion *) param[1])->Value();
  gRational value = ((RationalPortion *) param[2])->Value();

  (*c)[player->GetNumber()] = value;

  Portion* por = new OutcomeValPortion(c);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------
// SubgameRoots
//----------------

Portion *GSM_SubgameRoots(Portion **param)
{
  BaseEfg& E = *((EfgPortion*) param[0])->Value();
  gList<Node *> list;
  MarkedSubgameRoots(E, list);

  Portion *por = ArrayToList(list);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;  
}  

//--------------
// Support
//--------------

Portion *GSM_Support_Efg(Portion **param)
{
  BaseEfg &E = * ((EfgPortion *) param[0])->Value();
  Portion *por = new EfSupportValPortion(new EFSupport(E));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//-----------------
// UnmarkSubgames
//-----------------

Portion *GSM_UnmarkSubgames_Efg(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();

  E.UnmarkSubgames(E.RootNode());

  gList<Node *> roots;
  MarkedSubgameRoots(E, roots);

  Portion *por = ArrayToList(roots);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

Portion *GSM_UnmarkSubgames_Node(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  n->BelongsTo()->UnmarkSubgames(n);
  
  gList<Node *> roots;
  MarkedSubgameRoots(*n->BelongsTo(), roots);
  
  Portion *por = ArrayToList(roots);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//---------------------
// UnmarkThisSubgame
//---------------------

Portion *GSM_UnmarkThisSubgame(Portion **param)
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
				       PortionSpec(porACTION, 1), 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("infoset", porINFOSET));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("support", porEF_SUPPORT)); 
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("AddAction", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_AddAction, 
				       porEF_SUPPORT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porEF_SUPPORT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("action", porACTION));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("AppendNode", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_AppendNode, 
				       porNODE, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("infoset", porINFOSET));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("AttachOutcome", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_AttachOutcome, 
				       porOUTCOME, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("outcome", porOUTCOME));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Chance", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Chance, porPLAYER_EFG, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("ChanceProb", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ChanceProb, 
				       PortionSpec(porFLOAT | porRATIONAL, 1),
				       1));
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
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porEF_SUPPORT));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("CopyTree", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_CopyTree, porNODE, 2));
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

  FuncObj = new FuncDescObj("DeleteNode", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_DeleteNode, porNODE, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("keep", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("DeleteOutcome", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_DeleteOutcome, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("outcome", porOUTCOME));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("DeleteTree", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_DeleteTree, porNODE, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("DetachOutcome", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_DetachOutcome, porNODE, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("ElimDom", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ElimDom_Efg, 
				       porEF_SUPPORT, 6));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porEF_SUPPORT));
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

  FuncObj = new FuncDescObj("Game", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Game_EfgElements, porEFG, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("player", porPLAYER_EFG));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Game_EfgElements, porEFG, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("node", porNODE));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Game_EfgElements, porEFG, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("outcome", porOUTCOME));
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("Infoset", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Infoset_Node, porINFOSET, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Infoset_Action, porINFOSET, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("action", porACTION));
  gsm->AddFunction(FuncObj);
  
  
  FuncObj = new FuncDescObj("Infosets", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Infosets, 
				       PortionSpec(porINFOSET, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("player", porPLAYER_EFG));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("InsertAction", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_InsertAction, porACTION, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("infoset", porINFOSET));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_InsertActionAt, porACTION, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("infoset", porINFOSET));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("at", porACTION));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("InsertNode", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_InsertNode, porNODE, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("infoset", porINFOSET));
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
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsPredecessor, porBOOL, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("of", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsSuccessor", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsSuccessor, porBOOL, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("from", porNODE));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LoadEfg", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_LoadEfg, porEFG, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("file", porTEXT));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("MarkSubgames", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_MarkSubgames_Efg, 
				       PortionSpec(porNODE, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG, REQUIRED, BYREF));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_MarkSubgames_Node, 
				       PortionSpec(porNODE, 1), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("MarkThisSubgame", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_MarkThisSubgame, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Members", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Members, 
				       PortionSpec(porNODE, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("infoset", porINFOSET));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("MergeInfosets", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_MergeInfosets, porINFOSET, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("infoset1", porINFOSET));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("infoset2", porINFOSET));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("MoveToInfoset", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_MoveToInfoset, porNODE, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("infoset", porINFOSET));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("MoveTree", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_MoveTree, porNODE, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("from", porNODE));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("to", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Name", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Name_EfgElements, porTEXT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porACTION | porINFOSET | 
					    porNODE | porOUTCOME | 
					    porPLAYER_EFG | porEFG));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NewEfg", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NewEfg, porEFG, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("rational", porBOOL,
					    new BoolValPortion(false)));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("players", PortionSpec(porTEXT,1),
					    new ListValPortion));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NewInfoset", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NewInfoset, porINFOSET, 3));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("player", porPLAYER_EFG));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("actions", porINTEGER));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("name", porTEXT,
					    new TextValPortion("")));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NewOutcome", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NewOutcome, porOUTCOME, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("name", porTEXT,
					    new TextValPortion("")));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NewPlayer", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NewPlayer, porPLAYER_EFG, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("name", porTEXT,
					    new TextValPortion("")));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NextSibling", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NextSibling, porNODE, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NthChild", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NthChild, porNODE, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("n", porINTEGER));
  gsm->AddFunction(FuncObj);
  
  
  FuncObj = new FuncDescObj("Outcome", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Outcome, porOUTCOME, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);
  
  
  FuncObj = new FuncDescObj("Outcomes", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Outcomes, 
				       PortionSpec(porOUTCOME, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG));
  gsm->AddFunction(FuncObj);
  
  
  FuncObj = new FuncDescObj("Parent", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Parent, porNODE, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);
  
  
  FuncObj = new FuncDescObj("Payoff", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Payoff_Float, porFLOAT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("outcome", porOUTCOME_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("player", porPLAYER_EFG));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Payoff_Rational, 
				       porRATIONAL, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("outcome", porOUTCOME_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("player", porPLAYER_EFG));
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("Player", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Player, porPLAYER_EFG, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("infoset", porINFOSET));
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("Players", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Players_Efg, 
				       PortionSpec(porPLAYER_EFG, 1), 1));  
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
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_RandomEfg_Rational, 
				       porEFG_RATIONAL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porEFG_RATIONAL,
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_RandomEfg_SeedFloat, 
				       porEFG_FLOAT, 2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porEFG_FLOAT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("seed", porINTEGER));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_RandomEfg_SeedRational, 
				       porEFG_RATIONAL, 2));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", porEFG_RATIONAL,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("seed", porINTEGER));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Rational", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Rational_Efg, porEFG_RATIONAL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG_FLOAT));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RemoveAction", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_RemoveAction, porEF_SUPPORT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porEF_SUPPORT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("action", porACTION));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Reveal", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Reveal, porINFOSET, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("infoset", porINFOSET));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("who", 
					    PortionSpec(porPLAYER_EFG,1)));
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
				       porOUTCOME | porPLAYER_EFG |
				       porEFG, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porACTION | porINFOSET | 
					    porNODE | porOUTCOME | 
					    porPLAYER_EFG | porEFG));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("name", porTEXT));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetPayoff", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetPayoff_Float, porOUTCOME, 3));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("outcome", porOUTCOME_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("player", porPLAYER_EFG));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("payoff", porFLOAT));
 
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_SetPayoff_Rational, 
				       porOUTCOME, 3));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("outcome", porOUTCOME_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("player", porPLAYER_EFG));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("payoff", porRATIONAL));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("SubgameRoots", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SubgameRoots, 
				       PortionSpec(porNODE, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG, REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Support", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Support_Efg, porEF_SUPPORT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("UnmarkSubgames", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_UnmarkSubgames_Efg, 
				       PortionSpec(porNODE, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("efg", porEFG, REQUIRED, BYREF));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_UnmarkSubgames_Node,
				       PortionSpec(porNODE, 1), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("UnmarkThisSubgame", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_UnmarkThisSubgame, porNODE, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);

}

