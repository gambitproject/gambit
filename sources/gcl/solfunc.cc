//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// GCL functions on profiles and solutions
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "game/efg.h"
#include "game/nfg.h"
#include "nash/behavsol.h"
#include "nash/mixedsol.h"

#include "game/efgutils.h"

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

static Portion *GSM_ActionProb(GSM &, Portion **param)
{
  const BehavSolution *profile = ((BehavPortion *) param[0])->Value();
  Action* action = ((ActionPortion*) param[1])->Value();
  Infoset* infoset = action->BelongsTo();
  gbtEfgPlayer player = infoset->GetPlayer();
  
  if (player.IsChance()) {
    return new NumberPortion(profile->ActionProb(action));
  }
  else if (profile->Support().Contains(action)) {
    return new NumberPortion(profile->ActionProb(action));
  }
  else {
    return new NumberPortion(0.0);
  }
}

//------------------
// ActionValue
//------------------

static Portion *GSM_ActionValue(GSM &, Portion **param)
{
  BehavSolution *profile = ((BehavPortion *) param[0])->Value();
  Action* action = ((ActionPortion*) param[1])->Value();
  Infoset *infoset = action->BelongsTo();

  if (infoset->GetPlayer().IsChance())
    return new NullPortion(porNUMBER);
  else if (profile->Support().Contains(action))  {
    if(profile->IsetProb(infoset)>gNumber(0.0))
      return new NumberPortion(profile->ActionValue(action));
    else
      return new NullPortion(porNUMBER);
  }
  else
    return new NullPortion(porNUMBER);
}

//--------------
// Behav
//--------------

static Portion *GSM_Behav(GSM &, Portion **param)
{
  EFSupport *S = ((EfSupportPortion *) param[0])->Value();
  return new BehavPortion(new BehavSolution(BehavProfile<gNumber>(*S)));
}

//-------------
// Belief
//-------------

static Portion *GSM_Belief(GSM &, Portion **param)
{
  BehavSolution *bp = ((BehavPortion *) param[0])->Value();
  Node* n = ((NodePortion*) param[1])->Value();
  Infoset *s = n->GetInfoset();
  if(!s)
    return new NullPortion(porNUMBER);
  return new NumberPortion(bp->BeliefProb(n));
}

//-------
// Game
//-------

static Portion* GSM_Game_Mixed(GSM &, Portion** param)
{
  return new NfgPortion(&((MixedPortion *) param[0])->Value()->Game());
}

static Portion *GSM_Game_NfSupport(GSM &, Portion **param)
{
  Nfg *N = (Nfg *) &((NfSupportPortion *) param[0])->Value()->Game();

  return new NfgPortion(N);
}

static Portion* GSM_Game_EfgTypes(GSM &, Portion** param)
{
  if(param[0]->Game())  {
    assert(param[0]->GameIsEfg());
    return new EfgPortion((efgGame*) param[0]->Game());
  }
  else
    return 0;
}

//---------------
// Creator
//---------------

static Portion *GSM_Creator_Behav(GSM &, Portion** param)
{
  BehavSolution *bs = ((BehavPortion*) param[0])->Value();
  return new TextPortion(bs->Creator());
}

static Portion *GSM_Creator_Mixed(GSM &, Portion** param)
{
  MixedSolution *ms = ((MixedPortion*) param[0])->Value();
  return new TextPortion(ms->Creator());
}

//---------------
// QreLambda
//---------------

static Portion *GSM_QreLambda_Behav(GSM &, Portion** param)
{
  BehavSolution *bs = ((BehavPortion*) param[0])->Value();
  if (bs->Creator() != "Qre[EFG]" && bs->Creator() != "Qre[NFG]") {
    return new NullPortion(porNUMBER);
  }
  return new NumberPortion(bs->QreLambda());
}

