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
extern Portion *ArrayToList(const gArray<gNumber> &);
template <class T> Portion *ArrayToList(const gList<T> &);
template <class T> Portion *gDPVectorToList(const gDPVector<T> &);


//----------------
// ActionProb
//----------------

static Portion *GSM_ActionProb(Portion **param)
{
  const BehavSolution *profile = ((BehavPortion *) param[0])->Value();
  Action* action = ((ActionPortion*) param[1])->Value();
  Infoset* infoset = action->BelongsTo();
  EFPlayer* player = infoset->GetPlayer();
  
  if (player->IsChance())
    return new NumberPortion(infoset->Game()->
			       GetChanceProb(infoset, action->GetNumber()));
  else if (profile->Support().Find(action))
    return new NumberPortion((*profile)
			       (player->GetNumber(),
				infoset->GetNumber(),
				profile->Support().Find(action)));
  else
    return new NumberPortion(0.0);
}

//----------------
// ActionProbs
//----------------

static Portion *GSM_ActionProbs(Portion **param)
{
  const BehavSolution *profile = ((BehavPortion *) param[0])->Value();
  const EFSupport *support = &profile->Support();
  const Efg &efg = support->Game();

  ListPortion *por = new ListPortion;
  for (int pl = 1; pl <= efg.NumPlayers(); pl++)  {
    EFPlayer *player = efg.Players()[pl];
    ListPortion *p1 = new ListPortion;
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      Infoset *infoset = player->Infosets()[iset];
      ListPortion *p2 = new ListPortion;
      for (int act = 1; act <= infoset->NumActions(); act++)   {
	if (support->Find(infoset->Actions()[act]))
	  p2->Append(new NumberPortion(
	   	       (*profile)(player->GetNumber(),
				  infoset->GetNumber(),
				  support->Find(infoset->Actions()[act]))));
	else
	  p2->Append(new NumberPortion(0.0));
      }
      p1->Append(p2);
    }
    por->Append(p1);
  }

  return por;
}



//------------------
// StrategyValue
//------------------

static Portion *GSM_StrategyValue(Portion **param)
{
  MixedSolution *profile = ((MixedPortion *) param[0])->Value();
  Strategy* strategy = ((StrategyPortion*) param[1])->Value();
  Nfg *nfg = &profile->Game();

  const gArray<NFPlayer *> &player = nfg->Players();

  for(int i = 1; i <= nfg->NumPlayers(); i++)  
    if (profile->Support().Strategies(player[i]->GetNumber()).Find(strategy))
      return new NumberPortion(profile->Payoff(player[i]->GetNumber(),strategy));
      
  return new NullPortion(porNUMBER);
}




//------------------
// ActionValue
//------------------

static Portion *GSM_ActionValue(Portion **param)
{
  BehavSolution *profile = ((BehavPortion *) param[0])->Value();
  Action* action = ((ActionPortion*) param[1])->Value();
  Infoset *infoset = action->BelongsTo();

  if (infoset->GetPlayer()->IsChance())
    return new NullPortion(porNUMBER);
  else if (profile->Support().Find(action))  {
    Efg *efg = &profile->Game();

    gDPVector<gNumber> values(efg->NumActions());
    gPVector<gNumber> probs(efg->NumInfosets());

    profile->CondPayoff(values, probs);
  
    if (probs(infoset->GetPlayer()->GetNumber(),
	      infoset->GetNumber()) > gNumber(0.0))
      return new NumberPortion(values(infoset->GetPlayer()->GetNumber(),
					infoset->GetNumber(),
					profile->Support().Find(action)));
    else
      return new NullPortion(porNUMBER);
  }
  else
    return new NullPortion(porNUMBER);
}


//-----------------
// ActionValues
//-----------------

