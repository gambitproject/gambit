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

extern GSM *_gsm;


//---------------
// AddStrategy
//---------------

static Portion *GSM_AddStrategy(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  Strategy *s = ((StrategyPortion *) param[1])->Value();

  S->AddStrategy(s);

  return param[0]->RefCopy();
}

//---------------
// CompressNfg
//---------------

template <class T> Nfg<T> *CompressNfg(const Nfg<T> &, const NFSupport &);

static Portion *GSM_CompressNfg(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  
  if (((NfSupportPortion *) param[0])->SubType() == DOUBLE)   {
    Nfg<double> *N = (Nfg<double> *) ((NfSupportPortion *) param[0])->PayoffTable();
    return new NfgValPortion<double>(CompressNfg(*N, *S));
  }
  else  {
    Nfg<gRational> *N = (Nfg<gRational> *) ((NfSupportPortion *) param[0])->PayoffTable();
    return new NfgValPortion<gRational>(CompressNfg(*N, *S));
  }
}


//-----------------
// DeleteOutcome
//-----------------

static Portion *GSM_DeleteOutcome_Nfg(Portion **param)
{
  NFOutcome *outc = ((NfOutcomePortion *) param[0])->Value();

  _gsm->InvalidateGameProfile(outc->BelongsTo(), false);
//  _gsm->UnAssignGameElement(outc->BelongsTo(), false, porNFOUTCOME, outc);

  outc->BelongsTo()->DeleteOutcome(outc);

  return new BoolValPortion(true);
}


template <class T>  
NFSupport *ComputeDominated(const Nfg<T> &, NFSupport &S, bool strong,
			    const gArray<int> &players,
			    gOutput &tracefile, gStatus &status);
template <class T>
NFSupport *ComputeMixedDominated(const Nfg<T> &, NFSupport &S, bool strong,
				 const gArray<int> &players,
				 gOutput &tracefile, gStatus &status);


//-------------
// ElimDom
//-------------

static Portion *GSM_ElimDom_Nfg(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  bool strong = ((BoolPortion *) param[1])->Value();
  bool mixed = ((BoolPortion *) param[2])->Value();
  
  gWatch watch;
  gBlock<int> players(S->BelongsTo().NumPlayers());
  int i;
  for (i = 1; i <= players.Length(); i++)   players[i] = i;

  NFSupport *T;
  Portion *por;

  if (((NfSupportPortion *) param[0])->SubType() == DOUBLE)   {
    Nfg<double> *N = (Nfg<double> *) ((NfSupportPortion *) param[0])->PayoffTable();
    if (mixed) 
      T = ComputeMixedDominated(*N, *S, strong, players,
				((OutputPortion *) param[4])->Value(), gstatus);
    else
      T = ComputeDominated(*N, *S, strong, players,
			   ((OutputPortion *) param[4])->Value(), gstatus);

    por = (T) ? new NfSupportValPortion(T, N) :
                new NfSupportValPortion(S, N);
  }
  else  {
    Nfg<gRational> *N = (Nfg<gRational> *) ((NfSupportPortion *) param[0])->PayoffTable();
    if (mixed) 
      T = ComputeMixedDominated(*N, *S, strong, players,
				((OutputPortion *) param[4])->Value(), gstatus);
    else 
      T = ComputeDominated(*N, *S, strong, players,
			   ((OutputPortion *) param[4])->Value(), gstatus);

    por = (T) ? new NfSupportValPortion(T, N) :
                new NfSupportValPortion(S, N);
  }

  ((FloatPortion *) param[3])->Value() = watch.Elapsed();
  
  return por;
}

//----------
// Float
//----------

extern Nfg<double> *ConvertNfg(const Nfg<gRational> &);

static Portion *GSM_Float_Nfg(Portion **param)
{
  Nfg<gRational> &orig = *((NfgPortion<gRational> *) param[0])->Value();
  Nfg<double> *N = ConvertNfg(orig);

  if (N)
    return new NfgValPortion<double>(N);
  else
    return new ErrorPortion("Conversion failed.");
}

//----------
// Game
//----------

