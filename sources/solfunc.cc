//
// FILE: solfunc.cc -- GCL functions on profiles and solutions
//
// $Id$
//

#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "efg.h"
#include "nfg.h"
#include "behavsol.h"
#include "mixedsol.h"

#include "nfplayer.h"

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


//----------------
// ActionProb
//----------------

static Portion *GSM_ActionProb_Float(Portion **param)
{
  BehavSolution<double>* profile = 
    (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  Action* action = ((ActionPortion*) param[1])->Value();
  Infoset* infoset = action->BelongsTo();
  EFPlayer* player = infoset->GetPlayer();
  
  if (player->IsChance())
    return new FloatValPortion(((ChanceInfoset<double> *) infoset)->
			       GetActionProbs()[action->GetNumber()]);  
  else if (profile->GetEFSupport().Find(action))
    return new FloatValPortion((*profile)
			       (player->GetNumber(),
				infoset->GetNumber(),
				profile->GetEFSupport().Find(action)));
  else
    return new FloatValPortion(0.0);
}

static Portion *GSM_ActionProb_Rational(Portion **param)
{
  BehavSolution<gRational>* profile = 
    (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  Action* action = ((ActionPortion*) param[1])->Value();
  Infoset* infoset = action->BelongsTo();
  EFPlayer* player = infoset->GetPlayer();
  
  if (player->IsChance())
    return new RationalValPortion(((ChanceInfoset<gRational> *) infoset)->
				  GetActionProbs()[action->GetNumber()]);  
  else if (profile->GetEFSupport().Find(action))
    return new RationalValPortion((*profile)(player->GetNumber(),
					     infoset->GetNumber(),
					     profile->GetEFSupport().Find(action)));
  else
    return new RationalValPortion(0);
}

//----------------
// ActionProbs
//----------------

static Portion *GSM_ActionProbs_Float(Portion **param)
{
  BehavSolution<double> *profile =
    (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  const EFSupport *support = &profile->GetEFSupport();
  const BaseEfg &efg = support->BelongsTo();

  ListPortion *por = new ListValPortion;
  for (int pl = 1; pl <= efg.NumPlayers(); pl++)  {
    EFPlayer *player = efg.PlayerList()[pl];
    ListPortion *p1 = new ListValPortion;
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      Infoset *infoset = player->InfosetList()[iset];
      ListPortion *p2 = new ListValPortion;
      for (int act = 1; act <= infoset->NumActions(); act++)   {
	if (support->Find(infoset->GetActionList()[act]))
	  p2->Append(new FloatValPortion(
	   	       (*profile)(player->GetNumber(),
				  infoset->GetNumber(),
				  support->Find(infoset->GetActionList()[act]))));
	else
	  p2->Append(new FloatValPortion(0.0));
      }
      p1->Append(p2);
    }
    por->Append(p1);
  }

  return por;
}

static Portion *GSM_ActionProbs_Rational(Portion **param)
{
  BehavSolution<gRational> *profile =
    (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  const EFSupport *support = &profile->GetEFSupport();
  const BaseEfg &efg = support->BelongsTo();

  ListPortion *por = new ListValPortion;
  for (int pl = 1; pl <= efg.NumPlayers(); pl++)  {
    EFPlayer *player = efg.PlayerList()[pl];
    ListPortion *p1 = new ListValPortion;
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      Infoset *infoset = player->InfosetList()[iset];
      ListPortion *p2 = new ListValPortion;
      for (int act = 1; act <= infoset->NumActions(); act++)   {
	if (support->Find(infoset->GetActionList()[act]))
	  p2->Append(new RationalValPortion(
	   	       (*profile)(player->GetNumber(),
				  infoset->GetNumber(),
				  support->Find(infoset->GetActionList()[act]))));
	else
	  p2->Append(new RationalValPortion(0.0));
      }
      p1->Append(p2);
    }
    por->Append(p1);
  }

  return por;
}



//------------------
// ActionValue
//------------------

static Portion *GSM_ActionValue_Float(Portion **param)
{
  BehavSolution<double> *profile =
    (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  Action* action = ((ActionPortion*) param[1])->Value();
  Infoset *infoset = action->BelongsTo();

  if (infoset->GetPlayer()->IsChance())
    return new NullPortion(porFLOAT);
  else if (profile->GetEFSupport().Find(action))  {
    Efg<double> *efg = profile->BelongsTo();

    gDPVector<double> values(efg->Dimensionality());
    gPVector<double> probs(efg->Dimensionality().Lengths());

    profile->CondPayoff(values, probs);
  
    if (probs(infoset->GetPlayer()->GetNumber(),
	      infoset->GetNumber()) > 0.0)
      return new FloatValPortion(values(infoset->GetPlayer()->GetNumber(), 
					infoset->GetNumber(),
					profile->GetEFSupport().Find(action)));
    else
      return new NullPortion(porFLOAT);
  }
  else
    return new NullPortion(porFLOAT);
}

static Portion *GSM_ActionValue_Rational(Portion **param)
{
  BehavSolution<gRational> *profile =
    (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  Action* action = ((ActionPortion*) param[1])->Value();
  Infoset *infoset = action->BelongsTo();

  if (infoset->GetPlayer()->IsChance())
    return new NullPortion(porRATIONAL);
  else if (profile->GetEFSupport().Find(action))  {
    Efg<gRational> *efg = profile->BelongsTo();

    gDPVector<gRational> values(efg->Dimensionality());
    gPVector<gRational> probs(efg->Dimensionality().Lengths());

    profile->CondPayoff(values, probs);
  
    if (probs(infoset->GetPlayer()->GetNumber(),
	      infoset->GetNumber()) > (gRational) 0)
      return new RationalValPortion(values(infoset->GetPlayer()->GetNumber(), 
					   infoset->GetNumber(),
					   profile->GetEFSupport().Find(action)));
    else
      return new NullPortion(porRATIONAL);
  }
  else
    return new NullPortion(porRATIONAL);
}

//-----------------
// ActionValues
//-----------------

static Portion *GSM_ActionValues_Float(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();

  const EFSupport &support = bp->GetEFSupport(); 
  Efg<double> *E = bp->BelongsTo();
  ListPortion *por = new ListValPortion; 
  
  gDPVector<double> values(E->Dimensionality());
  gPVector<double> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);
  
  for (int pl = 1; pl <= E->NumPlayers(); pl++)  {
    EFPlayer *player = E->PlayerList()[pl];
    ListPortion *p1 = new ListValPortion;
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      Infoset *infoset = player->InfosetList()[iset];
      ListPortion *p2 = new ListValPortion;
 
      gVector<double> ret(infoset->NumActions());
      for (int act = 1; act <= infoset->NumActions(); act++)  {
	if (support.Find(infoset->GetActionList()[act]) &&
	    probs(pl, iset) > 0)
	  p2->Append(new FloatValPortion(values(pl, iset,
		      support.Find(infoset->GetActionList()[act]))));
	else
	  p2->Append(new NullPortion(porFLOAT));
      }
      p1->Append(p2);
    }
    por->Append(p1);
  }

  return por;
}

static Portion *GSM_ActionValues_Rational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();

  const EFSupport &support = bp->GetEFSupport();
  Efg<gRational> *E = bp->BelongsTo();
  ListPortion *por = new ListValPortion;

  gDPVector<gRational> values(E->Dimensionality());
  gPVector<gRational> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);

  for (int pl = 1; pl <= E->NumPlayers(); pl++)  {
	 EFPlayer *player = E->PlayerList()[pl];
	 ListPortion *p1 = new ListValPortion;
	 for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
		Infoset *infoset = player->InfosetList()[iset];
		ListPortion *p2 = new ListValPortion;

		gVector<double> ret(infoset->NumActions());
		for (int act = 1; act <= infoset->NumActions(); act++)  {
	if (support.Find(infoset->GetActionList()[act]) &&
		 probs(pl, iset) > (gRational)(0))
	  p2->Append(new RationalValPortion(values(pl, iset,
		      support.Find(infoset->GetActionList()[act]))));
	else
	  p2->Append(new NullPortion(porRATIONAL));
      }
      p1->Append(p2);
    }
    por->Append(p1);
  }

  return por;
}