static Portion *GSM_ActionValues(Portion **param)
{
  BehavSolution *bp = ((BehavPortion *) param[0])->Value();

  const EFSupport &support = bp->Support(); 
  Efg *E = &bp->Game();
  ListPortion *por = new ListPortion; 
  
  gDPVector<gNumber> values(E->NumActions());
  gPVector<gNumber> probs(E->NumInfosets());

  bp->CondPayoff(values, probs);
  
  for (int pl = 1; pl <= E->NumPlayers(); pl++)  {
    EFPlayer *player = E->Players()[pl];
    ListPortion *p1 = new ListPortion;
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      Infoset *infoset = player->Infosets()[iset];
      ListPortion *p2 = new ListPortion;
 
      gVector<double> ret(infoset->NumActions());
      for (int act = 1; act <= infoset->NumActions(); act++)  {
	if (support.Find(infoset->Actions()[act]) &&
	    probs(pl, iset) > gNumber(0.0))
	  p2->Append(new NumberPortion(values(pl, iset,
		      support.Find(infoset->Actions()[act]))));
	else
	  p2->Append(new NullPortion(porNUMBER));
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

static Portion *GSM_Behav(Portion **param)
{
  EFSupport *S = ((EfSupportPortion *) param[0])->Value();
  const Efg &E = S->Game();

  gArray<gNumber> values(E.Parameters()->Dmnsn());
  return new BehavPortion(new BehavSolution(BehavProfile<gNumber>(*S, values)));
}

//-------------
// Belief
//-------------

static Portion *GSM_Belief(Portion **param)
{
  BehavSolution *bp = ((BehavPortion *) param[0])->Value();
  Node* n = ((NodePortion*) param[1])->Value();
  const gDPVector<gNumber> &values(bp->Beliefs());
  Infoset *s = n->GetInfoset();
  const gArray<Node *> &members = s->Members();

  if (s->IsChanceInfoset() || n->NumChildren() == 0)
    return new NullPortion(porNUMBER);

  int index;
  for (index = 1; members[index] != n; index++);
  return new NumberPortion(values(s->GetPlayer()->GetNumber(), 
				    s->GetNumber(), index));
}


//-------------
// Beliefs
//-------------

static Portion *GSM_Beliefs(Portion **param)
{
  BehavSolution *bp = ((BehavPortion *) param[0])->Value();
  return gDPVectorToList(bp->Beliefs());
}


//-------
// Game
//-------

static Portion* GSM_Game_Mixed(Portion** param)
{
  return new NfgPortion(&((MixedPortion *) param[0])->Value()->Game());
}

static Portion *GSM_Game_NfSupport(Portion **param)
{
  Nfg *N = (Nfg *) &((NfSupportPortion *) param[0])->Value()->Game();

  return new NfgPortion(N);
}

static Portion* GSM_Game_EfgTypes(Portion** param)
{
  if(param[0]->Game())  {
    assert(param[0]->GameIsEfg());
    return new EfgPortion((Efg*) param[0]->Game());
  }
  else
    return 0;
}

//---------------
// GobitLambda
//---------------

static Portion *GSM_GobitLambda_Behav(Portion** param)
{
  BehavSolution *bs = ((BehavPortion*) param[0])->Value();
  if(bs->Creator() != EfgAlg_GOBIT)
    return new NullPortion(porNUMBER);
  return new NumberPortion(bs->GobitLambda());
}

static Portion *GSM_GobitLambda_Mixed(Portion** param)
{
  MixedSolution *bs = ((MixedPortion*) param[0])->Value();
  if(bs->Creator() != NfgAlg_GOBIT)
    return new NullPortion(porNUMBER);
  return new NumberPortion(bs->GobitLambda());
}

//--------------
// GobitValue
//--------------

static Portion *GSM_GobitValue_Behav(Portion** param)
{
  BehavSolution *bs = ((BehavPortion*) param[0])->Value();
  if(bs->Creator() != EfgAlg_GOBIT)
    return new NullPortion(porNUMBER);
  return new NumberPortion(bs->GobitValue());
}

static Portion *GSM_GobitValue_Mixed(Portion** param)
{
  MixedSolution *bs = ((MixedPortion*) param[0])->Value();
  if(bs->Creator() != NfgAlg_GOBIT)
    return new NullPortion(porNUMBER);
  return new NumberPortion(bs->GobitValue());
}


//----------------
// InfosetProb
//----------------

static Portion *GSM_InfosetProb(Portion **param)
{
  if( param[1]->Spec().Type == porNULL )
    return new NullPortion( porNUMBER );

  BehavSolution *bp = ((BehavPortion *) param[0])->Value();
  Infoset* s = ((InfosetPortion*) param[1])->Value();

  Efg *E = &bp->Game();

  gDPVector<gNumber> values(E->NumActions());
  gPVector<gNumber> probs(E->NumInfosets());

  bp->CondPayoff(values, probs);

  if (s->IsChanceInfoset())
    return new ErrorPortion("Not implemented for chance infosets");

  return new NumberPortion(probs(s->GetPlayer()->GetNumber(),
				   s->GetNumber()));
}


//----------------
// InfosetProbs
//----------------

static Portion *GSM_InfosetProbs(Portion **param)
{
  BehavSolution *bp = ((BehavPortion *) param[0])->Value();

  Efg *E = &bp->Game();

  gDPVector<gNumber> values(E->NumActions());
  gPVector<gNumber> probs(E->NumInfosets());

  bp->CondPayoff(values, probs);

  ListPortion *ret = new ListPortion;

  for (int i = 1; i <= E->NumPlayers(); i++)
    ret->Append(ArrayToList(probs.GetRow(i)));

  return ret;
}


//----------------
// IsKnownNash
//----------------

static Portion *GSM_IsKnownNash_Behav(Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new BoolPortion(P->IsNash() == T_YES);
}

static Portion *GSM_IsKnownNash_Mixed(Portion **param)
{
  MixedSolution *P = ((MixedPortion*) param[0])->Value();
  return new BoolPortion(P->IsNash() == T_YES);
}


//-----------------
// IsKnownNotNash
//-----------------

static Portion *GSM_IsKnownNotNash_Behav(Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new BoolPortion(P->IsNash() == T_NO);
}

static Portion *GSM_IsKnownNotNash_Mixed(Portion **param)
{
  MixedSolution *P = ((MixedPortion *) param[0])->Value();
  return new BoolPortion(P->IsNash() == T_NO);
}

//--------------------
// IsKnownNotPerfect
//--------------------

static Portion *GSM_IsKnownNotPerfect(Portion **param)
{
  MixedSolution *P = ((MixedPortion*) param[0])->Value();
  return new BoolPortion(P->IsPerfect() == T_NO);
}

//-----------------------
// IsKnownNotSequential
//-----------------------

static Portion *GSM_IsKnownNotSequential(Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new BoolPortion(P->IsSequential() == T_NO);
}

//---------------------------
// IsKnownNotSubgamePerfect
//---------------------------

static Portion *GSM_IsKnownNotSubgamePerfect(Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new BoolPortion(P->IsSubgamePerfect() == T_NO);
}

//-------------------
// IsKnownPerfect
//-------------------

static Portion *GSM_IsKnownPerfect(Portion **param)
{
  MixedSolution *P = ((MixedPortion*) param[0])->Value();
  return new BoolPortion(P->IsPerfect() == T_YES);
}

//--------------------
// IsKnownSequential
//--------------------

static Portion *GSM_IsKnownSequential(Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new BoolPortion(P->IsSequential() == T_YES);
}


//------------------------
// IsKnownSubgamePerfect
//------------------------

static Portion *GSM_IsKnownSubgamePerfect(Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new BoolPortion(P->IsSubgamePerfect() == T_YES);
}

//----------------
// LiapValue
//----------------

static Portion *GSM_LiapValue_Behav(Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new NumberPortion(P->LiapValue());
}

static Portion *GSM_LiapValue_Mixed(Portion **param)
{
  MixedSolution *P = ((MixedPortion*) param[0])->Value();
  return new NumberPortion(P->LiapValue());
}

//----------
// Mixed
//----------

Portion* GSM_Mixed(Portion** param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  gArray<gNumber> values(S->Game().Parameters()->Dmnsn());
  return new MixedPortion(new MixedSolution(MixedProfile<gNumber>(*S, values)));
}


//----------------
// NodeValue
//----------------

static Portion *GSM_NodeValue(Portion **param)
{
  BehavSolution *bp = ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();
  Node* n = ((NodePortion*) param[2])->Value();

  Efg* E = &bp->Game();
  gList<Node *> list;
  Nodes(*E, list);
  
  int i;
  int found = 0;
  for(i=1; i<=list.Length(); i++)
    if(n == list[i])
      found = i;

  return new NumberPortion(bp->NodeValues(p->GetNumber())[found]);
}


//----------------
// NodeValues
//----------------

static Portion *GSM_NodeValues(Portion **param)
{
  BehavSolution *bp = ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();

  return ArrayToList(bp->NodeValues(p->GetNumber()));
}

//----------------
// RealizProb
//----------------

static Portion *GSM_RealizProb(Portion **param)
{
  BehavSolution *bp = ((BehavPortion *) param[0])->Value();
  Node* n = ((NodePortion*) param[1])->Value();
  
  Efg* E = &bp->Game();
  gList<Node *> list;
  Nodes(*E, list);
  
  int i;
  int found = 0;
  for(i=1; i<=list.Length(); i++)
    if(n == list[i])
      found = i;
  
  return new NumberPortion(bp->NodeRealizProbs()[found]);
}  

//----------------
// RealizProbs
//----------------

static Portion *GSM_RealizProbs(Portion **param)
{
  BehavSolution *bp = ((BehavPortion *) param[0])->Value();
  
  return ArrayToList(bp->NodeRealizProbs());
}  
  
//-----------
// Regret
//-----------

static Portion *GSM_Regret_Mixed(Portion **param)
{
  MixedSolution *P = ((MixedPortion*) param[0])->Value();
  Strategy* s = ((StrategyPortion*) param[1])->Value();
  NFPlayer* p = s->nfp;
  Nfg &n = p->Game();

  gPVector<gNumber> v(n.NumStrats());
  P->Regret(v);

  return new NumberPortion(v(p->GetNumber(), s->number));
}

static Portion *GSM_Regret_Behav(Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();

  Action* a = ((ActionPortion*) param[1])->Value();
  Infoset* s = a->BelongsTo();
  EFPlayer* p = s->GetPlayer();

  if (s->IsChanceInfoset())
    return new NullPortion(porNUMBER);
  
  return new NumberPortion(P->Regret()(p->GetNumber(), s->GetNumber(),
					 a->GetNumber()));
}

//------------
// Regrets
//------------

static Portion *GSM_Regrets_Mixed(Portion **param)
{
  MixedSolution *profile = ((MixedPortion *) param[0])->Value();

  gPVector<gNumber> v(profile->Game().NumStrats());

  profile->Regret(v);

  ListPortion *por = new ListPortion;
  
  for (int pl = 1; pl <= profile->Lengths().Length(); pl++)  {
    ListPortion *p1 = new ListPortion;

    for (int st = 1; st <= profile->Lengths()[pl]; st++)
      p1->Append(new NumberPortion(v(pl, st)));

    por->Append(p1);
  }

  return por;
}


static Portion *GSM_Regrets_Behav(Portion **param)
{
  BehavSolution *bp = ((BehavPortion *) param[0])->Value();
  return gDPVectorToList(bp->Regret());
}

//-------------------
// SetActionProb
//-------------------

static Portion *GSM_SetActionProb(Portion **param)
{
  BehavSolution *P = new BehavSolution(*((BehavPortion *) param[0])->Value());
  Action *a = ((ActionPortion *) param[1])->Value();
  int infoset = a->BelongsTo()->GetNumber();
  int player = a->BelongsTo()->GetPlayer()->GetNumber();
  int action = P->Support().Find(a);

//  if (action == 0)
//    return new ErrorPortion("Action not present in support of profile");

  gNumber value = ((NumberPortion *) param[2])->Value();
  
  if (action != 0)
    (*P)(player, infoset, action) = value;
  ((BehavPortion *) param[0])->SetValue(P);
  return param[0]->RefCopy();
}

//-----------------
// SetActionProbs
//-----------------

static Portion *GSM_SetActionProbs(Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p3;
  int PlayerNum = 0;
  int InfosetNum = 0;
  
  BehavSolution *P = new BehavSolution(*((BehavPortion*) param[0])->Value());
  Efg& E = P->Game();
  gArray< EFPlayer* > player = E.Players();
  
  for(i = 1; i <= E.NumPlayers(); i++)
  {
    for(j = 1; j <= E.Players()[i]->NumInfosets(); j++)
    {
      if(((InfosetPortion*) param[1])->Value() ==
	 E.Players()[i]->Infosets()[j])
      {
	PlayerNum = i;
	InfosetNum = j;
	break;
      }
    }
  }
  
  if(((ListPortion*) param[2])->Length() != 
     E.Players()[PlayerNum]->Infosets()[InfosetNum]->NumActions())  {
    delete P;
    return new ErrorPortion("Mismatching number of actions");
  }  

  for(k = 1; 
      k <= E.Players()[PlayerNum]->Infosets()[InfosetNum]->NumActions();
      k++)
  {
    p3 = ((ListPortion*) param[2])->SubscriptCopy(k);
    if(p3->Spec().ListDepth > 0)
    {
      delete p3;
      delete P;
      return new ErrorPortion("Mismatching dimensionality");
    }

    assert(p3->Spec().Type == porNUMBER);
    (*P)(PlayerNum, InfosetNum, k) = ((NumberPortion*) p3)->Value();

    delete p3;
  }

  ((BehavPortion *) param[0])->SetValue(P);
  return param[0]->RefCopy();
}

//-------------------
// SetStrategyProb
//-------------------

static Portion *GSM_SetStrategyProb(Portion **param)
{
  MixedSolution *P = new MixedSolution(*((MixedPortion *) param[0])->Value());
  Strategy *s = ((StrategyPortion *) param[1])->Value();
  gNumber value = ((NumberPortion *) param[2])->Value();
  int player = s->nfp->GetNumber();
  int strat = (*P).Support().Strategies(player).Find(s);

//  if (strat == 0)
//    return new ErrorPortion("Strategy not found in support of profile");
  if(strat !=0)
    (*P)(player, strat) = value;
  ((MixedPortion *) param[0])->SetValue(P);
  return param[0]->RefCopy();
}

//-------------------
// SetStrategyProbs
//-------------------

static Portion *GSM_SetStrategyProbs(Portion **param)
{
  int i;
  int j;
  Portion* p2;
  int PlayerNum = 0;

  MixedSolution *P = new MixedSolution(*((MixedPortion *) param[0])->Value());
  Nfg& N = P->Game();
  const gArray<NFPlayer *> &player = N.Players();
  
  for(i = 1; i <= N.NumPlayers(); i++)  {
    if(((NfPlayerPortion*) param[1])->Value() == player[i])
    {
      PlayerNum = i;
      break;
    }
  }
  
  if(((ListPortion*) param[2])->Length() != N.NumStrats(PlayerNum))  {
    delete P;
    return new ErrorPortion("Mismatching number of strategies");
  }

  for(j = 1; j <= N.NumStrats(PlayerNum); j++)
  {
    p2 = ((ListPortion*) param[2])->SubscriptCopy(j);
    if(p2->Spec().ListDepth > 0)
    {
      delete p2;
      delete P;
      return new ErrorPortion("Mismatching dimensionality");
    }

    assert(p2->Spec().Type == porNUMBER);
    (*P)(PlayerNum, j) = ((NumberPortion*) p2)->Value();

    delete p2;
  }

  ((MixedPortion *) param[0])->SetValue(P);
  return param[0]->RefCopy();
}

//----------------
// StrategyProb
//----------------

static Portion *GSM_StrategyProb(Portion **param)
{
  const MixedSolution *profile = ((MixedPortion *) param[0])->Value();
  Strategy* strategy = ((StrategyPortion*) param[1])->Value();
  NFPlayer* player = strategy->nfp;
  
  if (profile->Support().Strategies(player->GetNumber()).Find(strategy))
    return new NumberPortion((*profile)
			     (player->GetNumber(),
			      profile->Support().Strategies(player->GetNumber()).Find(strategy)));
  else
    return new NumberPortion(0.0);
}

//----------------
// StrategyProbs
//----------------

static Portion *GSM_StrategyProbs(Portion **param)
{
  const MixedSolution *profile = ((MixedPortion *) param[0])->Value();
  const NFSupport *support = &profile->Support();
  const Nfg &nfg = support->Game();

  ListPortion *por = new ListPortion;
  for (int pl = 1; pl <= nfg.NumPlayers(); pl++)  {
    NFPlayer *player = nfg.Players()[pl];
    ListPortion *p1 = new ListPortion;

    for (int st = 1; st <= player->NumStrats(); st++)   {
      if (support->Find(player->Strategies()[st]))
	p1->Append(new NumberPortion(
	      (*profile)(player->GetNumber(),
			 support->Find(player->Strategies()[st]))));
      else
	p1->Append(new NumberPortion(0.0));
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
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new EfSupportPortion(new EFSupport(P->Support()));
}

static Portion *GSM_Support_Mixed(Portion** param)
{
  MixedSolution *P = ((MixedPortion *) param[0])->Value();
  return new NfSupportPortion(new NFSupport(P->Support()));
}


void Init_solfunc(GSM *gsm)
{
  static struct { char *sig; Portion *(*func)(Portion **); } ftable[] =
    { { "ActionProb[profile->BEHAV, action->ACTION] =: NUMBER",
 	GSM_ActionProb },
      { "ActionProbs[profile->BEHAV] =: LIST(LIST(LIST(NUMBER)))",
	GSM_ActionProbs },
      { "ActionValue[profile->BEHAV, action->ACTION] =: NUMBER",
	GSM_ActionValue },
      { "ActionValues[profile->BEHAV] =: LIST(LIST(LIST(NUMBER)))",
	GSM_ActionValues },
      { "Behav[support->EFSUPPORT] =: BEHAV", GSM_Behav },
      { "Belief[profile->BEHAV, node->NODE] =: NUMBER", GSM_Belief },
      { "Beliefs[profile->BEHAV] =: LIST(NUMBER)", GSM_Beliefs },
      { "Game[profile->MIXED] =: NFG", GSM_Game_Mixed },
      { "Game[profile->BEHAV] =: EFG", GSM_Game_EfgTypes },
      { "Game[support->NFSUPPORT] =: NFG", GSM_Game_NfSupport },
      { "Game[support->EFSUPPORT] =: EFG", GSM_Game_EfgTypes },
      { "Game[basis->EFBASIS] =: EFG", GSM_Game_EfgTypes },
      { "GobitLambda[profile->MIXED] =: NUMBER", GSM_GobitLambda_Mixed },
      { "GobitLambda[profile->BEHAV] =: NUMBER", GSM_GobitLambda_Behav },
      { "GobitValue[profile->MIXED] =: NUMBER", GSM_GobitValue_Mixed },
      { "GobitValue[profile->BEHAV] =: NUMBER", GSM_GobitValue_Behav },
      { "InfosetProb[profile->BEHAV, infoset->INFOSET*] =: NUMBER",
	GSM_InfosetProb },
      { "InfosetProbs[profile->BEHAV] =: LIST(LIST(NUMBER))", 
	GSM_InfosetProbs },
      { "IsKnownNash[profile->BEHAV] =: BOOLEAN", GSM_IsKnownNash_Behav },
      { "IsKnownNash[profile->MIXED] =: BOOLEAN", GSM_IsKnownNash_Mixed },
      { "IsKnownNotNash[profile->BEHAV] =: BOOLEAN", 
	GSM_IsKnownNotNash_Behav },
      { "IsKnownNotNash[profile->MIXED] =: BOOLEAN",
	GSM_IsKnownNotNash_Mixed },
      { "IsKnownNotPerfect[profile->MIXED] =: BOOLEAN", 
	GSM_IsKnownNotPerfect },
      { "IsKnownNotSequential[profile->BEHAV] =: BOOLEAN",
	GSM_IsKnownNotSequential },
      { "IsKnownNotSubgamePerfect[profile->BEHAV] =: BOOLEAN",
	GSM_IsKnownNotSubgamePerfect },
      { "IsKnownPerfect[profile->MIXED] =: BOOLEAN", GSM_IsKnownPerfect },
      { "IsKnownSequential[profile->BEHAV] =: BOOLEAN",
	GSM_IsKnownSequential },
      { "IsKnownSubgamePerfect[profile->BEHAV] =: BOOLEAN",
	GSM_IsKnownSubgamePerfect },
      { "LiapValue[profile->BEHAV] =: NUMBER", GSM_LiapValue_Behav },
      { "LiapValue[profile->MIXED] =: NUMBER", GSM_LiapValue_Mixed },
      { "Mixed[support->NFSUPPORT] =: MIXED", GSM_Mixed },
      { "NodeValue[profile->BEHAV, player->EFPLAYER, node->NODE] =: NUMBER",
	GSM_NodeValue },
      { "NodeValues[profile->BEHAV, player->EFPLAYER] =: LIST(NUMBER)",
	GSM_NodeValues },
      { "RealizProb[profile->BEHAV, node->NODE] =: NUMBER", GSM_RealizProb },
      { "RealizProbs[profile->BEHAV] =: LIST(NUMBER)", GSM_RealizProbs },
      { "Regret[profile->BEHAV, action->ACTION] =: NUMBER",
	GSM_Regret_Behav },
      { "Regret[profile->MIXED, strategy->STRATEGY] =: NUMBER",
	GSM_Regret_Mixed },
      { "Regrets[profile->BEHAV] =: LIST(LIST(LIST(NUMBER)))", 
	GSM_Regrets_Behav },
      { "Regrets[profile->MIXED] =: LIST(LIST(NUMBER))", GSM_Regrets_Mixed },
      { "SetActionProb[profile<->BEHAV, action->ACTION, value->NUMBER =: BEHAV", GSM_SetActionProb },
      { "SetActionProbs[profile<->BEHAV, infoset->INFOSET, value->LIST(NUMBER) =: BEHAV", GSM_SetActionProbs },
      { "SetStrategyProb[profile<->MIXED, strategy->STRATEGY, value->NUMBER =: MIXED", GSM_SetStrategyProb },
      { "SetStrategyProbs[profile<->MIXED, player->NFPLAYER, value->LIST(NUMBER) =: MIXED", GSM_SetStrategyProbs },
      { "StrategyProb[profile->MIXED, strategy->STRATEGY] =: NUMBER",
	GSM_StrategyProb },
      { "StrategyProbs[profile->MIXED] =: LIST(LIST(NUMBER))",
	GSM_StrategyProbs },
      { "StrategyValue[profile->MIXED, strategy->STRATEGY] =: NUMBER",
	GSM_StrategyValue },
      { "Support[profile->BEHAV] =: EFSUPPORT", GSM_Support_Behav },
      { "Support[profile->MIXED] =: NFSUPPORT", GSM_Support_Mixed },
      { 0, 0 }
    };

  for (int i = 0; ftable[i].sig != 0; i++) 
    gsm->AddFunction(new FuncDescObj(ftable[i].sig, ftable[i].func,
				     funcLISTABLE | funcGAMEMATCH));
}

