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
    return new NumberPortion((*profile)(action));
  else
    return new NumberPortion(0.0);
}

//----------------
// ActionProbs
//----------------

static Portion *GSM_ActionProbs(Portion **param)
{
  const BehavSolution *profile = ((BehavPortion *) param[0])->Value();
  const Efg &efg = profile->Game();

  ListPortion *por = new ListPortion;
  for (int pl = 1; pl <= efg.NumPlayers(); pl++)  {
    EFPlayer *player = efg.Players()[pl];
    ListPortion *p1 = new ListPortion;
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      Infoset *infoset = player->Infosets()[iset];
      ListPortion *p2 = new ListPortion;
      for (int act = 1; act <= infoset->NumActions(); act++) 
	p2->Append(new NumberPortion((*profile)(infoset->Actions()[act])));
      p1->Append(p2);
    }
    por->Append(p1);
  }

  return por;
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
  return new BehavPortion(new BehavSolution(BehavProfile<gNumber>(*S)));
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
// QreLambda
//---------------

static Portion *GSM_QreLambda_Behav(Portion** param)
{
  BehavSolution *bs = ((BehavPortion*) param[0])->Value();
  if (bs->Creator() != algorithmEfg_QRE_EFG &&
      bs->Creator() != algorithmEfg_QRE_NFG)
    return new NullPortion(porNUMBER);
  return new NumberPortion(bs->QreLambda());
}

static Portion *GSM_QreLambda_Mixed(Portion** param)
{
  MixedSolution *bs = ((MixedPortion*) param[0])->Value();
  if (bs->Creator() != algorithmNfg_QRE && 
      bs->Creator() != algorithmNfg_QREALL)
    return new NullPortion(porNUMBER);
  return new NumberPortion(bs->QreLambda());
}

//--------------
// QreValue
//--------------

static Portion *GSM_QreValue_Behav(Portion** param)
{
  BehavSolution *bs = ((BehavPortion*) param[0])->Value();
  if (bs->Creator() != algorithmEfg_QRE_EFG &&
      bs->Creator() != algorithmEfg_QRE_NFG)
    return new NullPortion(porNUMBER);
  return new NumberPortion(bs->QreValue());
}

static Portion *GSM_QreValue_Mixed(Portion** param)
{
  MixedSolution *bs = ((MixedPortion*) param[0])->Value();
  if (bs->Creator() != algorithmNfg_QRE && 
      bs->Creator() != algorithmNfg_QREALL)
    return new NullPortion(porNUMBER);
  return new NumberPortion(bs->QreValue());
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
    throw gclRuntimeError("Not implemented for chance infosets");

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


//---------
// IsNash
//---------

static Portion *GSM_IsNash_Behav(Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new BoolPortion(P->IsNash());
}

static Portion *GSM_IsNash_Mixed(Portion **param)
{
  MixedSolution *P = ((MixedPortion*) param[0])->Value();
  return new BoolPortion(P->IsNash());
}


//-------------
// IsPerfect
//-------------

static Portion *GSM_IsPerfect(Portion **param)
{
  MixedSolution *P = ((MixedPortion*) param[0])->Value();
  return new BoolPortion(P->IsPerfect());
}

//---------------
// IsSequential
//---------------

static Portion *GSM_IsSequential(Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new BoolPortion(P->IsSequential());
}


//--------------------
// IsSubgamePerfect
//--------------------

static Portion *GSM_IsSubgamePerfect(Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new BoolPortion(P->IsSubgamePerfect());
}

//-------------
// LiapValue
//-------------

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
  return new MixedPortion(new MixedSolution(MixedProfile<gNumber>(*S)));
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
  MixedProfile<gNumber> P(*((MixedPortion*) param[0])->Value());
  Strategy* s = ((StrategyPortion*) param[1])->Value();
  NFPlayer* p = s->Player();
  Nfg &n = p->Game();

  gPVector<gNumber> v(n.NumStrats());
  P.Regret(v);

  return new NumberPortion(v(p->GetNumber(), s->Number()));
}