//--------------
// Behav
//--------------

static Portion *GSM_Behav_EFSupport(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p1;
  Portion* p2;
  Portion* p3;
  Portion* por = 0;


  EFSupport *S = ((EfSupportPortion *) param[0])->Value();

  if (S->BelongsTo().Type() == DOUBLE && param[1]->Spec().Type & porFLOAT)
  {
    Efg<double> &E = * (Efg<double>*) &S->BelongsTo();
    BehavSolution<double> *P = new BehavSolution<double>(E);

    if(((ListPortion*) param[1])->Length() != E.NumPlayers())
    {
      delete P;
      return new ErrorPortion("Mismatching number of players");
    }
    
    for(i = 1; i <= E.NumPlayers(); i++)
    {
      p1 = ((ListPortion*) param[1])->SubscriptCopy(i);
      if(p1->Spec().ListDepth == 0)
      {
	delete p1;
	delete P;
	return new ErrorPortion("Mismatching dimensionality");
      }
      if(((ListPortion*) p1)->Length() != E.PlayerList()[i]->NumInfosets())
      {
	delete p1;
	delete P;
	return new ErrorPortion("Mismatching number of infosets");
      }
      
      for(j = 1; j <= E.PlayerList()[i]->NumInfosets(); j++)
      {
	p2 = ((ListPortion*) p1)->SubscriptCopy(j);
	if(p2->Spec().ListDepth == 0)
	{
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion("Mismatching dimensionality");
	}
	if (((ListPortion*) p2)->Length() != S->NumActions(i, j))
	{
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion("Mismatching number of actions");
	}
	
	for (k = 1; k <= S->NumActions(i, j); k++)  {
	  p3 = ((ListPortion*) p2)->SubscriptCopy(k);
	  if(p3->Spec().Type != porFLOAT)
	  {
	    delete p3;
	    delete p2;
	    delete p1;
	    delete P;
	    return new ErrorPortion("Mismatching dimensionality");
	  }
	  
	  (*P)(i, j, k) = ((FloatPortion*) p3)->Value();
	  
	  delete p3;
	}
	delete p2;
      }
      delete p1;
    }
    por = new BehavValPortion(P);


  }
  else if (S->BelongsTo().Type()== RATIONAL && param[1]->Spec().Type & porRATIONAL)
  {
    // The code here is entirely copied from GSM_Behav_EfgRational()

    Efg<gRational> &E = * (Efg<gRational>*) &S->BelongsTo();
    BehavSolution<gRational> *P = new BehavSolution<gRational>(E);
    
    if(((ListPortion*) param[1])->Length() != E.NumPlayers())
    {
      delete P;
      return new ErrorPortion("Mismatching number of players");
    }
    
    for(i = 1; i <= E.NumPlayers(); i++)
    {
      p1 = ((ListPortion*) param[1])->SubscriptCopy(i);
      if(p1->Spec().ListDepth == 0)
      {
	delete p1;
	delete P;
	return new ErrorPortion("Mismatching dimensionality");
      }
      if(((ListPortion*) p1)->Length() != E.PlayerList()[i]->NumInfosets())
      {
	delete p1;
	delete P;
	return new ErrorPortion("Mismatching number of infosets");
      }
      
      for(j = 1; j <= E.PlayerList()[i]->NumInfosets(); j++)
      {
	p2 = ((ListPortion*) p1)->SubscriptCopy(j);
	if(p2->Spec().ListDepth == 0)
	{
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion("Mismatching dimensionality");
	}
	if (((ListPortion*) p2)->Length() != S->NumActions(i, j))  {
	  delete p2;
	  delete p1;
	  delete P;
	  return new ErrorPortion("Mismatching number of actions");
	}
	
	for (k = 1; k <= S->NumActions(i, j); k++)  {
	  p3 = ((ListPortion*) p2)->SubscriptCopy(k);
	  if(p3->Spec().Type != porRATIONAL)
	  {
	    delete p3;
	    delete p2;
	    delete p1;
	    delete P;
	    return new ErrorPortion("Mismatching dimensionality");
	  }
	  
	  (*P)(i, j, k) = ((RationalPortion*) p3)->Value();
	  
	  delete p3;
	}
	delete p2;
      }
      delete p1;
    }

    por = new BehavValPortion(P);
  }

  if(por == 0)
    por = new ErrorPortion("Mismatching EFG and list type");
  else
  {
    por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  }
  return por;
}

