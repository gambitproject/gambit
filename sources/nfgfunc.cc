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

//
// Implementations of these are provided as necessary in gsmutils.cc
//
Portion *ArrayToList(const gArray<NFPlayer *> &);
Portion *ArrayToList(const gArray<Strategy *> &);



//---------------
// AddStrategy
//---------------

Portion *GSM_AddStrategy(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  Strategy *s = ((StrategyPortion *) param[1])->Value();

  S->GetNFStrategySet(s->nfp->GetNumber())->AddStrategy(s);

  Portion* por = new StrategyValPortion(s);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

//---------------
// CompressNfg
//---------------

template <class T> Nfg<T> *CompressNfg(const Nfg<T> &, const NFSupport &);

Portion *GSM_CompressNfg_Float(Portion **param)
{
  Nfg<double> *N = (Nfg<double> *) ((NfgPortion *) param[0])->Value();
  NFSupport *S = ((NfSupportPortion *) param[1])->Value();
  if (&S->BelongsTo() != N)  
    return new ErrorPortion("Support must belong to normal form");

  return new NfgValPortion(CompressNfg(*N, *S));
}

Portion *GSM_CompressNfg_Rational(Portion **param)
{
  Nfg<gRational> *N = (Nfg<gRational> *) ((NfgPortion *) param[0])->Value();
  NFSupport *S = ((NfSupportPortion *) param[1])->Value();
  if (&S->BelongsTo() != N)  
    return new ErrorPortion("Support must belong to normal form");

  return new NfgValPortion(CompressNfg(*N, *S));
}
  
extern NFSupport *ComputeDominated(NFSupport &S, bool strong, 
				   const gArray<int> &players,
				   gOutput &tracefile);

//--------------
// ElimAllDom
//--------------

Portion *GSM_ElimAllDom_NfSupport(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  bool strong = ((BoolPortion *) param[1])->Value();
  
  gWatch watch;
  gBlock<int> players(S->BelongsTo().NumPlayers());
  int i;
  for (i = 1; i <= players.Length(); i++)   players[i] = i;

  NFSupport* new_T = S;
  NFSupport* old_T = S;
  while( new_T )
  {
    old_T = new_T;
    new_T = ComputeDominated(*old_T, strong, players,
			     ((OutputPortion *) param[3])->Value());
  }

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  Portion *por = new NfSupportValPortion( old_T );
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}


Portion *GSM_ElimAllDom_Nfg(Portion **param)
{
  NFSupport *S = new NFSupport( * ((NfgPortion *) param[0])->Value() );
  bool strong = ((BoolPortion *) param[1])->Value();
  
  gWatch watch;
  gBlock<int> players(S->BelongsTo().NumPlayers());
  int i;
  for (i = 1; i <= players.Length(); i++)   players[i] = i;

  NFSupport* new_T = S;
  NFSupport* old_T = S;
  while( new_T )
  {
    old_T = new_T;
    new_T = ComputeDominated(*old_T, strong, players,
			     ((OutputPortion *) param[3])->Value());
  }

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  Portion *por = new NfSupportValPortion( old_T );
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

//-------------
// ElimDom
//-------------

Portion *GSM_ElimDom_NfSupport(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  bool strong = ((BoolPortion *) param[1])->Value();
  
  gWatch watch;
  gBlock<int> players(S->BelongsTo().NumPlayers());
  int i;
  for (i = 1; i <= players.Length(); i++)   players[i] = i;

  NFSupport *T = ComputeDominated(*S, strong, players,
				  ((OutputPortion *) param[3])->Value());

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  Portion *por = (T) ? new NfSupportValPortion(T) : new NfSupportValPortion(new NFSupport(*S));

  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}


Portion *GSM_ElimDom_Nfg(Portion **param)
{
  NFSupport *S = new NFSupport( * ((NfgPortion *) param[0])->Value() );
  bool strong = ((BoolPortion *) param[1])->Value();
  
  gWatch watch;
  gBlock<int> players(S->BelongsTo().NumPlayers());
  int i;
  for (i = 1; i <= players.Length(); i++)   players[i] = i;

  NFSupport *T = ComputeDominated(*S, strong, players,
				  ((OutputPortion *) param[3])->Value());

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  Portion *por = (T) ? new NfSupportValPortion(T) : new NfSupportValPortion(new NFSupport(*S));

  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

//-----------------
// ElimMixedDom
//-----------------

// WARNING:  This is a purely speculative function, don't include in
//           official distributions unless I say so!  --  Ted

NFSupport *ComputeMixedDominated(NFSupport &S, bool strong,
				 const gArray<int> &players,
				 gOutput &tracefile);

Portion *GSM_ElimMixedDom_NfSupport(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  bool strong = ((BoolPortion *) param[1])->Value();
  
  gWatch watch;
  gBlock<int> players(S->BelongsTo().NumPlayers());
  int i;
  for (i = 1; i <= players.Length(); i++)   players[i] = i;

  NFSupport *T = ComputeMixedDominated(*S, strong, players,
				       ((OutputPortion *) param[3])->Value());

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  Portion *por = (T) ? new NfSupportValPortion(T) : new NfSupportValPortion(new NFSupport(*S));

  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}


Portion *GSM_ElimMixedDom_Nfg(Portion **param)
{
  NFSupport *S = new NFSupport( * ((NfgPortion *) param[0])->Value() );
  bool strong = ((BoolPortion *) param[1])->Value();
  
  gWatch watch;
  gBlock<int> players(S->BelongsTo().NumPlayers());
  int i;
  for (i = 1; i <= players.Length(); i++)   players[i] = i;

  NFSupport *T = ComputeMixedDominated(*S, strong, players,
				       ((OutputPortion *) param[3])->Value());

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  Portion *por = (T) ? new NfSupportValPortion(T) : new NfSupportValPortion(new NFSupport(*S));

  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}
  

//----------
// Float
//----------

extern Nfg<double> *ConvertNfg(const Nfg<gRational> &);

Portion *GSM_Float_Nfg(Portion **param)
{
  Nfg<gRational> &orig = * (Nfg<gRational> *) ((NfgPortion *) param[0])->Value();
  Nfg<double> *N = ConvertNfg(orig);

  if (N)
    return new NfgValPortion(N);
  else
    return new ErrorPortion("Conversion failed.");
}

//--------------
// IsConstSum
//--------------

Portion *GSM_IsConstSum_Nfg(Portion **param)
{
  BaseNfg &N = * ((NfgPortion *) param[0])->Value();
  return new BoolValPortion(N.IsConstSum());
}

//-----------
// LoadNfg
//-----------

Portion *GSM_LoadNfg(Portion **param)
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

Portion *GSM_Name_Nfg(Portion **param)
{
  BaseNfg &N = * ((NfgPortion*) param[0])->Value();
  return new TextValPortion(N.GetTitle());
}

Portion* GSM_Name_NfPlayer( Portion** param )
{
  NFPlayer *p = ( (NfPlayerPortion*) param[ 0 ] )->Value();
  return new TextValPortion( p->GetName() );
}

Portion* GSM_Name_Strategy( Portion** param )
{
  Strategy *s = ( (StrategyPortion*) param[ 0 ] )->Value();
  return new TextValPortion( s->name );
}

//----------
// NewNfg
//----------

Portion *GSM_NewNfg(Portion **param)
{
  ListPortion *dim = ((ListPortion *) param[0]);
  gArray<int> d(dim->Length());
  Portion* p;
  
  for (int i = 1; i <= dim->Length(); i++)
  {
    p = dim->Subscript(i);
    d[i] = ((IntPortion *) p)->Value();
    delete p;
  }

  BaseNfg* N;
  if( !( (BoolPortion*) param[ 1 ] )->Value() )
    N = new Nfg<double>( d );
  else
    N = new Nfg<gRational>( d );
  return new NfgValPortion(N);
}


//--------------
// NewSupport
//--------------

Portion *GSM_NewSupport_Nfg(Portion **param)
{
  BaseNfg &N = * ((NfgPortion *) param[0])->Value();
  Portion *p = new NfSupportValPortion(new NFSupport(N));

  p->SetOwner( param[ 0 ]->Original() );
  p->AddDependency();
  return p;
}

//--------------
// NumPlayers
//--------------

Portion *GSM_NumPlayers_Nfg(Portion **param)
{
  BaseNfg &N = * ((NfgPortion*) param[0])->Value();
  return new IntValPortion(N.NumPlayers());
}

//-------------
// NumStrats
//-------------

Portion *GSM_NumStrats( Portion** param )
{
  int i;
  gArray< int > dim;

  NFPlayer* P = (NFPlayer *) ((NfPlayerPortion *) param[0])->Value();
  NFSupport* s = ( (NfSupportPortion*) param[ 1 ] )->Value();

  if( s == 0 )
    return new IntValPortion( P->StrategyList().Length() );
  else
  {
    dim = s->SupportDimensions();
    for( i = 1; i <= dim.Length(); i++ )
    {
      if( &( s->GetNFStrategySet( i )->GetPlayer() ) == P )
	return new IntValPortion( s->NumStrats( i ) );
    }
  }
  return new ErrorPortion( "Specified player is not found in the support" );
}


//------------
// Players
//------------

Portion *GSM_Players_Nfg(Portion **param)
{
  BaseNfg &N = *((NfgPortion*) param[0])->Value();

  Portion* p = ArrayToList(N.PlayerList());
  p->SetOwner( param[ 0 ]->Original() );
  p->AddDependency();
  return p;
}

//-------------
// RandomNfg
//-------------

Portion *GSM_RandomNfg_Float(Portion **param)
{
  Nfg<double> &N = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();
  
  RandomNfg(N);
  return param[0]->RefCopy();
}

Portion *GSM_RandomNfg_Rational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational> *) ((NfgPortion *) param[0])->Value();
  
  RandomNfg(N);
  return param[0]->RefCopy();
}

Portion *GSM_RandomNfg_SeedFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();
  int seed = ((IntPortion *) param[1])->Value();

  SetSeed(seed);
  RandomNfg(N);
  return param[0]->RefCopy();
}

