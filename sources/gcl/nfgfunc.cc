//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// GCL functions for normal form games
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

#include "base/base.h"
#include "base/gstatus.h"

#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "game/nfg.h"
#include "game/nfdom.h"
#include "game/mixed.h"
#include "game/nfgensup.h"


//
// Implementations of these are provided as necessary in gsmutils.cc
//
template <class T> Portion *ArrayToList(const gArray<T> &);
template <class T> Portion *ArrayToList(const gList<T> &);

//---------------
// AddStrategy
//---------------

static Portion *GSM_AddStrategy(GSM &, Portion **param)
{
  gbtNfgSupport &support = AsNfgSupport(param[0]);
  gbtNfgStrategy strategy = AsNfgStrategy(param[1]);
  gbtNfgSupport *S = new gbtNfgSupport(support);
  S->AddStrategy(strategy);
  return new NfSupportPortion(S);
}

//------------
// Comment
//------------

static Portion *GSM_Comment(GSM &, Portion **param)
{
  return new TextPortion(AsNfg(param[0]).GetComment());
}

//---------------
// CompressNfg
//---------------

Nfg *CompressNfg(const Nfg &, const gbtNfgSupport &);

static Portion *GSM_CompressNfg(GSM &, Portion **param)
{
  gbtNfgSupport &S = AsNfgSupport(param[0]);
  Nfg *N = (Nfg *) &S.Game();

  return new NfgPortion(CompressNfg(*N, S));
}


//-----------------
// DeleteOutcome
//-----------------

static Portion *GSM_DeleteOutcome(GSM &gsm, Portion **param)
{
  gbtNfgOutcome outc = AsNfgOutcome(param[0]);
  gsm.InvalidateGameProfile(outc.GetGame(), false);
  outc.GetGame()->DeleteOutcome(outc);

  return new BoolPortion(true);
}

//-------------
// IsDominated
//-------------

static Portion *GSM_IsDominated_Nfg(GSM &, Portion **param)
{
  gbtNfgStrategy str = AsNfgStrategy(param[0]);
  gbtNfgSupport &S = AsNfgSupport(param[1]);
  bool strong = AsBool(param[2]);
  bool mixed = AsBool(param[3]);
  gPrecision prec = AsPrecision(param[4]);

  gWatch watch;
  bool ret;

  if (mixed)
    ret = IsMixedDominated(S, str, strong, prec, 
			   ((OutputPortion *) param[6])->Value());
  else   {
    ret = S.IsDominated(str, strong);
  }

  ((NumberPortion *) param[5])->SetValue(watch.Elapsed());
  
  return new BoolPortion(ret);
}


static Portion *GSM_IsProfileDominated_Nfg(GSM &, Portion **param)
{
  MixedSolution soln(*(*((MixedPortion *) param[0])->Value()).Profile());
  MixedProfile<gNumber> pr(*soln.Profile());
  bool strong = AsBool(param[1]);
  /*  bool mixed = AsBool(param[2]); */
  gPrecision prec = (((MixedPortion *) param[0])->Value())->Precision();

  gWatch watch;

  bool ret = IsMixedDominated(pr,strong, prec, 
			      ((OutputPortion *) param[4])->Value());

  ((NumberPortion *) param[3])->SetValue(watch.Elapsed());
  
  return new BoolPortion(ret);
}

//----------
// Game
//----------

static Portion *GSM_Game_NfPlayer(GSM &, Portion **param)
{
  return new NfgPortion(AsNfgPlayer(param[0]).GetGame());
}


static Portion *GSM_Game_Strategy(GSM &, Portion **param)
{
  return new NfgPortion(AsNfgStrategy(param[0]).GetPlayer().GetGame());
}

static Portion *GSM_Game_NfOutcome(GSM &, Portion **param)
{
  return new NfgPortion(AsNfgOutcome(param[0]).GetGame());
}


//--------------
// IsConstSum
//--------------

static Portion *GSM_IsConstSum(GSM &, Portion **param)
{
  return new BoolPortion(IsConstSum(AsNfg(param[0])));
}

//-----------
// LoadNfg
//-----------