//-------------
// Belief
//-------------

static Portion *GSM_Belief_Float(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  Node* n = ((NodePortion*) param[1])->Value();
  const gDPVector<double> &values(bp->Beliefs());
  Infoset *s = n->GetInfoset();
  const gArray<Node *> &members = s->GetMemberList();

  if (s->IsChanceInfoset() || n->NumChildren() == 0)
    return new NullPortion(porFLOAT);

  int index;
  for (index = 1; members[index] != n; index++);
  return new FloatValPortion(values(s->GetPlayer()->GetNumber(), 
				    s->GetNumber(), index));
}

static Portion *GSM_Belief_Rational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  Node* n = ((NodePortion*) param[1])->Value();
  const gDPVector<gRational> &values(bp->Beliefs());
  Infoset *s = n->GetInfoset();
  const gArray<Node *> &members = s->GetMemberList();

  if (s->IsChanceInfoset() || n->NumChildren() == 0)
    return new NullPortion(porFLOAT);

  int index;
  for (index = 1; members[index] != n; index++);
  return new FloatValPortion(values(s->GetPlayer()->GetNumber(), 
				    s->GetNumber(), index));
}

//-------------
// Beliefs
//-------------

static Portion *GSM_Beliefs_Float(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  return gDPVectorToList(bp->Beliefs());
}

static Portion *GSM_Beliefs_Rational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  return gDPVectorToList(bp->Beliefs());
}

//--------------
// Centroid
//--------------
 
static Portion *GSM_CentroidEFSupport(Portion **param)
{
  EFSupport *S = ((EfSupportPortion *) param[0])->Value();
  BaseBehavProfile *P;

  if (S->BelongsTo().Type() == DOUBLE)
    P = new BehavSolution<double>(*S);
  else
    P = new BehavSolution<gRational>(*S);

  Portion *por = new BehavValPortion(P);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

static Portion *GSM_CentroidNFSupport(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  Portion *por;

  if (S->BelongsTo().Type() == DOUBLE)
    por = new MixedValPortion<double>(new MixedSolution<double>((Nfg<double> &) S->BelongsTo(), *S));
  else
    por = new MixedValPortion<gRational>(new MixedSolution<gRational>((Nfg<gRational> &) S->BelongsTo(), *S));

  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}


//-------
// Game
//-------

static Portion* GSM_Game_NfgTypes(Portion** param)
{
  if (param[0]->Game())  {
    assert(!param[0]->GameIsEfg());
    BaseNfg *nfg = (BaseNfg *) param[0]->Game();
    if (nfg->Type() == DOUBLE)
      return new NfgValPortion<double>((Nfg<double> *) nfg);
    else
      return new NfgValPortion<gRational>((Nfg<gRational> *) nfg);
  }
  else
    return 0;
}

static Portion* GSM_Game_EfgTypes(Portion** param)
{
  if(param[0]->Game())  {
    assert(param[0]->GameIsEfg());
    return new EfgValPortion((BaseEfg*) param[0]->Game());
  }
  else
    return 0;
}



//---------------
// GobitLambda
//---------------

static Portion *GSM_GobitLambda_BehavFloat(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  if(bs->Creator() != EfgAlg_GOBIT)
    return new NullPortion(porFLOAT);
  return new FloatValPortion(bs->GobitLambda());
}

static Portion *GSM_GobitLambda_MixedFloat(Portion** param)
{
  MixedSolution<double>* bs = 
    (MixedSolution<double>*) ((MixedPortion<double>*) param[0])->Value();
  if(bs->Creator() != NfgAlg_GOBIT)
    return new NullPortion(porFLOAT);
  return new FloatValPortion(bs->GobitLambda());
}

//--------------
// GobitValue
//--------------

static Portion *GSM_GobitValue_BehavFloat(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  if(bs->Creator() != EfgAlg_GOBIT)
    return new NullPortion(porFLOAT);
  return new FloatValPortion(bs->GobitValue());
}

static Portion *GSM_GobitValue_MixedFloat(Portion** param)
{
  MixedSolution<double>* bs = 
    (MixedSolution<double>*) ((MixedPortion<double>*) param[0])->Value();
  if(bs->Creator() != NfgAlg_GOBIT)
    return new NullPortion(porFLOAT);
  return new FloatValPortion(bs->GobitValue());
}


//----------------
// InfosetProb
//----------------

static Portion *GSM_InfosetProb_Float(Portion **param)
{
  if( param[1]->Spec().Type == porNULL )
    return new NullPortion( porFLOAT );

  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  Infoset* s = ((InfosetPortion*) param[1])->Value();

  Efg<double> *E = bp->BelongsTo();

  gDPVector<double> values(E->Dimensionality());
  gPVector<double> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);

  if (s->IsChanceInfoset())
    return new ErrorPortion("Not implemented for chance infosets");

  return new FloatValPortion(probs(s->GetPlayer()->GetNumber(),
				   s->GetNumber()));
}

static Portion *GSM_InfosetProb_Rational(Portion **param)
{
  if( param[1]->Spec().Type == porNULL )
    return new NullPortion( porRATIONAL );

  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  Infoset* s = ((InfosetPortion*) param[1])->Value();

  Efg<gRational> *E = bp->BelongsTo();
  gDPVector<gRational> values(E->Dimensionality());
  gPVector<gRational> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);

  if (s->IsChanceInfoset())
    return new ErrorPortion("Not implemented for chance infosets");

  return new RationalValPortion(probs(s->GetPlayer()->GetNumber(),
				      s->GetNumber()));
}

//----------------
// InfosetProbs
//----------------

static Portion *GSM_InfosetProbs_Float(Portion **param)
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

static Portion *GSM_InfosetProbs_Rational(Portion **param)
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


