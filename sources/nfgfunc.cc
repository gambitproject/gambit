//#
//# FILE: nfgfunc.cc -- Normal form command language builtins
//#
//# $Id$
//#


#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "nfg.h"

#include "glist.h"
#include "mixed.h"



extern double Funct_tolBrent;
extern double Funct_tolN;
extern int Funct_maxitsBrent;
extern int Funct_maxitsN;










Portion *ArrayToList(const gArray<NFPlayer *> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NfPlayerValPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<Strategy *> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new StrategyValPortion(A[i]));
  return ret;
}

//
// These functions are added to the function list in efgfunc.cc along with
// their extensive form counterparts.
// What's a good way of dealing with these sorts of functions?
//
Portion *GSM_CentroidNfgFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();
  MixedProfile<double> *P = new MixedProfile<double>(N);

  Portion* por = new MixedValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_CentroidNfgRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();
  MixedProfile<gRational> *P = new MixedProfile<gRational>(N);

  Portion* por = new MixedValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_CentroidNFSupport(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  BaseMixedProfile *P;

  if (S->BelongsTo().Type() == DOUBLE)
    P = new MixedProfile<double>((Nfg<double> &) S->BelongsTo(), *S);
  else
    P = new MixedProfile<gRational>((Nfg<gRational> &) S->BelongsTo(), *S);

  Portion *por = new MixedValPortion(P);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

Portion *GSM_NumPlayersNfg(Portion **param)
{
  BaseNfg &N = * ((NfgPortion*) param[0])->Value();
  return new IntValPortion(N.NumPlayers());
}

Portion *GSM_NameNfg(Portion **param)
{
  BaseNfg &N = * ((NfgPortion*) param[0])->Value();
  return new TextValPortion(N.GetTitle());
}

Portion *GSM_PlayersNfg(Portion **param)
{
  BaseNfg &N = *((NfgPortion*) param[0])->Value();

  Portion* p = ArrayToList(N.PlayerList());
  p->SetOwner( param[ 0 ]->Original() );
  p->AddDependency();
  return p;
}

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

Portion *GSM_RemoveStrategy(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  Strategy *s = ((StrategyPortion *) param[1])->Value();
  
  S->GetNFStrategySet(s->nfp->GetNumber())->RemoveStrategy(s);

  Portion* por = new StrategyValPortion(s);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

Portion *GSM_SetNameNfg(Portion **param)
{
  BaseNfg &N = * ((NfgPortion*) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  N.SetTitle(name);
  return param[0]->ValCopy();
}


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


#include "gwatch.h"

extern NFSupport *ComputeDominated(NFSupport &S, bool strong, 
				   const gArray<int> &players,
				   gOutput &tracefile);


Portion *GSM_ElimDom(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  bool strong = ((BoolPortion *) param[1])->Value();
  
  gWatch watch;
  gBlock<int> players(S->BelongsTo().NumPlayers());
  int i;
  for (i = 1; i <= players.Length(); i++)   players[i] = i;

  NFSupport *T = ComputeDominated(*S, strong, players, gout);

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  Portion *por = (T) ? new NfSupportValPortion(T) : new NfSupportValPortion(new NFSupport(*S));

  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}



template <class T> class Mixed_ListPortion : public ListValPortion   {
  public:
    Mixed_ListPortion(const gList<MixedProfile<T> > &);
};

Mixed_ListPortion<double>::Mixed_ListPortion(const gList<MixedProfile<double> > &list)
{
  _DataType = porMIXED_FLOAT;
  for (int i = 1; i <= list.Length(); i++)
    Append(new MixedValPortion( new MixedProfile<double>(list[i])));
}

Mixed_ListPortion<gRational>::Mixed_ListPortion(const gList<MixedProfile<gRational> > &list)
{
  _DataType = porMIXED_RATIONAL;
  for (int i = 1; i <= list.Length(); i++)
    Append(new MixedValPortion( new MixedProfile<gRational>(list[i])));
}



//-------------------------- LPSolve -----------------------------//

#include "csum.h"

Portion *GSM_ConstSumFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();

  ZSumParams ZP;

  NFSupport S(N);
  ZSumModule<double> ZM(N, ZP, S);
  ZM.ZSum();

  ((IntPortion *) param[1])->Value() = ZM.NumPivots();
  ((FloatPortion *) param[2])->Value() = ZM.Time();

  gList<MixedProfile<double> > solns;
  ZM.GetSolutions(solns);

  Portion* por = new Mixed_ListPortion<double>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_ConstSumRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational> *) ((NfgPortion *) param[0])->Value();

  ZSumParams ZP;
  
  NFSupport S(N);
  ZSumModule<gRational> ZM(N, ZP, S);
  ZM.ZSum();

  ((IntPortion *) param[1])->Value() = ZM.NumPivots();
  ((FloatPortion *) param[2])->Value() = ZM.Time();

  gList<MixedProfile<gRational> > solns;
  ZM.GetSolutions(solns);

  Portion* por = new Mixed_ListPortion<gRational>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_ConstSumSupport(Portion **param)
{
  NFSupport& S = * ( (NfSupportPortion*) param[ 0 ] )->Value();
  BaseNfg* N = (BaseNfg*) &( S.BelongsTo() );
  Portion* por = 0;

  ZSumParams ZP;

  switch( N->Type() )
  {
  case DOUBLE:
    {
      ZSumModule<double> ZM( * (Nfg<double>*) N, ZP, S);
      ZM.ZSum();
      ((IntPortion *) param[1])->Value() = ZM.NumPivots();
      ((FloatPortion *) param[2])->Value() = ZM.Time();
      gList<MixedProfile<double> > solns;
      ZM.GetSolutions(solns);  por = new Mixed_ListPortion<double>(solns);
    }
    break;
  case RATIONAL:
    {
      ZSumModule<gRational> ZM( * (Nfg<gRational>*) N, ZP, S);
      ZM.ZSum();
      ((IntPortion *) param[1])->Value() = ZM.NumPivots();
      ((FloatPortion *) param[2])->Value() = ZM.Time();
      gList<MixedProfile<gRational> > solns;
      ZM.GetSolutions(solns);  por = new Mixed_ListPortion<gRational>(solns);
    }
    break;
  default:
    assert( 0 );
  }

  assert( por != 0 );
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}




//----------------------- EnumMixedSolve -------------------------//

#include "enum.h"

Portion *GSM_EnumNfgFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();

  EnumParams EP;

  EP.stopAfter = ((IntPortion *) param[1])->Value();
  
  NFSupport S(N);
  EnumModule<double> EM(N, EP, S);
  EM.Enum();

  ((IntPortion *) param[2])->Value() = EM.NumPivots();
  ((FloatPortion *) param[3])->Value() = EM.Time();

  Portion* por = new Mixed_ListPortion<double>(EM.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_EnumNfgRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();

  EnumParams EP;

  EP.stopAfter = ((IntPortion *) param[1])->Value();
  
  NFSupport S(N);
  EnumModule<gRational> EM(N, EP, S);
  EM.Enum();

  ((IntPortion *) param[2])->Value() = EM.NumPivots();
  ((FloatPortion *) param[3])->Value() = EM.Time();

  Portion* por = new Mixed_ListPortion<gRational>(EM.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}



Portion *GSM_EnumSupport(Portion **param)
{
  NFSupport* S = ( (NfSupportPortion*) param[ 0 ] )->Value();
  BaseNfg* N = (BaseNfg*) &( S->BelongsTo() );
  Portion* por = 0;

  EnumParams EP;

  EP.stopAfter = ((IntPortion *) param[1])->Value();

  switch( N->Type() )
  {
  case DOUBLE:
    {
      EnumModule<double> EM( * (Nfg<double>*) N, EP, *S);
      EM.Enum();
      ((IntPortion *) param[2])->Value() = EM.NumPivots();
      ((FloatPortion *) param[3])->Value() = EM.Time();
      por = new Mixed_ListPortion<double>(EM.GetSolutions());
    }
    break;
  case RATIONAL:
    {
      EnumModule<gRational> EM( * (Nfg<gRational>*) N, EP, *S);
      EM.Enum();
      ((IntPortion *) param[2])->Value() = EM.NumPivots();
      ((FloatPortion *) param[3])->Value() = EM.Time();
      por = new Mixed_ListPortion<gRational>(EM.GetSolutions());
    }
    break;
  default:
    assert( 0 );
  }
  assert( por != 0 );
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}





//---------------------------- GobitSolve ----------------------------//

#include "ngobit.h"


Portion *GSM_GobitNfg_NfgFloat(Portion **param)
{
  int old_Funct_maxitsN = Funct_maxitsN;
  double old_Funct_tolN = Funct_tolN;
  int old_Funct_maxitsBrent = Funct_maxitsBrent;
  double old_Funct_tolBrent = Funct_tolBrent;

  Nfg<double> &E = *(Nfg<double> *) ((NfgPortion *) param[0])->Value();
  MixedProfile<double> start(E);
  
  NFGobitParams<double> EP;
  if( ((TextPortion*) param[1])->Value() != "" )
    EP.pxifile = new gFileOutput( ((TextPortion*) param[1])->Value() );
  else
    EP.pxifile = &gnull;
  EP.minLam = ((FloatPortion *) param[2])->Value();
  EP.maxLam = ((FloatPortion *) param[3])->Value();
  EP.delLam = ((FloatPortion *) param[4])->Value();
  EP.powLam = ((IntPortion *) param[5])->Value();
  EP.fullGraph = ((BoolPortion *) param[6])->Value();

  Funct_maxitsN = ( (IntPortion*) param[7] )->Value();
  Funct_tolN = ( (FloatPortion*) param[8] )->Value();
  Funct_maxitsBrent = ( (IntPortion*) param[9] )->Value();
  Funct_tolBrent = ( (FloatPortion*) param[10] )->Value();
  
  NFGobitModule<double> M(E, EP, start);
  M.Gobit(1);

  ((FloatPortion *) param[11])->Value() = M.Time();
  ((IntPortion *) param[12])->Value() = M.NumEvals();
  ((IntPortion *) param[13])->Value() = M.NumIters();

  Funct_maxitsN = old_Funct_maxitsN;
  Funct_tolN = old_Funct_tolN;
  Funct_maxitsBrent = old_Funct_maxitsBrent;
  Funct_tolBrent = old_Funct_tolBrent;

  Portion* por = new Mixed_ListPortion<double>(M.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}


Portion *GSM_GobitNfg_MixedFloat(Portion **param)
{
  int old_Funct_maxitsN = Funct_maxitsN;
  double old_Funct_tolN = Funct_tolN;
  int old_Funct_maxitsBrent = Funct_maxitsBrent;
  double old_Funct_tolBrent = Funct_tolBrent;

  MixedProfile<double>& start = 
    * (MixedProfile<double> *) ((MixedPortion *) param[0])->Value();
  Nfg<double> &E = *( start.BelongsTo() );
  
  NFGobitParams<double> EP;
  if( ((TextPortion*) param[1])->Value() != "" )
    EP.pxifile = new gFileOutput( ((TextPortion*) param[1])->Value() );
  else
    EP.pxifile = &gnull;
  EP.minLam = ((FloatPortion *) param[2])->Value();
  EP.maxLam = ((FloatPortion *) param[3])->Value();
  EP.delLam = ((FloatPortion *) param[4])->Value();
  EP.powLam = ((IntPortion *) param[5])->Value();
  EP.fullGraph = ((BoolPortion *) param[6])->Value();

  Funct_maxitsN = ( (IntPortion*) param[7] )->Value();
  Funct_tolN = ( (FloatPortion*) param[8] )->Value();
  Funct_maxitsBrent = ( (IntPortion*) param[9] )->Value();
  Funct_tolBrent = ( (FloatPortion*) param[10] )->Value();
  
  NFGobitModule<double> M(E, EP, start);
  M.Gobit(1);

  ((FloatPortion *) param[11])->Value() = M.Time();
  ((IntPortion *) param[12])->Value() = M.NumEvals();
  ((IntPortion *) param[13])->Value() = M.NumIters();

  Funct_maxitsN = old_Funct_maxitsN;
  Funct_tolN = old_Funct_tolN;
  Funct_maxitsBrent = old_Funct_maxitsBrent;
  Funct_tolBrent = old_Funct_tolBrent;

  Portion* por = new Mixed_ListPortion<double>(M.GetSolutions());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}






//-------------------------- GobitGridSolve -------------------------//

#include "grid.h"

Portion *GSM_GridSolveFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();
  
  GridParams<double> GP;
  if( ((TextPortion*) param[1])->Value() != "" )
    GP.pxifile = new gFileOutput( ((TextPortion*) param[1])->Value() );
  else
    GP.pxifile = &gnull;
  GP.minLam = ((FloatPortion *) param[2])->Value();
  GP.maxLam = ((FloatPortion *) param[3])->Value();
  GP.delLam = ((FloatPortion *) param[4])->Value();
  GP.powLam = ((IntPortion *) param[5])->Value();
  GP.delp = ((FloatPortion *) param[6])->Value();
  GP.tol = ((FloatPortion *) param[7])->Value();

  NFSupport S(N);
  GridSolveModule<double> GM(N, GP, S);
  GM.GridSolve();

//  ((IntPortion *) param[8])->Value() = GM.NumEvals();
//  ((FloatPortion *) param[9])->Value() = GM.Time();

  gList<MixedProfile<double> > solns;

  Portion* por = new Mixed_ListPortion<double>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}


Portion *GSM_GridSolveSupport(Portion **param)
{
  NFSupport& S = * ( (NfSupportPortion*) param[ 0 ] )->Value();
  BaseNfg* N = (BaseNfg*) &( S.BelongsTo() );
  Portion* por = 0;

  GridParams<double> GP;
  if( ((TextPortion*) param[1])->Value() != "" )
    GP.pxifile = new gFileOutput( ((TextPortion*) param[1])->Value() );
  else
    GP.pxifile = &gnull;
  GP.minLam = ((FloatPortion *) param[2])->Value();
  GP.maxLam = ((FloatPortion *) param[3])->Value();
  GP.delLam = ((FloatPortion *) param[4])->Value();
  GP.powLam = ((IntPortion *) param[5])->Value();
  GP.delp = ((FloatPortion *) param[6])->Value();
  GP.tol = ((FloatPortion *) param[7])->Value();

  switch( N->Type() )
  {
  case DOUBLE:
    {
      GridSolveModule<double> GM( * (Nfg<double>*) N, GP, S);
      GM.GridSolve();
      // ((IntPortion *) param[8])->Value() = GM.NumEvals();
      // ((FloatPortion *) param[9])->Value() = GM.Time();
      gList<MixedProfile<double> > solns;
      por = new Mixed_ListPortion<double>(solns);
    }
    break;
  case RATIONAL:
    {
      return new ErrorPortion( "The rational version of GobitGridSolve is not implemented" );
      /*
      GridSolveModule<gRational> GM( * (Nfg<gRational>*) N, GP, S);
      GM.GridSolve();
      // ((IntPortion *) param[8])->Value() = GM.NumEvals();
      // ((FloatPortion *) param[9])->Value() = GM.Time();
      gList<MixedProfile<gRational> > solns;
      por = new Mixed_ListPortion<gRational>(solns);
      */
    }
    break;
  default:
    assert( 0 );
  }

  assert( por != 0 );
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}


/*
Portion *GSM_GridSolveRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();
  
  GridParams<gRational> GP;
  if( ((TextPortion*) param[1])->Value() != "" )
    GP.pxifile = new gFileOutput( ((TextPortion*) param[1])->Value() );
  else
    GP.pxifile = &gnull;
  GP.minLam = ((RationalPortion *) param[2])->Value();
  GP.maxLam = ((RationalPortion *) param[3])->Value();
  GP.delLam = ((RationalPortion *) param[4])->Value();
  GP.powLam = ((IntPortion *) param[5])->Value();
  GP.delp = ((RationalPortion *) param[6])->Value();
  GP.tol = ((RationalPortion *) param[7])->Value();

  NFSupport S(N);
  GridSolveModule<gRational> GM(N, GP, S);
  GM.GridSolve();

//  ((IntPortion *) param[8])->Value() = GM.NumEvals();
//  ((FloatPortion *) param[9])->Value() = GM.Time();

  gList<MixedProfile<gRational> > solns;

  Portion* por = new Mixed_ListPortion<gRational>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}
*/


//---------------------------- LCPSolve -------------------------//

#include "lemke.h"

Portion *GSM_LemkeNfgFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();

  LemkeParams LP;
  LP.stopAfter = ((IntPortion *) param[1])->Value();
  
  NFSupport S(N);
  LemkeModule<double> LS(N, LP, S);
  LS.Lemke();

  ((IntPortion *) param[2])->Value() = LS.NumPivots();
  ((FloatPortion *) param[3])->Value() = LS.Time();

  Portion* por = new Mixed_ListPortion<double>(LS.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_LemkeNfgRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();

  LemkeParams LP;
  LP.stopAfter = ((IntPortion *) param[1])->Value();

  NFSupport S(N);
  LemkeModule<gRational> LS(N, LP, S);
  LS.Lemke();

  ((IntPortion *) param[2])->Value() = LS.NumPivots();
  ((FloatPortion *) param[3])->Value() = LS.Time();

  Portion* por = new Mixed_ListPortion<gRational>(LS.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_LemkeNfgSupport(Portion **param)
{
  NFSupport& S = * ( (NfSupportPortion*) param[ 0 ] )->Value();
  BaseNfg* N = (BaseNfg*) &( S.BelongsTo() );
  Portion* por = 0;

  LemkeParams LP;
  LP.stopAfter = ((IntPortion *) param[1])->Value();

  switch( N->Type() )
  {
  case DOUBLE:
    {
      LemkeModule<double> LS(* (Nfg<double>*) N, LP, S);
      LS.Lemke();
      ((IntPortion *) param[2])->Value() = LS.NumPivots();
      ((FloatPortion *) param[3])->Value() = LS.Time();
      por = new Mixed_ListPortion<double>(LS.GetSolutions());
    }
    break;
  case RATIONAL:
    {
      LemkeModule<gRational> LS(* (Nfg<gRational>*) N, LP, S);
      LS.Lemke();
      ((IntPortion *) param[2])->Value() = LS.NumPivots();
      ((FloatPortion *) param[3])->Value() = LS.Time();
      por = new Mixed_ListPortion<gRational>(LS.GetSolutions());
    }
    break;
  default:
    assert( 0 );
  }
  assert( por != 0 );
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}






//---------------------------- LiapSolve -------------------------//

#include "nliap.h"


Portion *GSM_LiapNfg_NfgFloat(Portion **param)
{
  int old_Funct_maxitsN = Funct_maxitsN;
  double old_Funct_tolN = Funct_tolN;
  int old_Funct_maxitsBrent = Funct_maxitsBrent;
  double old_Funct_tolBrent = Funct_tolBrent;

  Nfg<double> &E = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();
  MixedProfile<double> start(E);

  NFLiapParams<double> LP;

  LP.stopAfter = ((IntPortion *) param[1])->Value();
  LP.nTries = ((IntPortion *) param[2])->Value();
 
  Funct_maxitsN = ( (IntPortion*) param[3] )->Value();
  Funct_tolN = ( (FloatPortion*) param[4] )->Value();
  Funct_maxitsBrent = ( (IntPortion*) param[5] )->Value();
  Funct_tolBrent = ( (FloatPortion*) param[6] )->Value();

  NFLiapModule<double> LM(E, LP, start);
  LM.Liap();

  ((FloatPortion *) param[7])->Value() = LM.Time();
  ((IntPortion *) param[8])->Value() = LM.NumEvals();

  Funct_maxitsN = old_Funct_maxitsN;
  Funct_tolN = old_Funct_tolN;
  Funct_maxitsBrent = old_Funct_maxitsBrent;
  Funct_tolBrent = old_Funct_tolBrent;

  Portion* por = new Mixed_ListPortion<double>(LM.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_LiapNfg_MixedFloat(Portion **param)
{
  int old_Funct_maxitsN = Funct_maxitsN;
  double old_Funct_tolN = Funct_tolN;
  int old_Funct_maxitsBrent = Funct_maxitsBrent;
  double old_Funct_tolBrent = Funct_tolBrent;

  MixedProfile<double> &start = 
    * (MixedProfile<double> *) ((MixedPortion *) param[0])->Value();
  Nfg<double> &E = *( start.BelongsTo() );

  NFLiapParams<double> LP;

  LP.stopAfter = ((IntPortion *) param[1])->Value();
  LP.nTries = ((IntPortion *) param[2])->Value();
 
  Funct_maxitsN = ( (IntPortion*) param[3] )->Value();
  Funct_tolN = ( (FloatPortion*) param[4] )->Value();
  Funct_maxitsBrent = ( (IntPortion*) param[5] )->Value();
  Funct_tolBrent = ( (FloatPortion*) param[6] )->Value();
  
  NFLiapModule<double> LM(E, LP, start);
  LM.Liap();

  ((FloatPortion *) param[7])->Value() = LM.Time();
  ((IntPortion *) param[8])->Value() = LM.NumEvals();

  Funct_maxitsN = old_Funct_maxitsN;
  Funct_tolN = old_Funct_tolN;
  Funct_maxitsBrent = old_Funct_maxitsBrent;
  Funct_tolBrent = old_Funct_tolBrent;

  Portion* por = new Mixed_ListPortion<double>(LM.GetSolutions());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}






//---------------------- SimpDivSolve -------------------------//

#include "simpdiv.h"

Portion *GSM_SimpdivFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();
  
  SimpdivParams SP;
  SP.stopAfter = ((IntPortion *) param[1])->Value();
  SP.nRestarts = ((IntPortion *) param[2])->Value();
  SP.leashLength = ((IntPortion *) param[3])->Value();

  NFSupport S(N);
  SimpdivModule<double> SM(N, SP, S);
  SM.Simpdiv();

  ((IntPortion *) param[4])->Value() = SM.NumEvals();
  ((FloatPortion *) param[5])->Value() = SM.Time();
  
  Portion* por = new Mixed_ListPortion<double>(SM.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_SimpdivRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();
  
  SimpdivParams SP;
  SP.stopAfter = ((IntPortion *) param[1])->Value();
  SP.nRestarts = ((IntPortion *) param[2])->Value();
  SP.leashLength = ((IntPortion *) param[3])->Value();

  NFSupport S(N);
  SimpdivModule<gRational> SM(N, SP, S);
  SM.Simpdiv();

  ((IntPortion *) param[4])->Value() = SM.NumEvals();
  ((FloatPortion *) param[5])->Value() = SM.Time();
  
  Portion* por = new Mixed_ListPortion<gRational>(SM.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}


Portion *GSM_SimpdivSupport(Portion **param)
{
  NFSupport& S = * ( (NfSupportPortion*) param[ 0 ] )->Value();
  BaseNfg* N = (BaseNfg*) &( S.BelongsTo() );
  Portion* por = 0;

  SimpdivParams SP;
  SP.stopAfter = ((IntPortion *) param[1])->Value();
  SP.nRestarts = ((IntPortion *) param[2])->Value();
  SP.leashLength = ((IntPortion *) param[3])->Value();

  switch( N->Type() )
  {
  case DOUBLE:
    {
      SimpdivModule<double> SM( * (Nfg<double>*) N, SP, S);
      SM.Simpdiv();
      ((IntPortion *) param[4])->Value() = SM.NumEvals();
      ((FloatPortion *) param[5])->Value() = SM.Time();
      por = new Mixed_ListPortion<double>(SM.GetSolutions());
    }
    break;
  case RATIONAL:
    {
      SimpdivModule<gRational> SM( * (Nfg<gRational>*) N, SP, S);
      SM.Simpdiv();
      ((IntPortion *) param[4])->Value() = SM.NumEvals();
      ((FloatPortion *) param[5])->Value() = SM.Time();
      por = new Mixed_ListPortion<gRational>(SM.GetSolutions());
    }
    break;
  default:
    assert( 0 );
  }

  assert( por != 0 );
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}



//----------------------- EnumPureSolve --------------------------//

#include "purenash.h"

Portion *GSM_PureNashFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();

  gList<MixedProfile<double> > solns;

  gWatch watch;

  FindPureNash(N, solns);

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  Portion* por = new Mixed_ListPortion<double>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_PureNashRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();

  gList<MixedProfile<gRational> > solns;

  gWatch watch;

  FindPureNash(N, solns);

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  Portion* por = new Mixed_ListPortion<gRational>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_PureNashSupport(Portion **param)
{
  NFSupport* S = ( (NfSupportPortion*) param[ 0 ] )->Value();
  BaseNfg* N = (BaseNfg*) &( S->BelongsTo() );
  Portion* por;

  gWatch watch;

  switch( N->Type() )
  {
  case DOUBLE:
    {
      gList<MixedProfile<double> > solns;
      FindPureNash( * (Nfg<double>*) N, solns );
      por = new Mixed_ListPortion<double>(solns);
    }
    break;
  case RATIONAL:
    {
      gList<MixedProfile<gRational> > solns;
      FindPureNash( * (Nfg<gRational>*) N, solns );
      por = new Mixed_ListPortion<gRational>(solns);
    }
    break;
  default:
    assert( 0 );
  }

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}





 
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

Portion *GSM_CompressNfg(Portion **param)
{
  Nfg<double> &N = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();
  Nfg<double> *M = new Nfg<double>(N);
  return new NfgValPortion(M);
}

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

  Nfg<double> *N = new Nfg<double>(d);

  return new NfgValPortion(N);
}

Portion *GSM_RandomNfgFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();
  
  RandomNfg(N);
  return param[0]->RefCopy();
}

Portion *GSM_RandomNfgRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational> *) ((NfgPortion *) param[0])->Value();
  
  RandomNfg(N);
  return param[0]->RefCopy();
}

Portion *GSM_RandomNfgSeedFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();
  int seed = ((IntPortion *) param[1])->Value();

  SetSeed(seed);
  RandomNfg(N);
  return param[0]->RefCopy();
}

Portion *GSM_RandomNfgSeedRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational> *) ((NfgPortion *) param[0])->Value();
  int seed = ((IntPortion *) param[1])->Value();

  SetSeed(seed);
  RandomNfg(N);
  return param[0]->RefCopy();
}  

Portion *GSM_NewSupport(Portion **param)
{
  BaseNfg &N = * ((NfgPortion *) param[0])->Value();
  Portion *p = new NfSupportValPortion(new NFSupport(N));

  p->SetOwner( param[ 0 ]->Original() );
  p->AddDependency();
  return p;
}


//---------------------------- SaveNfg ------------------------//

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

Portion *GSM_SaveNfg_Support(Portion **param)
{
  NFSupport& S = * ( (NfSupportPortion*) param[ 0 ] )->Value();
  BaseNfg* N = (BaseNfg*) &( S.BelongsTo() );
  gString file = ((TextPortion *) param[1])->Value();
  gFileOutput f(file);

  if (!f.IsValid())
    return new ErrorPortion("Unable to open file for output");

  N->WriteNfgFile(f);

  Portion* por = param[ 0 ]->ValCopy();
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}



//---------------------- Mixed -------------------//


Portion *GSM_Mixed_NfgFloat(Portion **param)
{
  int i;
  int j;
  Portion* p1;
  Portion* p2;

  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();
  MixedProfile<double> *P = new MixedProfile<double>(N);

  if( ( (ListPortion*) param[1] )->Length() != N.NumPlayers() )
  {
    delete P;
    return new ErrorPortion( "Mismatching number of players" );
  }
  
  for( i = 1; i <= N.NumPlayers(); i++ )
  {
    p1 = ( (ListPortion*) param[1] )->Subscript( i );
    if( p1->Type() != porLIST )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching dimensionality" );
    }
    if( ( (ListPortion*) p1 )->Length() != N.NumStrats( i ) )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching number of strategies" );
    }

    for( j = 1; j <= N.NumStrats( i ); j++ )
    {
      p2 = ( (ListPortion*) p1 )->Subscript( j );
      if( p2->Type() != porFLOAT )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      
      (*P)( i, j ) = ( (FloatPortion*) p2 )->Value();
      
      delete p2;
    }
    delete p1;
  }


  Portion* por = new MixedValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}