static Portion *GSM_QreLambda_Mixed(GSM &, Portion** param)
{
  MixedSolution *bs = ((MixedPortion*) param[0])->Value();
  if (bs->Creator() != "Qre[NFG]" && bs->Creator() != "QreGrid[NFG]") {
    return new NullPortion(porNUMBER);
  }
  return new NumberPortion(bs->QreLambda());
}

//----------------
// InfosetProb
//----------------

static Portion *GSM_InfosetProb(GSM &, Portion **param)
{
  if( param[1]->Spec().Type == porNULL )
    return new NullPortion( porNUMBER );

  BehavSolution *bp = ((BehavPortion *) param[0])->Value();
  Infoset* s = ((InfosetPortion*) param[1])->Value();
  //  if (s->IsChanceInfoset())
  //    throw gclRuntimeError("Not implemented for chance infosets");

  return new NumberPortion(bp->IsetProb(s));
}

//---------
// PolishEq
//---------

static Portion *GSM_PolishEq_Behav(GSM &, Portion **param)
{
  BehavSolution *P = new 
    BehavSolution((*((BehavPortion *) param[0])->Value()).PolishEq());
  BehavPortion *result = new BehavPortion(P);
  return result;
}

static Portion *GSM_PolishEq_Mixed(GSM &, Portion **param)
{
  MixedSolution *P = new 
    MixedSolution((*((MixedPortion *) param[0])->Value()).PolishEq());
  MixedPortion *result = new MixedPortion(P);
  return result;
}


//----------------
// InfosetValue
//----------------

static Portion *GSM_InfosetValue(GSM &, Portion **param)
{
  if( param[1]->Spec().Type == porNULL )
    return new NullPortion( porNUMBER );

  BehavSolution *bp = ((BehavPortion *) param[0])->Value();
  Infoset* s = ((InfosetPortion*) param[1])->Value();
  if (s->IsChanceInfoset())
    return new NullPortion( porNUMBER );

  return new NumberPortion(bp->IsetValue(s));
}

//---------
// IsNash
//---------

static Portion *GSM_IsNash_Behav(GSM &, Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new BoolPortion(P->IsNash());
}

static Portion *GSM_IsANFNash_Behav(GSM &, Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new BoolPortion(P->IsANFNash());
}

static Portion *GSM_IsNash_Mixed(GSM &, Portion **param)
{
  MixedSolution *P = ((MixedPortion*) param[0])->Value();
  return new BoolPortion(P->IsNash());
}


//-------------
// IsPerfect
//-------------

static Portion *GSM_IsPerfect(GSM &, Portion **param)
{
  MixedSolution *P = ((MixedPortion*) param[0])->Value();
  return new BoolPortion(P->IsPerfect());
}

//---------------
// IsSequential
//---------------

static Portion *GSM_IsSequential(GSM &, Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new BoolPortion(P->IsSequential());
}


//--------------------
// IsSubgamePerfect
//--------------------

static Portion *GSM_IsSubgamePerfect(GSM &, Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new BoolPortion(P->IsSubgamePerfect());
}

//-------------
// LiapValue
//-------------

static Portion *GSM_LiapValue_Behav(GSM &, Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new NumberPortion(P->LiapValue());
}

static Portion *GSM_LiapValue_Mixed(GSM &, Portion **param)
{
  MixedSolution *P = ((MixedPortion*) param[0])->Value();
  return new NumberPortion(P->LiapValue());
}

//-------------
// Accuracy
//-------------

static Portion *GSM_Accuracy_Behav(GSM &, Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new NumberPortion(P->Epsilon());
}

static Portion *GSM_Accuracy_Mixed(GSM &, Portion **param)
{
  MixedSolution *P = ((MixedPortion*) param[0])->Value();
  return new NumberPortion(P->Epsilon());
}

//----------
// Mixed
//----------

Portion* GSM_Mixed(GSM &, Portion** param)
{
  gbtNfgSupport *S = ((NfSupportPortion *) param[0])->Value();
  return new MixedPortion(new MixedSolution(MixedProfile<gNumber>(*S)));
}

