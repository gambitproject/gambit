//#
//# FILE: efgfunc.cc -- Extensive form editing builtins
//#
//# $Id$
//#


#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "efg.h"
#include "efgutils.h"





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

Portion *ArrayToList(const gArray<Node *> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NodeValPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gList<Node *> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NodeValPortion(A[i]));
  return ret;
}

Portion* gDPVectorToList(const gDPVector<double>& A)
{
  ListPortion* p;
  ListPortion* s1;
  ListPortion* s2;
  p = new ListValPortion();
  int l = 1;
  for (int i = 1; i <= A.DPLengths().Length(); i++)  {
    s1 = new ListValPortion();
    for (int j = 1; j <= A.DPLengths()[i]; j++)  {
      s2 = new ListValPortion();
      for (int k = 1; k <= A.Lengths()[l]; k++)
	s2->Append(new FloatValPortion(A(i, j, k)));
      l++;
      s1->Append(s2);
    }
    p->Append(s1);
  }
  return p;
}

Portion* gDPVectorToList(const gDPVector<gRational>& A)
{
  ListPortion* p;
  ListPortion* s1;
  ListPortion* s2;
  p = new ListValPortion();
  int l = 1;
  for (int i = 1; i <= A.DPLengths().Length(); i++)  {
    s1 = new ListValPortion();
    for (int j = 1; j <= A.DPLengths()[i]; j++)  {
      s2 = new ListValPortion();
      for (int k = 1; k <= A.Lengths()[l]; k++)
	s2->Append(new RationalValPortion(A(i, j, k)));
      l++;
      s1->Append(s2);
    }
    p->Append(s1);
  }
  return p;
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
    return new ErrorPortion("Node and information set from different games");
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
    return new ErrorPortion("Node and outcome from different games");
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
    return new ErrorPortion("The nodes are from different games");
  if (keep->GetParent() != n)
    return new ErrorPortion("keep is not a child of node");

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

Portion *GSM_UnmarkThisSubgame(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  n->BelongsTo()->RemoveSubgame(n);
  
  Portion *por = new NodeValPortion(n);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
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

Portion *GSM_MarkSubgamesEfg(Portion **param)
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

Portion *GSM_MarkSubgamesNode(Portion **param)
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

Portion *GSM_UnmarkSubgamesEfg(Portion **param)
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

Portion *GSM_UnmarkSubgamesNode(Portion **param)
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

Portion *GSM_HasOutcome(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();
  return new BoolValPortion( n->GetOutcome() != 0 );
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
    return new ErrorPortion("Node and information set from different games");
  n->BelongsTo()->InsertNode(n, s);

  Portion* por = new NodeValPortion(n->GetParent());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

Portion* GSM_LastAction( Portion** param )
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
    return new ErrorPortion("Information sets from different games");
  s1->BelongsTo()->MergeInfoset(s1, s2);
  
  Portion* por = new InfosetValPortion(s1);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

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

Portion *GSM_MarkThisSubgame(Portion **param)
{
  Node *n = ((NodePortion *) param[0])->Value();

  return new BoolValPortion(n->BelongsTo()->DefineSubgame(n));
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

extern Efg<gRational> *ConvertEfg(const Efg<double> &);

Portion *GSM_RationalEfg(Portion **param)
{
  Efg<double> &orig = * (Efg<double> *) ((EfgPortion *) param[0])->Value();
  Efg<gRational> *E = ConvertEfg(orig);

  if (E)
    return new EfgValPortion(E);
  else
    return new ErrorPortion("Conversion failed.");
}
   

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



//------------------------- SetName -----------------------------//

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

Portion *GSM_SetNameNfPlayer(Portion **param)
{
  NFPlayer *p = ((NfPlayerPortion *) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  p->SetName(name);
  return param[0]->ValCopy();
}

Portion *GSM_SetNameStrategy(Portion **param)
{
  Strategy *s = ((StrategyPortion *) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  s->name = name;
  return param[0]->ValCopy();
}




Portion *GSM_SetPayoffFloat(Portion **param)
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

Portion *GSM_SetPayoffRational(Portion **param)
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


Portion *GSM_SetPayoff_NodeFloat(Portion **param)
{
  Portion* element;
  Node* node = ( (NodePortion*) param[ 0 ] )->Value();

  /*
  if (node->GetOutcome())
    return new ErrorPortion("An outcome is already attached to the node");
    */

  BaseEfg* efg = ( (EfgPortion*) param[ 0 ]->Owner() )->Value();
  if( efg->Type() != DOUBLE )
    return new ErrorPortion( "Type mismatch between node and list" );

  ListPortion *p = (ListPortion *) param[1];
  if (efg->NumPlayers() != p->Length())
    return new ErrorPortion("Wrong number of entries in payoff vector");

  OutcomeVector<double>* c = ( (Efg<double>*) efg )->NewOutcome();

  for (int i = 1; i <= c->Length(); i++)
  {
    element = p->Subscript(i);
    (*c)[i] = ((FloatPortion *) element)->Value();
    delete element;
  }

  node->SetOutcome( c );

  Portion* por = new OutcomeValPortion(c);
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}


Portion *GSM_SetPayoff_NodeRational(Portion **param)
{
  Portion* element;
  Node* node = ( (NodePortion*) param[ 0 ] )->Value();

  /*
  if (node->GetOutcome())
    return new ErrorPortion("An outcome is already attached to the node");
    */

  BaseEfg* efg = ( (EfgPortion*) param[ 0 ]->Owner() )->Value();
  if( efg->Type() != RATIONAL )
    return new ErrorPortion( "Type mismatch between node and list" );

  ListPortion *p = (ListPortion *) param[1];
  if (efg->NumPlayers() != p->Length())
    return new ErrorPortion("Wrong number of entries in payoff vector");

  OutcomeVector<gRational>* c = ( (Efg<gRational>*) efg )->NewOutcome();

  for (int i = 1; i <= c->Length(); i++)
  {
    element = p->Subscript(i);
    (*c)[i] = ((RationalPortion *) element)->Value();
    delete element;
  }

  node->SetOutcome( c );

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


//----------------------- AddAction -----------------------//

Portion* GSM_AddAction( Portion** param )
{  
  int pl;
  int iset;
  bool result = false;
  EFSupport *support = ((EfSupportPortion *) param[0])->Value();
  Infoset *infoset = ((InfosetPortion *) param[1])->Value();
  Action *action = ((ActionPortion *) param[2])->Value();

  for( pl = 1; pl <= support->NumPlayers(); pl++ )
    for( iset = 1; iset <= support->NumInfosets( pl ); iset++ )
      if( support->GetPlayer( pl ).InfosetList()[ iset ] == infoset )
      {
	support->AddAction( pl, iset, action );
	result = true;
	break;
      }

  if( !result )
    return new ErrorPortion( "Infoset not found in the given Support");
  else
    return param[0]->RefCopy();
}


//------------------------ RemoveAction ----------------------------//

Portion* GSM_RemoveAction( Portion** param )
{  
  int pl;
  int iset;
  bool result = false;
  EFSupport *support = ((EfSupportPortion *) param[0])->Value();
  Infoset *infoset = ((InfosetPortion *) param[1])->Value();
  Action *action = ((ActionPortion *) param[2])->Value();

  for( pl = 1; pl <= support->NumPlayers(); pl++ )
    for( iset = 1; iset <= support->NumInfosets( pl ); iset++ )
      if( support->GetPlayer( pl ).InfosetList()[ iset ] == infoset )
      {
	result = support->RemoveAction( pl, iset, action );
	break;
      }

  if( !result )
    return new ErrorPortion( "Action not in the given Support and Infoset");
  else
    return param[0]->RefCopy();
}

//----------------------- Centroid ----------------------//

Portion *GSM_CentroidEfgFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();
  BehavSolution<double> *P = new BehavSolution<double>(E);

  Portion* por = new BehavValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_CentroidEfgRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();
  BehavSolution<gRational> *P = new BehavSolution<gRational>(E);

  Portion* por = new BehavValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_CentroidEFSupport(Portion **param)
{
  EFSupport *S = ((EfSupportPortion *) param[0])->Value();
  BaseBehavProfile *P;

  if (S->BelongsTo().Type() == DOUBLE)
    P = new BehavSolution<double>( *S );
  else
    P = new BehavSolution<gRational>( *S );

  Portion *por = new BehavValPortion(P);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}


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

Portion *GSM_Infosets(Portion **param)
{
  EFPlayer *p = ((EfPlayerPortion *) param[0])->Value();

  Portion* por = ArrayToList(p->InfosetList());
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

Portion *GSM_IsConstSumEfg(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();
  return new BoolValPortion(E.IsConstSum());
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

Portion *GSM_Members(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();

  Portion* por = ArrayToList(s->GetMemberList());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}

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



//----------------------------- Name ------------------------//

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

Portion* GSM_NameNfPlayer( Portion** param )
{
  NFPlayer *p = ( (NfPlayerPortion*) param[ 0 ] )->Value();
  return new TextValPortion( p->GetName() );
}

Portion* GSM_NameStrategy( Portion** param )
{
  Strategy *s = ( (StrategyPortion*) param[ 0 ] )->Value();
  return new TextValPortion( s->name );
}


//------------------- NewSupport -------------------------//

Portion *GSM_NewEFSupport(Portion **param)
{
  BaseEfg &E = * ((EfgPortion *) param[0])->Value();
  Portion *p = new EfSupportValPortion(new EFSupport(E));

  p->SetOwner( param[ 0 ]->Original() );
  p->AddDependency();
  return p;
}




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



//---------------------- NumActions --------------------------//

Portion *GSM_NumActions(Portion **param)
{
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  return new IntValPortion(s->NumActions());
}

Portion *GSM_NumActions_EFSupport(Portion **param)
{
  int result = 0;
  int pl;
  int iset;
  Infoset *s = ((InfosetPortion *) param[0])->Value();
  EFSupport *S = ((EfSupportPortion *) param[1])->Value();
  for( pl = 1; pl <= S->NumPlayers() && result == 0; pl++ )
    for( iset = 1; iset <= S->NumInfosets( pl ) && result == 0; iset++ )
      if( S->GetPlayer( pl ).InfosetList()[ iset ] == s )
	result = S->NumActions( pl, iset );
      
  return new IntValPortion( result );
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
    return new ErrorPortion("Node is the root node");

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
    return new ErrorPortion("Node is a terminal node");

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

Portion *GSM_RootNode(Portion **param)
{
  BaseEfg &E = *((EfgPortion*) param[0])->Value();

  Portion* por = new NodeValPortion(E.RootNode());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

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


//---------------------- Node lists ---------------------------

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


Portion* GSM_NumNodes(Portion ** param)
{
  BaseEfg& E = *((EfgPortion*) param[0])->Value();
  return new IntValPortion(NumNodes(E));  
}

//--------------------------- List ---------------------------//

Portion *GSM_List_BehavFloat(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p1;
  Portion* p2;
  Portion* p3;
  Portion* por;

  BehavSolution<double> *P = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();

  por = new ListValPortion();

  for( i = 1; i <= P->DPLengths().Length(); i++ )
  {
    p1 = new ListValPortion();

    for( j = 1; j <= P->DPLengths()[i]; j++ )
    {
      p2 = new ListValPortion();

      for( k = 1; k <= P->Lengths()[j]; k++ )
      {
	p3 = new FloatValPortion( (*P)( i, j, k ) );
	((ListPortion*) p2)->Append( p3 );
      }
      ((ListPortion*) p1)->Append( p2 );
    }
    ((ListPortion*) por)->Append( p1 );
  }

  return por;

}


Portion *GSM_List_BehavRational(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p1;
  Portion* p2;
  Portion* p3;
  Portion* por;

  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ((BehavPortion*) param[0])->Value();

  por = new ListValPortion();

  for( i = 1; i <= P->DPLengths().Length(); i++ )
  {
    p1 = new ListValPortion();

    for( j = 1; j <= P->DPLengths()[i]; j++ )
    {
      p2 = new ListValPortion();

      for( k = 1; k <= P->Lengths()[j]; k++ )
      {
	p3 = new RationalValPortion( (*P)( i, j, k ) );
	((ListPortion*) p2)->Append( p3 );
      }
      ((ListPortion*) p1)->Append( p2 );
    }
    ((ListPortion*) por)->Append( p1 );
  }

  return por;

}






//----------------------- Behav -------------------------//


Portion *GSM_Behav_EfgFloat(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p1;
  Portion* p2;
  Portion* p3;

  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();
  BehavSolution<double> *P = new BehavSolution<double>(E);

  if( ( (ListPortion*) param[1] )->Length() != E.NumPlayers() )
  {
    delete P;
    return new ErrorPortion( "Mismatching number of players" );
  }
  
  for( i = 1; i <= E.NumPlayers(); i++ )
  {
    p1 = ( (ListPortion*) param[1] )->Subscript( i );
    if( p1->Type() != porLIST )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching dimensionality" );
    }
    if( ( (ListPortion*) p1 )->Length() != E.PlayerList()[i]->NumInfosets() )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching number of infosets" );
    }

    for( j = 1; j <= E.PlayerList()[i]->NumInfosets(); j++ )
    {
      p2 = ( (ListPortion*) p1 )->Subscript( j );
      if( p2->Type() != porLIST )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      if( ( (ListPortion*) p2 )->Length() !=
	 E.PlayerList()[i]->InfosetList()[j]->NumActions() )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching number of actions" );
      }

      for( k = 1; k <= E.PlayerList()[i]->InfosetList()[j]->NumActions(); k++ )
      {
	p3 = ( (ListPortion*) p2 )->Subscript( k );
	if( p3->Type() != porFLOAT )
	{
	  delete p3;
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion( "Mismatching dimensionality" );
	}
      
	(*P)( i, j, k ) = ( (FloatPortion*) p3 )->Value();

	delete p3;
      }
      delete p2;
    }
    delete p1;
  }

  Portion* por = new BehavValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;

}

Portion *GSM_Behav_EfgRational(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p1;
  Portion* p2;
  Portion* p3;

  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();
  BehavSolution<gRational> *P = new BehavSolution<gRational>(E);

  if( ( (ListPortion*) param[1] )->Length() != E.NumPlayers() )
  {
    delete P;
    return new ErrorPortion( "Mismatching number of players" );
  }
  
  for( i = 1; i <= E.NumPlayers(); i++ )
  {
    p1 = ( (ListPortion*) param[1] )->Subscript( i );
    if( p1->Type() != porLIST )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching dimensionality" );
    }
    if( ( (ListPortion*) p1 )->Length() != E.PlayerList()[i]->NumInfosets() )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching number of infosets" );
    }

    for( j = 1; j <= E.PlayerList()[i]->NumInfosets(); j++ )
    {
      p2 = ( (ListPortion*) p1 )->Subscript( j );
      if( p2->Type() != porLIST )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      if( ( (ListPortion*) p2 )->Length() !=
	 E.PlayerList()[i]->InfosetList()[j]->NumActions() )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching number of actions" );
      }

      for( k = 1; k <= E.PlayerList()[i]->InfosetList()[j]->NumActions(); k++ )
      {
	p3 = ( (ListPortion*) p2 )->Subscript( k );
	if( p3->Type() != porRATIONAL )
	{
	  delete p3;
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion( "Mismatching dimensionality" );
	}
      
	(*P)( i, j, k ) = ( (RationalPortion*) p3 )->Value();

	delete p3;
      }
      delete p2;
    }
    delete p1;
  }

  Portion* por = new BehavValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}


Portion *GSM_Behav_EFSupport(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p1;
  Portion* p2;
  Portion* p3;
  Portion* por = 0;


  EFSupport *S = ((EfSupportPortion *) param[0])->Value();

  // This is incredibly redundent; must find a way to reuse the code
  // from the previous two functions.
  if (S->BelongsTo().Type() == DOUBLE && param[1]->Type() & porFLOAT )
  {
    // The code here is completely copied from GSM_Behav_EfgFloat

    Efg<double> &E = * (Efg<double>*) &S->BelongsTo();
    BehavSolution<double> *P = new BehavSolution<double>(E);

    if( ( (ListPortion*) param[1] )->Length() != E.NumPlayers() )
    {
      delete P;
      return new ErrorPortion( "Mismatching number of players" );
    }
    
    for( i = 1; i <= E.NumPlayers(); i++ )
    {
      p1 = ( (ListPortion*) param[1] )->Subscript( i );
      if( p1->Type() != porLIST )
      {
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      if( ( (ListPortion*) p1 )->Length() != E.PlayerList()[i]->NumInfosets() )
      {
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching number of infosets" );
      }
      
      for( j = 1; j <= E.PlayerList()[i]->NumInfosets(); j++ )
      {
	p2 = ( (ListPortion*) p1 )->Subscript( j );
	if( p2->Type() != porLIST )
	{
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion( "Mismatching dimensionality" );
	}
	if( ( (ListPortion*) p2 )->Length() !=
	   E.PlayerList()[i]->InfosetList()[j]->NumActions() )
	{
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion( "Mismatching number of actions" );
	}
	
	for( k = 1; k <= E.PlayerList()[i]->InfosetList()[j]->NumActions(); k++ )
	{
	  p3 = ( (ListPortion*) p2 )->Subscript( k );
	  if( p3->Type() != porFLOAT )
	  {
	    delete p3;
	    delete p2;
	    delete p1;
	    delete P;
	    return new ErrorPortion( "Mismatching dimensionality" );
	  }
	  
	  (*P)( i, j, k ) = ( (FloatPortion*) p3 )->Value();
	  
	  delete p3;
	}
	delete p2;
      }
      delete p1;
    }
    por = new BehavValPortion(P);


  }
  else if (S->BelongsTo().Type()== RATIONAL && param[1]->Type() & porRATIONAL )
  {
    // The code here is entirely copied from GSM_Behav_EfgRational()

    Efg<gRational> &E = * (Efg<gRational>*) &S->BelongsTo();
    BehavSolution<gRational> *P = new BehavSolution<gRational>(E);
    
    if( ( (ListPortion*) param[1] )->Length() != E.NumPlayers() )
    {
      delete P;
      return new ErrorPortion( "Mismatching number of players" );
    }
    
    for( i = 1; i <= E.NumPlayers(); i++ )
    {
      p1 = ( (ListPortion*) param[1] )->Subscript( i );
      if( p1->Type() != porLIST )
      {
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      if( ( (ListPortion*) p1 )->Length() != E.PlayerList()[i]->NumInfosets() )
      {
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching number of infosets" );
      }
      
      for( j = 1; j <= E.PlayerList()[i]->NumInfosets(); j++ )
      {
	p2 = ( (ListPortion*) p1 )->Subscript( j );
	if( p2->Type() != porLIST )
	{
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion( "Mismatching dimensionality" );
	}
	if( ( (ListPortion*) p2 )->Length() !=
	   E.PlayerList()[i]->InfosetList()[j]->NumActions() )
	{
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion( "Mismatching number of actions" );
	}
	
	for( k = 1; k <= E.PlayerList()[i]->InfosetList()[j]->NumActions(); k++ )
	{
	  p3 = ( (ListPortion*) p2 )->Subscript( k );
	  if( p3->Type() != porRATIONAL )
	  {
	    delete p3;
	    delete p2;
	    delete p1;
	    delete P;
	    return new ErrorPortion( "Mismatching dimensionality" );
	  }
	  
	  (*P)( i, j, k ) = ( (RationalPortion*) p3 )->Value();
	  
	  delete p3;
	}
	delete p2;
      }
      delete p1;
    }

    por = new BehavValPortion(P);
  }

  if( por == 0 )
    por = new ErrorPortion( "Mismatching EFG and list type" );
  else
  {
    por->SetOwner(param[0]->Owner());
    por->AddDependency();
  }
  return por;
}


//---------------------- SetComponent -------------------//


Portion *GSM_SetComponent_BehavFloat(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p3;
  int PlayerNum = 0;
  int InfosetNum = 0;
  
  BehavSolution<double>* P = 
    (BehavSolution<double>*) ( (BehavPortion*) param[ 0 ] )->Value();
  Efg<double>& E = * P->BelongsTo();
  gArray< EFPlayer* > player = E.PlayerList();
  
  for( i = 1; i <= E.NumPlayers(); i++ )
  {
    for( j = 1; j <= E.PlayerList()[ i ]->NumInfosets(); j++ )
    {
      if( ( (InfosetPortion*) param[ 1 ] )->Value() ==
	 E.PlayerList()[ i ]->InfosetList()[ j ] )
      {
	PlayerNum = i;
	InfosetNum = j;
	break;
      }
    }
  }
  
  if( !InfosetNum )
    return new ErrorPortion( "No such infoset found" );

  if( ( (ListPortion*) param[ 2 ] )->Length() != 
     E.PlayerList()[PlayerNum]->InfosetList()[InfosetNum]->NumActions() )
    return new ErrorPortion( "Mismatching number of actions" );
  
  for( k = 1; 
      k <= E.PlayerList()[PlayerNum]->InfosetList()[InfosetNum]->NumActions();
      k++ )
  {
    p3 = ( (ListPortion*) param[ 2 ] )->Subscript( k );
    if( p3->Type() == porLIST )
    {
      delete p3;
      return new ErrorPortion( "Mismatching dimensionality" );
    }

    assert( p3->Type() == porFLOAT );
    (*P)( PlayerNum, InfosetNum, k ) = ( (FloatPortion*) p3 )->Value();

    delete p3;
  }

  return param[ 0 ]->RefCopy();
}


Portion *GSM_SetComponent_BehavRational(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p3;
  int PlayerNum = 0;
  int InfosetNum = 0;
  
  BehavSolution<gRational>* P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  Efg<gRational>& E = * P->BelongsTo();
  gArray< EFPlayer* > player = E.PlayerList();
  
  for( i = 1; i <= E.NumPlayers(); i++ )
  {
    for( j = 1; j <= E.PlayerList()[ i ]->NumInfosets(); j++ )
    {
      if( ( (InfosetPortion*) param[ 1 ] )->Value() ==
	 E.PlayerList()[ i ]->InfosetList()[ j ] )
      {
	PlayerNum = i;
	InfosetNum = j;
	break;
      }
    }
  }
  
  if( !InfosetNum )
    return new ErrorPortion( "No such infoset found" );

  if( ( (ListPortion*) param[ 2 ] )->Length() != 
     E.PlayerList()[PlayerNum]->InfosetList()[InfosetNum]->NumActions() )
    return new ErrorPortion( "Mismatching number of actions" );
  
  for( k = 1; 
      k <= E.PlayerList()[PlayerNum]->InfosetList()[InfosetNum]->NumActions();
      k++ )
  {
    p3 = ( (ListPortion*) param[ 2 ] )->Subscript( k );
    if( p3->Type() == porLIST )
    {
      delete p3;
      return new ErrorPortion( "Mismatching dimensionality" );
    }

    assert( p3->Type() == porRATIONAL );
    (*P)( PlayerNum, InfosetNum, k ) = ( (RationalPortion*) p3 )->Value();

    delete p3;
  }

  return param[ 0 ]->RefCopy();
}




//-------------------- BehavSolution data members --------------------//

Portion *GSM_IsNash_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new IntValPortion(P->IsNash());
}

Portion *GSM_IsNash_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new IntValPortion(P->IsNash());
}

Portion *GSM_IsSubgamePerfect_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new IntValPortion(P->IsSubgamePerfect());
}

Portion *GSM_IsSubgamePerfect_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new IntValPortion(P->IsSubgamePerfect());
}

Portion *GSM_IsSequential_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new IntValPortion(P->IsSequential());
}

Portion *GSM_IsSequential_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new IntValPortion(P->IsSequential());
}

Portion* GSM_Support_BehavFloat(Portion** param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new EfSupportValPortion(new EFSupport(P->Support()));
}

Portion* GSM_Support_BehavRational(Portion** param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new EfSupportValPortion(new EFSupport(P->Support()));
}

/*
Portion* GSM_GobitLambda_BehavFloat(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new FloatValPortion( bs->GobitLambda() );
}

Portion* GSM_GobitLambda_BehavRational(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new RationalValPortion( bs->GobitLambda() );
}

Portion* GSM_GobitValue_BehavFloat(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new FloatValPortion( bs->GobitValue() );
}

Portion* GSM_GobitValue_BehavRational(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new RationalValPortion( bs->GobitValue() );
}
*/

Portion *GSM_LiapValue_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new FloatValPortion(P->LiapValue());
}