Portion *GSM_Mixed_NfgRational(Portion **param)
{
  int i;
  int j;
  Portion* p1;
  Portion* p2;

  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();
  MixedProfile<gRational> *P = new MixedProfile<gRational>(N);

  if( ( (ListPortion*) param[1] )->Length() != N.NumPlayers() )
  {
    delete P;
    return new ErrorPortion( "Mismatching number of players" );
  }
  
  for( i = 1; i <= N.NumPlayers(); i++ )
  {
    p1 = ( (ListPortion*) param[1] )->Subscript( i );
    if( p1->Type() != porLIST )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching dimensionality" );
    }
    if( ( (ListPortion*) p1 )->Length() != N.NumStrats( i ) )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching number of strategies" );
    }

    for( j = 1; j <= N.NumStrats( i ); j++ )
    {
      p2 = ( (ListPortion*) p1 )->Subscript( j );
      if( p2->Type() != porRATIONAL )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      
      (*P)( i, j ) = ( (RationalPortion*) p2 )->Value();
      
      delete p2;
    }
    delete p1;
  }


  Portion* por = new MixedValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}



Portion* GSM_Mixed_NFSupport( Portion** param )
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  gArray<int> dim = S->SupportDimensions();
  BaseMixedProfile *P;
  Portion* por;
  PortionType datatype;
  int i;
  int j;
  Portion* p1;
  Portion* p2;

  switch( param[ 0 ]->Owner()->Type() )
  {
  case porNFG_FLOAT:
    P = new MixedProfile<double>((Nfg<double> &) S->BelongsTo(), *S);
    datatype = porFLOAT;
    break;
  case porNFG_RATIONAL:
    P = new MixedProfile<gRational>((Nfg<gRational> &) S->BelongsTo(), *S);
    datatype = porRATIONAL;
    break;
  default:
    assert( 0 );
  }


  if( ( (ListPortion*) param[1] )->Length() != dim.Length() )
  {
    delete P;
    return new ErrorPortion( "Mismatching number of players" );
  }
  
  for( i = 1; i <= dim.Length(); i++ )
  {
    p1 = ( (ListPortion*) param[1] )->Subscript( i );
    if( p1->Type() != porLIST )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching dimensionality" );
    }
    if( ( (ListPortion*) p1 )->Length() != dim[ i ] )
    {
      delete p1;
      delete P;
      return new ErrorPortion( "Mismatching number of strategies" );
    }
    
    for( j = 1; j <= dim[ i ]; j++ )
    {
      p2 = ( (ListPortion*) p1 )->Subscript( j );
      if( p2->Type() != datatype )
      {
	delete p2;
	delete p1;
	delete P;
	return new ErrorPortion( "Mismatching dimensionality" );
      }
      
      switch( datatype )
      {
      case porFLOAT:
	( * (MixedProfile<double>*) P )( i, j ) = 
	  ( (FloatPortion*) p2 )->Value();
	break;
      case porRATIONAL:
	( * (MixedProfile<gRational>*) P )( i, j ) = 
	  ( (RationalPortion*) p2 )->Value();
	break;
      default:
	assert( 0 );
      }
      
      delete p2;
    }
    delete p1;
  }
  
  por = new MixedValPortion(P);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}


