//
// FILE: efgfunc.cc -- Extensive form editing builtins
//
// $Id$
//


#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

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

  Portion* por = ArrayToList(s->GetActionList());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

//--------------
// AddAction
//--------------

Portion *GSM_AddAction(Portion **param)
{  
  EFSupport *support = ((EfSupportPortion *) param[0])->Value();
  Infoset *infoset = ((InfosetPortion *) param[1])->Value();
  Action *action = ((ActionPortion *) param[2])->Value();

  if (&support->BelongsTo() != infoset->BelongsTo())
    return new ErrorPortion("Support and infoset must be from same game");
  if (action->BelongsTo() != infoset)
    return new ErrorPortion("Action must belong to infoset");

  support->AddAction(infoset->GetPlayer()->GetNumber(),
		     infoset->GetNumber(), action);

  return param[0]->RefCopy();
}

//------------------
// AppendAction
//------------------

Portion *GSM_AppendAction(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  s->BelongsTo()->AppendAction(s);

  Portion* por = new ActionValPortion(s->GetActionList()[s->NumActions()]);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
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
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
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
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

//--------------
// Chance
//--------------

Portion *GSM_Chance(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();

  Portion* por = new EfPlayerValPortion(E.GetChance());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

//----------------
// ChanceProbs
//----------------

Portion *GSM_ChanceProbs(Portion **param)
{
  Portion* por;
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  if (!s->GetPlayer()->IsChance()) 
    return new ErrorPortion
      ("Only chance information sets have action probabilities");

  switch (s->BelongsTo()->Type())   {
    case DOUBLE:
      por = ArrayToList((gArray<double> &) ((ChanceInfoset<double> *) s)->GetActionProbs());
      break;
    case RATIONAL:
      por = ArrayToList((gArray<gRational> &) ((ChanceInfoset<gRational> *) s)->GetActionProbs());
      break;
    default:
      assert(0);
      return 0;
  }

  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
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
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

//-----------------
// DeleteAction
//-----------------

Portion *GSM_DeleteAction(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  Action *a = ((ActionPortion *) param[1])->Value();
  s->BelongsTo()->DeleteAction(s, a);

  Portion* por = new InfosetValPortion(s);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
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
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
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
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
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
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
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

//----------------
// HasOutcome
//----------------

Portion *GSM_HasOutcome(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  return new BoolValPortion( n->GetOutcome() != 0 );
}

//-------------
// Infoset
//-------------

Portion *GSM_Infoset(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  if (!n->GetInfoset())
    return new ErrorPortion("Terminal nodes have no information set");
  Portion* por = new InfosetValPortion(n->GetInfoset());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

//-------------
// Infosets
//-------------

Portion *GSM_Infosets(Portion **param)
{
  EFPlayer *p = ((EfPlayerPortion *) param[0])->Value();

  Portion* por = ArrayToList(p->InfosetList());
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

//----------------
// InsertAction
//----------------

Portion *GSM_InsertAction(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  Action *a = ((ActionPortion *) param[1])->Value();
  s->BelongsTo()->InsertAction(s, a);

  Portion* por = new InfosetValPortion(s);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
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
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
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

//--------------
// IsRoot
//--------------

Portion *GSM_IsRoot(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  return new BoolValPortion(n->GetParent() == 0);
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

//--------------
// JoinInfoset
//--------------

Portion *GSM_JoinInfoset(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  Node *n = ((NodePortion *) param[1])->Value();
  
  if (s->BelongsTo() != n->BelongsTo())
    return new ErrorPortion("Information set and node from different games");
  s->BelongsTo()->JoinInfoset(s, n);
  
  Portion* por = new InfosetValPortion(s);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

//--------------
// LastAction
//--------------

Portion *GSM_LastAction( Portion** param )
{
  Node *n = ((NodePortion *) param[0])->Value();
  Action* a = LastAction( n );
  if( a == 0 )
    return new ErrorPortion( "called on a root node" );

  Portion* por = new ActionValPortion( a );
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

//---------------
// LeaveInfoset
//---------------

Portion *GSM_LeaveInfoset(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  Portion* por = new InfosetValPortion(n->BelongsTo()->LeaveInfoset(n));
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
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
  por->SetOwner(param[0]->Original());
  por->AddDependency();
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
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
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
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
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
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
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
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

//----------
// Name
//----------

Portion *GSM_Name_EfgElements(Portion **param)
{
  switch (param[0]->Type())  {
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
    default:
      assert(0);
  }
}

Portion *GSM_Name_Efg(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();
  return new TextValPortion(E.GetTitle());
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
      E->NewPlayer()->SetName(((TextPortion *)players->Subscript(i))->Value());
    return new EfgValPortion(E);
  }
  else  {
    Efg<double> *E = new Efg<double>;
    ListPortion *players = (ListPortion *) param[1];
    for (int i = 1; i <= players->Length(); i++)
      E->NewPlayer()->SetName(((TextPortion *)players->Subscript(i))->Value());
    return new EfgValPortion(E);
  }
}


//--------------
// NewInfoset
//--------------

// The version specifying just the number of actions
Portion *GSM_NewInfoset1(Portion **param)
{
  EFPlayer *p = ((EfPlayerPortion *) param[0])->Value();
  int n = ((IntPortion *) param[1])->Value();
  gString name = ((TextPortion *) param[2])->Value();

  Infoset *s = p->BelongsTo()->CreateInfoset(p, n);
  s->SetName(name);

  Portion* por = new InfosetValPortion(s);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}
 
// The version specifying a list of names for actions
Portion *GSM_NewInfoset2(Portion **param)
{
  EFPlayer *p = ((EfPlayerPortion *) param[0])->Value();
  ListPortion *actions = (ListPortion *) param[1];
  gString name = ((TextPortion *) param[2])->Value();

  if (actions->Length() == 0)
    return new ErrorPortion("Must have at least one action");
  Infoset *s = p->BelongsTo()->CreateInfoset(p, actions->Length());
  s->SetName(name);
  for (int i = 1; i <= actions->Length(); i++)
    s->SetActionName(i, ((TextPortion *) actions->Subscript(i))->Value());

  Portion* por = new InfosetValPortion(s);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

//--------------
// NewOutcome
//--------------

Portion *GSM_NewOutcome(Portion **param)
{
  Outcome *c;
  gString name = ((TextPortion *) param[1])->Value();

  if (param[0]->Type() == porEFG_FLOAT)
    c = ((Efg<double> *) ((EfgPortion *) param[0])->Value())->NewOutcome();
  else
    c = ((Efg<gRational> *) ((EfgPortion *) param[0])->Value())->NewOutcome();

  c->SetName(name);

  Portion *por = new OutcomeValPortion(c);
  por->SetOwner( param[0]->Original());
  por->AddDependency();
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
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

//--------------
// NewSupport
//--------------

Portion *GSM_NewSupport_Efg(Portion **param)
{
  BaseEfg &E = * ((EfgPortion *) param[0])->Value();
  Portion *p = new EfSupportValPortion(new EFSupport(E));

  p->SetOwner( param[ 0 ]->Original() );
  p->AddDependency();
  return p;
}

//----------------
// NextSibling
//----------------

Portion *GSM_NextSibling(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value()->NextSibling();
  if (!n)
    return new ErrorPortion("Node is the last sibling");
  
  Portion* por = new NodeValPortion(n);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

//-----------
// Nodes
//-----------

Portion *GSM_Nodes(Portion **param)
{
  BaseEfg& E = *((EfgPortion *) param[0])->Value();
  gList<Node *> list;
  Nodes(E, list);

  Portion *por = ArrayToList(list);
  por->SetOwner(param[0]->Original());
  por->AddDependency();
  return por;
}

//--------------------
// NonterminalNodes
//--------------------

Portion* GSM_NonterminalNodes( Portion** param )
{
  BaseEfg& E = *((EfgPortion *) param[0])->Value();
  gList<Node *> list;
  NonTerminalNodes(E, list);

  Portion *por = ArrayToList(list);
  por->SetOwner(param[0]->Original());
  por->AddDependency();
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
    return new ErrorPortion("Child number out of range");

  Portion* por = new NodeValPortion(n->GetChild(child));
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

//---------------
// NumActions
//---------------

Portion *GSM_NumActions(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  return new IntValPortion(s->NumActions());
}

Portion *GSM_NumActions_EFSupport(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  EFSupport *S = ((EfSupportPortion *) param[1])->Value();
  return new IntValPortion(S->NumActions(s->GetPlayer()->GetNumber(),
					 s->GetNumber()));
}

//---------------
// NumChildren
//---------------

Portion *GSM_NumChildren(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  return new IntValPortion(n->NumChildren());
}

//---------------
// NumInfosets
//---------------

Portion *GSM_NumInfosets(Portion **param)
{
  EFPlayer *p = ((EfPlayerPortion *) param[0])->Value();
  return new IntValPortion(p->NumInfosets());
}

//--------------
// NumMembers
//--------------

Portion *GSM_NumMembers(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  return new IntValPortion(s->NumMembers());
}

//------------
// NumNodes
//------------

Portion* GSM_NumNodes(Portion ** param)
{
  BaseEfg& E = *((EfgPortion*) param[0])->Value();
  return new IntValPortion(NumNodes(E));  
}
//--------------
// NumOutcomes
//--------------

Portion *GSM_NumOutcomes(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();
  return new IntValPortion(E.NumOutcomes());
}

//--------------
// NumPlayers
//--------------

Portion *GSM_NumPlayers_Efg(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();
  return new IntValPortion(E.NumPlayers());
}

//------------
// Outcome
//------------

Portion *GSM_Outcome(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  if (!n->GetOutcome())
    return new ErrorPortion("No outcome attached to node");

  Portion* por = new OutcomeValPortion(n->GetOutcome());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

//------------
// Outcomes
//------------

Portion *GSM_Outcomes(Portion **param)
{
  BaseEfg *E = ((EfgPortion*) param[0])->Value();
  
  Portion* por = ArrayToList(E->OutcomeList());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

//------------
// Parent
//------------

Portion *GSM_Parent(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  if (!n->GetParent())
    return new ErrorPortion("Node is the root node");

  Portion* por = new NodeValPortion(n->GetParent());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

//-----------
// Payoff
//-----------

Portion *GSM_Payoff_Float(Portion **param)
{
  OutcomeVector<double> *c = 
    (OutcomeVector<double> *) ((OutcomePortion *) param[0])->Value();
 
  Portion* por = ArrayToList((gArray<double> &) *c);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_Payoff_Rational(Portion **param)
{
  OutcomeVector<gRational> *c = 
    (OutcomeVector<gRational> *) ((OutcomePortion *) param[0])->Value();
  
  Portion* por = ArrayToList((gArray<gRational> &) *c);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

//----------
// Player
//----------

Portion *GSM_Player_Infoset(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();

  Portion* por = new EfPlayerValPortion(s->GetPlayer());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_Player_Node(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  if (!n->GetPlayer())
    return new ErrorPortion("Node is a terminal node");

  Portion* por = new EfPlayerValPortion(n->GetPlayer());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

//------------
// Players
//------------

Portion *GSM_Players_Efg(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();

  Portion* p = ArrayToList(E.PlayerList());
  p->SetOwner( param[ 0 ]->Original() );
  p->AddDependency();
  return p;
}

//----------------
// PriorSibling
//----------------

Portion *GSM_PriorSibling(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value()->PriorSibling();
  if (!n)
    return new ErrorPortion("Node is the first sibling");
  
  Portion* por = new NodeValPortion(n);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
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
  Infoset *infoset = ((InfosetPortion *) param[1])->Value();
  Action *action = ((ActionPortion *) param[2])->Value();

  if (&support->BelongsTo() != infoset->BelongsTo())
    return new ErrorPortion("Support and infoset must be from same game");
  if (action->BelongsTo() != infoset)
    return new ErrorPortion("Action must belong to infoset");

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
    player[i] = ((EfPlayerPortion *) players->Subscript(i))->Value();
  
  s->BelongsTo()->Reveal(s, player);

  Portion *por = new InfosetValPortion(s);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

//-------------
// RootNode
//-------------

Portion *GSM_RootNode(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();

  Portion* por = new NodeValPortion(E.RootNode());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
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
  
  if ((s->BelongsTo()->Type() == DOUBLE && p->DataType() != porFLOAT) ||
      (s->BelongsTo()->Type() == RATIONAL && p->DataType() != porRATIONAL))
    return new ErrorPortion("Probability list does not match game type");
  if (p->Length() != s->NumActions())  
    return new ErrorPortion("Wrong number of probabilities");

  int i;

  switch (p->DataType())   {
    case porFLOAT:
      for (i = 1; i <= p->Length(); i++) 
	((ChanceInfoset<double> *) s)->SetActionProb
          (i, ((FloatPortion *) p->Subscript(i))->Value());
      break;
    case porRATIONAL:
      for (i = 1; i <= p->Length(); i++)
	((ChanceInfoset<gRational> *) s)->SetActionProb
   	  (i, ((RationalPortion *) p->Subscript(i))->Value());
      break;
  }

  Portion* por = new InfosetValPortion(s);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

//--------------
// SetName
//--------------

Portion *GSM_SetName_EfgElements(Portion **param)
{
  gString name(((TextPortion *) param[1])->Value());
  
  switch (param[0]->Type())   {
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
    default:
      assert(0);
  }

  return param[0]->ValCopy();
}

Portion *GSM_SetName_Efg(Portion **param)
{
  ((EfgPortion *) param[0])->Value()->SetTitle(((TextPortion *) param[1])->Value());
  return param[0]->RefCopy();
}

//----------------
// SetPayoff
//----------------

Portion *GSM_SetPayoff_Float(Portion **param)
{
  OutcomeVector<double> *c = 
    (OutcomeVector<double> *) ((OutcomePortion *) param[0])->Value();
  ListPortion *p = (ListPortion *) param[1];

  if (c->Length() != p->Length())
    return new ErrorPortion("Wrong number of entries in payoff vector");

  for (int i = 1; i <= c->Length(); i++)
    (*c)[i] = ((FloatPortion *) p->Subscript(i))->Value();

  Portion* por = new OutcomeValPortion(c);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_SetPayoff_Rational(Portion **param)
{
  OutcomeVector<gRational> *c = 
    (OutcomeVector<gRational> *) ((OutcomePortion *) param[0])->Value();
  ListPortion *p = (ListPortion *) param[1];

  if (c->Length() != p->Length()) 
    return new ErrorPortion("Wrong number of entries in payoff vector");

  for (int i = 1; i <= c->Length(); i++)
    (*c)[i] = ((RationalPortion *) p->Subscript(i))->Value();

  Portion* por = new OutcomeValPortion(c);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
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
  por->SetOwner(param[0]->Original());
  por->AddDependency();
  return por;  
}  

//-----------------
// TerminalNodes
//-----------------

Portion *GSM_TerminalNodes(Portion **param)
{
  BaseEfg& E = *((EfgPortion*) param[0])->Value();
  gList<Node *> list;
  TerminalNodes(E, list);

  Portion *por = ArrayToList(list);
  por->SetOwner(param[0]->Original());
  por->AddDependency();
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
  por->SetOwner(param[0]->Original());
  por->AddDependency();
  return por;
}

Portion *GSM_UnmarkSubgames_Node(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  n->BelongsTo()->UnmarkSubgames(n);
  
  gList<Node *> roots;
  MarkedSubgameRoots(*n->BelongsTo(), roots);
  
  Portion *por = ArrayToList(roots);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
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
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}



void Init_efgfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("Actions");
  FuncObj->SetFuncInfo(GSM_Actions, 1);
  FuncObj->SetParamInfo(GSM_Actions, 0, "infoset", porINFOSET);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("AddAction");
  FuncObj->SetFuncInfo(GSM_AddAction, 3);
  FuncObj->SetParamInfo(GSM_AddAction, 0, "support", porEF_SUPPORT);
  FuncObj->SetParamInfo(GSM_AddAction, 1, "infoset", porINFOSET);
  FuncObj->SetParamInfo(GSM_AddAction, 2, "action", porACTION);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("AppendAction");
  FuncObj->SetFuncInfo(GSM_AppendAction, 1);
  FuncObj->SetParamInfo(GSM_AppendAction, 0, "infoset", porINFOSET);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("AppendNode");
  FuncObj->SetFuncInfo(GSM_AppendNode, 2);
  FuncObj->SetParamInfo(GSM_AppendNode, 0, "node", porNODE);
  FuncObj->SetParamInfo(GSM_AppendNode, 1, "infoset", porINFOSET);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("AttachOutcome");
  FuncObj->SetFuncInfo(GSM_AttachOutcome, 2);
  FuncObj->SetParamInfo(GSM_AttachOutcome, 0, "node", porNODE);
  FuncObj->SetParamInfo(GSM_AttachOutcome, 1, "outcome", porOUTCOME);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Chance");
  FuncObj->SetFuncInfo(GSM_Chance, 1);
  FuncObj->SetParamInfo(GSM_Chance, 0, "efg", porEFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ChanceProbs");
  FuncObj->SetFuncInfo(GSM_ChanceProbs, 1);
  FuncObj->SetParamInfo(GSM_ChanceProbs, 0, "infoset", porINFOSET);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("CopyTree");
  FuncObj->SetFuncInfo(GSM_CopyTree, 2);
  FuncObj->SetParamInfo(GSM_CopyTree, 0, "from", porNODE);
  FuncObj->SetParamInfo(GSM_CopyTree, 1, "to", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("DeleteAction");
  FuncObj->SetFuncInfo(GSM_DeleteAction, 2);
  FuncObj->SetParamInfo(GSM_DeleteAction, 0, "infoset", porINFOSET);
  FuncObj->SetParamInfo(GSM_DeleteAction, 1, "action", porACTION);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("DeleteNode");
  FuncObj->SetFuncInfo(GSM_DeleteNode, 2);
  FuncObj->SetParamInfo(GSM_DeleteNode, 0, "node", porNODE);
  FuncObj->SetParamInfo(GSM_DeleteNode, 1, "keep", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("DeleteOutcome");
  FuncObj->SetFuncInfo(GSM_DeleteOutcome, 1);
  FuncObj->SetParamInfo(GSM_DeleteOutcome, 0, "outcome", porOUTCOME);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("DeleteTree");
  FuncObj->SetFuncInfo(GSM_DeleteTree, 1);
  FuncObj->SetParamInfo(GSM_DeleteTree, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("DetachOutcome");
  FuncObj->SetFuncInfo(GSM_DetachOutcome, 1);
  FuncObj->SetParamInfo(GSM_DetachOutcome, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Float");
  FuncObj->SetFuncInfo(GSM_FloatEfg, 1);
  FuncObj->SetParamInfo(GSM_FloatEfg, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("HasOutcome");
  FuncObj->SetFuncInfo(GSM_HasOutcome, 1);
  FuncObj->SetParamInfo(GSM_HasOutcome, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Infoset");
  FuncObj->SetFuncInfo(GSM_Infoset, 1);
  FuncObj->SetParamInfo(GSM_Infoset, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Infosets");
  FuncObj->SetFuncInfo(GSM_Infosets, 1);
  FuncObj->SetParamInfo(GSM_Infosets, 0, "player", porPLAYER_EFG);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("InsertAction");
  FuncObj->SetFuncInfo(GSM_InsertAction, 2);
  FuncObj->SetParamInfo(GSM_InsertAction, 0, "infoset", porINFOSET);
  FuncObj->SetParamInfo(GSM_InsertAction, 1, "at", porACTION);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("InsertNode");
  FuncObj->SetFuncInfo(GSM_InsertNode, 2);
  FuncObj->SetParamInfo(GSM_InsertNode, 0, "node", porNODE);
  FuncObj->SetParamInfo(GSM_InsertNode, 1, "infoset", porINFOSET);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsConstSum");
  FuncObj->SetFuncInfo(GSM_IsConstSumEfg, 1);
  FuncObj->SetParamInfo(GSM_IsConstSumEfg, 0, "efg", porEFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsPredecessor");
  FuncObj->SetFuncInfo(GSM_IsPredecessor, 2);
  FuncObj->SetParamInfo(GSM_IsPredecessor, 0, "node", porNODE);
  FuncObj->SetParamInfo(GSM_IsPredecessor, 1, "of", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsRoot");
  FuncObj->SetFuncInfo(GSM_IsRoot, 1);
  FuncObj->SetParamInfo(GSM_IsRoot, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsSuccessor");
  FuncObj->SetFuncInfo(GSM_IsSuccessor, 2);
  FuncObj->SetParamInfo(GSM_IsSuccessor, 0, "node", porNODE);
  FuncObj->SetParamInfo(GSM_IsSuccessor, 1, "from", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("JoinInfoset");
  FuncObj->SetFuncInfo(GSM_JoinInfoset, 2);
  FuncObj->SetParamInfo(GSM_JoinInfoset, 0, "infoset", porINFOSET);
  FuncObj->SetParamInfo(GSM_JoinInfoset, 1, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LastAction");
  FuncObj->SetFuncInfo( GSM_LastAction, 1 );
  FuncObj->SetParamInfo( GSM_LastAction, 0, "node", porNODE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj("LeaveInfoset");
  FuncObj->SetFuncInfo(GSM_LeaveInfoset, 1);
  FuncObj->SetParamInfo(GSM_LeaveInfoset, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LoadEfg");
  FuncObj->SetFuncInfo(GSM_LoadEfg, 1);
  FuncObj->SetParamInfo(GSM_LoadEfg, 0, "file", porTEXT);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("MarkSubgames");
  FuncObj->SetFuncInfo(GSM_MarkSubgames_Efg, 1);
  FuncObj->SetParamInfo(GSM_MarkSubgames_Efg, 0, "efg", porEFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_MarkSubgames_Node, 1);
  FuncObj->SetParamInfo(GSM_MarkSubgames_Node, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("MarkThisSubgame");
  FuncObj->SetFuncInfo(GSM_MarkThisSubgame, 1);
  FuncObj->SetParamInfo(GSM_MarkThisSubgame, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Members");
  FuncObj->SetFuncInfo(GSM_Members, 1);
  FuncObj->SetParamInfo(GSM_Members, 0, "infoset", porINFOSET);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("MergeInfosets");
  FuncObj->SetFuncInfo(GSM_MergeInfosets, 2);
  FuncObj->SetParamInfo(GSM_MergeInfosets, 0, "infoset1", porINFOSET);
  FuncObj->SetParamInfo(GSM_MergeInfosets, 1, "infoset2", porINFOSET);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("MoveTree");
  FuncObj->SetFuncInfo(GSM_MoveTree, 2);
  FuncObj->SetParamInfo(GSM_MoveTree, 0, "from", porNODE);
  FuncObj->SetParamInfo(GSM_MoveTree, 1, "to", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Name");
  FuncObj->SetFuncInfo(GSM_Name_EfgElements, 1);
  FuncObj->SetParamInfo(GSM_Name_EfgElements, 0, "x", porACTION |
		        porINFOSET | porNODE | porOUTCOME | porPLAYER_EFG);

  FuncObj->SetFuncInfo(GSM_Name_Efg, 1);
  FuncObj->SetParamInfo(GSM_Name_Efg, 0, "x", porEFG, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE); 
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NewEfg");
  FuncObj->SetFuncInfo(GSM_NewEfg, 2);
  FuncObj->SetParamInfo(GSM_NewEfg, 0, "rational", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_NewEfg, 1, "players", porLIST | porTEXT,
			new ListValPortion);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NewInfoset");
  FuncObj->SetFuncInfo(GSM_NewInfoset1, 3);
  FuncObj->SetParamInfo(GSM_NewInfoset1, 0, "player", porPLAYER_EFG);
  FuncObj->SetParamInfo(GSM_NewInfoset1, 1, "actions", porINTEGER);
  FuncObj->SetParamInfo(GSM_NewInfoset1, 2, "name", porTEXT,
			new TextValPortion(""));

  FuncObj->SetFuncInfo(GSM_NewInfoset2, 3);
  FuncObj->SetParamInfo(GSM_NewInfoset2, 0, "player", porPLAYER_EFG);
  FuncObj->SetParamInfo(GSM_NewInfoset2, 1, "actions", porLIST | porTEXT);
  FuncObj->SetParamInfo(GSM_NewInfoset2, 2, "name", porTEXT,
			new TextValPortion(""));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NewOutcome");
  FuncObj->SetFuncInfo(GSM_NewOutcome, 2);
  FuncObj->SetParamInfo(GSM_NewOutcome, 0, "efg", porEFG,
		       NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_NewOutcome, 1, "name", porTEXT,
		       new TextValPortion(""));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NewPlayer");
  FuncObj->SetFuncInfo(GSM_NewPlayer, 2);
  FuncObj->SetParamInfo(GSM_NewPlayer, 0, "efg", porEFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_NewPlayer, 1, "name", porTEXT,
			new TextValPortion(""));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NewSupport");
  FuncObj->SetFuncInfo(GSM_NewSupport_Efg, 1);
  FuncObj->SetParamInfo(GSM_NewSupport_Efg, 0, "efg", porEFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NextSibling");
  FuncObj->SetFuncInfo(GSM_NextSibling, 1);
  FuncObj->SetParamInfo(GSM_NextSibling, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Nodes");
  FuncObj->SetFuncInfo(GSM_Nodes, 1);
  FuncObj->SetParamInfo(GSM_Nodes, 0, "efg", porEFG, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NonterminalNodes");
  FuncObj->SetFuncInfo(GSM_NonterminalNodes, 1);
  FuncObj->SetParamInfo(GSM_NonterminalNodes, 0, "efg", porEFG, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NthChild");
  FuncObj->SetFuncInfo(GSM_NthChild, 2);
  FuncObj->SetParamInfo(GSM_NthChild, 0, "node", porNODE);
  FuncObj->SetParamInfo(GSM_NthChild, 1, "n", porINTEGER);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NumActions");
  FuncObj->SetFuncInfo(GSM_NumActions, 1);
  FuncObj->SetParamInfo(GSM_NumActions, 0, "infoset", porINFOSET);
  FuncObj->SetFuncInfo(GSM_NumActions_EFSupport, 2);
  FuncObj->SetParamInfo(GSM_NumActions_EFSupport, 0, "infoset", porINFOSET);
  FuncObj->SetParamInfo(GSM_NumActions_EFSupport, 1, "support", porEF_SUPPORT);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NumChildren");
  FuncObj->SetFuncInfo(GSM_NumChildren, 1);
  FuncObj->SetParamInfo(GSM_NumChildren, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("NumInfosets");
  FuncObj->SetFuncInfo(GSM_NumInfosets, 1);
  FuncObj->SetParamInfo(GSM_NumInfosets, 0, "player", porPLAYER_EFG);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NumMembers");
  FuncObj->SetFuncInfo(GSM_NumMembers, 1);
  FuncObj->SetParamInfo(GSM_NumMembers, 0, "infoset", porINFOSET);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NumNodes");
  FuncObj->SetFuncInfo(GSM_NumNodes, 1);
  FuncObj->SetParamInfo(GSM_NumNodes, 0, "efg", porEFG, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj); 

  FuncObj = new FuncDescObj("NumOutcomes");
  FuncObj->SetFuncInfo(GSM_NumOutcomes, 1);
  FuncObj->SetParamInfo(GSM_NumOutcomes, 0, "efg", porEFG, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NumPlayers");
  FuncObj->SetFuncInfo(GSM_NumPlayers_Efg, 1);
  FuncObj->SetParamInfo(GSM_NumPlayers_Efg, 0, "efg", porEFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Outcome");
  FuncObj->SetFuncInfo(GSM_Outcome, 1);
  FuncObj->SetParamInfo(GSM_Outcome, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Outcomes");
  FuncObj->SetFuncInfo(GSM_Outcomes, 1);
  FuncObj->SetParamInfo(GSM_Outcomes, 0, "efg", porEFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Parent");
  FuncObj->SetFuncInfo(GSM_Parent, 1);
  FuncObj->SetParamInfo(GSM_Parent, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Payoff");
  FuncObj->SetFuncInfo(GSM_Payoff_Float, 1);
  FuncObj->SetParamInfo(GSM_Payoff_Float, 0, "outcome", porOUTCOME_FLOAT);

  FuncObj->SetFuncInfo(GSM_Payoff_Rational, 1);
  FuncObj->SetParamInfo(GSM_Payoff_Rational, 0, "outcome", porOUTCOME_RATIONAL);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Player");
  FuncObj->SetFuncInfo(GSM_Player_Infoset, 1);
  FuncObj->SetParamInfo(GSM_Player_Infoset, 0, "infoset", porINFOSET);

  FuncObj->SetFuncInfo(GSM_Player_Node, 1);
  FuncObj->SetParamInfo(GSM_Player_Node, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Players");
  FuncObj->SetFuncInfo(GSM_Players_Efg, 1);
  FuncObj->SetParamInfo(GSM_Players_Efg, 0, "efg", porEFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("PriorSibling");
  FuncObj->SetFuncInfo(GSM_PriorSibling, 1);
  FuncObj->SetParamInfo(GSM_PriorSibling, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RandomEfg");
  FuncObj->SetFuncInfo(GSM_RandomEfg_Float, 1);
  FuncObj->SetParamInfo(GSM_RandomEfg_Float, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_RandomEfg_Rational, 1);
  FuncObj->SetParamInfo(GSM_RandomEfg_Rational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_RandomEfg_SeedFloat, 2);
  FuncObj->SetParamInfo(GSM_RandomEfg_SeedFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_RandomEfg_SeedFloat, 1, "seed", porINTEGER);

  FuncObj->SetFuncInfo(GSM_RandomEfg_SeedRational, 2);
  FuncObj->SetParamInfo(GSM_RandomEfg_SeedRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_RandomEfg_SeedRational, 1, "seed", porINTEGER);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Rational");
  FuncObj->SetFuncInfo(GSM_Rational_Efg, 1);
  FuncObj->SetParamInfo(GSM_Rational_Efg, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RemoveAction");
  FuncObj->SetFuncInfo(GSM_RemoveAction, 3);
  FuncObj->SetParamInfo(GSM_RemoveAction, 0, "support", porEF_SUPPORT);
  FuncObj->SetParamInfo(GSM_RemoveAction, 1, "infoset", porINFOSET);
  FuncObj->SetParamInfo(GSM_RemoveAction, 2, "action", porACTION);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Reveal");
  FuncObj->SetFuncInfo(GSM_Reveal, 2);
  FuncObj->SetParamInfo(GSM_Reveal, 0, "infoset", porINFOSET);
  FuncObj->SetParamInfo(GSM_Reveal, 1, "who", porLIST | porPLAYER_EFG);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RootNode");
  FuncObj->SetFuncInfo(GSM_RootNode, 1);
  FuncObj->SetParamInfo(GSM_RootNode, 0, "efg", porEFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SaveEfg");
  FuncObj->SetFuncInfo(GSM_SaveEfg, 2);
  FuncObj->SetParamInfo(GSM_SaveEfg, 0, "efg", porEFG, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SaveEfg, 1, "file", porTEXT);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetChanceProbs");
  FuncObj->SetFuncInfo(GSM_SetChanceProbs, 2);
  FuncObj->SetParamInfo(GSM_SetChanceProbs, 0, "infoset", porINFOSET);
  FuncObj->SetParamInfo(GSM_SetChanceProbs, 1, "probs",
			porLIST | porFLOAT | porRATIONAL);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetName");
  FuncObj->SetFuncInfo(GSM_SetName_EfgElements, 2);
  FuncObj->SetParamInfo(GSM_SetName_EfgElements, 0, "x", porACTION |
			porINFOSET | porNODE | porOUTCOME | porPLAYER_EFG);
  FuncObj->SetParamInfo(GSM_SetName_EfgElements, 1, "name", porTEXT);

  FuncObj->SetFuncInfo(GSM_SetName_Efg, 2);
  FuncObj->SetParamInfo(GSM_SetName_Efg, 0, "x", porEFG, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SetName_Efg, 1, "name", porTEXT);

  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetPayoff");
  FuncObj->SetFuncInfo(GSM_SetPayoff_Float, 2);
  FuncObj->SetParamInfo(GSM_SetPayoff_Float, 0, "outcome", porOUTCOME_FLOAT);
  FuncObj->SetParamInfo(GSM_SetPayoff_Float, 1, "payoff", porLIST | porFLOAT);

  FuncObj->SetFuncInfo(GSM_SetPayoff_Rational, 2);
  FuncObj->SetParamInfo(GSM_SetPayoff_Rational, 0, "outcome",
			porOUTCOME_RATIONAL);
  FuncObj->SetParamInfo(GSM_SetPayoff_Rational, 1, "payoff",
			porLIST | porRATIONAL);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SubgameRoots");
  FuncObj->SetFuncInfo(GSM_SubgameRoots, 1);
  FuncObj->SetParamInfo(GSM_SubgameRoots, 0, "efg", porEFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj( "TerminalNodes" );
  FuncObj->SetFuncInfo( GSM_TerminalNodes, 1 );
  FuncObj->SetParamInfo( GSM_TerminalNodes, 0, "efg", porEFG, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj("UnmarkSubgames");
  FuncObj->SetFuncInfo(GSM_UnmarkSubgames_Efg, 1);
  FuncObj->SetParamInfo(GSM_UnmarkSubgames_Efg, 0, "efg", porEFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_UnmarkSubgames_Node, 1);
  FuncObj->SetParamInfo(GSM_UnmarkSubgames_Node, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("UnmarkThisSubgame");
  FuncObj->SetFuncInfo(GSM_UnmarkThisSubgame, 1);
  FuncObj->SetParamInfo(GSM_UnmarkThisSubgame, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

}