static Portion *GSM_Game_NfPlayer(Portion **param)
{
  NFPayoffs *N = ((NfPlayerPortion *) param[0])->Value()->BelongsTo().PayoffTable();

  if (N->Type() == DOUBLE)
    return new NfgValPortion<double>((Nfg<double> *) N);
  else
    return new NfgValPortion<gRational>((Nfg<gRational> *) N);
}


static Portion *GSM_Game_Strategy(Portion **param)
{
  NFPayoffs *N = ((StrategyPortion *) param[0])->Value()->nfp->BelongsTo().PayoffTable();

  if (N->Type() == DOUBLE)
    return new NfgValPortion<double>((Nfg<double> *) N);
  else
    return new NfgValPortion<gRational>((Nfg<gRational> *) N);
}


//--------------
// IsConstSum
//--------------

static Portion *GSM_IsConstSum_Nfg(Portion **param)
{
  if (param[0]->Spec().Type == porNFG_FLOAT)   {
    Nfg<double> &N = * ((NfgPortion<double> *) param[0])->Value();
    return new BoolValPortion(IsConstSum(N));
  }
  else  {
    Nfg<gRational> &N = * ((NfgPortion<gRational> *) param[0])->Value();
    return new BoolValPortion(IsConstSum(N));
  }
}

//-----------
// LoadNfg
//-----------

extern int ReadNfgFile(gInput &f, Nfg<double> *& Ndbl, Nfg<gRational> *& Nrat);

static Portion *GSM_LoadNfg(Portion **param)
{
  gString file = ((TextPortion *) param[0])->Value();

  gFileInput f(file);

  Nfg<double> *Ndbl = 0;
  Nfg<gRational> *Nrat = 0;

  if (f.IsValid())   {
    if (!ReadNfgFile(f, Ndbl, Nrat))  
      return new ErrorPortion("Not a valid .nfg file");

    if (Ndbl)
      return new NfgValPortion<double>(Ndbl);
    else
      return new NfgValPortion<gRational>(Nrat);
  }
  else
    return new ErrorPortion("Unable to open file for reading");

}

//--------
// Name
//--------

