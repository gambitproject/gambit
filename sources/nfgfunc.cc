//
// FILE: nfgfunc.cc -- Normal form command language builtins
//
// $Id$
//


#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "nfg.h"

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

Portion *GSM_CompressNfg(Portion **param)
{
  Nfg<double> &N = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();
  Nfg<double> *M = new Nfg<double>(N);
  return new NfgValPortion(M);
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


  FuncObj = new FuncDescObj("AddStrategy");
  FuncObj->SetFuncInfo(GSM_AddStrategy, 2);
  FuncObj->SetParamInfo(GSM_AddStrategy, 0, "support", porNF_SUPPORT);
  FuncObj->SetParamInfo(GSM_AddStrategy, 1, "strategy", porSTRATEGY);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("CompressNfg");
  FuncObj->SetFuncInfo(GSM_CompressNfg, 1);
  FuncObj->SetParamInfo(GSM_CompressNfg, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("ElimAllDom");
  FuncObj->SetFuncInfo(GSM_ElimAllDom_NfSupport, 5);
  FuncObj->SetParamInfo(GSM_ElimAllDom_NfSupport, 0, "support", porNF_SUPPORT);
  FuncObj->SetParamInfo(GSM_ElimAllDom_NfSupport, 1, "strong", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_ElimAllDom_NfSupport, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ElimAllDom_NfSupport, 3, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ElimAllDom_NfSupport, 4, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_ElimAllDom_Nfg, 5);
  FuncObj->SetParamInfo(GSM_ElimAllDom_Nfg, 0, "nfg", porNFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_ElimAllDom_Nfg, 1, "strong", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_ElimAllDom_Nfg, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ElimAllDom_Nfg, 3, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ElimAllDom_Nfg, 4, "traceLevel", porINTEGER,
			new IntValPortion(0));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("ElimDom");
  FuncObj->SetFuncInfo(GSM_ElimDom_NfSupport, 5);
  FuncObj->SetParamInfo(GSM_ElimDom_NfSupport, 0, "support", porNF_SUPPORT);
  FuncObj->SetParamInfo(GSM_ElimDom_NfSupport, 1, "strong", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_ElimDom_NfSupport, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ElimDom_NfSupport, 3, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ElimDom_NfSupport, 4, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_ElimDom_Nfg, 5);
  FuncObj->SetParamInfo(GSM_ElimDom_Nfg, 0, "nfg", porNFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ElimDom_Nfg, 1, "strong", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_ElimDom_Nfg, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ElimDom_Nfg, 3, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ElimDom_Nfg, 4, "traceLevel", porINTEGER,
			new IntValPortion(0));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("ElimMixedDom");
  FuncObj->SetFuncInfo(GSM_ElimMixedDom_NfSupport, 5);
  FuncObj->SetParamInfo(GSM_ElimMixedDom_NfSupport, 0, "support", porNF_SUPPORT);
  FuncObj->SetParamInfo(GSM_ElimMixedDom_NfSupport, 1, "strong", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_ElimMixedDom_NfSupport, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ElimMixedDom_NfSupport, 3, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ElimMixedDom_NfSupport, 4, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_ElimMixedDom_Nfg, 5);
  FuncObj->SetParamInfo(GSM_ElimMixedDom_Nfg, 0, "nfg", porNFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ElimMixedDom_Nfg, 1, "strong", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_ElimMixedDom_Nfg, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ElimMixedDom_Nfg, 3, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ElimMixedDom_Nfg, 4, "traceLevel", porINTEGER,
			new IntValPortion(0));

  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Float");
  FuncObj->SetFuncInfo(GSM_Float_Nfg, 1);
  FuncObj->SetParamInfo(GSM_Float_Nfg, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsConstSum");
  FuncObj->SetFuncInfo(GSM_IsConstSum_Nfg, 1);
  FuncObj->SetParamInfo(GSM_IsConstSum_Nfg, 0, "nfg", porNFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("LoadNfg");
  FuncObj->SetFuncInfo(GSM_LoadNfg, 1);
  FuncObj->SetParamInfo(GSM_LoadNfg, 0, "file", porTEXT);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Name");
  FuncObj->SetFuncInfo(GSM_Name_Nfg, 1);
  FuncObj->SetParamInfo(GSM_Name_Nfg, 0, "x", porNFG, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );

  FuncObj->SetFuncInfo(GSM_Name_NfPlayer, 1);
  FuncObj->SetParamInfo(GSM_Name_NfPlayer, 0, "x", porPLAYER_NFG);

  FuncObj->SetFuncInfo(GSM_Name_Strategy, 1);
  FuncObj->SetParamInfo(GSM_Name_Strategy, 0, "x", porSTRATEGY);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NewNfg");
  FuncObj->SetFuncInfo(GSM_NewNfg, 2);
  FuncObj->SetParamInfo(GSM_NewNfg, 0, "dim", porLIST | porINTEGER);
  FuncObj->SetParamInfo(GSM_NewNfg, 1, "rational", porBOOL,
			new BoolValPortion(false));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NewSupport");
  FuncObj->SetFuncInfo(GSM_NewSupport_Nfg, 1);
  FuncObj->SetParamInfo(GSM_NewSupport_Nfg, 0, "nfg", porNFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NumPlayers");
  FuncObj->SetFuncInfo(GSM_NumPlayers_Nfg, 1);
  FuncObj->SetParamInfo(GSM_NumPlayers_Nfg, 0, "nfg", porNFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NumStrats");
  FuncObj->SetFuncInfo(GSM_NumStrats, 2);
  FuncObj->SetParamInfo(GSM_NumStrats, 0, "player", porPLAYER_NFG);
  FuncObj->SetParamInfo(GSM_NumStrats, 1, "support", porNF_SUPPORT, 
			new NfSupportValPortion( 0 ) );
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Players");
  FuncObj->SetFuncInfo(GSM_Players_Nfg, 1);
  FuncObj->SetParamInfo(GSM_Players_Nfg, 0, "nfg", porNFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("RandomNfg");
  FuncObj->SetFuncInfo(GSM_RandomNfg_Float, 1);
  FuncObj->SetParamInfo(GSM_RandomNfg_Float, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_RandomNfg_Rational, 1);
  FuncObj->SetParamInfo(GSM_RandomNfg_Rational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_RandomNfg_SeedFloat, 2);
  FuncObj->SetParamInfo(GSM_RandomNfg_SeedFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_RandomNfg_SeedFloat, 1, "seed", porINTEGER);

  FuncObj->SetFuncInfo(GSM_RandomNfg_SeedRational, 2);
  FuncObj->SetParamInfo(GSM_RandomNfg_SeedRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_RandomNfg_SeedRational, 1, "seed", porINTEGER);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Rational");
  FuncObj->SetFuncInfo(GSM_Rational_Nfg, 1);
  FuncObj->SetParamInfo(GSM_Rational_Nfg, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("RemoveStrategy");
  FuncObj->SetFuncInfo(GSM_RemoveStrategy, 2);
  FuncObj->SetParamInfo(GSM_RemoveStrategy, 0, "support", porNF_SUPPORT);
  FuncObj->SetParamInfo(GSM_RemoveStrategy, 1, "strategy", porSTRATEGY);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("SaveNfg");
  FuncObj->SetFuncInfo(GSM_SaveNfg, 2);
  FuncObj->SetParamInfo(GSM_SaveNfg, 0, "nfg", porNFG, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SaveNfg, 1, "file", porTEXT);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("SetName");
  FuncObj->SetFuncInfo(GSM_SetName_Nfg, 2);
  FuncObj->SetParamInfo(GSM_SetName_Nfg, 0, "x", porNFG, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SetName_Nfg, 1, "name", porTEXT);

  FuncObj->SetFuncInfo(GSM_SetName_NfPlayer, 2);
  FuncObj->SetParamInfo(GSM_SetName_NfPlayer, 0, "x", porPLAYER_NFG);
  FuncObj->SetParamInfo(GSM_SetName_NfPlayer, 1, "name", porTEXT);

  FuncObj->SetFuncInfo(GSM_SetName_Strategy, 2);
  FuncObj->SetParamInfo(GSM_SetName_Strategy, 0, "x", porSTRATEGY);
  FuncObj->SetParamInfo(GSM_SetName_Strategy, 1, "name", porTEXT);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Strategies");
  FuncObj->SetFuncInfo(GSM_Strategies, 2);
  FuncObj->SetParamInfo(GSM_Strategies, 0, "player", porPLAYER_NFG);
  FuncObj->SetParamInfo(GSM_Strategies, 1, "support", porNF_SUPPORT, 
			new NfSupportValPortion( 0 ) );
  gsm->AddFunction(FuncObj);
}





