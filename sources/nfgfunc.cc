//
// FILE: nfgfunc.cc -- Normal form command language builtins
//
// $Id$
//


#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "nfg.h"
#include "nfplayer.h"

#include "glist.h"
#include "mixed.h"

#include "gwatch.h"
#include "gstatus.h"

//
// Implementations of these are provided as necessary in gsmutils.cc
//
Portion *ArrayToList(const gArray<int> &);
Portion *ArrayToList(const gArray<NFPlayer *> &);
Portion *ArrayToList(const gArray<NFOutcome *> &);
Portion *ArrayToList(const gArray<Strategy *> &);
Portion *ArrayToList(const gList<const NFSupport> &);

extern GSM *_gsm;


//---------------
// AddStrategy
//---------------

static Portion *GSM_AddStrategy(Portion **param)
{
  NFSupport *support = ((NfSupportPortion *) param[0])->Value();
  Strategy *s = ((StrategyPortion *) param[1])->Value();

  NFSupport *S = new NFSupport(*support);
  S->AddStrategy(s);

  return new NfSupportPortion(S);
}

//------------
// Comment
//------------

static Portion *GSM_Comment(Portion **param)
{
  Nfg *nfg = ((NfgPortion *) param[0])->Value();
  return new TextPortion(nfg->GetComment());
}

//---------------
// CompressNfg
//---------------

Nfg *CompressNfg(const Nfg &, const NFSupport &);

static Portion *GSM_CompressNfg(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  Nfg *N = (Nfg *) &S->Game();

  return new NfgPortion(CompressNfg(*N, *S));
}


//-----------------
// DeleteOutcome
//-----------------

static Portion *GSM_DeleteOutcome(Portion **param)
{
  NFOutcome *outc = ((NfOutcomePortion *) param[0])->Value();

  _gsm->InvalidateGameProfile(outc->Game(), false);
//  _gsm->UnAssignGameElement(outc->BelongsTo(), false, porNFOUTCOME, outc);

  outc->Game()->DeleteOutcome(outc);

  return new BoolPortion(true);
}


//-------------
// ElimDom
//-------------

static Portion *GSM_ElimDom_Nfg(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  bool strong = ((BoolPortion *) param[1])->Value();
  bool mixed = ((BoolPortion *) param[2])->Value();
  gPrecision prec = ((PrecisionPortion *) param[3])->Value();

  gWatch watch;
  gBlock<int> players(S->Game().NumPlayers());
  for (int i = 1; i <= players.Length(); i++)   players[i] = i;

  NFSupport *T;
  Portion *por;

  Nfg *N = (Nfg *) &S->Game();
  if (mixed)
    T = ComputeMixedDominated(*S, strong, prec, players,
			      ((OutputPortion *) param[5])->Value(), gstatus);
  else   {
    T = ComputeDominated(*N, *S, strong, players,
			   ((OutputPortion *) param[5])->Value(), gstatus);
  }

  por = ((T) ? new NfSupportPortion(T) :
	 new NfSupportPortion(new NFSupport(*S)));

  ((NumberPortion *) param[4])->SetValue(watch.Elapsed());
  
  return por;
}

//----------
// Game
//----------

static Portion *GSM_Game_NfPlayer(Portion **param)
{
  Nfg &N = ((NfPlayerPortion *) param[0])->Value()->Game();

  return new NfgPortion(&N);
}


static Portion *GSM_Game_Strategy(Portion **param)
{
  Nfg &N = ((StrategyPortion *) param[0])->Value()->Player()->Game();

  return new NfgPortion(&N);
}

static Portion *GSM_Game_NfOutcome(Portion **param)
{
  Nfg *N = ((NfOutcomePortion *) param[0])->Value()->Game();

  return new NfgPortion(N);
}


//--------------
// IsConstSum
//--------------

static Portion *GSM_IsConstSum(Portion **param)
{
  Nfg *N = ((NfgPortion *) param[0])->Value();
  return new BoolPortion(IsConstSum(*N));
}

//-----------
// LoadNfg
//-----------

extern int ReadNfgFile(gInput &f, Nfg *& N);

static Portion *GSM_LoadNfg(Portion **param)
{
  gText file = ((TextPortion *) param[0])->Value();

  Nfg *nfg = 0;

  try { 
    gFileInput f(file);
    if (!ReadNfgFile(f, nfg))
      throw gclRuntimeError(file + "is not a valid .nfg file");
    return new NfgPortion(nfg);
  }
  catch (gFileInput::OpenFailed &)  {
    throw gclRuntimeError("Unable to open file " + file + " for reading");
  }
}

//--------
// Name
//--------