//----------------
// IsKnownNash
//----------------

static Portion *GSM_IsKnownNash_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new BoolValPortion(P->IsNash() == T_YES);
}

static Portion *GSM_IsKnownNash_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ((BehavPortion*) param[0])->Value();
  return new BoolValPortion(P->IsNash() == T_YES);
}

static Portion *GSM_IsKnownNash_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ((MixedPortion<double>*) param[0])->Value();
  return new BoolValPortion(P->IsNash() == T_YES);
}

static Portion *GSM_IsKnownNash_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ((MixedPortion<gRational>*) param[0])->Value();
  return new BoolValPortion(P->IsNash() == T_YES);
}


//-----------------
// IsKnownNotNash
//-----------------

static Portion *GSM_IsKnownNotNash_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new BoolValPortion(P->IsNash() == T_NO);
}

static Portion *GSM_IsKnownNotNash_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ((BehavPortion*) param[0])->Value();
  return new BoolValPortion(P->IsNash() == T_NO);
}

static Portion *GSM_IsKnownNotNash_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ((MixedPortion<double>*) param[0])->Value();
  return new BoolValPortion(P->IsNash() == T_NO);
}

static Portion *GSM_IsKnownNotNash_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ((MixedPortion<gRational>*) param[0])->Value();
  return new BoolValPortion(P->IsNash() == T_NO);
}

//--------------------
// IsKnownNotPerfect
//--------------------

static Portion *GSM_IsKnownNotPerfect_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ((MixedPortion<double>*) param[0])->Value();
  return new BoolValPortion(P->IsPerfect() == T_NO);
}

static Portion *GSM_IsKnownNotPerfect_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ((MixedPortion<gRational>*) param[0])->Value();
  return new BoolValPortion(P->IsPerfect() == T_NO);
}

//-----------------------
// IsKnownNotSequential
//-----------------------

static Portion *GSM_IsKnownNotSequential_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new BoolValPortion(P->IsSequential() == T_NO);
}

static Portion *GSM_IsKnownNotSequential_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ((BehavPortion*) param[0])->Value();
  return new BoolValPortion(P->IsSequential() == T_NO);
}

//---------------------------
// IsKnownNotSubgamePerfect
//---------------------------

static Portion *GSM_IsKnownNotSubgamePerfect_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new BoolValPortion(P->IsSubgamePerfect() == T_NO);
}

static Portion *GSM_IsKnownNotSubgamePerfect_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ((BehavPortion*) param[0])->Value();
  return new BoolValPortion(P->IsSubgamePerfect() == T_NO);
}

//-------------------
// IsKnownPerfect
//-------------------

static Portion *GSM_IsKnownPerfect_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ((MixedPortion<double>*) param[0])->Value();
  return new BoolValPortion(P->IsPerfect() == T_YES);
}

static Portion *GSM_IsKnownPerfect_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ((MixedPortion<gRational>*) param[0])->Value();
  return new BoolValPortion(P->IsPerfect() == T_YES);
}

//--------------------
// IsKnownSequential
//--------------------

static Portion *GSM_IsKnownSequential_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new BoolValPortion(P->IsSequential() == T_YES);
}

static Portion *GSM_IsKnownSequential_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ((BehavPortion*) param[0])->Value();
  return new BoolValPortion(P->IsSequential() == T_YES);
}

//------------------------
// IsKnownSubgamePerfect
//------------------------

static Portion *GSM_IsKnownSubgamePerfect_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new BoolValPortion(P->IsSubgamePerfect() == T_YES);
}

static Portion *GSM_IsKnownSubgamePerfect_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ((BehavPortion*) param[0])->Value();
  return new BoolValPortion(P->IsSubgamePerfect() == T_YES);
}

//----------------
// LiapValue
//----------------

static Portion *GSM_LiapValue_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new FloatValPortion(P->LiapValue());
}

static Portion *GSM_LiapValue_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ((BehavPortion*) param[0])->Value();
  return new RationalValPortion(P->LiapValue());
}


static Portion *GSM_LiapValue_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ((MixedPortion<double>*) param[0])->Value();
  return new FloatValPortion(P->LiapValue());
}

static Portion *GSM_LiapValue_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ((MixedPortion<gRational>*) param[0])->Value();
  return new RationalValPortion(P->LiapValue());
}


//----------
// Mixed
//----------

Portion* GSM_Mixed_NFSupport(Portion** param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  gArray<int> dim = S->NumStrats();
  Portion* por;
  MixedSolution<double> *Pd = 0;
  MixedSolution<gRational> *Pr = 0;
  unsigned long datatype;
  int i;
  int j;
  Portion* p1;
  Portion* p2;

  switch(S->BelongsTo().Type())
  {
  case DOUBLE:
    Pd = new MixedSolution<double>((Nfg<double> &) S->BelongsTo(), *S);
    datatype = porFLOAT;
    break;
  case RATIONAL:
    Pr = new MixedSolution<gRational>((Nfg<gRational> &) S->BelongsTo(), *S);
    datatype = porRATIONAL;
    break;
  default:
    assert(0);
  }


  if(((ListPortion*) param[1])->Length() != dim.Length())
  {
    if (Pd) delete Pd;
    if (Pr) delete Pr;
    return new ErrorPortion("Mismatching number of players");
  }
  
  for(i = 1; i <= dim.Length(); i++)
  {
    p1 = ((ListPortion*) param[1])->SubscriptCopy(i);
    if(p1->Spec().ListDepth == 0)
    {
      delete p1;
      if (Pd) delete Pd;
      if (Pr) delete Pr;
      return new ErrorPortion("Mismatching dimensionality");
    }
    if (((ListPortion*) p1)->Length() != S->NumStrats(i))  {
      delete p1;
      if (Pd) delete Pd;
      if (Pr) delete Pr;
      return new ErrorPortion("Mismatching number of strategies");
    }
    
    for (j = 1; j <= S->NumStrats(i); j++)  {
      p2 = ((ListPortion*) p1)->SubscriptCopy(j);
      if(p2->Spec().Type != datatype)
      {
	delete p2;
	delete p1;
	if (Pd)  delete Pd;
	if (Pr)  delete Pr;
	return new ErrorPortion("Mismatching dimensionality");
      }
      
      switch(datatype)
      {
      case porFLOAT:
	(* (MixedSolution<double>*) Pd)(i, j) = 
	  ((FloatPortion*) p2)->Value();
	break;
      case porRATIONAL:
	(* (MixedSolution<gRational>*) Pr)(i, j) = 
	  ((RationalPortion*) p2)->Value();
	break;
      default:
	assert(0);
      }
      
      delete p2;
    }
    delete p1;
  }

  if (datatype == porFLOAT)
    por = new MixedValPortion<double>(Pd);
  else
    por = new MixedValPortion<gRational>(Pr);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}