static Portion *GSM_LoadNfg(GSM &, Portion **param)
{
  gText file = AsText(param[0]);

  Nfg *nfg = 0;

  try { 
    gFileInput f(file);
    nfg = ReadNfgFile(f);
    if (!nfg) {
      throw gclRuntimeError(file + "is not a valid .nfg file");
    }
    return new NfgPortion(nfg);
  }
  catch (gFileInput::OpenFailed &)  {
    throw gclRuntimeError("Unable to open file " + file + " for reading");
  }
}

//--------
// Name
//--------

static Portion* GSM_Name(GSM &, Portion **param)
{
  if (param[0]->Spec().Type == porNULL)
    return new TextPortion("");

  switch (param[0]->Spec().Type) {
  case porNFG:
    return new TextPortion(AsNfg(param[0]).GetTitle());
  case porNFPLAYER:
    return new TextPortion(AsNfgPlayer(param[0]).GetLabel());
  case porSTRATEGY:
    return new TextPortion(AsNfgStrategy(param[0]).GetLabel());
  case porNFOUTCOME:
    return new TextPortion(AsNfgOutcome(param[0]).GetLabel());
  case porNFSUPPORT:
    return new TextPortion(AsNfgSupport(param[0]).GetName());
  default:
    throw gclRuntimeError("Unknown type passed to Name[]");
  }
}



//----------
// NewNfg
//----------

static Portion *GSM_NewNfg(GSM &, Portion **param)
{
  ListPortion *dim = ((ListPortion *) param[0]);
  gArray<int> d(dim->Length());
  
  for (int i = 1; i <= dim->Length(); i++)
    d[i] = AsNumber((*dim)[i]);

  return new NfgPortion(new Nfg(d));
}


//--------------
// NewOutcome
//--------------

static Portion *GSM_NewOutcome(GSM &, Portion **param)
{
  return new NfOutcomePortion(AsNfg(param[0]).NewOutcome());
}

//-----------
// Outcome
//-----------

static Portion* GSM_Outcome(GSM &, Portion** param)
{
  int i;

  Nfg &nfg = 
    *((StrategyPortion *) (*((ListPortion *) param[0]))[1])->Value().GetPlayer().GetGame();
  
  if (nfg.NumPlayers() != ((ListPortion *) param[0])->Length())
    throw gclRuntimeError("Invalid profile");

  StrategyProfile profile(nfg);
  for (i = 1; i <= nfg.NumPlayers(); i++)  {
    gbtNfgStrategy strat =
      ((StrategyPortion *) (*((ListPortion *) param[0]))[i])->Value();
    if (strat.GetPlayer().GetId() != i) {
      throw gclRuntimeError("Invalid profile");
    }
    profile.Set(i, strat);
  }
  
  if (!nfg.GetOutcome(profile).IsNull()) {
    return new NfOutcomePortion(nfg.GetOutcome(profile));
  }
  else {
    return new NullPortion(porNFOUTCOME);
  }
}

//------------
// Outcomes
//------------

static Portion *GSM_Outcomes(GSM &, Portion **param)
{
  Nfg &N = AsNfg(param[0]);
  
  ListPortion *ret = new ListPortion;
  for (int outc = 1; outc <= N.NumOutcomes(); outc++) {
    ret->Append(new NfOutcomePortion(N.GetOutcomeId(outc)));
  }

  return ret;
}

//-----------
// Payoff
//-----------

static Portion* GSM_Payoff(GSM &, Portion** param)
{
  if (param[0]->Spec().Type == porNULL)
    return new NumberPortion(0);

  gbtNfgOutcome outcome = AsNfgOutcome(param[0]);
  Nfg *nfg = outcome.GetGame();
  gbtNfgPlayer player = AsNfgPlayer(param[1]);

  return new NumberPortion(nfg->Payoff(outcome, player.GetId()));
}


//------------
// Player
//------------

static Portion *GSM_Player(GSM &, Portion **param)
{
  if (param[0]->Spec().Type == porNULL) {
    return new NullPortion(porNFPLAYER);
  }

  return new NfPlayerPortion(AsNfgStrategy(param[0]).GetPlayer());
}

//------------
// Players
//------------

static Portion *GSM_Players(GSM &, Portion **param)
{
  Nfg &N = AsNfg(param[0]);

  ListPortion *ret = new ListPortion;
  for (int pl = 1; pl <= N.NumPlayers(); pl++) {
    ret->Append(new NfPlayerPortion(N.GetPlayer(pl)));
  }

  return ret;
}