static Portion* GSM_Name_Nfg_Elements( Portion** param )
{
  if( param[0]->Spec().Type == porNULL )
  {
    return new TextValPortion( "" );
  }

  switch( param[0]->Spec().Type )
  {
  case porNFG_FLOAT:
    return new TextValPortion(((NfgPortion<double>*) param[0])->Value()->GameForm().GetTitle());
  case porNFG_RATIONAL:
    return new TextValPortion(((NfgPortion<gRational>*) param[0])->Value()->GameForm().GetTitle());
    break;
  case porNFPLAYER:
    return new TextValPortion(((NfPlayerPortion*) param[0])->Value()->
			      GetName());
    break;
  case porSTRATEGY:
    return new TextValPortion(((StrategyPortion*) param[0])->Value()->name);
    break;
  default:
    assert( 0 );
    return 0;
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
    d[i] = ((IntPortion *) (*dim)[i])->Value();

  if (!((BoolPortion*) param[1])->Value())
    return new NfgValPortion<double>(new Nfg<double>(d));
  else
    return new NfgValPortion<gRational>(new Nfg<gRational>(d));
}

/*
 * I am arbitrarily making the decision to comment out this function
 * since it is going to be heavily modified at best when the
 * final game form - payoff function split takes place
 */

#ifdef UNUSED
Portion* GSM_NewNfg_Payoff(Portion** param, bool rational)
{
  int dim = param[0]->Spec().ListDepth - 1;
  int players = 0;
  gArray<int> dmax(dim);
  Portion* list = param[0];

  if(dim == 0)
    return new ErrorPortion("Bad payoff dimensiontality");
  while(dim > 0)
  {
    assert(list->Spec().ListDepth > 1);
    dmax[dim] = ((ListPortion*) list)->Length();
    list = (*((ListPortion*) list))[1];
    dim--;
  }
  assert(list->Spec().ListDepth > 0);
  players = dmax.Length();
  // gout << "players: " << players << '\n';
  // gout << "dmax: " << dmax << '\n';
  

  // create new Nfg and call SetPayoff()
  BaseNfg* nfg;
  if(!rational)
    nfg = new Nfg<double>(dmax);
  else
    nfg = new Nfg<gRational>(dmax);

  int length = dmax.Length();
  gArray<int> d(length);
  int ci;
  int pl;

  for(ci=1; ci<=length; ci++)
    d[ci] = 1;
  d[length] = 0;

  while(d != dmax)
  {
    // increment indices
    ci = length;
    d[ci]++;
    while(d[ci] > dmax[ci])
    {
      d[ci] = 1;
      ci--;
      if(ci == 0)
	break;
      d[ci]++;
    }

    // begin valid index region
    // now extract the list with the payoffs at each position
    list = param[0];
    ci = length;
    while(ci > 0)
    {
      assert(list->Spec().ListDepth > 1);
      if(((ListPortion*) list)->Length() != dmax[ci])
      {
	delete nfg;
	return new ErrorPortion("Bad payoff dimensionality");
      }      
      list = (*((ListPortion*) list))[d[ci]];
      ci--;
    }
    assert(list->Spec().ListDepth == 1);
    // gout << "SetPayoff[n, " << d << ", " << list << "]\n";

    if(((ListPortion*) list)->Length() != players)
    {
      delete nfg;
      return new ErrorPortion("Bad payoff dimensionality");
    }
    
    // now we have the list; call SetPayoff() for each player
    for(pl=1; pl<=players; pl++)
      if(!rational)
	((Nfg<double> *) nfg)->SetPayoff(nfg->GetOutcome(d), pl,
             ((FloatPortion*) (*((ListPortion*) list))[pl])->Value());
      else
	((Nfg<gRational> *) nfg)->SetPayoff(nfg->GetOutcome(d), pl,
             ((RationalPortion*) (*((ListPortion*) list))[pl])->Value());

    // end valid index region
  }
  
  return new NfgValPortion(nfg);
}


Portion* GSM_NewNfg_Float(Portion** param)
{
  return GSM_NewNfg_Payoff(param, false);
}

Portion* GSM_NewNfg_Rational(Portion** param)
{
  return GSM_NewNfg_Payoff(param, true);
}
#endif    // UNUSED


//--------------
// NewOutcome
//--------------

static Portion *GSM_NewOutcome_Nfg_Float(Portion **param)
{
  Portion *por = new NfOutcomeValPortion(((NfgPortion<double> *) param[0])->Value()->GameForm().NewOutcome());
  return por;
}

static Portion *GSM_NewOutcome_Nfg_Rational(Portion **param)
{
  Portion *por = new NfOutcomeValPortion(((NfgPortion<gRational> *) param[0])->Value()->GameForm().NewOutcome());
  return por;
}


//--------------
// ListForm
//--------------


#ifdef UNUSED
Portion* GSM_ListForm_Nfg(Portion** param, bool rational)
{
  BaseNfg* nfg = ((NfgPortion*) param[0])->Value();
  gArray<int> dmax(nfg->NumStrats());
  ListPortion* result = new ListValPortion();
  ListPortion* list;
  int ci;
  int i;
  int pl;
  int length = dmax.Length();
  int players = length;
  gArray<int> d(length);
  for(i=1; i<=length; i++)
    d[i] = 1;
  d[length] = 0;
  // gout << "dmax: " << dmax << '\n';
  while(d != dmax)
  {
    // increment indices
    ci = length;
    d[ci]++;
    while(d[ci] > dmax[ci])
    {
      d[ci] = 1;
      ci--;
      if(ci == 0)
	break;
      d[ci]++;
    }

    // begin valid index region
    // now extract the list with the payoffs at each position
    list = result;
    ci = length;
    while(ci > 0)
    {
      //assert(list->Spec().ListDepth > 1);
      if(((ListPortion*) list)->Length() != dmax[ci])
	list->Append(new ListValPortion());
      list = (ListPortion*) (*((ListPortion*) list))[d[ci]];
      ci--;
    }
    assert(list->Spec().ListDepth == 1);

    // now we have the list; call SetPayoff() for each player
    for(pl=1; pl<=players; pl++)
      if(!rational)
	list->Append(new FloatValPortion(((Nfg<double> *) nfg)->Payoff(nfg->GetOutcome(d), pl)));
      else
	list->Append(new RationalValPortion(((Nfg<gRational> *) nfg)->Payoff(nfg->GetOutcome(d), pl)));

    // gout << "Payoff[n, " << d << "] = " << list << '\n';
    
    // end valid index region
  }

  if(!rational)
    result->SetDataType(porFLOAT);
  else
    result->SetDataType(porRATIONAL);
  return result;
}

Portion* GSM_ListForm_NfgFloat(Portion** param)
{
  return GSM_ListForm_Nfg(param, false);
}

Portion* GSM_ListForm_NfgRational(Portion** param)
{
  return GSM_ListForm_Nfg(param, true);
}
#endif   // UNUSED


//-----------
// Outcome
//-----------

static Portion* GSM_Outcome_Nfg(Portion** param)
{
  int i;

  NFGameForm &nfg = 
    ((StrategyPortion *) (*((ListPortion *) param[0]))[1])->Value()->nfp->BelongsTo();
  
  if (nfg.NumPlayers() != ((ListPortion *) param[0])->Length())
    return new ErrorPortion("Invalid profile");

  StrategyProfile profile(nfg);
  for (i = 1; i <= nfg.NumPlayers(); i++)  {
    Strategy *strat =
      ((StrategyPortion *) (*((ListPortion *) param[0]))[i])->Value();
    if (strat->nfp->GetNumber() != i)
      return new ErrorPortion("Invalid profile");
    profile.Set(i, strat);
  }
  
  Portion *por = new NfOutcomeValPortion(nfg.GetOutcome(profile));
  return por;
}

//------------
// Outcomes
//------------

static Portion *GSM_Outcomes_NfgFloat(Portion **param)
{
  Nfg<double> *N = ((NfgPortion<double> *) param[0])->Value();
  
  Portion* por = ArrayToList(N->Outcomes());
  return por;
}

static Portion *GSM_Outcomes_NfgRational(Portion **param)
{
  Nfg<gRational> *N = ((NfgPortion<gRational> *) param[0])->Value();
  
  Portion* por = ArrayToList(N->Outcomes());
  return por;
}

//-----------
// Payoff
//-----------

static Portion* GSM_Payoff_NFOutcome_Float(Portion** param)
{
  Nfg<double> *nfg = ((NfgPortion<double> *) param[0])->Value();
  NFOutcome *outcome = ((NfOutcomePortion *) param[1])->Value();
  NFPlayer *player = ((NfPlayerPortion *) param[2])->Value();
  
  return new FloatValPortion(nfg->Payoff(outcome, player->GetNumber()));
}

static Portion* GSM_Payoff_NFOutcome_Rational(Portion** param)
{
  Nfg<gRational> *nfg = ((NfgPortion<gRational> *) param[0])->Value();
  NFOutcome *outcome = ((NfOutcomePortion *) param[1])->Value();
  NFPlayer *player = ((NfPlayerPortion *) param[2])->Value();
  
  return new RationalValPortion(nfg->Payoff(outcome, player->GetNumber()));
}


//------------
// Player
//------------

static Portion *GSM_Player(Portion **param)
{
  if (param[0]->Spec().Type == porNULL)
    return new NullPortion(porNFPLAYER);

  Strategy *s = ((StrategyPortion *) param[0])->Value();

  Portion* por = new NfPlayerValPortion(s->nfp);
  return por;
}

//------------
// Players
//------------

static Portion *GSM_Players_Nfg(Portion **param)
{
  if (param[0]->Spec().Type == porNFG_FLOAT)   {
    Nfg<double> &N = *((NfgPortion<double> *) param[0])->Value();

    Portion* por = ArrayToList(N.Players());
    return por;
  }
  else  {
    Nfg<gRational> &N = *((NfgPortion<gRational> *) param[0])->Value();

    Portion* por = ArrayToList(N.Players());
    return por;
  }
}

//-------------
// Randomize
//-------------

static Portion *GSM_Randomize_NfgFloat(Portion **param)
{
  Nfg<double> &N = *((NfgPortion<double> *) param[0])->Value();
  
  RandomNfg(N);
  return param[0]->ValCopy();
}

static Portion *GSM_Randomize_NfgRational(Portion **param)
{
  Nfg<gRational> &N = *((NfgPortion<gRational> *) param[0])->Value();
  
  RandomNfg(N);
  return param[0]->ValCopy();
}

static Portion *GSM_Randomize_NfgSeedFloat(Portion **param)
{
  Nfg<double> &N = *((NfgPortion<double> *) param[0])->Value();
  int seed = ((IntPortion *) param[1])->Value();

  SetSeed(seed);
  RandomNfg(N);
  return param[0]->ValCopy();
}

static Portion *GSM_Randomize_NfgSeedRational(Portion **param)
{
  Nfg<gRational> &N = *((NfgPortion<gRational> *) param[0])->Value();
  int seed = ((IntPortion *) param[1])->Value();

  SetSeed(seed);
  RandomNfg(N);
  return param[0]->ValCopy();
}  

//------------
// Rational
//------------

extern Nfg<gRational> *ConvertNfg(const Nfg<double> &);

static Portion *GSM_Rational_Nfg(Portion **param)
{
  Nfg<double> &orig = *((NfgPortion<double> *) param[0])->Value();
  Nfg<gRational> *N = ConvertNfg(orig);

  if (N)
    return new NfgValPortion<gRational>(N);
  else
    return new ErrorPortion("Conversion failed.");
}

//------------------
// RemoveStrategy
//------------------

static Portion *GSM_RemoveStrategy(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  Strategy *s = ((StrategyPortion *) param[1])->Value();
  
  S->RemoveStrategy(s);

  return param[0]->RefCopy();
}

//------------
// SaveNfg
//------------

static Portion *GSM_SaveNfg(Portion **param)
{
  if (param[0]->Spec().Type == porNFG_FLOAT)  {
    Nfg<double> *N = ((NfgPortion<double> *) param[0])->Value();
    gString file = ((TextPortion *) param[1])->Value();
    gFileOutput f(file);

    if (!f.IsValid())
      return new ErrorPortion("Unable to open file for output");

    N->WriteNfgFile(f);

    return param[0]->ValCopy();
  }
  else  {
    Nfg<gRational> *N = ((NfgPortion<gRational> *) param[0])->Value();
    gString file = ((TextPortion *) param[1])->Value();
    gFileOutput f(file);

    if (!f.IsValid())
      return new ErrorPortion("Unable to open file for output");

    N->WriteNfgFile(f);

    return param[0]->ValCopy();
  }
}


//------------
// SetName
//------------

static Portion *GSM_SetName_Nfg(Portion **param)
{
  if (param[0]->Spec().Type == porNFG_FLOAT)  {
    Nfg<double> &N = * ((NfgPortion<double> *) param[0])->Value();
    gString name = ((TextPortion *) param[1])->Value();
    N.GameForm().SetTitle(name);
    return param[0]->ValCopy();
  }
  else  {
    Nfg<gRational> &N = * ((NfgPortion<gRational> *) param[0])->Value();
    gString name = ((TextPortion *) param[1])->Value();
    N.GameForm().SetTitle(name);
    return param[0]->ValCopy();
  }
}

static Portion *GSM_SetName_NfPlayer(Portion **param)
{
  NFPlayer *p = ((NfPlayerPortion *) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  p->SetName(name);
  return param[0]->ValCopy();
}

static Portion *GSM_SetName_Strategy(Portion **param)
{
  Strategy *s = ((StrategyPortion *) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  s->name = name;
  return param[0]->ValCopy();
}

//------------
// SetOutcome
//------------

static Portion* GSM_SetOutcome_Nfg(Portion** param)
{
  NFGameForm &nfg = 
    ((StrategyPortion *) (*((ListPortion *) param[0]))[1])->Value()->nfp->BelongsTo();
  
  if (nfg.NumPlayers() != ((ListPortion *) param[0])->Length())
    return new ErrorPortion("Invalid profile");

  StrategyProfile profile(nfg);
  for (int i = 1; i <= nfg.NumPlayers(); i++)  {
    Strategy *strat =
      ((StrategyPortion *) (*((ListPortion *) param[0]))[i])->Value();
    if (strat->nfp->GetNumber() != i)
      return new ErrorPortion("Invalid profile");
    profile.Set(i, strat);
  }
  
  NFOutcome *outcome = ((NfOutcomePortion *) param[1])->Value();

  nfg.SetOutcome(profile, outcome);

  _gsm->InvalidateGameProfile((NFGameForm *) &nfg, false);
 
  return param[1]->ValCopy();
}

//------------
// SetPayoff
//------------

static Portion* GSM_SetPayoff_NFOutcome_Float(Portion** param)
{
  Nfg<double> *nfg = ((NfgPortion<double> *) param[0])->Value();
  NFOutcome *outcome = ((NfOutcomePortion *) param[1])->Value();
  NFPlayer *player = ((NfPlayerPortion *) param[2])->Value();
  double value = ((FloatPortion *) param[3])->Value();
  
  nfg->SetPayoff(outcome, player->GetNumber(), value);

  _gsm->InvalidateGameProfile((NFGameForm *) nfg, false);
 
  return param[1]->ValCopy();
}

static Portion* GSM_SetPayoff_NFOutcome_Rational(Portion** param)
{
  Nfg<gRational> *nfg = ((NfgPortion<gRational> *) param[0])->Value();
  NFOutcome *outcome = ((NfOutcomePortion *) param[1])->Value();
  NFPlayer *player = ((NfPlayerPortion *) param[2])->Value();
  gRational value = ((RationalPortion *) param[3])->Value();
  
  nfg->SetPayoff(outcome, player->GetNumber(), value);

  _gsm->InvalidateGameProfile((NFGameForm *) nfg, false);
 
  return param[1]->ValCopy();
}

//--------------
// Strategies
//--------------

static Portion *GSM_Strategies(Portion **param)
{
  NFPlayer *P = (NFPlayer *) ((NfPlayerPortion *) param[0])->Value();
  NFSupport* s = ((NfSupportPortion*) param[1])->Value();

  Portion *por = ArrayToList(s->Strategies(P->GetNumber()));
  return por;
}

//--------------
// Support
//--------------

static Portion *GSM_Support_Nfg(Portion **param)
{
  if (param[0]->Spec().Type == porNFG_FLOAT)  {
    Nfg<double> &N = * ((NfgPortion<double> *) param[0])->Value();
    Portion *por = new NfSupportValPortion(new NFSupport(N.GameForm()), &N);
    return por;
  }
  else  {
    Nfg<gRational> &N = * ((NfgPortion<gRational> *) param[0])->Value();
    Portion *por = new NfSupportValPortion(new NFSupport(N.GameForm()), &N);
    return por;
  }

}



void Init_nfgfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;


  FuncObj = new FuncDescObj("AddStrategy", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_AddStrategy, porNFSUPPORT, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porNFSUPPORT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("strategy", porSTRATEGY));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("CompressNfg", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_CompressNfg, porNFG, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porNFSUPPORT));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("DeleteOutcome", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_DeleteOutcome_Nfg, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("outcome", porNFOUTCOME));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("ElimDom", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ElimDom_Nfg,
				       porNFSUPPORT, 6));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porNFSUPPORT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("strong", porBOOL,
					    new BoolValPortion(false)));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("mixed", porBOOL,
					    new BoolValPortion(false)));
  FuncObj->SetParamInfo(0, 3, ParamInfoType("time", porFLOAT,
					    new FloatValPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 4, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(0, 5, ParamInfoType("traceLevel", porINTEGER,
					    new IntValPortion(0)));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Float", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Float_Nfg, 
				       porNFG_FLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG_RATIONAL));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Game", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Game_NfPlayer, porNFG, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("player", porNFPLAYER));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Game_Strategy, porNFG, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("strategy", porSTRATEGY));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsConstSum", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsConstSum_Nfg, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LoadNfg", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_LoadNfg, porNFG, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("file", porTEXT));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Name", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Name_Nfg_Elements, porTEXT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", 
                              PortionSpec(porNFG | porNFPLAYER | porSTRATEGY, 
                                          0, porNULLSPEC) ));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NewNfg", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NewNfg, porNFG, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("dim", PortionSpec(porINTEGER,1)));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("rational", porBOOL,
					    new BoolValPortion(false)));

  /* commented out for now; don't forget to change
     the # of functions back if these are reenabled!
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_NewNfg_Float, 
				       porNFG_FLOAT, 1, 0, 
				       funcNONLISTABLE));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("payoffs", 
					    PortionSpec(porFLOAT,1)));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_NewNfg_Rational, 
				       porNFG_RATIONAL, 1, 0, 
				       funcNONLISTABLE));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("payoffs", 
					    PortionSpec(porRATIONAL,1)));
  */

  gsm->AddFunction(FuncObj);



  /* commented out temporarily
  FuncObj = new FuncDescObj("ListForm", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ListForm_NfgFloat, 
				       PortionSpec(porFLOAT, NLIST), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG_FLOAT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_ListForm_NfgRational, 
				       PortionSpec(porRATIONAL, NLIST), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("nfg", porNFG_RATIONAL));
  gsm->AddFunction(FuncObj);
  */

  FuncObj = new FuncDescObj("NewOutcome", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NewOutcome_Nfg_Float, porNFOUTCOME, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG_FLOAT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_NewOutcome_Nfg_Rational, porNFOUTCOME, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("nfg", porNFG_RATIONAL));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Outcome", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Outcome_Nfg, porNFOUTCOME, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", 
					    PortionSpec(porSTRATEGY, 1)));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Outcomes", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Outcomes_NfgFloat, 
				       PortionSpec(porNFOUTCOME, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG_FLOAT));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Outcomes_NfgRational, 
				       PortionSpec(porNFOUTCOME, 1), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("nfg", porNFG_RATIONAL));

  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Payoff", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Payoff_NFOutcome_Float, 
				       porFLOAT, 3,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("outcome", porNFOUTCOME));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("player", porNFPLAYER));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Payoff_NFOutcome_Rational, 
				       porRATIONAL, 3,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("nfg", porNFG_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("outcome", porNFOUTCOME));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("player", porNFPLAYER));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Player", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Player, porNFPLAYER, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("strategy", 
			      PortionSpec(porSTRATEGY, 0, porNULLSPEC) ));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Players", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Players_Nfg, 
				       PortionSpec(porNFPLAYER, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG));
  gsm->AddFunction(FuncObj);
  

  FuncObj = new FuncDescObj("Randomize", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Randomize_NfgFloat, 
				       porNFG_FLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porNFG_FLOAT, REQUIRED));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Randomize_NfgRational, 
				       porNFG_RATIONAL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porNFG_RATIONAL, REQUIRED));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Randomize_NfgSeedFloat, 
				       porNFG_FLOAT, 2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porNFG_FLOAT, REQUIRED));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("seed", porINTEGER));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Randomize_NfgSeedRational, 
				       porNFG_RATIONAL, 2));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", porNFG_RATIONAL, REQUIRED));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("seed", porINTEGER));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Rational", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Rational_Nfg, 
				       porNFG_RATIONAL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG_FLOAT));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RemoveStrategy", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_RemoveStrategy, 
				       porNFSUPPORT, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porNFSUPPORT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("strategy", porSTRATEGY));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("SaveNfg", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SaveNfg, porNFG, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("file", porTEXT));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetName", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetName_Nfg, porNFG, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porNFG));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("name", porTEXT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_SetName_NfPlayer, 
				       porNFPLAYER, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porNFPLAYER));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("name", porTEXT));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_SetName_Strategy, 
				       porSTRATEGY, 2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porSTRATEGY));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("name", porTEXT));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetOutcome", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetOutcome_Nfg, porNFOUTCOME, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", 
					    PortionSpec(porSTRATEGY, 1)));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("outcome", porNFOUTCOME));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetPayoff", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetPayoff_NFOutcome_Float, 
				       porNFOUTCOME, 4,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG_FLOAT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("outcome", porNFOUTCOME));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("player", porNFPLAYER));
  FuncObj->SetParamInfo(0, 3, ParamInfoType("payoff", porFLOAT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_SetPayoff_NFOutcome_Rational, 
				       porNFOUTCOME, 4,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("nfg", porNFG_RATIONAL));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("outcome", porNFOUTCOME));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("player", porNFPLAYER));
  FuncObj->SetParamInfo(1, 3, ParamInfoType("payoff", porRATIONAL));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Strategies", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Strategies, 
				       PortionSpec(porSTRATEGY, 1), 2, 
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("player", porNFPLAYER));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("support", porNFSUPPORT));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Support", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Support_Nfg, porNFSUPPORT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG));
  gsm->AddFunction(FuncObj);
}