Portion *GSM_LiapValue_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ( (BehavPortion*) param[ 0 ] )->Value();
  return new RationalValPortion(P->LiapValue());
}

Portion *GSM_ActionValuesFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  Infoset *s = ((InfosetPortion *) param[1])->Value();

  if (s->BelongsTo() != bp->BelongsTo())
    return new ErrorPortion("Solution and infoset must belong to same game");
  
  if (s->GetPlayer()->IsChance())
    return new ErrorPortion("Infoset must belong to personal player");

  Efg<double> *E = bp->BelongsTo();

  gDPVector<double> values(E->Dimensionality());
  gPVector<double> probs(E->Dimensionality().Lengths());

  // E->CondPayoff(*bp, values, probs);
  bp->CondPayoff(values, probs);
  
  gVector<double> ret(s->NumActions());
  for (int i = 1; i <= s->NumActions(); i++)
    ret[i] = values(s->GetPlayer()->GetNumber(), s->GetNumber(), i);

  return ArrayToList(ret);
}

Portion *GSM_ActionValuesRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  Infoset *s = ((InfosetPortion *) param[1])->Value();

  if (s->BelongsTo() != bp->BelongsTo())
    return new ErrorPortion("Solution and infoset must belong to same game");
  
  if (s->GetPlayer()->IsChance())
    return new ErrorPortion("Infoset must belong to personal player");

  Efg<gRational> *E = bp->BelongsTo();

  gDPVector<gRational> values(E->Dimensionality());
  gPVector<gRational> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);
  
  gVector<gRational> ret(s->NumActions());
  for (int i = 1; i <= s->NumActions(); i++)
    ret[i] = values(s->GetPlayer()->GetNumber(), s->GetNumber(), i);

  return ArrayToList(ret);
}