//------------------------
// PossibleNashSupports
//------------------------

static Portion *GSM_PossibleNashSupportsNFG(GSM &gsm, Portion **param)
{
  return ArrayToList(PossibleNashSubsupports(gbtNfgSupport(AsNfg(param[0])),
					     gsm.GetStatusMonitor()));
}

//------------
// StrategyNumber
//------------

static Portion *GSM_StrategyNumber(GSM &, Portion **param)
{
  gbtNfgStrategy strategy = AsNfgStrategy(param[0]);
  gbtNfgSupport &support = AsNfgSupport(param[1]);
  return new NumberPortion(support.GetIndex(strategy));
}

//------------------
// RemoveStrategy
//------------------

static Portion *GSM_RemoveStrategy(GSM &, Portion **param)
{
  gbtNfgSupport &support = AsNfgSupport(param[0]);
  gbtNfgStrategy strategy = AsNfgStrategy(param[1]);
  
  gbtNfgSupport *S = new gbtNfgSupport(support);
  S->RemoveStrategy(strategy);
  return new NfSupportPortion(S);
}

//------------
// SaveNfg
//------------

extern NumberPortion _WriteGameDecimals;

static Portion *GSM_SaveNfg(GSM &, Portion **param)
{
  Nfg &N = AsNfg(param[0]);
  gText file = AsText(param[1]);
  try {
    gFileOutput f(file);
    N.WriteNfgFile(f, _WriteGameDecimals.Value());
  }
  catch (gFileOutput::OpenFailed &)  {
    throw gclRuntimeError("Unable to open file " + file + " for output");
  }

  return param[0]->ValCopy();
}

//-------------
// SetComment
//-------------

static Portion *GSM_SetComment(GSM &, Portion **param)
{
  AsNfg(param[0]).SetComment(AsText(param[1]));
  return param[0]->ValCopy();
}

//------------
// SetName
//------------

static Portion *GSM_SetName_Nfg(GSM &, Portion **param)
{
  AsNfg(param[0]).SetTitle(AsText(param[1]));
  return param[0]->ValCopy();
}

static Portion *GSM_SetName_NfPlayer(GSM &, Portion **param)
{
  AsNfgPlayer(param[0]).SetLabel(AsText(param[1]));
  return param[0]->ValCopy();
}

static Portion *GSM_SetName_Strategy(GSM &, Portion **param)
{
  AsNfgStrategy(param[0]).SetLabel(AsText(param[1]));
  return param[0]->ValCopy();
}

static Portion *GSM_SetName_NfOutcome(GSM &, Portion **param)
{
  AsNfgOutcome(param[0]).GetGame()->SetLabel(AsNfgOutcome(param[0]),
					     AsText(param[1]));
  return param[0]->ValCopy();
}

static Portion *GSM_SetName_NfSupport(GSM &, Portion **param)
{
  AsNfgSupport(param[0]).SetName(AsText(param[1]));
  return param[0]->RefCopy();
}

//------------
// SetOutcome
//------------

static Portion* GSM_SetOutcome(GSM &gsm, Portion** param)
{
  Nfg &nfg = 
    *((StrategyPortion *) (*((ListPortion *) param[0]))[1])->Value().GetPlayer().GetGame();
  
  if (nfg.NumPlayers() != ((ListPortion *) param[0])->Length())
    throw gclRuntimeError("Invalid profile");

  StrategyProfile profile(nfg);
  for (int i = 1; i <= nfg.NumPlayers(); i++)  {
    gbtNfgStrategy strat =
      ((StrategyPortion *) (*((ListPortion *) param[0]))[i])->Value();
    if (strat.GetPlayer().GetId() != i) {
      throw gclRuntimeError("Invalid profile");
    }
    profile.Set(i, strat);
  }
  
  gbtNfgOutcome outcome = ((NfOutcomePortion *) param[1])->Value();

  nfg.SetOutcome(profile, outcome);

  gsm.InvalidateGameProfile((Nfg *) &nfg, false);
 
  return param[1]->ValCopy();
}

//------------
// SetPayoff
//------------