//----------------
// NodeValue
//----------------

static Portion *GSM_NodeValue_Float(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();
  Node* n = ((NodePortion*) param[2])->Value();

  BaseEfg* E = bp->BelongsTo();
  gList<Node *> list;
  Nodes(*E, list);
  
  int i;
  int found = 0;
  for(i=1; i<=list.Length(); i++)
    if(n == list[i])
      found = i;

  return new FloatValPortion(bp->NodeValues(p->GetNumber())[found]);
}

static Portion *GSM_NodeValue_Rational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();
  Node* n = ((NodePortion*) param[2])->Value();

  BaseEfg* E = bp->BelongsTo();
  gList<Node *> list;
  Nodes(*E, list);
  
  int i;
  int found = 0;
  for(i=1; i<=list.Length(); i++)
    if(n == list[i])
      found = i;

  return new RationalValPortion(bp->NodeValues(p->GetNumber())[found]);
}


//----------------
// NodeValues
//----------------

static Portion *GSM_NodeValues_Float(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();

  return ArrayToList(bp->NodeValues(p->GetNumber()));
}

static Portion *GSM_NodeValues_Rational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();

  return ArrayToList(bp->NodeValues(p->GetNumber()));
}
 

//----------------
// RealizProb
//----------------

static Portion *GSM_RealizProb_Float(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  Node* n = ((NodePortion*) param[1])->Value();
  
  BaseEfg* E = bp->BelongsTo();
  gList<Node *> list;
  Nodes(*E, list);
  
  int i;
  int found = 0;
  for(i=1; i<=list.Length(); i++)
    if(n == list[i])
      found = i;
  
  return new FloatValPortion(bp->NodeRealizProbs()[found]);
}  

static Portion *GSM_RealizProb_Rational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  Node* n = ((NodePortion*) param[1])->Value();
  
  BaseEfg* E = bp->BelongsTo();
  gList<Node *> list;
  Nodes(*E, list);
  
  int i;
  int found = 0;
  for(i=1; i<=list.Length(); i++)
    if(n == list[i])
      found = i;
  
  return new RationalValPortion(bp->NodeRealizProbs()[found]);
}  

//----------------
// RealizProbs
//----------------

static Portion *GSM_RealizProbs_Float(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  
  return ArrayToList(bp->NodeRealizProbs());
}  
  
static Portion *GSM_RealizProbs_Rational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  
  return ArrayToList(bp->NodeRealizProbs());
}

//-----------
// Regret
//-----------

static Portion *GSM_Regret_MixedFloat(Portion **param)
{
  MixedSolution<double> *P = 
    (MixedSolution<double>*) ((MixedPortion<double>*) param[0])->Value();
  Strategy* s = ((StrategyPortion*) param[1])->Value();
  NFPlayer* p = s->nfp;
  BaseNfg &n = p->BelongsTo();
  
  gPVector<double> v(n.NumStrats());
  P->Regret(v);

  return new FloatValPortion(v(p->GetNumber(), s->number));
}

static Portion *GSM_Regret_MixedRational(Portion **param)
{
  MixedSolution<gRational> *P = 
    (MixedSolution<gRational>*) ((MixedPortion<gRational>*) param[0])->Value();
  Strategy* s = ((StrategyPortion*) param[1])->Value();
  NFPlayer* p = s->nfp;
  BaseNfg &n = p->BelongsTo();
  
  gPVector<gRational> v(n.NumStrats());
  P->Regret(v);

  return new RationalValPortion(v(p->GetNumber(), s->number));
}

static Portion *GSM_Regret_BehavFloat(Portion **param)
{
  BehavSolution<double> *P = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();

  Action* a = ((ActionPortion*) param[1])->Value();
  Infoset* s = a->BelongsTo();
  EFPlayer* p = s->GetPlayer();

  if (s->IsChanceInfoset())
    return new NullPortion(porFLOAT);
  
  return new FloatValPortion(P->Regret()(p->GetNumber(), s->GetNumber(),
					 a->GetNumber()));
}


static Portion *GSM_Regret_BehavRational(Portion **param)
{
  BehavSolution<gRational> *P = 
    (BehavSolution<gRational>*) ((BehavPortion*) param[0])->Value();

  Action* a = ((ActionPortion*) param[1])->Value();
  Infoset* s = a->BelongsTo();
  EFPlayer* p = s->GetPlayer();

  if (s->IsChanceInfoset())
    return new NullPortion(porRATIONAL);
  
  return new RationalValPortion(P->Regret()(p->GetNumber(), s->GetNumber(),
					    a->GetNumber()));
}

//------------
// Regrets
//------------

static Portion *GSM_Regrets_MixedFloat(Portion **param)
{
  MixedSolution<double> *profile = 
    (MixedSolution<double> *) ((MixedPortion<double> *) param[0])->Value();

  gPVector<double> v(profile->BelongsTo().NumStrats());

  profile->Regret(v);

  ListPortion *por = new ListValPortion;
  
  for (int pl = 1; pl <= profile->Lengths().Length(); pl++)  {
    ListPortion *p1 = new ListValPortion;

    for (int st = 1; st <= profile->Lengths()[pl]; st++)
      p1->Append(new FloatValPortion(v(pl, st)));

    por->Append(p1);
  }

  return por;
}