static Portion* GSM_Name(Portion **param)
{
  if (param[0]->Spec().Type == porNULL)
    return new TextPortion("");

  switch (param[0]->Spec().Type) {
  case porNFG:
    return new TextPortion(((NfgPortion*) param[0])->Value()->GetTitle());
  case porNFPLAYER:
    return new TextPortion(((NfPlayerPortion*) param[0])->Value()->
			      GetName());
  case porSTRATEGY:
    return new TextPortion(((StrategyPortion*) param[0])->Value()->Name());
  case porNFOUTCOME:
    return new TextPortion(((NfOutcomePortion*) param[0])->Value()->
			   GetName());
  case porNFSUPPORT:
    return new TextPortion(((NfSupportPortion*) param[0])->Value()->GetName());
  default:
    throw gclRuntimeError("Unknown type passed to Name[]");
  }
}



//----------
// NewNfg
//----------

static Portion *GSM_NewNfg(Portion **param)
{
  ListPortion *dim = ((ListPortion *) param[0]);
  gArray<int> d(dim->Length());
  
  for (int i = 1; i <= dim->Length(); i++)
    d[i] = ((NumberPortion *) (*dim)[i])->Value();

  return new NfgPortion(new Nfg(d));
}


//--------------
// NewOutcome
//--------------

static Portion *GSM_NewOutcome(Portion **param)
{
  return new NfOutcomePortion(((NfgPortion *) param[0])->Value()->NewOutcome());
}




//-----------
// Outcome
//-----------

static Portion* GSM_Outcome(Portion** param)
{
  int i;

  Nfg &nfg = 
    ((StrategyPortion *) (*((ListPortion *) param[0]))[1])->Value()->Player()->Game();
  
  if (nfg.NumPlayers() != ((ListPortion *) param[0])->Length())
    throw gclRuntimeError("Invalid profile");

  StrategyProfile profile(nfg);
  for (i = 1; i <= nfg.NumPlayers(); i++)  {
    Strategy *strat =
      ((StrategyPortion *) (*((ListPortion *) param[0]))[i])->Value();
    if (strat->Player()->GetNumber() != i)
      throw gclRuntimeError("Invalid profile");
    profile.Set(i, strat);
  }
  
  if (nfg.GetOutcome(profile))
    return new NfOutcomePortion(nfg.GetOutcome(profile));
  else
    return new NullPortion(porNFOUTCOME);
}

//------------
// Outcomes
//------------

static Portion *GSM_Outcomes(Portion **param)
{
  Nfg *N = ((NfgPortion *) param[0])->Value();
  
  return ArrayToList(N->Outcomes());
}

//-----------
// Payoff
//-----------

static Portion* GSM_Payoff(Portion** param)
{
  if (param[0]->Spec().Type == porNULL)
    return new NumberPortion(0);

  NFOutcome *outcome = ((NfOutcomePortion *) param[0])->Value();
  Nfg *nfg = outcome->Game();
  NFPlayer *player = ((NfPlayerPortion *) param[1])->Value();

  return new NumberPortion(nfg->Payoff(outcome, player->GetNumber()));
}


//------------
// Player
//------------

static Portion *GSM_Player(Portion **param)
{
  if (param[0]->Spec().Type == porNULL)
    return new NullPortion(porNFPLAYER);

  Strategy *s = ((StrategyPortion *) param[0])->Value();

  return new NfPlayerPortion(s->Player());
}

//------------
// Players
//------------

static Portion *GSM_Players(Portion **param)
{
  Nfg *N = ((NfgPortion *) param[0])->Value();

  return ArrayToList(N->Players());
}

//------------------------
// PossibleNashSupports
//------------------------

#include "nfgensup.h"

static Portion *GSM_PossibleNashSupportsNFG(Portion **param)
{
  Nfg &N = *((NfgPortion*) param[0])->Value();

  gList<const NFSupport> list =
    PossibleNashSubsupports(NFSupport(N), gstatus);

  Portion *por = ArrayToList(list);
  return por;
}

//------------
// StrategyNumber
//------------

static Portion *GSM_StrategyNumber(Portion **param)
{
  Strategy *s = ((StrategyPortion *) param[0])->Value();
  NFSupport *support = ((NfSupportPortion *) param[1])->Value();

  return new NumberPortion(support->Find(s));
}

//------------------
// RemoveStrategy
//------------------

static Portion *GSM_RemoveStrategy(Portion **param)
{
  NFSupport *support = ((NfSupportPortion *) param[0])->Value();
  Strategy *s = ((StrategyPortion *) param[1])->Value();
  
  NFSupport *S = new NFSupport(*support);
  S->RemoveStrategy(s);

  return new NfSupportPortion(S);
}

//------------
// SaveNfg
//------------

extern NumberPortion _WriteGameDecimals;