//----------------
// NodeValue
//----------------

static Portion *GSM_NodeValue(GSM &, Portion **param)
{
  BehavSolution *bp = ((BehavPortion *) param[0])->Value();
  gbtEfgPlayer player = AsEfgPlayer(param[1]);
  Node* n = ((NodePortion*) param[2])->Value();

  return new NumberPortion(bp->NodeValue(n)[player.GetId()]);
}

//----------------
// RealizProb
//----------------

static Portion *GSM_RealizProb(GSM &, Portion **param)
{
  BehavSolution *bp = ((BehavPortion *) param[0])->Value();
  Node* n = ((NodePortion*) param[1])->Value();
  
  return new NumberPortion(bp->RealizProb(n)); 
}  

//-----------
// Regret
//-----------

static Portion *GSM_Regret_Mixed(GSM &, Portion **param)
{
  MixedProfile<gNumber> P(*(*((MixedPortion*) param[0])->Value()).Profile());
  gbtNfgStrategy s = ((StrategyPortion*) param[1])->Value();
  gbtNfgPlayer player = s.GetPlayer();
  Nfg &n = *player.GetGame();

  gPVector<gNumber> v(n.NumStrats());
  P.Regret(v);

  return new NumberPortion(v(player.GetId(), s.GetId()));
}

static Portion *GSM_Regret_Behav(GSM &, Portion **param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();

  const Action* a = ((ActionPortion*) param[1])->Value();

  if (a->BelongsTo()->IsChanceInfoset())
    return new NullPortion(porNUMBER);
  
  return new NumberPortion(P->Regret(a));
}

//------------
// Regrets
//------------