static Portion *GSM_Regrets_MixedRational(Portion **param)
{
  MixedSolution<gRational> *profile = 
    (MixedSolution<gRational> *) ((MixedPortion<gRational> *) param[0])->Value();

  gPVector<gRational> v(profile->BelongsTo().NumStrats());

  profile->Regret(v);

  ListPortion *por = new ListValPortion;
  
  for (int pl = 1; pl <= profile->Lengths().Length(); pl++)  {
    ListPortion *p1 = new ListValPortion;

    for (int st = 1; st <= profile->Lengths()[pl]; st++)
      p1->Append(new RationalValPortion(v(pl, st)));

    por->Append(p1);
  }

  return por;
}

static Portion *GSM_Regrets_BehavFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  return gDPVectorToList(bp->Regret());
}

static Portion *GSM_Regrets_BehavRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  return gDPVectorToList(bp->Regret());
}

//-----------------
// SetActionProbs
//-----------------

static Portion *GSM_SetActionProbs_Float(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p3;
  int PlayerNum = 0;
  int InfosetNum = 0;
  
  BehavSolution<double>* P = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  Efg<double>& E = * P->BelongsTo();
  gArray< EFPlayer* > player = E.PlayerList();
  
  for(i = 1; i <= E.NumPlayers(); i++)
  {
    for(j = 1; j <= E.PlayerList()[i]->NumInfosets(); j++)
    {
      if(((InfosetPortion*) param[1])->Value() ==
	 E.PlayerList()[i]->InfosetList()[j])
      {
	PlayerNum = i;
	InfosetNum = j;
	break;
      }
    }
  }
  
  if(((ListPortion*) param[2])->Length() != 
     E.PlayerList()[PlayerNum]->InfosetList()[InfosetNum]->NumActions())
    return new ErrorPortion("Mismatching number of actions");
  
  for(k = 1; 
      k <= E.PlayerList()[PlayerNum]->InfosetList()[InfosetNum]->NumActions();
      k++)
  {
    p3 = ((ListPortion*) param[2])->SubscriptCopy(k);
    if(p3->Spec().ListDepth > 0)
    {
      delete p3;
      return new ErrorPortion("Mismatching dimensionality");
    }

    assert(p3->Spec().Type == porFLOAT);
    (*P)(PlayerNum, InfosetNum, k) = ((FloatPortion*) p3)->Value();

    delete p3;
  }

  return param[0]->RefCopy();
}


static Portion *GSM_SetActionProbs_Rational(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p3;
  int PlayerNum = 0;
  int InfosetNum = 0;
  
  BehavSolution<gRational>* P = 
    (BehavSolution<gRational>*) ((BehavPortion*) param[0])->Value();
  Efg<gRational>& E = * P->BelongsTo();
  gArray< EFPlayer* > player = E.PlayerList();
  
  for(i = 1; i <= E.NumPlayers(); i++)
  {
    for(j = 1; j <= E.PlayerList()[i]->NumInfosets(); j++)
    {
      if(((InfosetPortion*) param[1])->Value() ==
	 E.PlayerList()[i]->InfosetList()[j])
      {
	PlayerNum = i;
	InfosetNum = j;
	break;
      }
    }
  }
  
  if(((ListPortion*) param[2])->Length() != 
     E.PlayerList()[PlayerNum]->InfosetList()[InfosetNum]->NumActions())
    return new ErrorPortion("Mismatching number of actions");
  
  for(k = 1; 
      k <= E.PlayerList()[PlayerNum]->InfosetList()[InfosetNum]->NumActions();
      k++)
  {
    p3 = ((ListPortion*) param[2])->SubscriptCopy(k);
    if(p3->Spec().ListDepth > 0)
    {
      delete p3;
      return new ErrorPortion("Mismatching dimensionality");
    }

    assert(p3->Spec().Type == porRATIONAL);
    (*P)(PlayerNum, InfosetNum, k) = ((RationalPortion*) p3)->Value();

    delete p3;
  }

  return param[0]->RefCopy();
}

//-------------------
// SetStrategyProbs
//-------------------

static Portion *GSM_SetStrategyProbs_Float(Portion **param)
{
  int i;
  int j;
  Portion* p2;
  int PlayerNum = 0;

  MixedSolution<double>* P = 
    (MixedSolution<double>*) ((MixedPortion<double> *) param[0])->Value();
  Nfg<double>& N = P->BelongsTo();
  const gArray<NFPlayer *> &player = N.Players();
  
  for(i = 1; i <= N.NumPlayers(); i++)  {
    if(((NfPlayerPortion*) param[1])->Value() == player[i])
    {
      PlayerNum = i;
      break;
    }
  }
  
  if(((ListPortion*) param[2])->Length() != N.NumStrats(PlayerNum))
    return new ErrorPortion("Mismatching number of strategies");

  for(j = 1; j <= N.NumStrats(PlayerNum); j++)
  {
    p2 = ((ListPortion*) param[2])->SubscriptCopy(j);
    if(p2->Spec().ListDepth > 0)
    {
      delete p2;
      return new ErrorPortion("Mismatching dimensionality");
    }

    assert(p2->Spec().Type == porFLOAT);
    (*P)(PlayerNum, j) = ((FloatPortion*) p2)->Value();

    delete p2;
  }

  return param[0]->RefCopy();
}


static Portion *GSM_SetStrategyProbs_Rational(Portion **param)
{
  int i;
  int j;
  Portion* p2;
  int PlayerNum = 0;

  MixedSolution<gRational>* P = 
    (MixedSolution<gRational>*) ((MixedPortion<gRational>*) param[0])->Value();
  Nfg<gRational>& N = P->BelongsTo();
  const gArray<NFPlayer *> &player = N.Players();
  
  for(i = 1; i <= N.NumPlayers(); i++)
  {
    if(((NfPlayerPortion*) param[1])->Value() == player[i])
    {
      PlayerNum = i;
      break;
    }
  }
  
  if(((ListPortion*) param[2])->Length() != N.NumStrats(PlayerNum))
    return new ErrorPortion("Mismatching number of strategies");

  for(j = 1; j <= N.NumStrats(PlayerNum); j++)
  {
    p2 = ((ListPortion*) param[2])->SubscriptCopy(j);
    if(p2->Spec().ListDepth > 0)
    {
      delete p2;
      return new ErrorPortion("Mismatching dimensionality");
    }

    assert(p2->Spec().Type == porRATIONAL);
    (*P)(i, j) = ((RationalPortion*) p2)->Value();

    delete p2;
  }

  return param[0]->RefCopy();
}