static Portion* GSM_SetPayoff(GSM &gsm, Portion** param)
{
  gbtNfgOutcome outcome = AsNfgOutcome(param[0]);
  Nfg *nfg = outcome.GetGame();
  gbtNfgPlayer player = AsNfgPlayer(param[1]);
  gNumber value = AsNumber(param[2]);

  nfg->SetPayoff(outcome, player.GetId(), value);

  gsm.InvalidateGameProfile((Nfg *) nfg, false);
 
  return param[0]->ValCopy();
}


//--------------
// Strategies
//--------------

static Portion *GSM_Strategies(GSM &, Portion **param)
{
  gbtNfgPlayer player = AsNfgPlayer(param[0]);
  const gbtNfgSupport &support = AsNfgSupport(param[1]);

  ListPortion *ret = new ListPortion;
  for (int st = 1; st <= player.NumStrategies(); st++) {
    ret->Append(new StrategyPortion(support.GetStrategy(player.GetId(), st)));
  }
  return ret;
}

//--------------
// Support
//--------------

static Portion *GSM_Support(GSM &, Portion **param)
{
  return new NfSupportPortion(new gbtNfgSupport(AsNfg(param[0])));
}

//-------------
// UnDominated
//-------------

static Portion *GSM_UnDominated(GSM &gsm, Portion **param)
{
  gbtNfgSupport &S = AsNfgSupport(param[0]);
  bool strong = AsBool(param[1]);
  bool mixed = AsBool(param[2]);
  gPrecision prec = AsPrecision(param[3]);

  gWatch watch;
  gBlock<int> players(S.Game().NumPlayers());
  for (int i = 1; i <= players.Length(); i++)   players[i] = i;

  gbtNfgSupport *T;

  if (mixed)
    T = new gbtNfgSupport(S.MixedUndominated(strong, prec, players,
					     ((OutputPortion *) param[5])->Value(), 
					  gsm.GetStatusMonitor()));
  else   {
    T = new gbtNfgSupport(S.Undominated(strong, players,
					((OutputPortion *) param[5])->Value(),
					gsm.GetStatusMonitor()));
  }

  return new NfSupportPortion(T);
}