static Portion *GSM_Regrets_Mixed(GSM &, Portion **param)
{
  MixedProfile<gNumber> profile(*(*((MixedPortion *) param[0])->Value()).Profile());

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

static Portion *GSM_NfgRegrets_Behav(GSM &, Portion **param)
{
  BehavSolution *bp = ((BehavPortion *) param[0])->Value();
  gPVector<gNumber> v((*bp).ReducedNormalFormRegret());

  ListPortion *por = new ListPortion;

  for (int pl = 1; pl <= v.Lengths().Length(); pl++)  {
    ListPortion *p1 = new ListPortion;

    for (int st = 1; st <= v.Lengths()[pl]; st++)
      p1->Append(new NumberPortion(v(pl, st)));

    por->Append(p1);
  }

  return por;
}

//-------------------
// SetActionProb
//-------------------

static Portion *GSM_SetActionProb(GSM &, Portion **param)
{
  BehavSolution *P = new BehavSolution(*((BehavPortion *) param[0])->Value());
  Action *action = ((ActionPortion *) param[1])->Value();
  gNumber value = ((NumberPortion *) param[2])->Value();
  
  //  P->Set(action, value);
  (*P)[action] = value;
  ((BehavPortion *) param[0])->SetValue(P);
  return param[0]->RefCopy();
}

//-----------------
// SetActionProbs
//-----------------

static Portion *GSM_SetActionProbs(GSM &, Portion **param)
{
  int i;
  int j;
  int k;
  Portion* p3;
  int PlayerNum = 0;
  int InfosetNum = 0;
  
  BehavSolution *P = new BehavSolution(*((BehavPortion*) param[0])->Value());
  efgGame &E = P->GetGame();
  
  for (i = 1; i <= E.NumPlayers(); i++) {
    for(j = 1; j <= E.GetPlayer(i).NumInfosets(); j++) {
      if (((InfosetPortion*) param[1])->Value() ==
	  E.GetPlayer(i).GetInfoset(j)) {
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

  for (k = 1; 
       k <= E.GetPlayer(PlayerNum).GetInfoset(InfosetNum)->NumActions();
       k++) {
    p3 = ((ListPortion*) param[2])->SubscriptCopy(k);
    if(p3->Spec().ListDepth > 0)
    {
      delete p3;
      delete P;
      throw gclRuntimeError("Mismatching dimensionality");
    }

    assert(p3->Spec().Type == porNUMBER);
    P->Set((E.GetPlayer(PlayerNum).GetInfoset(InfosetNum)->GetAction(k)),
	   ((NumberPortion*) p3)->Value());

    delete p3;
  }

  ((BehavPortion *) param[0])->SetValue(P);
  return param[0]->RefCopy();
}

//-------------------
// SetStrategyProb
//-------------------

static Portion *GSM_SetStrategyProb(GSM &, Portion **param)
{
  MixedSolution *P = new MixedSolution(*((MixedPortion *) param[0])->Value());
  gbtNfgStrategy strategy = ((StrategyPortion *) param[1])->Value();
  gNumber value = ((NumberPortion *) param[2])->Value();
  P->Set(strategy, value);
  ((MixedPortion *) param[0])->SetValue(P);
  return param[0]->RefCopy();
}

//-------------------
// SetStrategyProbs
//-------------------

static Portion *GSM_SetStrategyProbs(GSM &, Portion **param)
{
  gbtNfgPlayer player = ((NfPlayerPortion *) param[1])->Value();

  MixedSolution *P = new MixedSolution(*((MixedPortion *) param[0])->Value());
  
  if (((ListPortion*) param[2])->Length() != player.NumStrategies()) {
    delete P;
    throw gclRuntimeError("Mismatching number of strategies");
  }

  for (int st = 1; st <= player.NumStrategies(); st++) {
    Portion *p2 = ((ListPortion*) param[2])->SubscriptCopy(st);
    if (p2->Spec().ListDepth > 0) {
      delete p2;
      delete P;
      throw gclRuntimeError("Mismatching dimensionality");
    }

    P->Set(player.GetStrategy(st), ((NumberPortion*) p2)->Value());
    delete p2;
  }

  ((MixedPortion *) param[0])->SetValue(P);
  return param[0]->RefCopy();
}

//----------------
// StrategyProb
//----------------

static Portion *GSM_StrategyProb(GSM &, Portion **param)
{
  const MixedSolution *profile = ((MixedPortion *) param[0])->Value();
  gbtNfgStrategy strategy = ((StrategyPortion*) param[1])->Value();
  return new NumberPortion((*profile)(strategy));
}

//----------------
// StrategyProbs
//----------------

static Portion *GSM_StrategyProbs(GSM &, Portion **param)
{
  const MixedSolution *profile = ((MixedPortion *) param[0])->Value();
  const Nfg &nfg = profile->Game();

  ListPortion *por = new ListPortion;
  for (int pl = 1; pl <= nfg.NumPlayers(); pl++)  {
    gbtNfgPlayer player = nfg.GetPlayer(pl);
    ListPortion *p1 = new ListPortion;

    for (int st = 1; st <= player.NumStrategies(); st++) {
      gbtNfgStrategy strategy = player.GetStrategy(st);
      p1->Append(new NumberPortion((*profile)(strategy)));
    }

    por->Append(p1);
  }

  return por;
}



//------------------
// StrategyValue
//------------------

static Portion *GSM_StrategyValue(GSM &, Portion **param)
{
  MixedSolution *profile = ((MixedPortion *) param[0])->Value();
  gbtNfgStrategy strategy = ((StrategyPortion*) param[1])->Value();

  return new NumberPortion(profile->Payoff(strategy.GetPlayer(), strategy));
}

//---------------
// Support
//---------------

static Portion *GSM_Support_Behav(GSM &, Portion** param)
{
  BehavSolution *P = ((BehavPortion *) param[0])->Value();
  return new EfSupportPortion(new EFSupport(P->Support()));
}

static Portion *GSM_Support_Mixed(GSM &, Portion** param)
{
  MixedSolution *P = ((MixedPortion *) param[0])->Value();
  return new NfSupportPortion(new gbtNfgSupport(P->Support()));
}


void Init_solfunc(GSM *gsm)
{
  static struct { char *sig; Portion *(*func)(GSM &, Portion **); } ftable[] =
    { { "Accuracy[profile->BEHAV] =: NUMBER", GSM_Accuracy_Behav },
      { "Accuracy[profile->MIXED] =: NUMBER", GSM_Accuracy_Mixed },
      { "ActionProb[profile->BEHAV, action->ACTION] =: NUMBER",
 	GSM_ActionProb },
      { "ActionValue[profile->BEHAV, action->ACTION] =: NUMBER",
	GSM_ActionValue },
      { "Behav[support->EFSUPPORT] =: BEHAV", GSM_Behav },
      { "Belief[profile->BEHAV, node->NODE] =: NUMBER", GSM_Belief },
      { "Game[profile->MIXED] =: NFG", GSM_Game_Mixed },
      { "Game[profile->BEHAV] =: EFG", GSM_Game_EfgTypes },
      { "Game[support->NFSUPPORT] =: NFG", GSM_Game_NfSupport },
      { "Game[support->EFSUPPORT] =: EFG", GSM_Game_EfgTypes },
      { "Game[basis->EFBASIS] =: EFG", GSM_Game_EfgTypes },
      { "Creator[profile->MIXED] =: TEXT", GSM_Creator_Mixed },
      { "Creator[profile->BEHAV] =: TEXT", GSM_Creator_Behav },
      { "QreLambda[profile->MIXED] =: NUMBER", GSM_QreLambda_Mixed },
      { "QreLambda[profile->BEHAV] =: NUMBER", GSM_QreLambda_Behav },
      { "InfosetProb[profile->BEHAV, infoset->INFOSET*] =: NUMBER",
	GSM_InfosetProb },
      { "InfosetValue[profile->BEHAV, infoset->INFOSET*] =: NUMBER",
	GSM_InfosetValue },
      { "IsNash[profile->BEHAV] =: BOOLEAN", GSM_IsNash_Behav },
      { "PolishEq[profile->BEHAV] =: BEHAV", GSM_PolishEq_Behav },
      { "IsANFNash[profile->BEHAV] =: BOOLEAN", GSM_IsANFNash_Behav },
      { "IsNash[profile->MIXED] =: BOOLEAN", GSM_IsNash_Mixed },
      { "PolishEq[profile->MIXED] =: MIXED", GSM_PolishEq_Mixed },
      { "IsPerfect[profile->MIXED] =: BOOLEAN", GSM_IsPerfect },
      { "IsSequential[profile->BEHAV] =: BOOLEAN", GSM_IsSequential },
      { "IsSubgamePerfect[profile->BEHAV] =: BOOLEAN", GSM_IsSubgamePerfect },
      { "LiapValue[profile->BEHAV] =: NUMBER", GSM_LiapValue_Behav },
      { "LiapValue[profile->MIXED] =: NUMBER", GSM_LiapValue_Mixed },
      { "Mixed[support->NFSUPPORT] =: MIXED", GSM_Mixed },
      { "NodeValue[profile->BEHAV, player->EFPLAYER, node->NODE] =: NUMBER",
	GSM_NodeValue },
      { "RealizProb[profile->BEHAV, node->NODE] =: NUMBER", GSM_RealizProb },
      { "Regret[profile->BEHAV, action->ACTION] =: NUMBER",
	GSM_Regret_Behav },
      { "Regret[profile->MIXED, strategy->STRATEGY] =: NUMBER",
	GSM_Regret_Mixed },
      { "NfgRegrets[profile->BEHAV] =: LIST(NUMBER)", 
	GSM_NfgRegrets_Behav },
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

  for (int i = 0; ftable[i].sig != 0; i++) {
    gsm->AddFunction(new gclFunction(*gsm, ftable[i].sig, ftable[i].func,
				     funcLISTABLE | funcGAMEMATCH));
  }
}