//----------------------- SetComponent ---------------------------//

Portion *GSM_SetComponent_MixedFloat(Portion **param)
{
  int i;
  int j;
  Portion* p2;
  int PlayerNum = 0;

  MixedProfile<double>* P = 
    (MixedProfile<double>*) ( (MixedPortion*) param[ 0 ] )->Value();
  Nfg<double>& N = * P->BelongsTo();
  gArray< NFPlayer* > player = N.PlayerList();
  
  for( i = 1; i <= N.NumPlayers(); i++ )
  {
    if( ( (NfPlayerPortion*) param[ 1 ] )->Value() == player[ i ] )
    {
      PlayerNum = i;
      break;
    }
  }
  
  if( !PlayerNum )
    return new ErrorPortion( "No such player found" );

  if( ( (ListPortion*) param[ 2 ] )->Length() != N.NumStrats( PlayerNum ) )
    return new ErrorPortion( "Mismatching number of strategies" );

  for( j = 1; j <= N.NumStrats( PlayerNum ); j++ )
  {
    p2 = ( (ListPortion*) param[ 2 ] )->Subscript( j );
    if( p2->Type() == porLIST )
    {
      delete p2;
      return new ErrorPortion( "Mismatching dimensionality" );
    }

    assert( p2->Type() == porFLOAT );
    (*P)( PlayerNum, j ) = ( (FloatPortion*) p2 )->Value();

    delete p2;
  }

  return param[ 0 ]->RefCopy();
}


