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
  BehavSolution *profile = ((BehavPortion *) param[0])->Value();
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
  BehavSolution *profile = ((BehavPortion *) param[0])->Value();
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

static Portion *GSM_Behav_EFSupport(Portion **param)
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

static Portion *GSM_IsKnownNotPerfect_Mixed(Portion **param)
{
  MixedSolution *P = ((MixedPortion*) param[0])->Value();
  return new BoolPortion(P->IsPerfect() == T_NO);
}

//-----------------------
// IsKnownNotSequential
//-----------------------

static Portion *GSM_IsKnownNotSequential_Behav(Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new BoolPortion(P->IsSequential() == T_NO);
}

//---------------------------
// IsKnownNotSubgamePerfect
//---------------------------

static Portion *GSM_IsKnownNotSubgamePerfect_Behav(Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new BoolPortion(P->IsSubgamePerfect() == T_NO);
}

//-------------------
// IsKnownPerfect
//-------------------

static Portion *GSM_IsKnownPerfect_Mixed(Portion **param)
{
  MixedSolution *P = ((MixedPortion*) param[0])->Value();
  return new BoolPortion(P->IsPerfect() == T_YES);
}

//--------------------
// IsKnownSequential
//--------------------

static Portion *GSM_IsKnownSequential_Behav(Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new BoolPortion(P->IsSequential() == T_YES);
}


//------------------------
// IsKnownSubgamePerfect
//------------------------

static Portion *GSM_IsKnownSubgamePerfect_Behav(Portion **param)
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

Portion* GSM_Mixed_NFSupport(Portion** param)
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
  
  BehavSolution *P = ((BehavPortion*) param[0])->Value();
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
     E.Players()[PlayerNum]->Infosets()[InfosetNum]->NumActions())
    return new ErrorPortion("Mismatching number of actions");
  
  for(k = 1; 
      k <= E.Players()[PlayerNum]->Infosets()[InfosetNum]->NumActions();
      k++)
  {
    p3 = ((ListPortion*) param[2])->SubscriptCopy(k);
    if(p3->Spec().ListDepth > 0)
    {
      delete p3;
      return new ErrorPortion("Mismatching dimensionality");
    }

    assert(p3->Spec().Type == porNUMBER);
    (*P)(PlayerNum, InfosetNum, k) = ((NumberPortion*) p3)->Value();

    delete p3;
  }

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

  MixedSolution *P = ((MixedPortion *) param[0])->Value();
  Nfg& N = P->Game();
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

    assert(p2->Spec().Type == porNUMBER);
    (*P)(PlayerNum, j) = ((NumberPortion*) p2)->Value();

    delete p2;
  }

  return param[0]->RefCopy();
}

//----------------
// StrategyProb
//----------------

static Portion *GSM_StrategyProb(Portion **param)
{
  Portion *por;
  MixedSolution *profile = ((MixedPortion *) param[0])->Value();
  Strategy* strategy = ((StrategyPortion*) param[1])->Value();
  NFPlayer* player = strategy->nfp;
  
  if (profile->Support().Strategies(player->GetNumber()).Find(strategy))
    por = new NumberPortion((*profile)
			      (player->GetNumber(),
			       profile->Support().Strategies(player->GetNumber()).Find(strategy)));
  else
    por = new NumberPortion(0.0);
  
  return por;
}

//----------------
// StrategyProbs
//----------------

static Portion *GSM_StrategyProbs(Portion **param)
{
  MixedSolution *profile = ((MixedPortion *) param[0])->Value();
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
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("ActionProb", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ActionProb, porNUMBER, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("action", porACTION));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ActionProbs", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ActionProbs,
				       PortionSpec(porNUMBER, 3), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("StrategyValue", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_StrategyValue, porNUMBER, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porMIXED));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("strategy", porSTRATEGY));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ActionValue", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ActionValue, porNUMBER, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("action", porACTION));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ActionValues", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ActionValues,
				       PortionSpec(porNUMBER, 3), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Behav", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Behav_EFSupport, 
				       porBEHAV, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porEFSUPPORT));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Belief", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Belief, porNUMBER, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Beliefs", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Beliefs,
				       PortionSpec(porNUMBER, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Game", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Game_Mixed, porNFG, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porMIXED));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Game_EfgTypes, porEFG, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Game_NfSupport, porNFG, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("support", porNFSUPPORT));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Game_EfgTypes, porEFG, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("support", porEFSUPPORT));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("GobitLambda", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_GobitLambda_Mixed,
				       porNUMBER, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porMIXED));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_GobitLambda_Behav,
				       porNUMBER, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("GobitValue", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_GobitValue_Mixed,
				       porNUMBER, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porMIXED));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_GobitValue_Behav,
				       porNUMBER, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porBEHAV));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("InfosetProb", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_InfosetProb, porNUMBER, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("infoset", 
                              PortionSpec(porINFOSET, 0, porNULLSPEC) ));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("InfosetProbs", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_InfosetProbs,
				       PortionSpec(porNUMBER, 2), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsKnownNash", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsKnownNash_Behav, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsKnownNash_Mixed, porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porMIXED));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsKnownNotNash", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsKnownNotNash_Behav, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IsKnownNotNash_Mixed, porBOOL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porMIXED));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsKnownNotPerfect", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsKnownNotPerfect_Mixed, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porMIXED));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsKnownNotSequential", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsKnownNotSequential_Behav, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsKnownNotSubgamePerfect", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsKnownNotSubgamePerfect_Behav,
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsKnownPerfect", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsKnownPerfect_Mixed, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porMIXED));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsKnownSequential", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsKnownSequential_Behav, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsKnownSubgamePerfect", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsKnownSubgamePerfect_Behav,
				       porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("LiapValue", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_LiapValue_Behav, porNUMBER, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_LiapValue_Mixed, porNUMBER, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porMIXED));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Mixed", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Mixed_NFSupport, porMIXED, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porNFSUPPORT));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NodeValue", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NodeValue, porNUMBER, 3,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("player", porEFPLAYER));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NodeValues", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NodeValues,
				       PortionSpec(porNUMBER, 1), 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("player", porEFPLAYER));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("RealizProb", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_RealizProb, porNUMBER, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("node", porNODE));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RealizProbs", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_RealizProbs,
				       PortionSpec(porNUMBER, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Regret", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Regret_Behav, porNUMBER, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("action", porACTION));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Regret_Mixed, porNUMBER, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porMIXED));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("strategy", porSTRATEGY));
 gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Regrets", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Regrets_Behav,
				       PortionSpec(porNUMBER, 3), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Regrets_Mixed,
				       PortionSpec(porNUMBER, 2), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porMIXED));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("SetActionProbs", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetActionProbs, porBEHAV, 3,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("infoset", porINFOSET));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("value", PortionSpec(porNUMBER, 1)));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("SetStrategyProbs", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetStrategyProbs, porMIXED, 3,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porMIXED, REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("player", porNFPLAYER));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("value", PortionSpec(porNUMBER,1)));
 gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("StrategyProb", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_StrategyProb, porNUMBER, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porMIXED));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("strategy", porSTRATEGY));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("StrategyProbs", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_StrategyProbs,
				       PortionSpec(porNUMBER, 2), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porMIXED));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Support", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Support_Behav, porEFSUPPORT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", porBEHAV));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Support_Mixed, porNFSUPPORT, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("profile", porMIXED));
  gsm->AddFunction(FuncObj);
}