Portion *GSM_BeliefsFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  return gDPVectorToList(bp->Beliefs());
}

Portion *GSM_BeliefsRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  return gDPVectorToList(bp->Beliefs());
}

Portion *GSM_InfosetProbsFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();

  Efg<double> *E = bp->BelongsTo();

  gDPVector<double> values(E->Dimensionality());
  gPVector<double> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);

  ListPortion *ret = new ListValPortion;

  for (int i = 1; i <= E->NumPlayers(); i++)
    ret->Append(ArrayToList(probs.GetRow(i)));

  return ret;
}

Portion *GSM_InfosetProbsRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();

  Efg<gRational> *E = bp->BelongsTo();

  gDPVector<gRational> values(E->Dimensionality());
  gPVector<gRational> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);

  ListPortion *ret = new ListValPortion;

  for (int i = 1; i <= E->NumPlayers(); i++)
    ret->Append(ArrayToList(probs.GetRow(i)));

  return ret;
}

Portion *GSM_NodeValuesFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();

  if (bp->BelongsTo() != p->BelongsTo())
    return new ErrorPortion("Solution and player are from different games");

  return ArrayToList(bp->NodeValues(p->GetNumber()));
}

Portion *GSM_NodeValuesRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();

  if (bp->BelongsTo() != p->BelongsTo())
    return new ErrorPortion("Solution and player are from different games");

  return ArrayToList(bp->NodeValues(p->GetNumber()));
}
 