Portion *GSM_RandomNfg_SeedRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational> *) ((NfgPortion *) param[0])->Value();
  int seed = ((IntPortion *) param[1])->Value();

  SetSeed(seed);
  RandomNfg(N);
  return param[0]->RefCopy();
}  

//------------
// Rational
//------------

extern Nfg<gRational> *ConvertNfg(const Nfg<double> &);

Portion *GSM_Rational_Nfg(Portion **param)
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

Portion *GSM_RemoveStrategy(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  Strategy *s = ((StrategyPortion *) param[1])->Value();
  
  S->GetNFStrategySet(s->nfp->GetNumber())->RemoveStrategy(s);

  Portion* por = new NfSupportValPortion(S);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

//------------
// SaveNfg
//------------

Portion *GSM_SaveNfg(Portion **param)
{
  BaseNfg* N = ((NfgPortion*) param[0])->Value();
  gString file = ((TextPortion *) param[1])->Value();
  gFileOutput f(file);

  if (!f.IsValid())
    return new ErrorPortion("Unable to open file for output");

  N->WriteNfgFile(f);

  return param[0]->RefCopy();
}


//------------
// SetName
//------------

Portion *GSM_SetName_Nfg(Portion **param)
{
  BaseNfg &N = * ((NfgPortion*) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  N.SetTitle(name);
  return param[0]->ValCopy();
}

Portion *GSM_SetName_NfPlayer(Portion **param)
{
  NFPlayer *p = ((NfPlayerPortion *) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  p->SetName(name);
  return param[0]->ValCopy();
}

Portion *GSM_SetName_Strategy(Portion **param)
{
  Strategy *s = ((StrategyPortion *) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  s->name = name;
  return param[0]->ValCopy();
}


//--------------
// Strategies
//--------------

Portion *GSM_Strategies(Portion **param)
{
  int i;
  gArray< int > dim;
  Portion* por = 0;

  NFPlayer *P = (NFPlayer *) ((NfPlayerPortion *) param[0])->Value();
  NFSupport* s = ( (NfSupportPortion*) param[ 1 ] )->Value();

  if( s == 0 )
    por = ArrayToList(P->StrategyList());
  else
  {
    dim = s->SupportDimensions();
    for( i = 1; i <= dim.Length(); i++ )
    {
      if( &( s->GetNFStrategySet( i )->GetPlayer() ) == P )
      {
	por = ArrayToList( s->GetStrategy( i ) );
	break;
      }
    }
  }

  if( por != 0 )
  {
    por->SetOwner(param[0]->Owner());
    por->AddDependency();
  }
  else
  {
    por = new ErrorPortion( "Specified player is not found in the support" );
  }
  return por;
}


void Init_nfgfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;


  FuncObj = new FuncDescObj("AddStrategy", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_AddStrategy, porNF_SUPPORT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porNF_SUPPORT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("strategy", porSTRATEGY));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("CompressNfg", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_CompressNfg_Float,
				       porNFG_FLOAT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG_FLOAT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("support", porNF_SUPPORT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_CompressNfg_Rational,
				       porNFG_RATIONAL, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("nfg", porNFG_RATIONAL,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("support", porNF_SUPPORT));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("ElimAllDom", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ElimAllDom_NfSupport, 
				       porNF_SUPPORT, 5));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porNF_SUPPORT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("strong", porBOOL,
					    new BoolValPortion(false)));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("time", porFLOAT,
					    new FloatValPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 3, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(0, 4, ParamInfoType("traceLevel", porINTEGER,
					    new IntValPortion(0)));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_ElimAllDom_Nfg, 
				       porNF_SUPPORT, 5));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("nfg", porNFG,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("strong", porBOOL,
					    new BoolValPortion(false)));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("time", porFLOAT,
					    new FloatValPortion(0.0), BYREF));
  FuncObj->SetParamInfo(1, 3, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(1, 4, ParamInfoType("traceLevel", porINTEGER,
					    new IntValPortion(0)));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("ElimDom", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ElimDom_NfSupport, 
				       porNF_SUPPORT, 5));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porNF_SUPPORT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("strong", porBOOL,
					    new BoolValPortion(false)));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("time", porFLOAT,
					    new FloatValPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 3, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(0, 4, ParamInfoType("traceLevel", porINTEGER,
					    new IntValPortion(0)));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_ElimDom_Nfg, 
				       porNF_SUPPORT, 5));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("nfg", porNFG,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("strong", porBOOL,
					    new BoolValPortion(false)));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("time", porFLOAT,
					    new FloatValPortion(0.0), BYREF));
  FuncObj->SetParamInfo(1, 3, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(1, 4, ParamInfoType("traceLevel", porINTEGER,
					    new IntValPortion(0)));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("ElimMixedDom", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ElimMixedDom_NfSupport, 
				       porNF_SUPPORT, 5));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porNF_SUPPORT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("strong", porBOOL,
					    new BoolValPortion(false)));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("time", porFLOAT,
					    new FloatValPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 3, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(0, 4, ParamInfoType("traceLevel", porINTEGER,
					    new IntValPortion(0)));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_ElimMixedDom_Nfg, 
				       porNF_SUPPORT, 5));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("nfg", porNFG,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("strong", porBOOL,
					    new BoolValPortion(false)));
  FuncObj->SetParamInfo(1, 2, ParamInfoType("time", porFLOAT,
					    new FloatValPortion(0.0), BYREF));
  FuncObj->SetParamInfo(1, 3, ParamInfoType("traceFile", porOUTPUT,
					    new OutputRefPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(1, 4, ParamInfoType("traceLevel", porINTEGER,
					    new IntValPortion(0)));

  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("Float", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Float_Nfg, 
				       porNFG_FLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG_RATIONAL,
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("IsConstSum", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsConstSum_Nfg, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG,
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("LoadNfg", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_LoadNfg, porNFG, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("file", porTEXT));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("Name", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Name_Nfg, porTEXT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porNFG, REQUIRED, BYREF));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Name_NfPlayer, porTEXT, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porPLAYER_NFG));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Name_Strategy, porTEXT, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porSTRATEGY));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("NewNfg", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NewNfg, porNFG, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("dim", PortionSpec(porINTEGER,1)));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("rational", porBOOL,
					    new BoolValPortion(false)));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("Support", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NewSupport_Nfg, porNF_SUPPORT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG,
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("NumPlayers", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NumPlayers_Nfg, porINTEGER, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG,
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("NumStrats", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NumStrats, porINTEGER, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("player", porPLAYER_NFG));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("support", porNF_SUPPORT, 
					    new NfSupportValPortion( 0 )));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("Players", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Players_Nfg, 
				       PortionSpec(porPLAYER_NFG, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG,
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);
  
  

  FuncObj = new FuncDescObj("Randomize", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_RandomNfg_Float, 
				       porNFG_FLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porNFG_FLOAT,
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_RandomNfg_Rational, 
				       porNFG_RATIONAL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porNFG_RATIONAL,
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_RandomNfg_SeedFloat, 
				       porNFG_FLOAT, 2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porNFG_FLOAT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("seed", porINTEGER));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_RandomNfg_SeedRational, 
				       porNFG_RATIONAL, 2));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", porNFG_RATIONAL,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("seed", porINTEGER));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("RandomNfg", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_RandomNfg_Float, 
				       porNFG_FLOAT, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porNFG_FLOAT,
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_RandomNfg_Rational, 
				       porNFG_RATIONAL, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porNFG_RATIONAL,
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_RandomNfg_SeedFloat, 
				       porNFG_FLOAT, 2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porNFG_FLOAT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("seed", porINTEGER));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_RandomNfg_SeedRational, 
				       porNFG_RATIONAL, 2));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", porNFG_RATIONAL,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("seed", porINTEGER));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("Rational", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Rational_Nfg, 
				       porNFG_RATIONAL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG_FLOAT,
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("RemoveStrategy", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_RemoveStrategy, 
				       porNF_SUPPORT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("support", porNF_SUPPORT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("strategy", porSTRATEGY));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("SaveNfg", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SaveNfg, porNFG, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("nfg", porNFG, REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("file", porTEXT));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("SetName", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetName_Nfg, porNFG, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porNFG, REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("name", porTEXT));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_SetName_NfPlayer, 
				       porPLAYER_NFG, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porPLAYER_NFG));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("name", porTEXT));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_SetName_Strategy, 
				       porSTRATEGY, 2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porSTRATEGY));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("name", porTEXT));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("Strategies", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Strategies, 
				       PortionSpec(porSTRATEGY, 1), 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("player", porPLAYER_NFG));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("support", porNF_SUPPORT, 
					    new NfSupportValPortion( 0 )));
  gsm->AddFunction(FuncObj);
}