//----------------
// StrategyProb
//----------------

static Portion *GSM_StrategyProb_Float(Portion **param)
{
  Portion *por;
  MixedSolution<double>* profile = 
    (MixedSolution<double> *) ((MixedPortion<double> *) param[0])->Value();
  Strategy* strategy = ((StrategyPortion*) param[1])->Value();
  NFPlayer* player = strategy->nfp;
  
  if (profile->Support().Strategies(player->GetNumber()).Find(strategy))
    por = new FloatValPortion((*profile)
			      (player->GetNumber(),
			       profile->Support().Strategies(player->GetNumber()).Find(strategy)));
  else
    por = new FloatValPortion(0.0);
  
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

static Portion *GSM_StrategyProb_Rational(Portion **param)
{
  Portion *por;
  MixedSolution<gRational>* profile = 
    (MixedSolution<gRational> *) ((MixedPortion<gRational> *) param[0])->Value();
  Strategy* strategy = ((StrategyPortion*) param[1])->Value();
  NFPlayer* player = strategy->nfp;
  
  if (profile->Support().Strategies(player->GetNumber()).Find(strategy))
    por = new RationalValPortion((*profile)
			      (player->GetNumber(),
			       profile->Support().Strategies(player->GetNumber()).Find(strategy)));
  else
    por = new RationalValPortion(0.0);
  
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------------
// StrategyProbs
//----------------

static Portion *GSM_StrategyProbs_Float(Portion **param)
{
  MixedSolution<double> *profile =
    (MixedSolution<double> *) ((BehavPortion *) param[0])->Value();
  const NFSupport *support = &profile->Support();
  const BaseNfg &nfg = support->BelongsTo();

  ListPortion *por = new ListValPortion;
  for (int pl = 1; pl <= nfg.NumPlayers(); pl++)  {
    NFPlayer *player = nfg.Players()[pl];
    ListPortion *p1 = new ListValPortion;

    for (int st = 1; st <= player->NumStrats(); st++)   {
      if (support->Find(player->Strategies()[st]))
	p1->Append(new FloatValPortion(
	      (*profile)(player->GetNumber(),
			 support->Find(player->Strategies()[st]))));
      else
	p1->Append(new FloatValPortion(0.0));
    }
    por->Append(p1);
  }

  return por;
}

static Portion *GSM_StrategyProbs_Rational(Portion **param)
{
  MixedSolution<gRational> *profile =
    (MixedSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  const NFSupport *support = &profile->Support();
  const BaseNfg &nfg = support->BelongsTo();

  ListPortion *por = new ListValPortion;
  for (int pl = 1; pl <= nfg.NumPlayers(); pl++)  {
    NFPlayer *player = nfg.Players()[pl];
    ListPortion *p1 = new ListValPortion;

    for (int st = 1; st <= player->NumStrats(); st++)   {
      if (support->Find(player->Strategies()[st]))
	p1->Append(new RationalValPortion(
	      (*profile)(player->GetNumber(),
			 support->Find(player->Strategies()[st]))));
      else
	p1->Append(new RationalValPortion(0.0));
    }
    por->Append(p1);
  }

  return por;
}


//---------------
// Support
//---------------

static Portion *GSM_Support_Behav(Portion** param)
{
  BaseBehavProfile *P = ((BehavPortion*) param[0])->Value();
  return new EfSupportValPortion(new EFSupport(P->GetEFSupport()));
}

static Portion *GSM_Support_MixedFloat(Portion** param)
{
  MixedProfile<double> *P = ((MixedPortion<double> *) param[0])->Value();
  return new NfSupportValPortion(new NFSupport(P->Support()));
}

static Portion *GSM_Support_MixedRational(Portion** param)
{
  MixedProfile<gRational> *P = ((MixedPortion<gRational> *) param[0])->Value();
  return new NfSupportValPortion(new NFSupport(P->Support()));
}




void Init_solfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("ActionProb", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ActionProb_Float, 
				       porFLOAT, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("action", porACTION));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_ActionProb_Rational, 
				       porRATIONAL, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("action", porACTION));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ActionProbs", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ActionProbs_Float, 
				       PortionSpec(porFLOAT, 3), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_ActionProbs_Rational, 
				       PortionSpec(porRATIONAL, 3), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("ActionValue", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ActionValue_Float, 
				       porFLOAT, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("action", porACTION));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_ActionValue_Rational, 
				       porRATIONAL, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("action", porACTION));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ActionValues", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ActionValues_Float, 
				       PortionSpec(porFLOAT, 3), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_ActionValues_Rational, 
				       PortionSpec(porRATIONAL, 3), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Behav", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Behav_EFSupport, 
				       porBEHAV, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porEFSUPPORT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("value", 
					    PortionSpec(porFLOAT | 
							porRATIONAL, 3)));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Belief", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Belief_Float, porFLOAT, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("node", porNODE));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Belief_Rational, porRATIONAL, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL)); 
  FuncObj->SetParamInfo(1, 1, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Beliefs", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Beliefs_Float, 
				       PortionSpec(porFLOAT, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Beliefs_Rational, 
				       PortionSpec(porRATIONAL, 1), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL)); 
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Centroid", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_CentroidEFSupport, 
				       porBEHAV, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porEFSUPPORT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_CentroidNFSupport,
				       porMIXED, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("support", porNFSUPPORT));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Game", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Game_NfgTypes, porNFG, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porMIXED));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Game_EfgTypes, porEFG, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Game_NfgTypes, porNFG, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("support", porNFSUPPORT));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Game_EfgTypes, porEFG, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("support", porEFSUPPORT));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("GobitLambda", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_GobitLambda_MixedFloat, 
				       porFLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porMIXED_FLOAT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_GobitLambda_BehavFloat, 
				       porFLOAT, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_FLOAT));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("GobitValue", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_GobitValue_MixedFloat, 
				       porFLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porMIXED_FLOAT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_GobitValue_BehavFloat, 
				       porFLOAT, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_FLOAT));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("InfosetProb", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_InfosetProb_Float, 
				       porFLOAT, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("infoset", 
                              PortionSpec(porINFOSET, 0, porNULLSPEC) ));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_InfosetProb_Rational, 
				       porRATIONAL, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("infoset", 
                              PortionSpec(porINFOSET, 0, porNULLSPEC) ));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("InfosetProbs", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_InfosetProbs_Float, 
				       PortionSpec(porFLOAT, 2), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_InfosetProbs_Rational, 
				       PortionSpec(porRATIONAL, 2), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsKnownNash", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsKnownNash_BehavFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT)); 
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsKnownNash_BehavRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL)); 

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_IsKnownNash_MixedFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("profile", porMIXED_FLOAT)); 
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_IsKnownNash_MixedRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("profile", porMIXED_RATIONAL)); 
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsKnownNotNash", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsKnownNotNash_BehavFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT)); 
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsKnownNotNash_BehavRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL)); 

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_IsKnownNotNash_MixedFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("profile", porMIXED_FLOAT)); 
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_IsKnownNotNash_MixedRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("profile", porMIXED_RATIONAL)); 
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsKnownNotPerfect", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsKnownNotPerfect_MixedFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porMIXED_FLOAT)); 
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsKnownNotPerfect_MixedRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porMIXED_RATIONAL)); 
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsKnownNotSequential", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsKnownNotSequential_BehavFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT)); 
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsKnownNotSequential_BehavRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL)); 
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsKnownNotSubgamePerfect", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsKnownNotSubgamePerfect_BehavFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT)); 
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsKnownNotSubgamePerfect_BehavRational,
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL)); 
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsKnownPerfect", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsKnownPerfect_MixedFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porMIXED_FLOAT)); 
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsKnownPerfect_MixedRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porMIXED_RATIONAL)); 
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsKnownSequential", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsKnownSequential_BehavFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsKnownSequential_BehavRational, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL)); 
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsKnownSubgamePerfect", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsKnownSubgamePerfect_BehavFloat, 
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT)); 
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsKnownSubgamePerfect_BehavRational,
				       porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL)); 
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("LiapValue", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_LiapValue_BehavFloat, 
				       porFLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_LiapValue_BehavRational, 
				       porRATIONAL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL)); 

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_LiapValue_MixedFloat, 
				       porFLOAT, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("profile", porMIXED_FLOAT)); 
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_LiapValue_MixedRational, 
				       porRATIONAL, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("profile", porMIXED_RATIONAL)); 
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Mixed", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Mixed_NFSupport, 
				       porMIXED, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porNFSUPPORT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("value", 
					    PortionSpec(porFLOAT | 
							porRATIONAL, 2)));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NodeValue", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NodeValue_Float, 
				       porFLOAT, 3,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("player", porEFPLAYER));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("node", porNODE));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_NodeValue_Rational, 
				       porRATIONAL, 3,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("player", porEFPLAYER));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NodeValues", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NodeValues_Float, 
				       PortionSpec(porFLOAT, 1), 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("player", porEFPLAYER));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_NodeValues_Rational, 
				       PortionSpec(porRATIONAL, 1), 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("player", porEFPLAYER));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("RealizProb", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_RealizProb_Float, porFLOAT, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("node", porNODE));
  
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_RealizProb_Rational, 
				       porRATIONAL, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RealizProbs", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_RealizProbs_Float, 
				       PortionSpec(porFLOAT, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_RealizProbs_Rational, 
				       PortionSpec(porRATIONAL, 1), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Regret", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Regret_BehavFloat, porFLOAT, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("action", porACTION));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Regret_BehavRational, 
				       porRATIONAL, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("action", porACTION));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Regret_MixedFloat, porFLOAT, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("profile", porMIXED_FLOAT));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("strategy", porSTRATEGY));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Regret_MixedRational, 
				       porRATIONAL, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("profile", porMIXED_RATIONAL));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("strategy", porSTRATEGY));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Regrets", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Regrets_BehavFloat,
				       PortionSpec(porFLOAT, 3), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Regrets_BehavRational,
				       PortionSpec(porRATIONAL, 3), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Regrets_MixedFloat,
				       PortionSpec(porFLOAT, 2), 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("profile", porMIXED_FLOAT));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Regrets_MixedRational,
				       PortionSpec(porRATIONAL, 2), 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("profile", porMIXED_RATIONAL));

  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("SetActionProbs", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetActionProbs_Float, 
				       porBEHAV_FLOAT, 3,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV_FLOAT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("infoset", porINFOSET));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("value", PortionSpec(porFLOAT, 1)));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_SetActionProbs_Rational,
				       porBEHAV_RATIONAL, 3,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV_RATIONAL,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("infoset", porINFOSET));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("value", 
					    PortionSpec(porRATIONAL,1)));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("SetStrategyProbs", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetStrategyProbs_Float,
				       porMIXED_FLOAT, 3,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porMIXED_FLOAT, 
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("player", porNFPLAYER));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("value", PortionSpec(porFLOAT,1)));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_SetStrategyProbs_Rational,
				       porMIXED_RATIONAL, 3,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porMIXED_RATIONAL, 
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("player", porNFPLAYER));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("value", 
					    PortionSpec(porRATIONAL,1)));

  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("StrategyProb", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_StrategyProb_Float, 
				       porFLOAT, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porMIXED_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("strategy", porSTRATEGY));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_StrategyProb_Rational, 
				       porRATIONAL, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porMIXED_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("strategy", porSTRATEGY));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("StrategyProbs", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_StrategyProbs_Float, 
				       PortionSpec(porFLOAT, 2), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porMIXED_FLOAT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_StrategyProbs_Rational, 
				       PortionSpec(porRATIONAL, 2), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porMIXED_RATIONAL));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Support", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Support_Behav, porEFSUPPORT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV)); 

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Support_MixedFloat, porNFSUPPORT, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porMIXED_FLOAT)); 

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Support_MixedRational, porNFSUPPORT, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("profile", porMIXED_RATIONAL)); 
  gsm->AddFunction(FuncObj);
}