static Portion *GSM_Regret_Behav(Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();

  const Action* a = ((ActionPortion*) param[1])->Value();
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
  MixedProfile<gNumber> profile(*((MixedPortion *) param[0])->Value());

  gPVector<gNumber> v(profile.Game().NumStrats());

  profile.Regret(v);

  ListPortion *por = new ListPortion;
  
  for (int pl = 1; pl <= profile.Lengths().Length(); pl++)  {
    ListPortion *p1 = new ListPortion;

    for (int st = 1; st <= profile.Lengths()[pl]; st++)
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
  Action *action = ((ActionPortion *) param[1])->Value();
  gNumber value = ((NumberPortion *) param[2])->Value();
  
  P->Set(action, value);
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
  
  if (((ListPortion*) param[2])->Length() != 
      P->Support().NumActions(PlayerNum, InfosetNum))  {
    delete P;
    throw gclRuntimeError("Mismatching number of actions");
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
      throw gclRuntimeError("Mismatching dimensionality");
    }

    assert(p3->Spec().Type == porNUMBER);
    P->Set((E.Players()[PlayerNum]->Infosets()[InfosetNum]->Actions()[k]),
	   ((NumberPortion*) p3)->Value());

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
  Strategy *strategy = ((StrategyPortion *) param[1])->Value();
  gNumber value = ((NumberPortion *) param[2])->Value();
  P->Set(strategy, value);
  ((MixedPortion *) param[0])->SetValue(P);
  return param[0]->RefCopy();
}

//-------------------
// SetStrategyProbs
//-------------------

static Portion *GSM_SetStrategyProbs(Portion **param)
{
  NFPlayer *player = ((NfPlayerPortion *) param[1])->Value();

  MixedSolution *P = new MixedSolution(*((MixedPortion *) param[0])->Value());
  
  if (((ListPortion*) param[2])->Length() != player->NumStrats()) {
    delete P;
    throw gclRuntimeError("Mismatching number of strategies");
  }

  for (int st = 1; st <= player->NumStrats(); st++) {
    Portion *p2 = ((ListPortion*) param[2])->SubscriptCopy(st);
    if (p2->Spec().ListDepth > 0) {
      delete p2;
      delete P;
      throw gclRuntimeError("Mismatching dimensionality");
    }

    P->Set(player->Strategies()[st], ((NumberPortion*) p2)->Value());
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
  return new NumberPortion((*profile)(strategy));
}

//----------------
// StrategyProbs
//----------------

static Portion *GSM_StrategyProbs(Portion **param)
{
  const MixedSolution *profile = ((MixedPortion *) param[0])->Value();
  const Nfg &nfg = profile->Game();

  ListPortion *por = new ListPortion;
  for (int pl = 1; pl <= nfg.NumPlayers(); pl++)  {
    NFPlayer *player = nfg.Players()[pl];
    ListPortion *p1 = new ListPortion;

    for (int st = 1; st <= player->NumStrats(); st++) {
      Strategy *strategy = player->Strategies()[st];
      p1->Append(new NumberPortion((*profile)(strategy)));
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
  Strategy *strategy = ((StrategyPortion*) param[1])->Value();

  return new NumberPortion(profile->Payoff(strategy->Player(), strategy));
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
  return new NfSupportPortion(new NFSupport(P->Game()));
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
      { "QreLambda[profile->MIXED] =: NUMBER", GSM_QreLambda_Mixed },
      { "QreLambda[profile->BEHAV] =: NUMBER", GSM_QreLambda_Behav },
      { "QreValue[profile->MIXED] =: NUMBER", GSM_QreValue_Mixed },
      { "QreValue[profile->BEHAV] =: NUMBER", GSM_QreValue_Behav },
      { "InfosetProb[profile->BEHAV, infoset->INFOSET*] =: NUMBER",
	GSM_InfosetProb },
      { "InfosetProbs[profile->BEHAV] =: LIST(LIST(NUMBER))", 
	GSM_InfosetProbs },
      { "IsNash[profile->BEHAV] =: BOOLEAN", GSM_IsNash_Behav },
      { "IsNash[profile->MIXED] =: BOOLEAN", GSM_IsNash_Mixed },
      { "IsPerfect[profile->MIXED] =: BOOLEAN", GSM_IsPerfect },
      { "IsSequential[profile->BEHAV] =: BOOLEAN", GSM_IsSequential },
      { "IsSubgamePerfect[profile->BEHAV] =: BOOLEAN", GSM_IsSubgamePerfect },
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
    gsm->AddFunction(new gclFunction(ftable[i].sig, ftable[i].func,
				     funcLISTABLE | funcGAMEMATCH));
}