Portion *GSM_RealizProbsFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  
  return ArrayToList(bp->NodeRealizProbs());
}  
  
Portion *GSM_RealizProbsRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  
  return ArrayToList(bp->NodeRealizProbs());
}
  
  



//--------------------------------------------------------------//

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
  FuncObj->SetParamInfo(GSM_DeleteOutcome, 0, "outcome", porOUTCOME);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("UnmarkThisSubgame");
  FuncObj->SetFuncInfo(GSM_UnmarkThisSubgame, 1);
  FuncObj->SetParamInfo(GSM_UnmarkThisSubgame, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("DeleteTree");
  FuncObj->SetFuncInfo(GSM_DeleteTree, 1);
  FuncObj->SetParamInfo(GSM_DeleteTree, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("DetachOutcome");
  FuncObj->SetFuncInfo(GSM_DetachOutcome, 1);
  FuncObj->SetParamInfo(GSM_DetachOutcome, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("MarkSubgames");
  FuncObj->SetFuncInfo(GSM_MarkSubgamesEfg, 1);
  FuncObj->SetParamInfo(GSM_MarkSubgamesEfg, 0, "efg", porEFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_MarkSubgamesNode, 1);
  FuncObj->SetParamInfo(GSM_MarkSubgamesNode, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("UnmarkSubgames");
  FuncObj->SetFuncInfo(GSM_UnmarkSubgamesEfg, 1);
  FuncObj->SetParamInfo(GSM_UnmarkSubgamesEfg, 0, "efg", porEFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_UnmarkSubgamesNode, 1);
  FuncObj->SetParamInfo(GSM_UnmarkSubgamesNode, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("HasOutcome");
  FuncObj->SetFuncInfo(GSM_HasOutcome, 1);
  FuncObj->SetParamInfo(GSM_HasOutcome, 0, "node", porNODE);
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

  FuncObj = new FuncDescObj( "LastAction" );
  FuncObj->SetFuncInfo( GSM_LastAction, 1 );
  FuncObj->SetParamInfo( GSM_LastAction, 0, "node", porNODE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj("LeaveInfoset");
  FuncObj->SetFuncInfo(GSM_LeaveInfoset, 1);
  FuncObj->SetParamInfo(GSM_LeaveInfoset, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("MergeInfosets");
  FuncObj->SetFuncInfo(GSM_MergeInfosets, 2);
  FuncObj->SetParamInfo(GSM_MergeInfosets, 0, "infoset1", porINFOSET);
  FuncObj->SetParamInfo(GSM_MergeInfosets, 1, "infoset2", porINFOSET);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("JoinInfoset");
  FuncObj->SetFuncInfo(GSM_JoinInfoset, 2);
  FuncObj->SetParamInfo(GSM_JoinInfoset, 0, "infoset", porINFOSET);
  FuncObj->SetParamInfo(GSM_JoinInfoset, 1, "node", porNODE);
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

  FuncObj = new FuncDescObj("MarkThisSubgame");
  FuncObj->SetFuncInfo(GSM_MarkThisSubgame, 1);
  FuncObj->SetParamInfo(GSM_MarkThisSubgame, 0, "node", porNODE);
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

  FuncObj = new FuncDescObj("Float");
  FuncObj->SetFuncInfo(GSM_FloatEfg, 1);
  FuncObj->SetParamInfo(GSM_FloatEfg, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Rational");
  FuncObj->SetFuncInfo(GSM_RationalEfg, 1);
  FuncObj->SetParamInfo(GSM_RationalEfg, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Reveal");
  FuncObj->SetFuncInfo(GSM_Reveal, 2);
  FuncObj->SetParamInfo(GSM_Reveal, 0, "infoset", porINFOSET);
  FuncObj->SetParamInfo(GSM_Reveal, 1, "who", porLIST | porPLAYER_EFG);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetChanceProbs");
  FuncObj->SetFuncInfo(GSM_SetChanceProbs, 2);
  FuncObj->SetParamInfo(GSM_SetChanceProbs, 0, "infoset", porINFOSET);
  FuncObj->SetParamInfo(GSM_SetChanceProbs, 1, "probs",
			porLIST | porFLOAT | porRATIONAL);
  gsm->AddFunction(FuncObj);



  //------------------------- SetName ----------------------------//

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

  FuncObj->SetFuncInfo(GSM_SetNameNode, 2);
  FuncObj->SetParamInfo(GSM_SetNameNode, 0, "x", porNODE);
  FuncObj->SetParamInfo(GSM_SetNameNode, 1, "name", porTEXT);

  FuncObj->SetFuncInfo(GSM_SetNameOutcome, 2);
  FuncObj->SetParamInfo(GSM_SetNameOutcome, 0, "x", porOUTCOME);
  FuncObj->SetParamInfo(GSM_SetNameOutcome, 1, "name", porTEXT);

  FuncObj->SetFuncInfo(GSM_SetNamePlayer, 2);
  FuncObj->SetParamInfo(GSM_SetNamePlayer, 0, "x", porPLAYER_EFG);
  FuncObj->SetParamInfo(GSM_SetNamePlayer, 1, "name", porTEXT);

  FuncObj->SetFuncInfo(GSM_SetNameNfPlayer, 2);
  FuncObj->SetParamInfo(GSM_SetNameNfPlayer, 0, "x", porPLAYER_NFG);
  FuncObj->SetParamInfo(GSM_SetNameNfPlayer, 1, "name", porTEXT);

  FuncObj->SetFuncInfo(GSM_SetNameStrategy, 2);
  FuncObj->SetParamInfo(GSM_SetNameStrategy, 0, "x", porSTRATEGY);
  FuncObj->SetParamInfo(GSM_SetNameStrategy, 1, "name", porTEXT);

  gsm->AddFunction(FuncObj);


  //----------------------------------------------------------//

  FuncObj = new FuncDescObj("SetPayoff");
  FuncObj->SetFuncInfo(GSM_SetPayoffFloat, 2);
  FuncObj->SetParamInfo(GSM_SetPayoffFloat, 0, "outcome", porOUTCOME_FLOAT);
  FuncObj->SetParamInfo(GSM_SetPayoffFloat, 1, "payoff", porLIST | porFLOAT);

  FuncObj->SetFuncInfo(GSM_SetPayoffRational, 2);
  FuncObj->SetParamInfo(GSM_SetPayoffRational, 0, "outcome",
			porOUTCOME_RATIONAL);
  FuncObj->SetParamInfo(GSM_SetPayoffRational, 1, "payoff",
			porLIST | porRATIONAL);

  FuncObj->SetFuncInfo( GSM_SetPayoff_NodeFloat, 2 );
  FuncObj->SetParamInfo( GSM_SetPayoff_NodeFloat, 0, "node", porNODE );
  FuncObj->SetParamInfo(GSM_SetPayoff_NodeFloat, 
			1, "payoff", porLIST | porFLOAT);

  FuncObj->SetFuncInfo( GSM_SetPayoff_NodeRational, 2 );
  FuncObj->SetParamInfo( GSM_SetPayoff_NodeRational, 0, "node", porNODE );
  FuncObj->SetParamInfo(GSM_SetPayoff_NodeRational, 
			1, "payoff", porLIST | porRATIONAL);

  gsm->AddFunction(FuncObj);

  //-----------------------------------------------------------// 

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

  FuncObj = new FuncDescObj("RemoveAction");
  FuncObj->SetFuncInfo(GSM_RemoveAction, 3);
  FuncObj->SetParamInfo(GSM_RemoveAction, 0, "support", porEF_SUPPORT);
  FuncObj->SetParamInfo(GSM_RemoveAction, 1, "infoset", porINFOSET);
  FuncObj->SetParamInfo(GSM_RemoveAction, 2, "action", porACTION);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Centroid");
  FuncObj->SetFuncInfo(GSM_CentroidEfgFloat, 1);
  FuncObj->SetParamInfo(GSM_CentroidEfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_CentroidEfgRational, 1);
  FuncObj->SetParamInfo(GSM_CentroidEfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_CentroidEFSupport, 1);
  FuncObj->SetParamInfo(GSM_CentroidEFSupport, 0, "support",
			porEF_SUPPORT);

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

  FuncObj = new FuncDescObj("Members");
  FuncObj->SetFuncInfo(GSM_Members, 1);
  FuncObj->SetParamInfo(GSM_Members, 0, "infoset", porINFOSET);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("MoveTree");
  FuncObj->SetFuncInfo(GSM_MoveTree, 2);
  FuncObj->SetParamInfo(GSM_MoveTree, 0, "from", porNODE);
  FuncObj->SetParamInfo(GSM_MoveTree, 1, "to", porNODE);
  gsm->AddFunction(FuncObj);


  //-------------------------- Name ------------------------------//

  FuncObj = new FuncDescObj("Name");

  FuncObj->SetFuncInfo(GSM_NameAction, 1);
  FuncObj->SetParamInfo(GSM_NameAction, 0, "x", porACTION);

  FuncObj->SetFuncInfo(GSM_NameEfg, 1);
  FuncObj->SetParamInfo(GSM_NameEfg, 0, "x", porEFG, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE ); 

  FuncObj->SetFuncInfo(GSM_NamePlayer, 1);
  FuncObj->SetParamInfo(GSM_NamePlayer, 0, "x", porPLAYER_EFG);

  FuncObj->SetFuncInfo(GSM_NameNode, 1);
  FuncObj->SetParamInfo(GSM_NameNode, 0, "x", porNODE);

  FuncObj->SetFuncInfo(GSM_NameInfoset, 1);
  FuncObj->SetParamInfo(GSM_NameInfoset, 0, "x", porINFOSET);

  FuncObj->SetFuncInfo(GSM_NameOutcome, 1);
  FuncObj->SetParamInfo(GSM_NameOutcome, 0, "x", porOUTCOME);

  FuncObj->SetFuncInfo(GSM_NameNfPlayer, 1);
  FuncObj->SetParamInfo(GSM_NameNfPlayer, 0, "x", porPLAYER_NFG);

  FuncObj->SetFuncInfo(GSM_NameStrategy, 1);
  FuncObj->SetParamInfo(GSM_NameStrategy, 0, "x", porSTRATEGY);

  gsm->AddFunction(FuncObj);


  //---------------------- NewSupport ----------------------//

  FuncObj = new FuncDescObj( "NewSupport" );
  FuncObj->SetFuncInfo(GSM_NewEFSupport, 1);
  FuncObj->SetParamInfo(GSM_NewEFSupport, 0, "efg", porEFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NextSibling");
  FuncObj->SetFuncInfo(GSM_NextSibling, 1);
  FuncObj->SetParamInfo(GSM_NextSibling, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj( "Nodes" );
  FuncObj->SetFuncInfo( GSM_Nodes, 1 );
  FuncObj->SetParamInfo( GSM_Nodes, 0, "efg", porEFG, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  gsm->AddFunction( FuncObj );

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

  FuncObj = new FuncDescObj( "NonterminalNodes" );
  FuncObj->SetFuncInfo( GSM_NonterminalNodes, 1 );
  FuncObj->SetParamInfo( GSM_NonterminalNodes, 0, "efg", porEFG, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( "NumNodes" );
  FuncObj->SetFuncInfo( GSM_NumNodes, 1 );
  FuncObj->SetParamInfo( GSM_NumNodes, 0, "efg", porEFG, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  gsm->AddFunction( FuncObj );



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

  FuncObj = new FuncDescObj("NumOutcomes");
  FuncObj->SetFuncInfo(GSM_NumOutcomes, 1);
  FuncObj->SetParamInfo(GSM_NumOutcomes, 0, "efg", porEFG, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NumPlayers");
  FuncObj->SetFuncInfo(GSM_NumPlayersEfg, 1);
  FuncObj->SetParamInfo(GSM_NumPlayersEfg, 0, "efg", porEFG,
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
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("PriorSibling");
  FuncObj->SetFuncInfo(GSM_PriorSibling, 1);
  FuncObj->SetParamInfo(GSM_PriorSibling, 0, "node", porNODE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LoadEfg");
  FuncObj->SetFuncInfo(GSM_LoadEfg, 1);
  FuncObj->SetParamInfo(GSM_LoadEfg, 0, "file", porTEXT);
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

  

  //--------------------- Behav -------------------------//
 
  FuncObj = new FuncDescObj( "Behav" );
  FuncObj->SetFuncInfo( GSM_Behav_EfgFloat, 2 );
  FuncObj->SetParamInfo( GSM_Behav_EfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo( GSM_Behav_EfgFloat, 
			1, "list", porLIST | porFLOAT,
			NO_DEFAULT_VALUE, PASS_BY_VALUE, 3);

  FuncObj->SetFuncInfo( GSM_Behav_EfgRational, 2 );
  FuncObj->SetParamInfo( GSM_Behav_EfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo( GSM_Behav_EfgRational, 
			1, "list", porLIST | porRATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_VALUE, 3);

  FuncObj->SetFuncInfo( GSM_Behav_EFSupport, 2 );
  FuncObj->SetParamInfo( GSM_Behav_EFSupport, 0, "support", porEF_SUPPORT,
			NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Behav_EFSupport, 
			1, "list", porLIST | porFLOAT | porRATIONAL );

  gsm->AddFunction(FuncObj);

  //--------------------- SetComponent -------------------//
  
  FuncObj = new FuncDescObj( "SetComponent" );

  FuncObj->SetFuncInfo( GSM_SetComponent_BehavFloat, 3 );
  FuncObj->SetParamInfo( GSM_SetComponent_BehavFloat,
			0, "behav", porBEHAV_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo( GSM_SetComponent_BehavFloat,
			1, "infoset", porINFOSET );
  FuncObj->SetParamInfo( GSM_SetComponent_BehavFloat,
			2, "list", porLIST | porFLOAT );

  FuncObj->SetFuncInfo( GSM_SetComponent_BehavRational, 3 );
  FuncObj->SetParamInfo( GSM_SetComponent_BehavRational,
			0, "behav", porBEHAV_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo( GSM_SetComponent_BehavRational,
			1, "infoset", porINFOSET );
  FuncObj->SetParamInfo( GSM_SetComponent_BehavRational,
			2, "list", porLIST | porRATIONAL );

  gsm->AddFunction( FuncObj );



  //------------------------- BehavSolution member functions ----------//

  //----------------------- IsNash ------------------------//

  FuncObj = new FuncDescObj("IsNash");
  FuncObj->SetFuncInfo(GSM_IsNash_BehavFloat, 1);
  FuncObj->SetParamInfo(GSM_IsNash_BehavFloat, 0, "strategy",
			porBEHAV_FLOAT, NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetFuncInfo(GSM_IsNash_BehavRational, 1);
  FuncObj->SetParamInfo(GSM_IsNash_BehavRational, 0, "strategy",
			porBEHAV_RATIONAL, NO_DEFAULT_VALUE,PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsSubgamePerfect");
  FuncObj->SetFuncInfo(GSM_IsSubgamePerfect_BehavFloat, 1);
  FuncObj->SetParamInfo(GSM_IsSubgamePerfect_BehavFloat, 0, "strategy",
			porBEHAV_FLOAT, NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetFuncInfo(GSM_IsSubgamePerfect_BehavRational, 1);
  FuncObj->SetParamInfo(GSM_IsSubgamePerfect_BehavRational, 0, "strategy",
			porBEHAV_RATIONAL, NO_DEFAULT_VALUE,PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsSequential");
  FuncObj->SetFuncInfo(GSM_IsSequential_BehavFloat, 1);
  FuncObj->SetParamInfo(GSM_IsSequential_BehavFloat, 0, "strategy",
			porBEHAV_FLOAT, NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetFuncInfo(GSM_IsSequential_BehavRational, 1);
  FuncObj->SetParamInfo(GSM_IsSequential_BehavRational, 0, "strategy",
			porBEHAV_RATIONAL, NO_DEFAULT_VALUE,PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Support");
  FuncObj->SetFuncInfo(GSM_Support_BehavFloat, 1);
  FuncObj->SetParamInfo(GSM_Support_BehavFloat, 0, "strategy",
			porBEHAV_FLOAT, NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetFuncInfo(GSM_Support_BehavRational, 1);
  FuncObj->SetParamInfo(GSM_Support_BehavRational, 0, "strategy",
			porBEHAV_RATIONAL, NO_DEFAULT_VALUE,PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LiapValue");
  FuncObj->SetFuncInfo(GSM_LiapValue_BehavFloat, 1);
  FuncObj->SetParamInfo(GSM_LiapValue_BehavFloat, 0, "strategy",
			porBEHAV_FLOAT, NO_DEFAULT_VALUE, PASS_BY_REFERENCE);  
  FuncObj->SetFuncInfo(GSM_LiapValue_BehavRational, 1);
  FuncObj->SetParamInfo(GSM_LiapValue_BehavRational, 0, "strategy",
			porBEHAV_RATIONAL, NO_DEFAULT_VALUE,PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);



  //----------------------- List --------------------------//
  FuncObj = new FuncDescObj( "ListForm" );
  FuncObj->SetFuncInfo( GSM_List_BehavFloat, 1 );
  FuncObj->SetParamInfo(GSM_List_BehavFloat, 
			0, "behav", porBEHAV_FLOAT );
  FuncObj->SetFuncInfo( GSM_List_BehavRational, 1 );
  FuncObj->SetParamInfo(GSM_List_BehavRational, 
			0, "behav", porBEHAV_RATIONAL );
  gsm->AddFunction( FuncObj );


  FuncObj = new FuncDescObj("ActionValues");
  FuncObj->SetFuncInfo(GSM_ActionValuesFloat, 2);
  FuncObj->SetParamInfo(GSM_ActionValuesFloat, 0, "strategy",
			porBEHAV_FLOAT);
  FuncObj->SetParamInfo(GSM_ActionValuesFloat, 1, "infoset", porINFOSET);

  FuncObj->SetFuncInfo(GSM_ActionValuesRational, 2);
  FuncObj->SetParamInfo(GSM_ActionValuesRational, 0, "strategy",
			porBEHAV_RATIONAL);
  FuncObj->SetParamInfo(GSM_ActionValuesRational, 1, "infoset", porINFOSET);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Beliefs");
  FuncObj->SetFuncInfo(GSM_BeliefsFloat, 1);
  FuncObj->SetParamInfo(GSM_BeliefsFloat, 0, "strategy", 
			porBEHAV_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_BeliefsRational, 1);
  FuncObj->SetParamInfo(GSM_BeliefsRational, 0, "strategy",
			porBEHAV_RATIONAL, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("InfosetProbs");
  FuncObj->SetFuncInfo(GSM_InfosetProbsFloat, 1);
  FuncObj->SetParamInfo(GSM_InfosetProbsFloat, 0, "strategy", porBEHAV_FLOAT);

  FuncObj->SetFuncInfo(GSM_InfosetProbsRational, 1);
  FuncObj->SetParamInfo(GSM_InfosetProbsRational, 0, "strategy",
			porBEHAV_RATIONAL);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NodeValues");
  FuncObj->SetFuncInfo(GSM_NodeValuesFloat, 2);
  FuncObj->SetParamInfo(GSM_NodeValuesFloat, 0, "strategy", porBEHAV_FLOAT);
  FuncObj->SetParamInfo(GSM_NodeValuesFloat, 1, "player", porPLAYER_EFG);

  FuncObj->SetFuncInfo(GSM_NodeValuesRational, 2);
  FuncObj->SetParamInfo(GSM_NodeValuesRational, 0, "strategy",
			porBEHAV_RATIONAL);
  FuncObj->SetParamInfo(GSM_NodeValuesRational, 1, "player", porPLAYER_EFG);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RealizProbs");
  FuncObj->SetFuncInfo(GSM_RealizProbsFloat, 1);
  FuncObj->SetParamInfo(GSM_RealizProbsFloat, 0, "strategy", porBEHAV_FLOAT);

  FuncObj->SetFuncInfo(GSM_RealizProbsRational, 1);
  FuncObj->SetParamInfo(GSM_RealizProbsRational, 0, "strategy",
			porBEHAV_RATIONAL);
  gsm->AddFunction(FuncObj);

}