Portion *GSM_SetComponent_MixedRational(Portion **param)
{
  int i;
  int j;
  Portion* p2;
  int PlayerNum = 0;

  MixedProfile<gRational>* P = 
    (MixedProfile<gRational>*) ( (MixedPortion*) param[ 0 ] )->Value();
  Nfg<gRational>& N = * P->BelongsTo();
  gArray< NFPlayer* > player = N.PlayerList();
  
  for( i = 1; i <= N.NumPlayers(); i++ )
  {
    if( ( (NfPlayerPortion*) param[ 1 ] )->Value() == player[ i ] )
    {
      PlayerNum = i;
      break;
    }
  }
  
  if( !PlayerNum )
    return new ErrorPortion( "No such player found" );

  if( ( (ListPortion*) param[ 2 ] )->Length() != N.NumStrats( PlayerNum ) )
    return new ErrorPortion( "Mismatching number of strategies" );

  for( j = 1; j <= N.NumStrats( PlayerNum ); j++ )
  {
    p2 = ( (ListPortion*) param[ 2 ] )->Subscript( j );
    if( p2->Type() == porLIST )
    {
      delete p2;
      return new ErrorPortion( "Mismatching dimensionality" );
    }

    assert( p2->Type() == porRATIONAL );
    (*P)( i, j ) = ( (RationalPortion*) p2 )->Value();

    delete p2;
  }

  return param[ 0 ]->RefCopy();
}








