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
Portion *ArrayToList(const gArray<Strategy *> &);

extern GSM *_gsm;


//---------------
// AddStrategy
//---------------

static Portion *GSM_AddStrategy(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  Strategy *s = ((StrategyPortion *) param[1])->Value();

  S->GetNFStrategySet(s->nfp->GetNumber())->AddStrategy(s);

  Portion* por = new StrategyValPortion(s);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//---------------
// CompressNfg
//---------------

template <class T> Nfg<T> *CompressNfg(const Nfg<T> &, const NFSupport &);

static Portion *GSM_CompressNfg(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  const BaseNfg &N = S->BelongsTo();
  
  if (N.Type() == DOUBLE)
    return new NfgValPortion(CompressNfg((const Nfg<double> &) N, *S));
  else
    return new NfgValPortion(CompressNfg((const Nfg<gRational> &) N, *S));
}

  
extern NFSupport *ComputeDominated(NFSupport &S, bool strong,
				   const gArray<int> &players,
				   gOutput &tracefile,gStatus &status=gstatus);
NFSupport *ComputeMixedDominated(NFSupport &S, bool strong,
				 const gArray<int> &players,
				 gOutput &tracefile,gStatus &status=gstatus);


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

  NFSupport *T = (mixed) ?
    ComputeMixedDominated(*S, strong, players,
			  ((OutputPortion *) param[4])->Value()) :
    ComputeDominated(*S, strong, players,
		     ((OutputPortion *) param[4])->Value());

  ((FloatPortion *) param[3])->Value() = watch.Elapsed();
  
  Portion *por = (T) ? new NfSupportValPortion(T) : new NfSupportValPortion(new NFSupport(*S));

  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//----------
// Float
//----------

extern Nfg<double> *ConvertNfg(const Nfg<gRational> &);

static Portion *GSM_Float_Nfg(Portion **param)
{
  Nfg<gRational> &orig = * (Nfg<gRational> *) ((NfgPortion *) param[0])->Value();
  Nfg<double> *N = ConvertNfg(orig);

  if (N)
    return new NfgValPortion(N);
  else
    return new ErrorPortion("Conversion failed.");
}

//----------
// Game
//----------

Portion* GSM_Game_NfgElements(Portion** param)
{
  if (param[0]->Game())  {
    assert(!param[0]->GameIsEfg());
    return new NfgValPortion((BaseNfg*) param[0]->Game());
  }
  else
    return 0;
}

//--------------
// IsConstSum
//--------------

static Portion *GSM_IsConstSum_Nfg(Portion **param)
{
  BaseNfg &N = * ((NfgPortion *) param[0])->Value();
  return new BoolValPortion(N.IsConstSum());
}

//-----------
// LoadNfg
//-----------

static Portion *GSM_LoadNfg(Portion **param)
{
  gString file = ((TextPortion *) param[0])->Value();

  gFileInput f(file);

  if (f.IsValid())   {
    DataType type;
    bool valid;

    NfgFileType(f, valid, type);
    if (!valid)   return new ErrorPortion("Not a valid .nfg file");
    
    switch (type)   {
      case DOUBLE:  {
	Nfg<double> *N = 0;
	ReadNfgFile((gInput &) f, N);

	if (N)
	  return new NfgValPortion(N);
	else
	  return new ErrorPortion("Not a valid .nfg file");
      }
      case RATIONAL:   {
	Nfg<gRational> *N = 0;
	ReadNfgFile((gInput &) f, N);
	
	if (N)
	  return new NfgValPortion(N);
	else
	  return new ErrorPortion("Not a valid .nfg file");
      }
      default:
	assert(0);
	return 0;
    }
  }
  else
    return new ErrorPortion("Unable to open file for reading");

}

//--------
// Name
//--------

Portion* GSM_Name_Nfg_Elements( Portion** param )
{
  if( param[0]->Spec().Type == porNULL )
  {
    return new TextValPortion( "" );
  }

  switch( param[0]->Spec().Type )
  {
  case porNFG_FLOAT:
  case porNFG_RATIONAL:
    return new TextValPortion(((NfgPortion*) param[0])->Value()->GetTitle());
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

  BaseNfg* N;
  if(!((BoolPortion*) param[1])->Value())
    N = new Nfg<double>(d);
  else
    N = new Nfg<gRational>(d);
  return new NfgValPortion(N);
}


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
	((Nfg<double>*) nfg)->SetPayoff(pl, d, ((FloatPortion*) 
           (*((ListPortion*) list))[pl])->Value());
      else
	((Nfg<gRational>*) nfg)->SetPayoff(pl, d, ((RationalPortion*) 
           (*((ListPortion*) list))[pl])->Value());

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


//--------------
// ListForm
//--------------

Portion* GSM_ListForm_Nfg(Portion** param, bool rational)
{
  BaseNfg* nfg = ((NfgPortion*) param[0])->Value();
  gArray<int> dmax(nfg->Dimensionality());
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
	list->Append(new FloatValPortion(((Nfg<double>*) nfg)->Payoff(pl, d)));
      else
	list->Append(new RationalValPortion(((Nfg<gRational>*) nfg)->Payoff(pl, d)));

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


//-----------
// Payoff
//-----------

Portion* GSM_Payoff_Nfg(Portion** param)
{
  int i;

  BaseNfg *nfg = 
    ((StrategyPortion *) (*((ListPortion *) param[0]))[1])->Value()->nfp->BelongsTo();
  
  if (nfg->NumPlayers() != ((ListPortion *) param[0])->Length())
    return new ErrorPortion("Invalid profile");

  StrategyProfile profile(nfg->NumPlayers());
  for (i = 1; i <= nfg->NumPlayers(); i++)  {
    Strategy *strat =
      ((StrategyPortion *) (*((ListPortion *) param[0]))[i])->Value();
    if (strat->nfp->GetNumber() != i)
      return new ErrorPortion("Invalid profile");
    profile.Set(i, strat);
  }
  
  NFPlayer *player = ((NfPlayerPortion *) param[1])->Value();

  switch (nfg->Type())  {
    case DOUBLE:
      return new FloatValPortion(((const Nfg<double> *) nfg)->Payoff(player->GetNumber(), &profile));
    case RATIONAL:
      return new RationalValPortion(((const Nfg<gRational> *) nfg)->Payoff(player->GetNumber(), &profile));
    default:
      assert(0);
    }
}


//------------
// Players
//------------

static Portion *GSM_Players_Nfg(Portion **param)
{
  BaseNfg &N = *((NfgPortion*) param[0])->Value();

  Portion* por = ArrayToList(N.PlayerList());
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//-------------
// Randomize
//-------------

static Portion *GSM_Randomize_NfgFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();
  
  RandomNfg(N);
  return param[0]->ValCopy();
}

static Portion *GSM_Randomize_NfgRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational> *) ((NfgPortion *) param[0])->Value();
  
  RandomNfg(N);
  return param[0]->ValCopy();
}

static Portion *GSM_Randomize_NfgSeedFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();
  int seed = ((IntPortion *) param[1])->Value();

  SetSeed(seed);
  RandomNfg(N);
  return param[0]->ValCopy();
}