static Portion *GSM_SaveNfg(Portion **param)
{
  Nfg *N = ((NfgPortion *) param[0])->Value();
  gText file = ((TextPortion *) param[1])->Value();
  try {
    gFileOutput f(file);
    N->WriteNfgFile(f, _WriteGameDecimals.Value());
  }
  catch (gFileOutput::OpenFailed &)  {
    throw gclRuntimeError("Unable to open file " + file + " for output");
  }

  return param[0]->ValCopy();
}

//-------------
// SetComment
//-------------

static Portion *GSM_SetComment(Portion **param)
{
  Nfg *nfg = ((NfgPortion *) param[0])->Value();
  gText comment = ((TextPortion *) param[1])->Value();
  nfg->SetComment(comment);
  return param[0]->ValCopy();
}

//------------
// SetName
//------------

static Portion *GSM_SetName_Nfg(Portion **param)
{
  Nfg &N = * ((NfgPortion *) param[0])->Value();
  gText name = ((TextPortion *) param[1])->Value();
  N.SetTitle(name);
  return param[0]->ValCopy();
}

static Portion *GSM_SetName_NfPlayer(Portion **param)
{
  NFPlayer *p = ((NfPlayerPortion *) param[0])->Value();
  gText name = ((TextPortion *) param[1])->Value();
  p->SetName(name);
  return param[0]->ValCopy();
}

static Portion *GSM_SetName_Strategy(Portion **param)
{
  Strategy *s = ((StrategyPortion *) param[0])->Value();
  gText name = ((TextPortion *) param[1])->Value();
  s->SetName(name);
  return param[0]->ValCopy();
}

static Portion *GSM_SetName_NfOutcome(Portion **param)
{
  NFOutcome *c = ((NfOutcomePortion *) param[0])->Value();
  gText name = ((TextPortion *) param[1])->Value();
  c->SetName(name);
  return param[0]->ValCopy();
}

static Portion *GSM_SetName_NfSupport(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  gText name = ((TextPortion *) param[1])->Value();
  S->SetName(name);
  return param[0]->RefCopy();
}

//------------
// SetOutcome
//------------

static Portion* GSM_SetOutcome(Portion** param)
{
  Nfg &nfg = 
    ((StrategyPortion *) (*((ListPortion *) param[0]))[1])->Value()->Player()->Game();
  
  if (nfg.NumPlayers() != ((ListPortion *) param[0])->Length())
    throw gclRuntimeError("Invalid profile");

  StrategyProfile profile(nfg);
  for (int i = 1; i <= nfg.NumPlayers(); i++)  {
    Strategy *strat =
      ((StrategyPortion *) (*((ListPortion *) param[0]))[i])->Value();
    if (strat->Player()->GetNumber() != i)
      throw gclRuntimeError("Invalid profile");
    profile.Set(i, strat);
  }
  
  NFOutcome *outcome = ((NfOutcomePortion *) param[1])->Value();

  nfg.SetOutcome(profile, outcome);

  _gsm->InvalidateGameProfile((Nfg *) &nfg, false);
 
  return param[1]->ValCopy();
}

//------------
// SetPayoff
//------------

static Portion* GSM_SetPayoff(Portion** param)
{
  NFOutcome *outcome = ((NfOutcomePortion *) param[0])->Value();
  Nfg *nfg = outcome->Game();
  NFPlayer *player = ((NfPlayerPortion *) param[1])->Value();
  gNumber value = ((NumberPortion *) param[2])->Value();

  nfg->SetPayoff(outcome, player->GetNumber(), value);

  _gsm->InvalidateGameProfile((Nfg *) nfg, false);
 
  return param[0]->ValCopy();
}


//--------------
// Strategies
//--------------

static Portion *GSM_Strategies(Portion **param)
{
  NFPlayer *P = (NFPlayer *) ((NfPlayerPortion *) param[0])->Value();
  NFSupport* s = ((NfSupportPortion*) param[1])->Value();

  return ArrayToList(s->Strategies(P->GetNumber()));
}

//--------------
// Support
//--------------

static Portion *GSM_Support(Portion **param)
{
  Nfg &N = * ((NfgPortion *) param[0])->Value();
  return new NfSupportPortion(new NFSupport(N));
}


void Init_nfgfunc(GSM *gsm)
{
  gclFunction *FuncObj;

  static struct { char *sig; Portion *(*func)(Portion **); } ftable[] =
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

  for (int i = 0; ftable[i].sig != 0; i++) 
    gsm->AddFunction(new gclFunction(ftable[i].sig, ftable[i].func,
				     funcLISTABLE | funcGAMEMATCH));

  FuncObj = new gclFunction("ElimDom", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_ElimDom_Nfg,
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
					    new OutputPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(0, 6, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));
  gsm->AddFunction(FuncObj);
}