//---------------------------------------------------------------------


void Init_nfgfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  //------------------------ LPSolve -----------------------//

  FuncObj = new FuncDescObj("LPSolve");
  FuncObj->SetFuncInfo(GSM_ConstSumFloat, 3);
  FuncObj->SetParamInfo(GSM_ConstSumFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ConstSumFloat, 1, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ConstSumFloat, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_ConstSumRational, 3);
  FuncObj->SetParamInfo(GSM_ConstSumRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ConstSumRational, 1, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ConstSumRational, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_ConstSumSupport, 3);
  FuncObj->SetParamInfo(GSM_ConstSumSupport, 0, "support", porNF_SUPPORT );
  FuncObj->SetParamInfo(GSM_ConstSumSupport, 1, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ConstSumSupport, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("ElimDom");
  FuncObj->SetFuncInfo(GSM_ElimDom, 3);
  FuncObj->SetParamInfo(GSM_ElimDom, 0, "support", porNF_SUPPORT);
  FuncObj->SetParamInfo(GSM_ElimDom, 1, "strong", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_ElimDom, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);


  //--------------------- EnumMixedSolve ----------------------//

  FuncObj = new FuncDescObj("EnumMixedSolve");
  FuncObj->SetFuncInfo(GSM_EnumNfgFloat, 4);
  FuncObj->SetParamInfo(GSM_EnumNfgFloat, 0, "nfg", 
			porNFG_FLOAT, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_EnumNfgFloat, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumNfgFloat, 2, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumNfgFloat, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_EnumNfgRational, 4);
  FuncObj->SetParamInfo(GSM_EnumNfgRational, 0, "nfg", 
			porNFG_RATIONAL, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_EnumNfgRational, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumNfgRational, 2, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumNfgRational, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_EnumSupport, 4);
  FuncObj->SetParamInfo(GSM_EnumSupport, 0, "support", porNF_SUPPORT);
  FuncObj->SetParamInfo(GSM_EnumSupport, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumSupport, 2, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumSupport, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);


  
  //----------------------- GobitGridSolve ----------------------//
  
  FuncObj = new FuncDescObj("GobitGridSolve");
  FuncObj->SetFuncInfo(GSM_GridSolveFloat, 10);
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 1, "pxifile", porTEXT,
			new TextValPortion("") );
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 2, "minLam", porFLOAT,
			new FloatRefPortion(Gobit_default_minLam));
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 3, "maxLam", porFLOAT,
			new FloatRefPortion(Gobit_default_maxLam));
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 4, "delLam", porFLOAT,
			new FloatRefPortion(Gobit_default_delLam));
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 5, "powLam", porINTEGER,
		        new IntRefPortion( (long&) Gobit_default_powLam));
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 6, "delp", porFLOAT,
			new FloatValPortion(.01));
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 7, "tol", porFLOAT,
			new FloatValPortion(.01));
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 8, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 9, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_GridSolveSupport, 10);
  FuncObj->SetParamInfo(GSM_GridSolveSupport, 0, "support", porNF_SUPPORT );
  FuncObj->SetParamInfo(GSM_GridSolveSupport, 1, "pxifile", porTEXT,
			new TextValPortion("") );
  FuncObj->SetParamInfo(GSM_GridSolveSupport, 2, "minLam", porFLOAT,
			new FloatRefPortion(Gobit_default_minLam));
  FuncObj->SetParamInfo(GSM_GridSolveSupport, 3, "maxLam", porFLOAT,
			new FloatRefPortion(Gobit_default_maxLam));
  FuncObj->SetParamInfo(GSM_GridSolveSupport, 4, "delLam", porFLOAT,
			new FloatRefPortion(Gobit_default_delLam));
  FuncObj->SetParamInfo(GSM_GridSolveSupport, 5, "powLam", porINTEGER,
		        new IntRefPortion( (long&) Gobit_default_powLam));
  FuncObj->SetParamInfo(GSM_GridSolveSupport, 6, "delp", porFLOAT,
			new FloatValPortion(.01));
  FuncObj->SetParamInfo(GSM_GridSolveSupport, 7, "tol", porFLOAT,
			new FloatValPortion(.01));
  FuncObj->SetParamInfo(GSM_GridSolveSupport, 8, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GridSolveSupport, 9, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  /*
  FuncObj->SetFuncInfo(GSM_GridSolveRational, 10);
  FuncObj->SetParamInfo(GSM_GridSolveRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 1, "pxifile", porTEXT,
			new TextValPortion("") );
  FuncObj->SetParamInfo(GSM_GobitSolveFloat, 2, "minLam", porFLOAT,
			new FloatRefPortion(Gobit_default_minLam));
  FuncObj->SetParamInfo(GSM_GobitSolveFloat, 3, "maxLam", porFLOAT,
			new FloatRefPortion(Gobit_default_maxLam));
  FuncObj->SetParamInfo(GSM_GobitSolveFloat, 4, "delLam", porFLOAT,
			new FloatRefPortion(Gobit_default_delLam));
  FuncObj->SetParamInfo(GSM_GobitSolveFloat, 5, "powLam", porINTEGER,
		        new IntRefPortion( (long&) Gobit_default_powLam));
  FuncObj->SetParamInfo(GSM_GridSolveRational, 6, "delp", porRATIONAL,
			new FloatValPortion(.01));
  FuncObj->SetParamInfo(GSM_GridSolveRational, 7, "tol", porRATIONAL,
			new FloatValPortion(.01));
  FuncObj->SetParamInfo(GSM_GridSolveRational, 8, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GridSolveRational, 9, "time", porRATIONAL,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  */
  gsm->AddFunction(FuncObj);



  //-------------------------- EnumPureSolve ------------------------//

  FuncObj = new FuncDescObj("EnumPureSolve");
  FuncObj->SetFuncInfo(GSM_PureNashFloat, 3);
  FuncObj->SetParamInfo(GSM_PureNashFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_PureNashFloat, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_PureNashFloat, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_PureNashRational, 3);
  FuncObj->SetParamInfo(GSM_PureNashRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_PureNashRational, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_PureNashRational, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_PureNashSupport, 3);
  FuncObj->SetParamInfo(GSM_PureNashSupport, 0, "support", porNF_SUPPORT );
  FuncObj->SetParamInfo(GSM_PureNashSupport, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_PureNashSupport, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);



  //------------------------ SimpDivSolve ----------------------//

  FuncObj = new FuncDescObj("SimpDivSolve");
  FuncObj->SetFuncInfo(GSM_SimpdivFloat, 6);
  FuncObj->SetParamInfo(GSM_SimpdivFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SimpdivFloat, 1, "stopAfter", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_SimpdivFloat, 2, "nRestarts", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_SimpdivFloat, 3, "leashLength", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_SimpdivFloat, 4, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SimpdivFloat, 5, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_SimpdivRational, 6);
  FuncObj->SetParamInfo(GSM_SimpdivRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SimpdivRational, 1, "stopAfter", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_SimpdivRational, 2, "nRestarts", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_SimpdivRational, 3, "leashLength", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_SimpdivRational, 4, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SimpdivRational, 5, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_SimpdivSupport, 6);
  FuncObj->SetParamInfo(GSM_SimpdivSupport, 0, "support", porNF_SUPPORT );
  FuncObj->SetParamInfo(GSM_SimpdivSupport, 1, "stopAfter", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_SimpdivSupport, 2, "nRestarts", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_SimpdivSupport, 3, "leashLength", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_SimpdivSupport, 4, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SimpdivSupport, 5, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  gsm->AddFunction(FuncObj);




  //----------------------------------------------------------//

  FuncObj = new FuncDescObj("CompressNfg");
  FuncObj->SetFuncInfo(GSM_CompressNfg, 1);
  FuncObj->SetParamInfo(GSM_CompressNfg, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NewNfg");
  FuncObj->SetFuncInfo(GSM_NewNfg, 3);
  FuncObj->SetParamInfo(GSM_NewNfg, 0, "dim", porLIST | porINTEGER);
  FuncObj->SetParamInfo(GSM_NewNfg, 1, "random", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_NewNfg, 2, "seed", porINTEGER,
			new IntValPortion(0));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RandomNfg");
  FuncObj->SetFuncInfo(GSM_RandomNfgFloat, 1);
  FuncObj->SetParamInfo(GSM_RandomNfgFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_RandomNfgRational, 1);
  FuncObj->SetParamInfo(GSM_RandomNfgRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_RandomNfgSeedFloat, 2);
  FuncObj->SetParamInfo(GSM_RandomNfgSeedFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_RandomNfgSeedFloat, 1, "seed", porINTEGER);

  FuncObj->SetFuncInfo(GSM_RandomNfgSeedRational, 2);
  FuncObj->SetParamInfo(GSM_RandomNfgSeedRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_RandomNfgSeedRational, 1, "seed", porINTEGER);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NewSupport");
  FuncObj->SetFuncInfo(GSM_NewSupport, 1);
  FuncObj->SetParamInfo(GSM_NewSupport, 0, "nfg", porNFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("AddStrategy");
  FuncObj->SetFuncInfo(GSM_AddStrategy, 2);
  FuncObj->SetParamInfo(GSM_AddStrategy, 0, "support", porNF_SUPPORT);
  FuncObj->SetParamInfo(GSM_AddStrategy, 1, "strategy", porSTRATEGY);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RemoveStrategy");
  FuncObj->SetFuncInfo(GSM_RemoveStrategy, 2);
  FuncObj->SetParamInfo(GSM_RemoveStrategy, 0, "support", porNF_SUPPORT);
  FuncObj->SetParamInfo(GSM_RemoveStrategy, 1, "strategy", porSTRATEGY);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NumStrats");
  FuncObj->SetFuncInfo(GSM_NumStrats, 2);
  FuncObj->SetParamInfo(GSM_NumStrats, 0, "player", porPLAYER_NFG);
  FuncObj->SetParamInfo(GSM_NumStrats, 1, "support", porNF_SUPPORT, 
			new NfSupportValPortion( 0 ) );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Strategies");
  FuncObj->SetFuncInfo(GSM_Strategies, 2);
  FuncObj->SetParamInfo(GSM_Strategies, 0, "player", porPLAYER_NFG);
  FuncObj->SetParamInfo(GSM_Strategies, 1, "support", porNF_SUPPORT, 
			new NfSupportValPortion( 0 ) );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LoadNfg");
  FuncObj->SetFuncInfo(GSM_LoadNfg, 1);
  FuncObj->SetParamInfo(GSM_LoadNfg, 0, "file", porTEXT);
  gsm->AddFunction(FuncObj);


  //------------------------ SaveNfg ----------------------------//

  FuncObj = new FuncDescObj("SaveNfg");

  FuncObj->SetFuncInfo(GSM_SaveNfg, 2);
  FuncObj->SetParamInfo(GSM_SaveNfg, 0, "nfg", porNFG, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SaveNfg, 1, "file", porTEXT);

  FuncObj->SetFuncInfo(GSM_SaveNfg_Support, 2);
  FuncObj->SetParamInfo(GSM_SaveNfg_Support, 0, "support", porNF_SUPPORT );
  FuncObj->SetParamInfo(GSM_SaveNfg_Support, 1, "file", porTEXT);

  gsm->AddFunction(FuncObj);


  //----------------------- Mixed --------------------------//
  FuncObj = new FuncDescObj( "Mixed" );
  FuncObj->SetFuncInfo( GSM_Mixed_NfgFloat, 2 );
  FuncObj->SetParamInfo(GSM_Mixed_NfgFloat, 0, "nfg", porNFG_FLOAT, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Mixed_NfgFloat, 1, "list", porLIST | porFLOAT);

  FuncObj->SetFuncInfo( GSM_Mixed_NfgRational, 2 );
  FuncObj->SetParamInfo(GSM_Mixed_NfgRational, 0, "nfg", porNFG_RATIONAL, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Mixed_NfgRational, 
			1, "list", porLIST | porRATIONAL);

  FuncObj->SetFuncInfo( GSM_Mixed_NFSupport, 2 );
  FuncObj->SetParamInfo(GSM_Mixed_NFSupport, 0, "support", porNF_SUPPORT );
  FuncObj->SetParamInfo(GSM_Mixed_NFSupport, 
			1, "list", porLIST | porFLOAT | porRATIONAL );
  gsm->AddFunction( FuncObj );

  
  //--------------------- SetComponet -------------------------//
  FuncObj = new FuncDescObj( "SetComponent" );

  FuncObj->SetFuncInfo( GSM_SetComponent_MixedFloat, 3 );
  FuncObj->SetParamInfo( GSM_SetComponent_MixedFloat, 
			0, "mixed", porMIXED_FLOAT, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo( GSM_SetComponent_MixedFloat, 
			1, "player", porPLAYER_NFG );
  FuncObj->SetParamInfo( GSM_SetComponent_MixedFloat, 
			2, "list", porLIST | porFLOAT );

  FuncObj->SetFuncInfo( GSM_SetComponent_MixedRational, 3 );
  FuncObj->SetParamInfo( GSM_SetComponent_MixedRational, 
			0, "mixed", porMIXED_RATIONAL, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo( GSM_SetComponent_MixedRational, 
			1, "player", porPLAYER_NFG );
  FuncObj->SetParamInfo( GSM_SetComponent_MixedRational, 
			2, "list", porLIST | porRATIONAL );

  gsm->AddFunction( FuncObj );
}




#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class Mixed_ListPortion<double>;
TEMPLATE class Mixed_ListPortion<gRational>;