static Portion *GSM_Randomize_NfgSeedRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational> *) ((NfgPortion *) param[0])->Value();
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
  Nfg<double> &orig = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();
  Nfg<gRational> *N = ConvertNfg(orig);

  if (N)
    return new NfgValPortion(N);
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
  
  S->GetNFStrategySet(s->nfp->GetNumber())->RemoveStrategy(s);

  Portion* por = new NfSupportValPortion(S);
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//------------
// SaveNfg
//------------

static Portion *GSM_SaveNfg(Portion **param)
{
  BaseNfg* N = ((NfgPortion*) param[0])->Value();
  gString file = ((TextPortion *) param[1])->Value();
  gFileOutput f(file);

  if (!f.IsValid())
    return new ErrorPortion("Unable to open file for output");

  N->WriteNfgFile(f);

  return param[0]->ValCopy();
}


//------------
// SetName
//------------

static Portion *GSM_SetName_Nfg(Portion **param)
{
  BaseNfg &N = * ((NfgPortion*) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  N.SetTitle(name);
  return param[0]->ValCopy();
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
// SetPayoff
//------------

Portion* GSM_SetPayoff_Nfg(Portion** param)
{
  int i;

  BaseNfg *nfg = ((NfPlayerPortion *) param[1])->Value()->BelongsTo();

  if (nfg->NumPlayers() != ((ListPortion *) param[0])->Length())
    return new ErrorPortion("Invalid profile");

  StrategyProfile profile(nfg->NumPlayers());
  for (i = 1; i <= nfg->NumPlayers(); i++)  {
    Strategy *strat =
      ((StrategyPortion *) (*((ListPortion *) param[0]))[i])->Value();
    if (strat->nfp->GetNumber() != i)
      return new ErrorPortion("Invalid profile");
    profile.Set(i, strat);
  }
  
  NFPlayer *player = ((NfPlayerPortion *) param[1])->Value();

  switch (nfg->Type())  {
    case DOUBLE:
      if (param[2]->Spec().Type != porFLOAT)
	return new ErrorPortion("Type mismatch in payoff value");
      ((Nfg<double> *) nfg)->SetPayoff(player->GetNumber(),
				       &profile,
				       ((FloatPortion *) param[2])->Value());
      break;
    case RATIONAL:
      if (param[2]->Spec().Type != porRATIONAL)
	return new ErrorPortion("Type mismatch in payoff value");
      ((Nfg<gRational> *) nfg)->SetPayoff(player->GetNumber(),
					  &profile,
					  ((RationalPortion *) param[2])->Value());
      break;
    default:
      assert(0);
  }

  _gsm->InvalidateGameProfile((BaseNfg *) nfg, false);
 
  return param[0]->ValCopy();
}

//--------------
// Strategies
//--------------

static Portion *GSM_Strategies(Portion **param)
{
  NFPlayer *P = (NFPlayer *) ((NfPlayerPortion *) param[0])->Value();
  NFSupport* s = ((NfSupportPortion*) param[1])->Value();

  Portion *por = ArrayToList(s->GetStrategy(P->GetNumber()));
  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
}

//--------------
// Support
//--------------

static Portion *GSM_Support_Nfg(Portion **param)
{
  BaseNfg &N = * ((NfgPortion *) param[0])->Value();
  Portion *por = new NfSupportValPortion(new NFSupport(N));

  por->SetGame(param[0]->Game(), param[0]->GameIsEfg());
  return por;
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
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Game_NfgElements, porNFG, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("player", porNFPLAYER));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Game_NfgElements, porNFG, 1));
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



  FuncObj = new FuncDescObj("Payoff", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Payoff_Nfg, 
				       porFLOAT | porRATIONAL, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", 
					    PortionSpec(porSTRATEGY, 1)));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("player", porNFPLAYER));
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

  FuncObj = new FuncDescObj("SetPayoff", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetPayoff_Nfg, 
				       PortionSpec(porSTRATEGY, 1), 3,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("profile", 
					    PortionSpec(porSTRATEGY, 1)));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("player", porNFPLAYER));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("payoff", porFLOAT | porRATIONAL));
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

