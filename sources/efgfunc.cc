//#
//# FILE: efgfunc.cc -- Extensive form editing builtins
//#
//# $Id$
//#


#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "efg.h"



//
// Utility functions for converting gArrays to ListPortions
// (perhaps these are more generally useful and should appear elsewhere?
//
Portion *ArrayToList(const gArray<double> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new FloatValPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<gRational> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new RationalValPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<Action *> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new ActionValPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<EFPlayer *> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new EfPlayerValPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<Infoset *> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new InfosetValPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<Outcome *> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new OutcomeValPortion(A[i]));
  return ret;
}

//
// Implementation of extensive form editing functions, in alpha order
//
Portion *GSM_AppendAction(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  s->BelongsTo()->AppendAction(s);

  Portion* por = new ActionValPortion(s->GetActionList()[s->NumActions()]);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_AppendNode(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  Infoset *s = ((InfosetPortion *) param[1])->Value();

  if (n->BelongsTo() != s->BelongsTo()) 
    return new ErrorPortion("Node and information set from different games\n");
  n->BelongsTo()->AppendNode(n, s);

  Portion* por = new NodeValPortion(n->GetChild(1));
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_AttachOutcome(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  Outcome *c = ((OutcomePortion *) param[1])->Value();

  if (n->BelongsTo() != c->BelongsTo())
    return new ErrorPortion("Node and outcome from different games\n");
  n->SetOutcome(c);
  
  Portion* por = new OutcomeValPortion(c);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

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

Portion *GSM_DeleteNode(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  Node *keep = ((NodePortion *) param[1])->Value();

  if (n->BelongsTo() != keep->BelongsTo())
    return new ErrorPortion("The nodes are from different games\n");
  if (keep->GetParent() != n)
    return new ErrorPortion("keep is not a child of node\n");

  Portion* por = new NodeValPortion(n->BelongsTo()->DeleteNode(n, keep));
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_DeleteOutcome(Portion **param)
{
  Outcome *outc = ((OutcomePortion *) param[0])->Value();
  outc->BelongsTo()->DeleteOutcome(outc);
  return new BoolValPortion(true);
}

Portion *GSM_DeleteTree(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  n->BelongsTo()->DeleteTree(n);

  Portion* por = new NodeValPortion(n);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_DetachOutcome(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  n->SetOutcome(0);

  Portion* por = new NodeValPortion(n);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

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

Portion *GSM_InsertNode(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  Infoset *s = ((InfosetPortion *) param[1])->Value();

  if (n->BelongsTo() != s->BelongsTo()) 
    return new ErrorPortion("Node and information set from different games\n");
  n->BelongsTo()->InsertNode(n, s);

  Portion* por = new NodeValPortion(n->GetParent());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_LeaveInfoset(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  Portion* por = new InfosetValPortion(n->BelongsTo()->LeaveInfoset(n));
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_MergeInfosets(Portion **param)
{
  Infoset *s1 = ((InfosetPortion *) param[0])->Value();
  Infoset *s2 = ((InfosetPortion *) param[1])->Value();
  
  if (s1->BelongsTo() != s2->BelongsTo())
    return new ErrorPortion("Information sets from different games\n");
  s1->BelongsTo()->MergeInfoset(s1, s2);
  
  Portion* por = new InfosetValPortion(s1);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

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


//
// The version specifying just the number of actions
//
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
 
//
// The version specifying a list of names for actions
//
Portion *GSM_NewInfoset2(Portion **param)
{
  EFPlayer *p = ((EfPlayerPortion *) param[0])->Value();
  ListPortion *actions = (ListPortion *) param[1];
  gString name = ((TextPortion *) param[2])->Value();

  if (actions->Length() == 0)
    return new ErrorPortion("Must have at least one action\n");
  Infoset *s = p->BelongsTo()->CreateInfoset(p, actions->Length());
  s->SetName(name);
  for (int i = 1; i <= actions->Length(); i++)
    s->SetActionName(i, ((TextPortion *) actions->Subscript(i))->Value());

  Portion* por = new InfosetValPortion(s);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_NewOutcomeFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();

  Outcome *c = E.NewOutcome();
  c->SetName(name);

  Portion* por = new OutcomeValPortion(c);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_NewOutcomeRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();

  Outcome *c = E.NewOutcome();
  c->SetName(name);

  Portion* por = new OutcomeValPortion(c);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

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

Portion *GSM_RandomEfgFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double> *) ((EfgPortion *) param[0])->Value();
  
  RandomEfg(E);
  return param[0]->RefCopy();
}

Portion *GSM_RandomEfgRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational> *) ((EfgPortion *) param[0])->Value();
  
  RandomEfg(E);
  return param[0]->RefCopy();
}

Portion *GSM_RandomEfgSeedFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double> *) ((EfgPortion *) param[0])->Value();
  int seed = ((IntPortion *) param[1])->Value();

  SetSeed(seed);
  RandomEfg(E);
  return param[0]->RefCopy();
}

Portion *GSM_RandomEfgSeedRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational> *) ((EfgPortion *) param[0])->Value();
  int seed = ((IntPortion *) param[1])->Value();

  SetSeed(seed);
  RandomEfg(E);
  return param[0]->RefCopy();
}  

Portion *GSM_SetChanceProbs(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  ListPortion *p = (ListPortion *) param[1];

  if (!s->GetPlayer()->IsChance())
    return new ErrorPortion("Information set does not belong to chance player\n");
  if ((s->BelongsTo()->Type() == DOUBLE && p->DataType() != porFLOAT) ||
      (s->BelongsTo()->Type() == RATIONAL && p->DataType() != porRATIONAL))
    return new ErrorPortion("Probability list does not match game type\n");
  if (p->Length() != s->NumActions())  
    return new ErrorPortion("Wrong number of probabilities\n");

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

Portion *GSM_SetNameAction(Portion **param)
{
  Action *a = ((ActionPortion *) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  a->SetName(name);
  return param[0]->ValCopy();
}

Portion *GSM_SetNameEfg(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  E.SetTitle(name);
  return param[0]->ValCopy();
}

Portion *GSM_SetNameInfoset(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  s->SetName(name);
  return param[0]->ValCopy();
}

extern Portion *GSM_SetNameNfg(Portion **);

Portion *GSM_SetNameNode(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  n->SetName(name);
  return param[0]->ValCopy();
}

Portion *GSM_SetNameOutcome(Portion **param)
{
  Outcome *c = ((OutcomePortion *) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  c->SetName(name);
  return param[0]->ValCopy();
}

Portion *GSM_SetNamePlayer(Portion **param)
{
  EFPlayer *p = ((EfPlayerPortion *) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  p->SetName(name);
  return param[0]->ValCopy();
}

Portion *GSM_SetPayoffFloat(Portion **param)
{
  OutcomeVector<double> *c = 
    (OutcomeVector<double> *) ((OutcomePortion *) param[0])->Value();
  ListPortion *p = (ListPortion *) param[1];

  if (c->Length() != p->Length())
    return new ErrorPortion("Wrong number of entries in payoff vector\n");

  for (int i = 1; i <= c->Length(); i++)
    (*c)[i] = ((FloatPortion *) p->Subscript(i))->Value();

  Portion* por = new OutcomeValPortion(c);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_SetPayoffRational(Portion **param)
{
  OutcomeVector<gRational> *c = 
    (OutcomeVector<gRational> *) ((OutcomePortion *) param[0])->Value();
  ListPortion *p = (ListPortion *) param[1];

  if (c->Length() != p->Length()) 
    return new ErrorPortion("Wrong number of entries in payoff vector\n");

  for (int i = 1; i <= c->Length(); i++)
    (*c)[i] = ((RationalPortion *) p->Subscript(i))->Value();

  Portion* por = new OutcomeValPortion(c);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

//
// Implementation of extensive form query functions, in alpha order
//

Portion *GSM_Actions(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();

  Portion* por = ArrayToList(s->GetActionList());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_CentroidEfgFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();
  BehavProfile<double> *P = new BehavProfile<double>(E);

  Portion* por = new BehavValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_CentroidEfgRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();
  BehavProfile<gRational> *P = new BehavProfile<gRational>(E);

  Portion* por = new BehavValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

extern Portion *GSM_CentroidNfgFloat(Portion **);
extern Portion *GSM_CentroidNfgRational(Portion **);
extern Portion *GSM_CentroidNFSupport(Portion **);

Portion *GSM_Chance(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();

  Portion* por = new EfPlayerValPortion(E.GetChance());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_ChanceProbs(Portion **param)
{
  Portion* por;
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  if (!s->GetPlayer()->IsChance()) 
    return new ErrorPortion("Only chance information sets have action probabilities\n");

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

Portion *GSM_CopyTree(Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();

  if (n1->BelongsTo() != n2->BelongsTo())
    return new ErrorPortion("n1 and n2 belong to different trees\n");

  Portion* por = new NodeValPortion(n1->BelongsTo()->CopyTree(n1, n2));
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_Infoset(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  if (!n->GetInfoset())
    return new ErrorPortion("Terminal nodes have no information set\n");
  Portion* por = new InfosetValPortion(n->GetInfoset());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_Infosets(Portion **param)
{
  EFPlayer *p = ((EfPlayerPortion *) param[0])->Value();

  Portion* por = ArrayToList(p->InfosetList());
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

Portion *GSM_IsPredecessor(Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();
  return new BoolValPortion(n1->BelongsTo()->IsPredecessor(n1, n2));
}

Portion *GSM_IsRoot(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  return new BoolValPortion(n->GetParent() == 0);
}

Portion *GSM_IsSuccessor(Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();
  return new BoolValPortion(n1->BelongsTo()->IsSuccessor(n1, n2));
}

Portion *GSM_MoveTree(Portion **param)
{
  Node *n1 = ((NodePortion *) param[0])->Value();
  Node *n2 = ((NodePortion *) param[1])->Value();

  if (n1->BelongsTo() != n2->BelongsTo())
    return new ErrorPortion("n1 and n2 belong to different trees\n");

  Portion* por = new NodeValPortion(n1->BelongsTo()->MoveTree(n1, n2));
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_NameAction(Portion **param)
{
  Action *a = ((ActionPortion *) param[0])->Value();
  
  Portion* por = new TextValPortion(a->GetName());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_NameEfg(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();
  return new TextValPortion(E.GetTitle());
}

Portion *GSM_NamePlayer(Portion **param)
{
  EFPlayer *p = ((EfPlayerPortion *) param[0])->Value();
  return new TextValPortion(p->GetName());
}

extern Portion *GSM_NameNfg(Portion **);

Portion *GSM_NameNode(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  return new TextValPortion(n->GetName());
}

Portion *GSM_NameInfoset(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  return new TextValPortion(s->GetName());
}

Portion *GSM_NameOutcome(Portion **param)
{
  Outcome *c = ((OutcomePortion *) param[0])->Value();
  return new TextValPortion(c->GetName());
}

Portion *GSM_NextSibling(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value()->NextSibling();
  if (!n)
    return new ErrorPortion("Node is the last sibling\n");
  
  Portion* por = new NodeValPortion(n);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_NthChild(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  int child = ((IntPortion *) param[1])->Value();
  if (child < 1 || child > n->NumChildren())  
    return new ErrorPortion("Child number out of range\n");

  Portion* por = new NodeValPortion(n->GetChild(child));
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_NumActions(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  return new IntValPortion(s->NumActions());
}

Portion *GSM_NumChildren(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  return new IntValPortion(n->NumChildren());
}

Portion *GSM_NumInfosets(Portion **param)
{
  EFPlayer *p = ((EfPlayerPortion *) param[0])->Value();
  return new IntValPortion(p->NumInfosets());
}

Portion *GSM_NumMembers(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  return new IntValPortion(s->NumMembers());
}

Portion *GSM_NumOutcomes(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();
  return new IntValPortion(E.NumOutcomes());
}

Portion *GSM_NumPlayersEfg(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();
  return new IntValPortion(E.NumPlayers());
}

// This function currently lives in with the normal form stuff...
extern Portion*GSM_NumPlayersNfg(Portion **);


Portion *GSM_Outcome(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  if (!n->GetOutcome())
    return new ErrorPortion("No outcome attached to node\n");

  Portion* por = new OutcomeValPortion(n->GetOutcome());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_Outcomes(Portion **param)
{
  BaseEfg *E = ((EfgPortion*) param[0])->Value();
  
  Portion* por = ArrayToList(E->OutcomeList());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_Parent(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  if (!n->GetParent())
    return new ErrorPortion("Node is the root node\n");

  Portion* por = new NodeValPortion(n->GetParent());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_PayoffFloat(Portion **param)
{
  OutcomeVector<double> *c = 
    (OutcomeVector<double> *) ((OutcomePortion *) param[0])->Value();
 
  Portion* por = ArrayToList((gArray<double> &) *c);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_PayoffRational(Portion **param)
{
  OutcomeVector<gRational> *c = 
    (OutcomeVector<gRational> *) ((OutcomePortion *) param[0])->Value();
  
  Portion* por = ArrayToList((gArray<gRational> &) *c);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_PlayerInfoset(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();

  Portion* por = new EfPlayerValPortion(s->GetPlayer());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_PlayerNode(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  if (!n->GetPlayer())
    return new ErrorPortion("Node is a terminal node\n");

  Portion* por = new EfPlayerValPortion(n->GetPlayer());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_PlayersEfg(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();

  Portion* p = ArrayToList(E.PlayerList());
  p->SetOwner( param[ 0 ]->Original() );
  p->AddDependency();
  return p;
}

extern Portion *GSM_PlayersNfg(Portion **);

Portion *GSM_PriorSibling(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value()->PriorSibling();
  if (!n)
    return new ErrorPortion("Node is the first sibling\n");
  
  Portion* por = new NodeValPortion(n);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion *GSM_ReadEfg(Portion **param)
{
  gInput &f = ((InputPortion *) param[0])->Value();

  if (f.IsValid())   {
    DataType type;
    bool valid;

    EfgFileType(f, valid, type);
    if (!valid)   return new ErrorPortion("Not a valid .efg file\n");
    
    switch (type)   {
      case DOUBLE:  {
	Efg<double> *E = 0;
	ReadEfgFile((gInput &) f, E);

	if (E)
	  return new EfgValPortion(E);
	else
	  return new ErrorPortion("Not a valid .efg file\n");
      }
      case RATIONAL:   {
	Efg<gRational> *E = 0;
	ReadEfgFile((gInput &) f, E);
	
	if (E)
	  return new EfgValPortion(E);
	else
	  return new ErrorPortion("Not a valid .efg file\n");
      }
      default:
	assert(0);
	return 0;
    }
  }
  else
    return new ErrorPortion("Unable to open file for reading\n");
}

Portion *GSM_RootNode(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();

  Portion* por = new NodeValPortion(E.RootNode());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_WriteEfg(Portion **param)
{
  Portion* result;
  gOutput &f = ((OutputPortion *) param[0])->Value();
  BaseEfg *E = ((EfgPortion*) param[1])->Value();
  
  E->WriteEfgFile(f);

  result = param[ 0 ];
  param[ 0 ] = 0;
  return result;
}

void Init_efgfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

//-----------------------------------------------------------

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
  FuncObj->SetParamInfo(GSM_DeleteOutcome, 0, "infoset", porINFOSET);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("DeleteTree");
  FuncObj->SetFuncInfo(GSM_DeleteTree, 1);
  FuncObj->SetParamInfo(GSM_DeleteTree, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("DetachOutcome");
  FuncObj->SetFuncInfo(GSM_DetachOutcome, 1);
  FuncObj->SetParamInfo(GSM_DetachOutcome, 0, "node", porNODE);
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

  FuncObj = new FuncDescObj("LeaveInfoset");
  FuncObj->SetFuncInfo(GSM_LeaveInfoset, 1);
  FuncObj->SetParamInfo(GSM_LeaveInfoset, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("MergeInfosets");
  FuncObj->SetFuncInfo(GSM_MergeInfosets, 2);
  FuncObj->SetParamInfo(GSM_MergeInfosets, 0, "infoset1", porINFOSET);
  FuncObj->SetParamInfo(GSM_MergeInfosets, 1, "infoset2", porINFOSET);
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
  FuncObj->SetFuncInfo(GSM_NewOutcomeFloat, 2);
  FuncObj->SetParamInfo(GSM_NewOutcomeFloat, 0, "efg", porEFG_FLOAT,
		       NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_NewOutcomeFloat, 1, "name", porTEXT,
		       new TextValPortion(""));

  FuncObj->SetFuncInfo(GSM_NewOutcomeRational, 2);
  FuncObj->SetParamInfo(GSM_NewOutcomeRational, 0, "efg", porEFG_RATIONAL,
		       NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_NewOutcomeRational, 1, "name", porTEXT,
		       new TextValPortion(""));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NewPlayer");
  FuncObj->SetFuncInfo(GSM_NewPlayer, 2);
  FuncObj->SetParamInfo(GSM_NewPlayer, 0, "efg", porEFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_NewPlayer, 1, "name", porTEXT,
			new TextValPortion(""));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RandomEfg");
  FuncObj->SetFuncInfo(GSM_RandomEfgFloat, 1);
  FuncObj->SetParamInfo(GSM_RandomEfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_RandomEfgRational, 1);
  FuncObj->SetParamInfo(GSM_RandomEfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_RandomEfgSeedFloat, 2);
  FuncObj->SetParamInfo(GSM_RandomEfgSeedFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_RandomEfgSeedFloat, 1, "seed", porINTEGER);

  FuncObj->SetFuncInfo(GSM_RandomEfgSeedRational, 2);
  FuncObj->SetParamInfo(GSM_RandomEfgSeedRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_RandomEfgSeedRational, 1, "seed", porINTEGER);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetChanceProbs");
  FuncObj->SetFuncInfo(GSM_SetChanceProbs, 2);
  FuncObj->SetParamInfo(GSM_SetChanceProbs, 0, "infoset", porINFOSET);
  FuncObj->SetParamInfo(GSM_SetChanceProbs, 1, "probs",
			porLIST | porFLOAT | porRATIONAL);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetName");
  FuncObj->SetFuncInfo(GSM_SetNameAction, 2);
  FuncObj->SetParamInfo(GSM_SetNameAction, 0, "x", porACTION);
  FuncObj->SetParamInfo(GSM_SetNameAction, 1, "name", porTEXT);

  FuncObj->SetFuncInfo(GSM_SetNameEfg, 2);
  FuncObj->SetParamInfo(GSM_SetNameEfg, 0, "x", porEFG, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SetNameEfg, 1, "name", porTEXT);

  FuncObj->SetFuncInfo(GSM_SetNameInfoset, 2);
  FuncObj->SetParamInfo(GSM_SetNameInfoset, 0, "x", porINFOSET);
  FuncObj->SetParamInfo(GSM_SetNameInfoset, 1, "name", porTEXT);

  FuncObj->SetFuncInfo(GSM_SetNameNfg, 2);
  FuncObj->SetParamInfo(GSM_SetNameNfg, 0, "x", porNFG, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SetNameNfg, 1, "name", porTEXT);

  FuncObj->SetFuncInfo(GSM_SetNameNode, 2);
  FuncObj->SetParamInfo(GSM_SetNameNode, 0, "x", porNODE);
  FuncObj->SetParamInfo(GSM_SetNameNode, 1, "name", porTEXT);

  FuncObj->SetFuncInfo(GSM_SetNameOutcome, 2);
  FuncObj->SetParamInfo(GSM_SetNameOutcome, 0, "x", porOUTCOME);
  FuncObj->SetParamInfo(GSM_SetNameOutcome, 1, "name", porTEXT);

  FuncObj->SetFuncInfo(GSM_SetNamePlayer, 2);
  FuncObj->SetParamInfo(GSM_SetNamePlayer, 0, "x", porPLAYER_EFG);
  FuncObj->SetParamInfo(GSM_SetNamePlayer, 1, "name", porTEXT);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetPayoff");
  FuncObj->SetFuncInfo(GSM_SetPayoffFloat, 2);
  FuncObj->SetParamInfo(GSM_SetPayoffFloat, 0, "outcome", porOUTCOME_FLOAT);
  FuncObj->SetParamInfo(GSM_SetPayoffFloat, 1, "payoffs", porLIST | porFLOAT);

  FuncObj->SetFuncInfo(GSM_SetPayoffRational, 2);
  FuncObj->SetParamInfo(GSM_SetPayoffRational, 0, "outcome",
			porOUTCOME_RATIONAL);
  FuncObj->SetParamInfo(GSM_SetPayoffRational, 1, "payoffs",
			porLIST | porRATIONAL);
  gsm->AddFunction(FuncObj);

//-----------------------------------------------------------

  FuncObj = new FuncDescObj("Actions");
  FuncObj->SetFuncInfo(GSM_Actions, 1);
  FuncObj->SetParamInfo(GSM_Actions, 0, "infoset", porINFOSET);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Centroid");
  FuncObj->SetFuncInfo(GSM_CentroidEfgFloat, 1);
  FuncObj->SetParamInfo(GSM_CentroidEfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_CentroidEfgRational, 1);
  FuncObj->SetParamInfo(GSM_CentroidEfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_CentroidNfgFloat, 1);
  FuncObj->SetParamInfo(GSM_CentroidNfgFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_CentroidNFSupport, 1);
  FuncObj->SetParamInfo(GSM_CentroidNFSupport, 0, "support",
			porNF_SUPPORT);

  FuncObj->SetFuncInfo(GSM_CentroidNfgRational, 1);
  FuncObj->SetParamInfo(GSM_CentroidNfgRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
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

  FuncObj = new FuncDescObj("Infoset");
  FuncObj->SetFuncInfo(GSM_Infoset, 1);
  FuncObj->SetParamInfo(GSM_Infoset, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Infosets");
  FuncObj->SetFuncInfo(GSM_Infosets, 1);
  FuncObj->SetParamInfo(GSM_Infosets, 0, "player", porPLAYER_EFG);
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

  FuncObj = new FuncDescObj("MoveTree");
  FuncObj->SetFuncInfo(GSM_MoveTree, 2);
  FuncObj->SetParamInfo(GSM_MoveTree, 0, "from", porNODE);
  FuncObj->SetParamInfo(GSM_MoveTree, 1, "to", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Name");
  FuncObj->SetFuncInfo(GSM_NameAction, 1);
  FuncObj->SetParamInfo(GSM_NameAction, 0, "x", porACTION);

  FuncObj->SetFuncInfo(GSM_NameEfg, 1);
  FuncObj->SetParamInfo(GSM_NameEfg, 0, "x", porEFG, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE ); 

  FuncObj->SetFuncInfo(GSM_NamePlayer, 1);
  FuncObj->SetParamInfo(GSM_NamePlayer, 0, "x", porPLAYER_EFG);

  FuncObj->SetFuncInfo(GSM_NameNfg, 1);
  FuncObj->SetParamInfo(GSM_NameNfg, 0, "x", porNFG, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );

  FuncObj->SetFuncInfo(GSM_NameNode, 1);
  FuncObj->SetParamInfo(GSM_NameNode, 0, "x", porNODE);

  FuncObj->SetFuncInfo(GSM_NameInfoset, 1);
  FuncObj->SetParamInfo(GSM_NameInfoset, 0, "x", porINFOSET);

  FuncObj->SetFuncInfo(GSM_NameOutcome, 1);
  FuncObj->SetParamInfo(GSM_NameOutcome, 0, "x", porOUTCOME);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NextSibling");
  FuncObj->SetFuncInfo(GSM_NextSibling, 1);
  FuncObj->SetParamInfo(GSM_NextSibling, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NthChild");
  FuncObj->SetFuncInfo(GSM_NthChild, 2);
  FuncObj->SetParamInfo(GSM_NthChild, 0, "node", porNODE);
  FuncObj->SetParamInfo(GSM_NthChild, 1, "n", porINTEGER);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NumActions");
  FuncObj->SetFuncInfo(GSM_NumActions, 1);
  FuncObj->SetParamInfo(GSM_NumActions, 0, "infoset", porINFOSET);
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

  FuncObj = new FuncDescObj("NumOutcomes");
  FuncObj->SetFuncInfo(GSM_NumOutcomes, 1);
  FuncObj->SetParamInfo(GSM_NumOutcomes, 0, "efg", porEFG, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NumPlayers");
  FuncObj->SetFuncInfo(GSM_NumPlayersEfg, 1);
  FuncObj->SetParamInfo(GSM_NumPlayersEfg, 0, "efg", porEFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetFuncInfo(GSM_NumPlayersNfg, 1);
  FuncObj->SetParamInfo(GSM_NumPlayersNfg, 0, "nfg", porNFG,
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
  FuncObj->SetFuncInfo(GSM_PayoffFloat, 1);
  FuncObj->SetParamInfo(GSM_PayoffFloat, 0, "outcome", porOUTCOME_FLOAT);

  FuncObj->SetFuncInfo(GSM_PayoffRational, 1);
  FuncObj->SetParamInfo(GSM_PayoffRational, 0, "outcome", porOUTCOME_RATIONAL);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Player");
  FuncObj->SetFuncInfo(GSM_PlayerInfoset, 1);
  FuncObj->SetParamInfo(GSM_PlayerInfoset, 0, "infoset", porINFOSET);

  FuncObj->SetFuncInfo(GSM_PlayerNode, 1);
  FuncObj->SetParamInfo(GSM_PlayerNode, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Players");
  FuncObj->SetFuncInfo(GSM_PlayersEfg, 1);
  FuncObj->SetParamInfo(GSM_PlayersEfg, 0, "efg", porEFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_PlayersNfg, 1);
  FuncObj->SetParamInfo(GSM_PlayersNfg, 0, "nfg", porNFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("PriorSibling");
  FuncObj->SetFuncInfo(GSM_PriorSibling, 1);
  FuncObj->SetParamInfo(GSM_PriorSibling, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ReadEfg");
  FuncObj->SetFuncInfo(GSM_ReadEfg, 1);
  FuncObj->SetParamInfo(GSM_ReadEfg, 0, "file", porINPUT);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RootNode");
  FuncObj->SetFuncInfo(GSM_RootNode, 1);
  FuncObj->SetParamInfo(GSM_RootNode, 0, "efg", porEFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("WriteEfg");
  FuncObj->SetFuncInfo(GSM_WriteEfg, 2);
  FuncObj->SetParamInfo(GSM_WriteEfg, 0, "output", porOUTPUT);
  FuncObj->SetParamInfo(GSM_WriteEfg, 1, "efg", porEFG, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  gsm->AddFunction(FuncObj);
}