void Init_nfgfunc(GSM *gsm)
{
  gclFunction *FuncObj;

  static struct { char *sig; Portion *(*func)(GSM &, Portion **); } ftable[] =
    { { "AddStrategy[support->NFSUPPORT, strategy->STRATEGY] =: NFSUPPORT",
 	GSM_AddStrategy },
      { "Comment[nfg->EFG] =: TEXT", GSM_Comment },
      { "CompressNfg[support->NFSUPPORT] =: NFG", GSM_CompressNfg },
      { "DeleteOutcome[outcome->NFOUTCOME] =: BOOLEAN", GSM_DeleteOutcome },
      { "Game[player->NFPLAYER] =: NFG", GSM_Game_NfPlayer },
      { "Game[strategy->STRATEGY] =: NFG", GSM_Game_Strategy },
      { "Game[outcome->NFOUTCOME] =: NFG", GSM_Game_NfOutcome }, 
      { "IsConstSum[nfg->NFG] =: BOOLEAN", GSM_IsConstSum },
      { "LoadNfg[file->TEXT] =: NFG", GSM_LoadNfg },
      { "Name[x->NFG*] =: TEXT", GSM_Name },
      { "Name[x->NFPLAYER*] =: TEXT", GSM_Name },
      { "Name[x->STRATEGY*] =: TEXT", GSM_Name },
      { "Name[x->NFOUTCOME*] =: TEXT", GSM_Name },
      { "Name[x->NFSUPPORT*] =: TEXT", GSM_Name },
      { "NewNfg[dim->LIST(INTEGER)] =: NFG", GSM_NewNfg },
      { "NewOutcome[nfg->NFG] =: NFOUTCOME", GSM_NewOutcome },
      { "Outcome[profile->LIST(STRATEGY)] =: NFOUTCOME", GSM_Outcome },
      { "Outcomes[nfg->NFG] =: LIST(NFOUTCOME)", GSM_Outcomes },
      { "Payoff[outcome->NFOUTCOME*, player->NFPLAYER] =: NUMBER",
	GSM_Payoff },
      { "Player[strategy->STRATEGY*] =: NFPLAYER", GSM_Player },
      { "Players[nfg->NFG] =: LIST(NFPLAYER)", GSM_Players },
      { "RemoveStrategy[support->NFSUPPORT, strategy->STRATEGY] =: NFSUPPORT",
	GSM_RemoveStrategy },
      { "SaveNfg[nfg->NFG, file->TEXT] =: NFG", GSM_SaveNfg },
      { "SetComment[nfg->NFG, comment->TEXT] =: NFG", GSM_SetComment },
      { "SetName[x->NFG, name->TEXT] =: NFG", GSM_SetName_Nfg },
      { "SetName[x->NFPLAYER, name->TEXT] =: NFPLAYER", GSM_SetName_NfPlayer },
      { "SetName[x->STRATEGY, name->TEXT] =: STRATEGY", GSM_SetName_Strategy },
      { "SetName[x->NFOUTCOME, name->TEXT] =: NFOUTCOME",
	GSM_SetName_NfOutcome },
      { "SetName[x->NFSUPPORT, name->TEXT] =: NFSUPPORT", GSM_SetName_NfSupport },
      { "SetOutcome[profile->LIST(STRATEGY), outcome->NFOUTCOME] =: NFOUTCOME",
	GSM_SetOutcome },
      { "SetPayoff[outcome->NFOUTCOME, player->NFPLAYER, payoff->NUMBER] =: NFOUTCOME", GSM_SetPayoff },
      { "Strategies[player->NFPLAYER, support->NFSUPPORT] =: LIST(STRATEGY)",
	GSM_Strategies },
      { "StrategyNumber[strategy->STRATEGY, sup->NFSUPPORT] =: NUMBER", 
	GSM_StrategyNumber },
      { "Support[nfg->NFG] =: NFSUPPORT", GSM_Support },
      { "PossibleNashSupports[nfg->NFG] =: LIST(NFSUPPORT)", 
	GSM_PossibleNashSupportsNFG },

      { 0, 0 }
    };

  for (int i = 0; ftable[i].sig != 0; i++) {
    gsm->AddFunction(new gclFunction(*gsm, ftable[i].sig, ftable[i].func,
				     funcLISTABLE | funcGAMEMATCH));
  }

  FuncObj = new gclFunction(*gsm, "UnDominated", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_UnDominated,
				       porNFSUPPORT, 7));
  FuncObj->SetParamInfo(0, 0, gclParameter("support", porNFSUPPORT));
  FuncObj->SetParamInfo(0, 1, gclParameter("strong", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(0, 2, gclParameter("mixed", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(0, 3, gclParameter("precision", porPRECISION,
					   new PrecisionPortion(precRATIONAL)));
  FuncObj->SetParamInfo(0, 4, gclParameter("time", porNUMBER,
					   new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 5, gclParameter("traceFile", porOUTPUT,
					   new OutputPortion(*new gNullOutput), 
					   BYREF));
  FuncObj->SetParamInfo(0, 6, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "IsDominated", 2);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_IsDominated_Nfg,
				       porBOOLEAN, 8));
  FuncObj->SetParamInfo(0, 0, gclParameter("strategy", porSTRATEGY));
  FuncObj->SetParamInfo(0, 1, gclParameter("support", porNFSUPPORT));
  FuncObj->SetParamInfo(0, 2, gclParameter("strong", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(0, 3, gclParameter("mixed", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(0, 4, gclParameter("precision", porPRECISION,
					   new PrecisionPortion(precRATIONAL)));
  FuncObj->SetParamInfo(0, 5, gclParameter("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 6, gclParameter("traceFile", porOUTPUT,
					   new OutputPortion(*new gNullOutput), 
					   BYREF));
  FuncObj->SetParamInfo(0, 7, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));
  FuncObj->SetFuncInfo(1, gclSignature(GSM_IsProfileDominated_Nfg,
				       porBOOLEAN, 6));
  FuncObj->SetParamInfo(1, 0, gclParameter("profile", porMIXED));
  FuncObj->SetParamInfo(1, 1, gclParameter("strong", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(1, 2, gclParameter("mixed", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(1, 3, gclParameter("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(1, 4, gclParameter("traceFile", porOUTPUT,
					   new OutputPortion(*new gNullOutput), 
					   BYREF));
  FuncObj->SetParamInfo(1, 5, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));
  gsm->AddFunction(FuncObj);
}

