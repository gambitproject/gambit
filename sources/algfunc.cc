//
// FILE: algfunc.cc -- Solution algorithm functions for GCL
//
// $Id$
//

#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "rational.h"

#include "gwatch.h"
#include "mixedsol.h"
#include "behavsol.h"
#include "nfg.h"
#include "efg.h"


extern Portion *ArrayToList(const gArray<double> &A);
extern Portion *ArrayToList(const gArray<gRational> &A);
extern gVector<double>* ListToVector_Float(ListPortion* list);
extern gVector<gRational>* ListToVector_Rational(ListPortion* list);
extern gMatrix<double>* ListToMatrix_Float(ListPortion* list);
extern gMatrix<gRational>* ListToMatrix_Rational(ListPortion* list);


//
// Useful utilities for creation of lists of profiles
//
template <class T> class Mixed_ListPortion : public ListValPortion   {
  public:
    Mixed_ListPortion(const gList<MixedSolution<T> > &);
    virtual ~Mixed_ListPortion()   { }
};

Mixed_ListPortion<double>::Mixed_ListPortion(const gList<MixedSolution<double> > &list)
{
  _DataType = porMIXED_FLOAT;
  for (int i = 1; i <= list.Length(); i++)
    Append(new MixedValPortion( new MixedSolution<double>(list[i])));
}

Mixed_ListPortion<gRational>::Mixed_ListPortion(const gList<MixedSolution<gRational> > &list)
{
  _DataType = porMIXED_RATIONAL;
  for (int i = 1; i <= list.Length(); i++)
    Append(new MixedValPortion( new MixedSolution<gRational>(list[i])));
}


template <class T> class Behav_ListPortion : public ListValPortion   {
  public:
    Behav_ListPortion(const gList<BehavSolution<T> > &);
    virtual ~Behav_ListPortion()   { }
};

Behav_ListPortion<double>::Behav_ListPortion(
			   const gList<BehavSolution<double> > &list)
{
  _DataType = porBEHAV_FLOAT;
  for (int i = 1; i <= list.Length(); i++)
    Append( new BehavValPortion( new BehavSolution<double>(list[i])));
}

Behav_ListPortion<gRational>::Behav_ListPortion(
			      const gList<BehavSolution<gRational> > &list)
{
  _DataType = porBEHAV_RATIONAL;
  for (int i = 1; i <= list.Length(); i++)
    Append( new BehavValPortion( new BehavSolution<gRational>(list[i])));
}


//-------
// Afg
//-------

Portion *GSM_Afg_Float(Portion **param)
{
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();
  gWatch watch;

  Nfg<double> *N = MakeAfg(E);
  
  ((FloatPortion *) param[1])->Value() = watch.Elapsed();
  
  if (N)
    return new NfgValPortion(N);
  else
    return new ErrorPortion("Conversion to agent form failed");
}

Portion *GSM_Afg_Rational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();
  gWatch watch;

  Nfg<gRational> *N = MakeAfg(E);
  
  ((FloatPortion *) param[1])->Value() = watch.Elapsed();

  if (N)
    return new NfgValPortion(N);
  else
    return new ErrorPortion("Conversion to agent form failed");
}

//------------
// Behav
//------------

Portion *GSM_Behav_Float(Portion **param)
{
  MixedSolution<double> &mp = * (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[1])->Value();
  Nfg<double> &N = *mp.BelongsTo(); 

  BehavSolution<double>* bp;

  if( AssociatedNfg( &E ) != &N )  
    return new ErrorPortion("Normal and extensive form games not associated");
  else
  {
    bp = new BehavSolution<double>(E);
    MixedToBehav(N, mp, E, *bp);
  }

  Portion* por = new BehavValPortion(bp);
  por->SetOwner( param[ 1 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_Behav_Rational(Portion **param)
{
  MixedSolution<gRational> &mp = 
    * (MixedSolution<gRational>*) ((MixedPortion*) param[0])->Value();
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[1])->Value();
  Nfg<gRational> &N = *mp.BelongsTo(); 

  BehavSolution<gRational>* bp;

  if( AssociatedNfg( &E ) != &N )  
    return new ErrorPortion("Normal and extensive form games not associated");
  else
  {
    bp = new BehavSolution<gRational>(E);
    MixedToBehav(N, mp, E, *bp);
  }

  Portion* por = new BehavValPortion(bp);
  por->SetOwner( param[ 1 ]->Original() );
  por->AddDependency();
  return por;
}

//------------------
// EnumMixedSolve
//------------------

#include "enum.h"

Portion *GSM_EnumMixed_NfgFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();

  EnumParams EP;

  EP.stopAfter = ((IntPortion *) param[1])->Value();
  
  EP.tracefile = &((OutputPortion *) param[4])->Value();
  EP.trace = ((IntPortion *) param[5])->Value();

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

Portion *GSM_EnumMixed_NfgRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();

  EnumParams EP;

  EP.stopAfter = ((IntPortion *) param[1])->Value();

  EP.tracefile = &((OutputPortion *) param[4])->Value();
  EP.trace = ((IntPortion *) param[5])->Value();

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


Portion *GSM_EnumMixed_Support(Portion **param)
{
  NFSupport* S = ( (NfSupportPortion*) param[ 0 ] )->Value();
  BaseNfg* N = (BaseNfg*) &( S->BelongsTo() );
  Portion* por = 0;

  EnumParams EP;

  EP.stopAfter = ((IntPortion *) param[1])->Value();

  EP.tracefile = &((OutputPortion *) param[4])->Value();
  EP.trace = ((IntPortion *) param[5])->Value();

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

#include "enumsub.h"


Portion *GSM_EnumMixed_EfgFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();

  if (!((BoolPortion *) param[1])->Value())
    return new ErrorPortion("algorithm not implemented for extensive forms");

  EnumParams EP;
  EP.stopAfter = ((IntPortion *) param[2])->Value();

  EP.tracefile = &((OutputPortion *) param[5])->Value();
  EP.trace = ((IntPortion *) param[6])->Value();
  
  EnumBySubgame<double> EM(E, EP);
  
  EM.Solve();
  gList<BehavSolution<double> > solns(EM.GetSolutions());

  ((IntPortion *) param[3])->Value() = EM.NumPivots();
  ((FloatPortion *) param[4])->Value() = EM.Time();

  Portion* por = new Behav_ListPortion<double>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_EnumMixed_EfgRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();

  if (!((BoolPortion *) param[1])->Value())
    return new ErrorPortion("algorithm not implemented for extensive forms");

  EnumParams EP;
  EP.stopAfter = ((IntPortion *) param[2])->Value();
  
  EP.tracefile = &((OutputPortion *) param[5])->Value();
  EP.trace = ((IntPortion *) param[6])->Value();

  EnumBySubgame<gRational> EM(E, EP);
  
  EM.Solve();

  gList<BehavSolution<gRational> > solns(EM.GetSolutions());

  ((IntPortion *) param[3])->Value() = EM.NumPivots();
  ((FloatPortion *) param[4])->Value() = EM.Time();

  Portion* por = new Behav_ListPortion<gRational>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

//-----------------
// EnumPureSolve
//-----------------

#include "nfgpure.h"

Portion *GSM_EnumPure_NfgFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();

  gList<MixedSolution<double> > solns;

  gWatch watch;

  FindPureNash(N, solns);

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  Portion* por = new Mixed_ListPortion<double>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_EnumPure_NfgRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();

  gList<MixedSolution<gRational> > solns;

  gWatch watch;

  FindPureNash(N, solns);

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  Portion* por = new Mixed_ListPortion<gRational>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_EnumPure_Support(Portion **param)
{
  NFSupport* S = ( (NfSupportPortion*) param[ 0 ] )->Value();
  BaseNfg* N = (BaseNfg*) &( S->BelongsTo() );
  Portion* por;

  gWatch watch;

  switch( N->Type() )
  {
  case DOUBLE:
    {
      gList<MixedSolution<double> > solns;
      FindPureNash( * (Nfg<double>*) N, solns );
      por = new Mixed_ListPortion<double>(solns);
    }
    break;
  case RATIONAL:
    {
      gList<MixedSolution<gRational> > solns;
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

#include "efgpure.h"
#include "psnesub.h"

Portion *GSM_EnumPure_EfgFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();
  gList<BehavSolution<double> > solns;

  if (((BoolPortion *) param[1])->Value())   {
    PureNashBySubgame<double> M(E);
    M.Solve();
    solns = M.GetSolutions();
    ((FloatPortion *) param[3])->Value() = M.Time();
  }
  else  {
    EfgPSNEBySubgame<double> M(E);
    M.Solve();
    solns = M.GetSolutions();
    ((FloatPortion *) param[3])->Value() = M.Time();
  }

  Portion* por = new Behav_ListPortion<double>(solns);
  por->SetOwner(param[0]->Original());
  por->AddDependency();
  return por;
}

Portion *GSM_EnumPure_EfgRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();
  gList<BehavSolution<gRational> > solns;
  
  if (((BoolPortion *) param[1])->Value())  {
    PureNashBySubgame<gRational> M(E);
    M.Solve();
    solns = M.GetSolutions();
    ((FloatPortion *) param[3])->Value() = M.Time();
  }
  else  {
    EfgPSNEBySubgame<gRational> M(E);
    M.Solve();
    solns = M.GetSolutions();
    ((FloatPortion *) param[3])->Value() = M.Time();
  } 
 
  Portion* por = new Behav_ListPortion<gRational>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

//------------------
// GobitGridSolve
//------------------

#include "grid.h"

Portion *GSM_GobitGrid_NfgFloat(Portion **param)
{
  Nfg<double> &N = *(Nfg<double> *) ((NfgPortion *) param[0])->Value();

  GridParams<double> GP;
  if (((TextPortion *) param[1])->Value() != "")
    GP.pxifile = new gFileOutput(((TextPortion*) param[1])->Value());
  else
    GP.pxifile = &gnull;

  GP.minLam = ((FloatPortion *) param[2])->Value();
  GP.maxLam = ((FloatPortion *) param[3])->Value();
  GP.delLam = ((FloatPortion *) param[4])->Value();
  GP.powLam = ((IntPortion *) param[5])->Value();
  GP.delp = ((FloatPortion *) param[6])->Value();
  GP.tol = ((FloatPortion *) param[7])->Value();

  GP.tracefile = &((OutputPortion *) param[10])->Value();
  GP.trace = ((IntPortion *) param[11])->Value();

  NFSupport S(N);

  GridSolveModule<double> GM(N, GP, S);
  GM.GridSolve();

//  ((IntPortion *) param[8])->Value() = GM.NumEvals();
//  ((FloatPortion *) param[9])->Value() = GM.Time();

  gList<MixedSolution<double> > solns;

  if (GP.pxifile != &gnull)   delete GP.pxifile;

  Portion *por = new Mixed_ListPortion<double>(solns);
  por->SetOwner(param[0]->Original());
  por->AddDependency();
  return por;
}

Portion *GSM_GobitGrid_Support(Portion **param)
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
      gList<MixedSolution<double> > solns;
      por = new Mixed_ListPortion<double>(solns);
    }
    break;
  case RATIONAL:
    return new ErrorPortion( "The rational version of GobitGridSolve is not implemented" );
    break;
  default:
    assert(0);
  }

  assert( por != 0 );
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

//---------------
// GobitSolve
//---------------

#include "ngobit.h"
#include "egobit.h"

Portion *GSM_Gobit(Portion **param)
{
  if (param[0]->Spec().Type == porNFG_FLOAT)  {
    Nfg<double> &N = *(Nfg<double> *) ((NfgPortion *) param[0])->Value();
    MixedSolution<double> start(N);
  
    NFGobitParams NP;
    if( ((TextPortion*) param[1])->Value() != "" )
      NP.pxifile = new gFileOutput( ((TextPortion*) param[1])->Value() );
    else
      NP.pxifile = &gnull;
    NP.minLam = ((FloatPortion *) param[2])->Value();
    NP.maxLam = ((FloatPortion *) param[3])->Value();
    NP.delLam = ((FloatPortion *) param[4])->Value();
    NP.powLam = ((IntPortion *) param[5])->Value();
    NP.fullGraph = ((BoolPortion *) param[6])->Value();

    NP.maxitsN = ((IntPortion *) param[7])->Value();
    NP.tolN = ((FloatPortion *) param[8])->Value();
    NP.maxits1 = ((IntPortion *) param[9])->Value();
    NP.tol1 = ((FloatPortion *) param[10] )->Value();

    gWatch watch;
    gList<MixedSolution<double> > solutions;
    Gobit(N, NP, start, solutions,
	  ((IntPortion *) param[12])->Value(),
	  ((IntPortion *) param[13])->Value());

    ((FloatPortion *) param[11])->Value() = watch.Elapsed();

    if (NP.pxifile != &gnull)  delete NP.pxifile;

    Portion *por = new Mixed_ListPortion<double>(solutions);
    por->SetOwner(param[0]->Original());
    por->AddDependency();
    return por;
  }
  else   {  // extensive form
    Efg<double> &E = *(Efg<double> *) ((EfgPortion *) param[0])->Value();
    BehavSolution<double> start(E);
  
    EFGobitParams EP;
    if( ((TextPortion*) param[1])->Value() != "" )
      EP.pxifile = new gFileOutput( ((TextPortion*) param[1])->Value() );
    else
      EP.pxifile = &gnull;
    EP.minLam = ((FloatPortion *) param[2])->Value();
    EP.maxLam = ((FloatPortion *) param[3])->Value();
    EP.delLam = ((FloatPortion *) param[4])->Value();
    EP.powLam = ((IntPortion *) param[5])->Value();
    EP.fullGraph = ((BoolPortion *) param[6])->Value();
    
    EP.maxitsN = ((IntPortion *) param[7])->Value();
    EP.tolN = ((FloatPortion *) param[8])->Value();
    EP.maxits1 = ((IntPortion *) param[9])->Value();
    EP.tol1 = ((FloatPortion *) param[10])->Value();

    EP.tracefile = &((OutputPortion *) param[14])->Value();
    EP.trace = ((IntPortion *) param[15])->Value();

    gWatch watch;
  
    gList<BehavSolution<double> > solutions;
    Gobit(E, EP, start, solutions,
	  ((IntPortion *) param[12])->Value(),
	  ((IntPortion *) param[13])->Value());

    ((FloatPortion *) param[11])->Value() = watch.Elapsed();
    
    if (EP.pxifile != &gnull)  delete EP.pxifile;

    Portion *por = new Behav_ListPortion<double>(solutions);
    por->SetOwner(param[0]->Original());
    por->AddDependency();
    return por;
  }
}

Portion *GSM_Gobit_Start(Portion **param)
{
  if (param[0]->Spec().Type == porMIXED_FLOAT)  {
    MixedSolution<double> &start = 
      * (MixedSolution<double> *) ((MixedPortion *) param[0])->Value();
    Nfg<double> &N = *start.BelongsTo();
  
    NFGobitParams NP;
    if (((TextPortion *) param[1])->Value() != "")
      NP.pxifile = new gFileOutput(((TextPortion *) param[1])->Value());
    else
      NP.pxifile = &gnull;
    NP.minLam = ((FloatPortion *) param[2])->Value();
    NP.maxLam = ((FloatPortion *) param[3])->Value();
    NP.delLam = ((FloatPortion *) param[4])->Value();
    NP.powLam = ((IntPortion *) param[5])->Value();
    NP.fullGraph = ((BoolPortion *) param[6])->Value();

    NP.maxitsN = ((IntPortion *) param[7])->Value();
    NP.tolN = ((FloatPortion *) param[8])->Value();
    NP.maxits1 = ((IntPortion *) param[9])->Value();
    NP.tol1 = ((FloatPortion *) param[10])->Value();

    gWatch watch;
    gList<MixedSolution<double> > solutions;
    Gobit(N, NP, start, solutions, 
	  ((IntPortion *) param[12])->Value(),
	  ((IntPortion *) param[13])->Value());

    ((FloatPortion *) param[11])->Value() = watch.Elapsed();

    Portion *por = new Mixed_ListPortion<double>(solutions);
    por->SetOwner(param[0]->Owner());
    por->AddDependency();
    return por;
  }
  else  {     // BEHAV_FLOAT  
    BehavSolution<double>& start = 
      * (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
    Efg<double> &E = *( start.BelongsTo() );
  
    EFGobitParams EP;
    if( ((TextPortion*) param[1])->Value() != "" )
      EP.pxifile = new gFileOutput( ((TextPortion*) param[1])->Value() );
    else
      EP.pxifile = &gnull;
    EP.minLam = ((FloatPortion *) param[2])->Value();
    EP.maxLam = ((FloatPortion *) param[3])->Value();
    EP.delLam = ((FloatPortion *) param[4])->Value();
    EP.powLam = ((IntPortion *) param[5])->Value();
    EP.fullGraph = ((BoolPortion *) param[6])->Value();
    
    EP.maxitsN = ((IntPortion *) param[7])->Value();
    EP.tolN = ((FloatPortion *) param[8])->Value();
    EP.maxits1 = ((IntPortion *) param[9])->Value();
    EP.tol1 = ((FloatPortion *) param[10])->Value();
  
    EP.tracefile = &((OutputPortion *) param[14])->Value();
    EP.trace = ((IntPortion *) param[15])->Value();
    
    gWatch watch;
    
    gList<BehavSolution<double> > solutions;
    Gobit(E, EP, start, solutions,
	  ((IntPortion *) param[12])->Value(),
	  ((IntPortion *) param[13])->Value());
    
    ((FloatPortion *) param[11])->Value() = watch.Elapsed();
    
    Portion * por = new Behav_ListPortion<double>(solutions);
    por->SetOwner(param[0]->Owner());
    por->AddDependency();
    return por;
  }
}


//------------
// LcpSolve
//------------

#include "lemke.h"

Portion *GSM_Lcp_NfgFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();

  LemkeParams LP;
  LP.stopAfter = ((IntPortion *) param[1])->Value();
  
  LP.tracefile = &((OutputPortion *) param[4])->Value();
  LP.trace = ((IntPortion *) param[5])->Value();

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

Portion *GSM_Lcp_NfgRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();

  LemkeParams LP;
  LP.stopAfter = ((IntPortion *) param[1])->Value();

  LP.tracefile = &((OutputPortion *) param[4])->Value();
  LP.trace = ((IntPortion *) param[5])->Value();

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

Portion *GSM_Lcp_Support(Portion **param)
{
  NFSupport& S = * ( (NfSupportPortion*) param[ 0 ] )->Value();
  BaseNfg* N = (BaseNfg*) &( S.BelongsTo() );
  Portion* por = 0;

  LemkeParams LP;
  LP.stopAfter = ((IntPortion *) param[1])->Value();

  LP.tracefile = &((OutputPortion *) param[4])->Value();
  LP.trace = ((IntPortion *) param[5])->Value();

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


#include "lemketab.h"

Portion* GSM_Lcp_ListFloat(Portion** param)
{
  gMatrix<double>* a = ListToMatrix_Float((ListPortion*) param[0]);
  gVector<double>* b = ListToVector_Float((ListPortion*) param[1]);
  
  LTableau<double>* tab = new LTableau<double>(*a, *b);
  tab->LemkePath(0);
  gVector<double> vector;
  tab->BasisVector(vector);
  Portion* result = ArrayToList(vector);
  delete tab;
  delete a;
  delete b;
  
  return result;
}

Portion* GSM_Lcp_ListRational(Portion** param)
{
  gMatrix<gRational>* a = ListToMatrix_Rational((ListPortion*) param[0]);
  gVector<gRational>* b = ListToVector_Rational((ListPortion*) param[1]);
  
  LTableau<gRational>* tab = new LTableau<gRational>(*a, *b);
  tab->LemkePath(0);
  gVector<gRational> vector;
  tab->BasisVector(vector);
  Portion* result = ArrayToList(vector);
  delete tab;
  delete a;
  delete b;
  
  return result;
}



#include "seqform.h"
#include "lemkesub.h"

Portion *GSM_Lcp_EfgFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();

  gList<BehavSolution<double> > solns;
  
  if (((BoolPortion *) param[1])->Value())  {
    LemkeParams LP;
    LP.stopAfter = ((IntPortion *) param[2])->Value();

    LP.tracefile = &((OutputPortion *) param[5])->Value();
    LP.trace = ((IntPortion *) param[6])->Value();

    LemkeBySubgame<double> LM(E, LP);

    LM.Solve();

    solns = LM.GetSolutions();

    ((IntPortion *) param[3])->Value() = LM.NumPivots();
    ((FloatPortion *) param[4])->Value() = LM.Time();
  }
  else   {
    SeqFormParams SP;
    SP.stopAfter = ((IntPortion *) param[2])->Value();

    SP.tracefile = &((OutputPortion *) param[5])->Value();
    SP.trace = ((IntPortion *) param[6])->Value();

    SeqFormBySubgame<double> SM(E, SP);
    
    SM.Solve();

    solns = SM.GetSolutions();

    ((IntPortion *) param[3])->Value() = SM.NumPivots();
    ((FloatPortion *) param[4])->Value() = SM.Time();
  }

  Portion *por = new Behav_ListPortion<double>(solns);
  por->SetOwner(param[0]->Original());
  por->AddDependency();

  return por;
}

Portion *GSM_Lcp_EfgRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();

  gList<BehavSolution<gRational> > solns;
  
  if (((BoolPortion *) param[1])->Value())  {
    LemkeParams LP;
    LP.stopAfter = ((IntPortion *) param[2])->Value();

    LP.tracefile = &((OutputPortion *) param[5])->Value();
    LP.trace = ((IntPortion *) param[6])->Value();

    LemkeBySubgame<gRational> LM(E, LP);
    
    LM.Solve();

    solns = LM.GetSolutions();

    ((IntPortion *) param[3])->Value() = LM.NumPivots();
    ((FloatPortion *) param[4])->Value() = LM.Time();
  }
  else   {
    SeqFormParams SP;
    SP.stopAfter = ((IntPortion *) param[2])->Value();

    SP.tracefile = &((OutputPortion *) param[5])->Value();
    SP.trace = ((IntPortion *) param[6])->Value();

    SeqFormBySubgame<gRational> SM(E, SP);
    
    SM.Solve();

    solns = SM.GetSolutions();

    ((IntPortion *) param[3])->Value() = SM.NumPivots();
    ((FloatPortion *) param[4])->Value() = SM.Time();
  }

  Portion *por = new Behav_ListPortion<gRational>(solns);
  por->SetOwner(param[0]->Original());
  por->AddDependency();

  return por;
}

//-------------
// LiapSolve
//-------------

#include "liapsub.h"
#include "eliap.h"

Portion *GSM_Liap_EfgFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();

  gList<BehavSolution<double> > solns;

  BehavSolution<double> start(E);

  if (((BoolPortion *) param[1])->Value())   {
    NFLiapParams params;

    params.maxitsN = ((IntPortion *) param[4])->Value();
    params.tolN = ((FloatPortion *) param[5])->Value();
    params.maxits1 = ((IntPortion *) param[6])->Value();
    params.tol1 = ((FloatPortion *) param[7])->Value();

    params.stopAfter = ((IntPortion *) param[2])->Value();
    params.nTries = ((IntPortion *) param[3])->Value();

    params.tracefile = &((OutputPortion *) param[10])->Value();
    params.trace = ((IntPortion *) param[11])->Value();

    NFLiapBySubgame LM(E, params, start);
    LM.Solve();

    solns = LM.GetSolutions();

    ((FloatPortion *) param[8])->Value() = LM.Time();
    ((IntPortion *) param[9])->Value() = LM.NumEvals();
  }
  else  {
    EFLiapParams params;

    params.maxitsN = ((IntPortion *) param[4])->Value();
    params.tolN = ((FloatPortion *) param[5])->Value();
    params.maxits1 = ((IntPortion *) param[6])->Value();
    params.tol1 = ((FloatPortion *) param[7])->Value();

    params.stopAfter = ((IntPortion *) param[2])->Value();
    params.nTries = ((IntPortion *) param[3])->Value();

    params.tracefile = &((OutputPortion *) param[10])->Value();
    params.trace = ((IntPortion *) param[11])->Value();

    EFLiapBySubgame LM(E, params, start);

    LM.Solve();

    solns = LM.GetSolutions();

    ((FloatPortion *) param[8])->Value() = LM.Time();
    ((IntPortion *) param[9])->Value() = LM.NumEvals();
  }

  Portion *por = new Behav_ListPortion<double>(solns);
  por->SetOwner(param[0]->Original());
  por->AddDependency();
  return por;
}

Portion *GSM_Liap_BehavFloat(Portion **param)
{
  BehavSolution<double> &start = 
    * (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  Efg<double> &E = *( start.BelongsTo() );
  
  if (((BoolPortion *) param[1])->Value())   {
    return new ErrorPortion("This function not implemented yet");
  }
  else  {
    EFLiapParams LP;

    LP.stopAfter = ((IntPortion *) param[2])->Value();
    LP.nTries = ((IntPortion *) param[3])->Value();

    LP.maxitsN = ((IntPortion *) param[4])->Value();
    LP.tolN = ((FloatPortion *) param[5])->Value();
    LP.maxits1 = ((IntPortion *) param[6])->Value();
    LP.tol1 = ((FloatPortion *) param[7])->Value();
    
    LP.tracefile = &((OutputPortion *) param[10])->Value();
    LP.trace = ((IntPortion *) param[11])->Value();

    long niters;
    gWatch watch;
    gList<BehavSolution<double> > solutions;
    Liap(E, LP, start, solutions,
	 ((IntPortion *) param[8])->Value(),
	 niters);

    ((FloatPortion *) param[7])->Value() = watch.Elapsed();

    Portion *por = new Behav_ListPortion<double>(solutions);
    por->SetOwner(param[0]->Owner());
    por->AddDependency();
    return por;
  }
}

#include "nliap.h"

Portion *GSM_Liap_NfgFloat(Portion **param)
{
  Nfg<double> &N = *(Nfg<double> *) ((NfgPortion*) param[0])->Value();
  MixedSolution<double> start(N);

  NFLiapParams params;

  params.stopAfter = ((IntPortion *) param[1])->Value();
  params.nTries = ((IntPortion *) param[2])->Value();

  params.maxitsN = ((IntPortion *) param[3])->Value();
  params.tolN = ((FloatPortion *) param[4])->Value();
  params.maxits1 = ((IntPortion *) param[5])->Value();
  params.tol1 = ((FloatPortion *) param[6])->Value();
 
  long niters;
  gWatch watch;
  gList<MixedSolution<double> > solutions;
  Liap(N, params, start, solutions,
       ((IntPortion *) param[8])->Value(),
       niters);

  ((FloatPortion *) param[7])->Value() = watch.Elapsed();

  Portion *por = new Mixed_ListPortion<double>(solutions);
  por->SetOwner(param[0]->Original());
  por->AddDependency();
  return por;
}

Portion *GSM_Liap_MixedFloat(Portion **param)
{
  MixedSolution<double> &start = 
    * (MixedSolution<double> *) ((MixedPortion *) param[0])->Value();
  Nfg<double> &N = *start.BelongsTo();

  NFLiapParams params;
  
  params.stopAfter = ((IntPortion *) param[1])->Value();
  params.nTries = ((IntPortion *) param[2])->Value();

  params.maxitsN = ((IntPortion *) param[3])->Value();
  params.tolN = ((FloatPortion *) param[4])->Value();
  params.maxits1 = ((IntPortion *) param[5])->Value();
  params.tol1 = ((FloatPortion *) param[6])->Value();
 
  long niters;
  gWatch watch;
  gList<MixedSolution<double> > solutions;
  Liap(N, params, start, solutions,
       ((IntPortion *) param[8])->Value(),
       niters);

  ((FloatPortion *) param[7])->Value() = watch.Elapsed();

  Portion *por = new Mixed_ListPortion<double>(solutions);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

//------------
// LpSolve
//------------

#include "nfgcsum.h"

Portion *GSM_Lp_NfgFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();

  if (N.NumPlayers() > 2 || !N.IsConstSum())
    return new ErrorPortion("Only valid for two-person zero-sum games");

  ZSumParams ZP;

  ZP.tracefile = &((OutputPortion *) param[3])->Value();
  ZP.trace = ((IntPortion *) param[4])->Value();

  NFSupport S(N);
  ZSumModule<double> ZM(N, ZP, S);
  ZM.ZSum();

  ((IntPortion *) param[1])->Value() = ZM.NumPivots();
  ((FloatPortion *) param[2])->Value() = ZM.Time();

  gList<MixedSolution<double> > solns;
  ZM.GetSolutions(solns);

  Portion* por = new Mixed_ListPortion<double>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_Lp_NfgRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational> *) ((NfgPortion *) param[0])->Value();

  if (N.NumPlayers() > 2 || !N.IsConstSum())
    return new ErrorPortion("Only valid for two-person zero-sum games");

  ZSumParams ZP;

  ZP.tracefile = &((OutputPortion *) param[3])->Value();
  ZP.trace = ((IntPortion *) param[4])->Value();

  NFSupport S(N);
  ZSumModule<gRational> ZM(N, ZP, S);
  ZM.ZSum();

  ((IntPortion *) param[1])->Value() = ZM.NumPivots();
  ((FloatPortion *) param[2])->Value() = ZM.Time();

  gList<MixedSolution<gRational> > solns;
  ZM.GetSolutions(solns);

  Portion* por = new Mixed_ListPortion<gRational>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_Lp_Support(Portion **param)
{
  NFSupport& S = * ( (NfSupportPortion*) param[ 0 ] )->Value();
  BaseNfg* N = (BaseNfg*) &( S.BelongsTo() );
  Portion* por = 0;

  if (N->NumPlayers() > 2 || !N->IsConstSum())
    return new ErrorPortion("Only valid for two-person zero-sum games");

  ZSumParams ZP;

  ZP.tracefile = &((OutputPortion *) param[3])->Value();
  ZP.trace = ((IntPortion *) param[4])->Value();

  switch( N->Type() )  {
  case DOUBLE:
    {
      ZSumModule<double> ZM( * (Nfg<double>*) N, ZP, S);
      ZM.ZSum();
      ((IntPortion *) param[1])->Value() = ZM.NumPivots();
      ((FloatPortion *) param[2])->Value() = ZM.Time();
      gList<MixedSolution<double> > solns;
      ZM.GetSolutions(solns);  por = new Mixed_ListPortion<double>(solns);
    }
    break;
  case RATIONAL:
    {
      ZSumModule<gRational> ZM( *(Nfg<gRational>*) N, ZP, S);
      ZM.ZSum();
      ((IntPortion *) param[1])->Value() = ZM.NumPivots();
      ((FloatPortion *) param[2])->Value() = ZM.Time();
      gList<MixedSolution<gRational> > solns;
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


#include "lpsolve.h"

Portion* GSM_Lp_ListFloat(Portion** param)
{
  gMatrix<double>* a = ListToMatrix_Float((ListPortion*) param[0]);
  gVector<double>* b = ListToVector_Float((ListPortion*) param[1]);
  gVector<double>* c = ListToVector_Float((ListPortion*) param[2]);
  if(!a || !b || !c)
    return 0;
  int nequals = ((IntPortion*) param[3])->Value();
  bool isFeasible;
  bool isBounded;
  
  LPSolve<double>* s = new LPSolve<double>(*a, *b, *c, nequals);
  Portion* result = ArrayToList(s->OptimumVector());
  isFeasible = s->IsFeasible();
  isBounded = s->IsBounded();
  delete s;
  delete a;
  delete b;
  delete c;
  
  ((BoolPortion*) param[3])->Value() = isFeasible;
  ((BoolPortion*) param[4])->Value() = isBounded;
  return result;
}

Portion* GSM_Lp_ListRational(Portion** param)
{
  gMatrix<gRational>* a = ListToMatrix_Rational((ListPortion*) param[0]);
  gVector<gRational>* b = ListToVector_Rational((ListPortion*) param[1]);
  gVector<gRational>* c = ListToVector_Rational((ListPortion*) param[2]);
  if(!a || !b || !c)
    return 0;
  int nequals = ((IntPortion*) param[3])->Value();
  bool isFeasible;
  bool isBounded;
  
  LPSolve<gRational>* s = new LPSolve<gRational>(*a, *b, *c, nequals);
  Portion* result = ArrayToList(s->OptimumVector());
  isFeasible = s->IsFeasible();
  isBounded = s->IsBounded();
  delete s;
  delete a;
  delete b;
  delete c;
  
  ((BoolPortion*) param[3])->Value() = isFeasible;
  ((BoolPortion*) param[4])->Value() = isBounded;
  return result;
}




#include "csumsub.h"
#include "efgcsum.h"

Portion *GSM_Lp_EfgFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double> *) ((EfgPortion*) param[0])->Value();
  
  if (E.NumPlayers() > 2 || !E.IsConstSum())
    return new ErrorPortion("Only valid for two-person zero-sum games");

  gList<BehavSolution<double> > solns;
  
  if (((BoolPortion *) param[1])->Value())   {
    ZSumParams ZP;

    ZP.tracefile = &((OutputPortion *) param[4])->Value();
    ZP.trace = ((IntPortion *) param[5])->Value();

    ZSumBySubgame<double> ZM(E, ZP);

    ZM.Solve();

    solns = ZM.GetSolutions();

    ((IntPortion *) param[2])->Value() = ZM.NumPivots();
    ((FloatPortion *) param[3])->Value() = ZM.Time();
  }
  else  {
    CSSeqFormParams ZP;

    ZP.tracefile = &((OutputPortion *) param[4])->Value();
    ZP.trace = ((IntPortion *) param[5])->Value();

    CSSeqFormBySubgame<double> ZM(E, ZP);

    ZM.Solve();

    solns = ZM.GetSolutions();

    ((IntPortion *) param[2])->Value() = ZM.NumPivots();
    ((FloatPortion *) param[3])->Value() = ZM.Time();
  }

  Portion* por = new Behav_ListPortion<double>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_Lp_EfgRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational> *) ((EfgPortion*) param[0])->Value();
  
  if (E.NumPlayers() > 2 || !E.IsConstSum())
    return new ErrorPortion("Only valid for two-person zero-sum games");

  gList<BehavSolution<gRational> > solns;

  if (((BoolPortion *) param[1])->Value())   {
    ZSumParams ZP;

    ZP.tracefile = &((OutputPortion *) param[4])->Value();
    ZP.trace = ((IntPortion *) param[5])->Value();

    ZSumBySubgame<gRational> ZM(E, ZP);

    ZM.Solve();

    solns = ZM.GetSolutions();

    ((IntPortion *) param[2])->Value() = ZM.NumPivots();
    ((FloatPortion *) param[3])->Value() = ZM.Time();
  }
  else  {
    CSSeqFormParams ZP;

    ZP.tracefile = &((OutputPortion *) param[4])->Value();
    ZP.trace = ((IntPortion *) param[5])->Value();

    CSSeqFormBySubgame<gRational> ZM(E, ZP);

    ZM.Solve();

    solns = ZM.GetSolutions();

    ((IntPortion *) param[2])->Value() = ZM.NumPivots();
    ((FloatPortion *) param[3])->Value() = ZM.Time();
  }

  Portion* por = new Behav_ListPortion<gRational>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

//---------
// Nfg
//---------

Portion *GSM_Nfg_Float(Portion **param)
{
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();
  gWatch watch;

  Nfg<double> *N = MakeReducedNfg(E);
  
  ((FloatPortion *) param[1])->Value() = watch.Elapsed();
  
  if (N)
    return new NfgValPortion(N);
  else
    return new ErrorPortion("Conversion to reduced nfg failed");
}

Portion *GSM_Nfg_Rational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();
  gWatch watch;

  Nfg<gRational> *N = MakeReducedNfg(E);
  
  ((FloatPortion *) param[1])->Value() = watch.Elapsed();

  if (N)
    return new NfgValPortion(N);
  else
    return new ErrorPortion("Conversion to reduced nfg failed");
}


//----------
// Payoff
//----------

Portion* GSM_Payoff_BehavFloat( Portion** param )
{
  int i;
  Portion* por = new ListValPortion;
  BehavSolution<double>* bp = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  for( i = 1; i <= bp->BelongsTo()->PlayerList().Length(); i++ )
  {
    ( (ListValPortion*) por )->Append( new FloatValPortion( bp->Payoff( i ) ));
  }
  return por;
}

 Portion* GSM_Payoff_BehavRational( Portion** param )
{
  int i;
  Portion* por = new ListValPortion;
  BehavSolution<gRational>* bp = 
    (BehavSolution<gRational>*) ((BehavPortion*) param[0])->Value();
  for( i = 1; i <= bp->BelongsTo()->PlayerList().Length(); i++ )
  {
    ((ListValPortion*) por)->Append( new RationalValPortion( bp->Payoff( i )));
  }
  return por;
}


Portion* GSM_Payoff_MixedFloat( Portion** param )
{
  int i;
  Portion* por = new ListValPortion;
  MixedSolution<double>* bp = 
    (MixedSolution<double>*) ((MixedPortion*) param[0])->Value();
  for( i = 1; i <= bp->BelongsTo()->NumPlayers(); i++ )
  {
    ( (ListValPortion*) por )->Append( new FloatValPortion( bp->Payoff( i ) ));
  }
  return por;
}

Portion* GSM_Payoff_MixedRational( Portion** param )
{
  int i;
  Portion* por = new ListValPortion;
  MixedSolution<gRational>* bp = 
    (MixedSolution<gRational>*) ((MixedPortion*) param[0])->Value();
  for( i = 1; i <= bp->BelongsTo()->NumPlayers(); i++ )
  {
    ((ListValPortion*) por)->Append( new RationalValPortion( bp->Payoff( i )));
  }
  return por;
}


Portion* GSM_Payoff_NfgFloat( Portion** param )
{
  int i;
  Portion* p;
  Portion* por = new ListValPortion;
  Nfg<double>* nfg = (Nfg<double>*) ( (NfgPortion*) param[0] )->Value();
  gArray<int> Solution( ( (ListPortion*) param[ 1 ] )->Length() );
  
  if( ( (ListPortion*) param[ 1 ] )->Length() != nfg->NumPlayers() )
    return new ErrorPortion("Invalid number of players specified in \"list\"");
  
  for( i = 1; i <= nfg->NumPlayers() ; i++ )
  {
    p = ( (ListPortion*) param[ 1 ] )->Subscript( i );
    assert( p->Spec().Type == porINTEGER );
    Solution[ i ] = ( (IntPortion*) p )->Value();
    delete p;
  }
  for( i = 1; i <= nfg->NumPlayers(); i++ )
  {
    ( (ListValPortion*) por )->
      Append( new FloatValPortion( nfg->Payoff( i, Solution ) ) );
  }
  return por;
}

Portion* GSM_Payoff_NfgRational( Portion** param )
{
  int i;
  Portion* p;
  Portion* por = new ListValPortion;
  Nfg<gRational>* nfg = (Nfg<gRational>*) ( (NfgPortion*) param[0] )->Value();
  gArray<int> Solution( ( (ListPortion*) param[ 1 ] )->Length() );
  
  if( ( (ListPortion*) param[ 1 ] )->Length() != nfg->NumPlayers() )
    return new ErrorPortion("Invalid number of players specified in \"list\"");
  
  for( i = 1; i <= nfg->NumPlayers() ; i++ )
  {
    p = ( (ListPortion*) param[ 1 ] )->Subscript( i );
    assert( p->Spec().Type == porINTEGER );
    Solution[ i ] = ( (IntPortion*) p )->Value();
    delete p;
  }
  for( i = 1; i <= nfg->NumPlayers(); i++ )
  {
    ( (ListValPortion*) por )->
      Append( new RationalValPortion( nfg->Payoff( i, Solution ) ) );
  }
  return por;
}


//------------
// SetPayoff
//------------

Portion* GSM_SetPayoff_NfgFloat( Portion** param )
{
  int i;
  Portion* p;
  Nfg<double>* nfg = (Nfg<double>*) ( (NfgPortion*) param[0] )->Value();
  gArray<int> Solution( ( (ListPortion*) param[ 1 ] )->Length() );
  
  if( ( (ListPortion*) param[ 1 ] )->Length() != nfg->NumPlayers() )
    return new ErrorPortion("Invalid number of players specified in \"list\"");
  if( ( (ListPortion*) param[ 2 ] )->Length() != nfg->NumPlayers() )
    return new ErrorPortion("Invalid number of players specified in \"payoff\"");
  
  for( i = 1; i <= nfg->NumPlayers() ; i++ )
  {
    p = ( (ListPortion*) param[ 1 ] )->Subscript( i );
    assert( p->Spec().Type == porINTEGER );
    Solution[ i ] = ( (IntPortion*) p )->Value();
    delete p;
  }
  for( i = 1; i <= nfg->NumPlayers(); i++ )
  {
    p = ( (ListPortion*) param[ 2 ] )->Subscript( i );
    assert( p->Spec().Type == porFLOAT );
    nfg->SetPayoff( i, Solution, ( (FloatPortion*) p )->Value() );
    delete p;
  }
  return param[ 1 ]->ValCopy();
}

Portion* GSM_SetPayoff_NfgRational( Portion** param )
{
  int i;
  Portion* p;
  Nfg<gRational>* nfg = (Nfg<gRational>*) ( (NfgPortion*) param[0] )->Value();
  gArray<int> Solution( ( (ListPortion*) param[ 1 ] )->Length() );
  
  if( ( (ListPortion*) param[ 1 ] )->Length() != nfg->NumPlayers() )
    return new ErrorPortion("Invalid number of players specified in \"list\"");
  if( ( (ListPortion*) param[ 2 ] )->Length() != nfg->NumPlayers() )
    return new ErrorPortion("Invalid number of players specified in \"payoff\"");
  
  for( i = 1; i <= nfg->NumPlayers() ; i++ )
  {
    p = ( (ListPortion*) param[ 1 ] )->Subscript( i );
    assert( p->Spec().Type == porINTEGER );
    Solution[ i ] = ( (IntPortion*) p )->Value();
    delete p;
  }
  for( i = 1; i <= nfg->NumPlayers(); i++ )
  {
    p = ( (ListPortion*) param[ 2 ] )->Subscript( i );
    assert( p->Spec().Type == porRATIONAL );
    nfg->SetPayoff( i, Solution, ( (RationalPortion*) p )->Value() );
    delete p;
  }
  return param[ 1 ]->ValCopy();
}

//----------------
// SimpDivSolve
//----------------

#include "simpdiv.h"

Portion *GSM_Simpdiv_NfgFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();
  
  SimpdivParams SP;
  SP.stopAfter = ((IntPortion *) param[1])->Value();
  SP.nRestarts = ((IntPortion *) param[2])->Value();
  SP.leashLength = ((IntPortion *) param[3])->Value();
  
  SP.tracefile = &((OutputPortion *) param[6])->Value();
  SP.trace = ((IntPortion *) param[7])->Value();

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

Portion *GSM_Simpdiv_NfgRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();
  
  SimpdivParams SP;
  SP.stopAfter = ((IntPortion *) param[1])->Value();
  SP.nRestarts = ((IntPortion *) param[2])->Value();
  SP.leashLength = ((IntPortion *) param[3])->Value();

  SP.tracefile = &((OutputPortion *) param[6])->Value();
  SP.trace = ((IntPortion *) param[7])->Value();

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


Portion *GSM_Simpdiv_Support(Portion **param)
{
  NFSupport& S = * ( (NfSupportPortion*) param[ 0 ] )->Value();
  BaseNfg* N = (BaseNfg*) &( S.BelongsTo() );
  Portion* por = 0;

  SimpdivParams SP;
  SP.stopAfter = ((IntPortion *) param[1])->Value();
  SP.nRestarts = ((IntPortion *) param[2])->Value();
  SP.leashLength = ((IntPortion *) param[3])->Value();

  SP.tracefile = &((OutputPortion *) param[6])->Value();
  SP.trace = ((IntPortion *) param[7])->Value();

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

#include "simpsub.h"

Portion *GSM_Simpdiv_EfgFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double> *) ((EfgPortion*) param[0])->Value();
  
  if (!((BoolPortion *) param[1])->Value())  
    return new ErrorPortion("algorithm not implemented for extensive forms");

  SimpdivParams SP;
  SP.stopAfter = ((IntPortion *) param[2])->Value();
  SP.nRestarts = ((IntPortion *) param[3])->Value();
  SP.leashLength = ((IntPortion *) param[4])->Value();

  SP.tracefile = &((OutputPortion *) param[7])->Value();
  SP.trace = ((IntPortion *) param[8])->Value();

  SimpdivBySubgame<double> SM(E, SP);
  SM.Solve();

  gList<BehavSolution<double> > solns(SM.GetSolutions());

  ((IntPortion *) param[5])->Value() = SM.NumEvals();
  ((FloatPortion *) param[6])->Value() = SM.Time();
  
  Portion* por = new Behav_ListPortion<double>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_Simpdiv_EfgRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational> *) ((EfgPortion*) param[0])->Value();
  
  if (!((BoolPortion *) param[1])->Value())  
    return new ErrorPortion("algorithm not implemented for extensive forms");

  SimpdivParams SP;
  SP.stopAfter = ((IntPortion *) param[2])->Value();
  SP.nRestarts = ((IntPortion *) param[3])->Value();
  SP.leashLength = ((IntPortion *) param[4])->Value();

  SP.tracefile = &((OutputPortion *) param[7])->Value();
  SP.trace = ((IntPortion *) param[8])->Value();

  SimpdivBySubgame<gRational> SM(E, SP);

  SM.Solve();

  gList<BehavSolution<gRational> > solns(SM.GetSolutions());

  ((IntPortion *) param[5])->Value() = SM.NumEvals();
  ((FloatPortion *) param[6])->Value() = SM.Time();
  
  Portion* por = new Behav_ListPortion<gRational>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}


void Init_algfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("Afg");
  FuncObj->SetFuncInfo(GSM_Afg_Float, 2);
  FuncObj->SetParamInfo(GSM_Afg_Float, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Afg_Float, 1, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  
  FuncObj->SetFuncInfo(GSM_Afg_Rational, 2);
  FuncObj->SetParamInfo(GSM_Afg_Rational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Afg_Rational, 1, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Behav");
  FuncObj->SetFuncInfo(GSM_Behav_Float, 2);
  FuncObj->SetParamInfo(GSM_Behav_Float, 0, "mixed", porMIXED_FLOAT);
  FuncObj->SetParamInfo(GSM_Behav_Float, 1, "efg", porEFG_FLOAT,
		        NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_Behav_Rational, 2);
  FuncObj->SetParamInfo(GSM_Behav_Rational, 0, "mixed", porMIXED_RATIONAL);
  FuncObj->SetParamInfo(GSM_Behav_Rational, 1, "efg", porEFG_RATIONAL,
		        NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("EnumMixedSolve");
  FuncObj->SetFuncInfo(GSM_EnumMixed_NfgFloat, 6);
  FuncObj->SetParamInfo(GSM_EnumMixed_NfgFloat, 0, "nfg", 
			porNFG_FLOAT, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_EnumMixed_NfgFloat, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumMixed_NfgFloat, 2, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixed_NfgFloat, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixed_NfgFloat, 4, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixed_NfgFloat, 5, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_EnumMixed_NfgRational, 6);
  FuncObj->SetParamInfo(GSM_EnumMixed_NfgRational, 0, "nfg", 
			porNFG_RATIONAL, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_EnumMixed_NfgRational, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumMixed_NfgRational, 2, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixed_NfgRational, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixed_NfgRational, 4, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixed_NfgRational, 5, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_EnumMixed_Support, 6);
  FuncObj->SetParamInfo(GSM_EnumMixed_Support, 0, "support", porNF_SUPPORT);
  FuncObj->SetParamInfo(GSM_EnumMixed_Support, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumMixed_Support, 2, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixed_Support, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixed_Support, 4, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixed_Support, 5, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_EnumMixed_EfgFloat, 7);
  FuncObj->SetParamInfo(GSM_EnumMixed_EfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_EnumMixed_EfgFloat, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_EnumMixed_EfgFloat, 2, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumMixed_EfgFloat, 3, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixed_EfgFloat, 4, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixed_EfgFloat, 5, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixed_EfgFloat, 6, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_EnumMixed_EfgRational, 7);
  FuncObj->SetParamInfo(GSM_EnumMixed_EfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_EnumMixed_EfgRational, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_EnumMixed_EfgRational, 2, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumMixed_EfgRational, 3, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixed_EfgRational, 4, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixed_EfgRational, 5, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixed_EfgRational, 6, "traceLevel", porINTEGER,
			new IntValPortion(0));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("EnumPureSolve");
  FuncObj->SetFuncInfo(GSM_EnumPure_NfgFloat, 5);
  FuncObj->SetParamInfo(GSM_EnumPure_NfgFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumPure_NfgFloat, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumPure_NfgFloat, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumPure_NfgFloat, 3, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumPure_NfgFloat, 4, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_EnumPure_NfgRational, 5);
  FuncObj->SetParamInfo(GSM_EnumPure_NfgRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumPure_NfgRational, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumPure_NfgRational, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumPure_NfgRational, 3, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumPure_NfgRational, 4, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_EnumPure_Support, 5);
  FuncObj->SetParamInfo(GSM_EnumPure_Support, 0, "support", porNF_SUPPORT );
  FuncObj->SetParamInfo(GSM_EnumPure_Support, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumPure_Support, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumPure_Support, 3, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumPure_Support, 4, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_EnumPure_EfgFloat, 6);
  FuncObj->SetParamInfo(GSM_EnumPure_EfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumPure_EfgFloat, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_EnumPure_EfgFloat, 2, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumPure_EfgFloat, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumPure_EfgFloat, 4, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumPure_EfgFloat, 5, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_EnumPure_EfgRational, 6);
  FuncObj->SetParamInfo(GSM_EnumPure_EfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumPure_EfgRational, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_EnumPure_EfgRational, 2, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumPure_EfgRational, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumPure_EfgRational, 4, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumPure_EfgRational, 5, "traceLevel", porINTEGER,
			new IntValPortion(0));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("GobitGridSolve");
  FuncObj->SetFuncInfo(GSM_GobitGrid_NfgFloat, 12);
  FuncObj->SetParamInfo(GSM_GobitGrid_NfgFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitGrid_NfgFloat, 1, "pxifile", porTEXT,
			new TextValPortion("") );
  FuncObj->SetParamInfo(GSM_GobitGrid_NfgFloat, 2, "minLam", porFLOAT,
			new FloatValPortion(0.01));
  FuncObj->SetParamInfo(GSM_GobitGrid_NfgFloat, 3, "maxLam", porFLOAT,
			new FloatValPortion(30.0));
  FuncObj->SetParamInfo(GSM_GobitGrid_NfgFloat, 4, "delLam", porFLOAT,
			new FloatValPortion(0.01));
  FuncObj->SetParamInfo(GSM_GobitGrid_NfgFloat, 5, "powLam", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_GobitGrid_NfgFloat, 6, "delp", porFLOAT,
			new FloatValPortion(.01));
  FuncObj->SetParamInfo(GSM_GobitGrid_NfgFloat, 7, "tol", porFLOAT,
			new FloatValPortion(.01));
  FuncObj->SetParamInfo(GSM_GobitGrid_NfgFloat, 8, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitGrid_NfgFloat, 9, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitGrid_NfgFloat, 10, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitGrid_NfgFloat, 11, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_GobitGrid_Support, 12);
  FuncObj->SetParamInfo(GSM_GobitGrid_Support, 0, "support", porNF_SUPPORT );
  FuncObj->SetParamInfo(GSM_GobitGrid_Support, 1, "pxifile", porTEXT,
			new TextValPortion("") );
  FuncObj->SetParamInfo(GSM_GobitGrid_Support, 2, "minLam", porFLOAT,
			new FloatValPortion(0.01));
  FuncObj->SetParamInfo(GSM_GobitGrid_Support, 3, "maxLam", porFLOAT,
			new FloatValPortion(30.0));
  FuncObj->SetParamInfo(GSM_GobitGrid_Support, 4, "delLam", porFLOAT,
			new FloatValPortion(0.01));
  FuncObj->SetParamInfo(GSM_GobitGrid_Support, 5, "powLam", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_GobitGrid_Support, 6, "delp", porFLOAT,
			new FloatValPortion(.01));
  FuncObj->SetParamInfo(GSM_GobitGrid_Support, 7, "tol", porFLOAT,
			new FloatValPortion(.01));
  FuncObj->SetParamInfo(GSM_GobitGrid_Support, 8, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitGrid_Support, 9, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitGrid_Support, 10, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitGrid_Support, 11, "traceLevel", porINTEGER,
			new IntValPortion(0));

  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("GobitSolve");
  FuncObj->SetFuncInfo(GSM_Gobit, 16);
  FuncObj->SetParamInfo(GSM_Gobit, 0, "game", porNFG_FLOAT | porEFG_FLOAT, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_Gobit, 1, "pxifile", porTEXT,
			new TextValPortion("") );
  FuncObj->SetParamInfo(GSM_Gobit, 2, "minLam", porFLOAT,
			new FloatValPortion(0.01));
  FuncObj->SetParamInfo(GSM_Gobit, 3, "maxLam", porFLOAT,
			new FloatValPortion(30.0));
  FuncObj->SetParamInfo(GSM_Gobit, 4, "delLam", porFLOAT,
			new FloatValPortion(0.01));
  FuncObj->SetParamInfo(GSM_Gobit, 5, "powLam", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_Gobit, 6, "fullGraph", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_Gobit, 7, "maxitsN", porINTEGER,
			new IntValPortion(20));
  FuncObj->SetParamInfo(GSM_Gobit, 8, "tolN", porFLOAT,
			new FloatValPortion(1.0e-10));
  FuncObj->SetParamInfo(GSM_Gobit, 9, "maxits1", porINTEGER,
			new IntValPortion(100));
  FuncObj->SetParamInfo(GSM_Gobit, 10, "tol1", porFLOAT,
			new FloatValPortion(2.0e-10));
  FuncObj->SetParamInfo(GSM_Gobit, 11, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Gobit, 12, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Gobit, 13, "nIters", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Gobit, 14, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Gobit, 15, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_Gobit_Start, 16);
  FuncObj->SetParamInfo(GSM_Gobit_Start, 0, "start",
			porMIXED_FLOAT | porBEHAV_FLOAT);
  FuncObj->SetParamInfo(GSM_Gobit_Start, 1, "pxifile", porTEXT,
			new TextValPortion("") );
  FuncObj->SetParamInfo(GSM_Gobit_Start, 2, "minLam", porFLOAT,
			new FloatValPortion(0.01));
  FuncObj->SetParamInfo(GSM_Gobit_Start, 3, "maxLam", porFLOAT,
			new FloatValPortion(30.0));
  FuncObj->SetParamInfo(GSM_Gobit_Start, 4, "delLam", porFLOAT,
			new FloatValPortion(0.01));
  FuncObj->SetParamInfo(GSM_Gobit_Start, 5, "powLam", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_Gobit_Start, 6, "fullGraph", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_Gobit_Start, 7, "maxitsN", porINTEGER,
			new IntValPortion(20));
  FuncObj->SetParamInfo(GSM_Gobit_Start, 8, "tolN", porFLOAT,
			new FloatValPortion(1.0e-10));
  FuncObj->SetParamInfo(GSM_Gobit_Start, 9, "maxits1", porINTEGER,
			new IntValPortion(100));
  FuncObj->SetParamInfo(GSM_Gobit_Start, 10, "tol1", porFLOAT,
			new FloatValPortion(2.0e-10));
  FuncObj->SetParamInfo(GSM_Gobit_Start, 11, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Gobit_Start, 12, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Gobit_Start, 13, "nIters", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Gobit_Start, 14, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Gobit_Start, 15, "traceLevel", porINTEGER,
			new IntValPortion(0));

  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LcpSolve");
  FuncObj->SetFuncInfo(GSM_Lcp_EfgFloat, 7);
  FuncObj->SetParamInfo(GSM_Lcp_EfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lcp_EfgFloat, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_Lcp_EfgFloat, 2, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_Lcp_EfgFloat, 3, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lcp_EfgFloat, 4, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lcp_EfgFloat, 5, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lcp_EfgFloat, 6, "traceLevel", porINTEGER,
			new IntValPortion(0));
  
  FuncObj->SetFuncInfo(GSM_Lcp_EfgRational, 7);
  FuncObj->SetParamInfo(GSM_Lcp_EfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lcp_EfgRational, 1, "asNFG", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_Lcp_EfgRational, 2, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_Lcp_EfgRational, 3, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lcp_EfgRational, 4, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lcp_EfgRational, 5, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lcp_EfgRational, 6, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_Lcp_NfgFloat, 6);
  FuncObj->SetParamInfo(GSM_Lcp_NfgFloat, 0, "nfg",
			porNFG_FLOAT, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_Lcp_NfgFloat, 1, "stopAfter", 
			porINTEGER, new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_Lcp_NfgFloat, 2, "nPivots", 
			porINTEGER, new IntValPortion(0),
		        PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lcp_NfgFloat, 3, "time", 
			porFLOAT, new FloatValPortion(0),
			PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lcp_NfgFloat, 4, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lcp_NfgFloat, 5, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_Lcp_NfgRational, 6);
  FuncObj->SetParamInfo(GSM_Lcp_NfgRational, 0, "nfg",
			porNFG_RATIONAL, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_Lcp_NfgRational, 1, "stopAfter", 
			porINTEGER, new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_Lcp_NfgRational, 2, "nPivots", 
			porINTEGER, new IntValPortion(0),
		        PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lcp_NfgRational, 3, "time", 
			porFLOAT, new FloatValPortion(0),
			PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lcp_NfgRational, 4, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lcp_NfgRational, 5, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_Lcp_Support, 6);
  FuncObj->SetParamInfo(GSM_Lcp_Support, 0, "support", porNF_SUPPORT );
  FuncObj->SetParamInfo(GSM_Lcp_Support, 1, "stopAfter", 
			porINTEGER, new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_Lcp_Support, 2, "nPivots", 
			porINTEGER, new IntValPortion(0),
		        PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lcp_Support, 3, "time", 
			porFLOAT, new FloatValPortion(0),
			PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lcp_Support, 4, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lcp_Support, 5, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_Lcp_ListFloat, 2);
  FuncObj->SetParamInfo(GSM_Lcp_ListFloat, 0, "a", PortionSpec(porFLOAT,2),
			NO_DEFAULT_VALUE, PASS_BY_VALUE);
  FuncObj->SetParamInfo(GSM_Lcp_ListFloat, 1, "b", PortionSpec(porFLOAT,1),
			NO_DEFAULT_VALUE, PASS_BY_VALUE);

  FuncObj->SetFuncInfo(GSM_Lcp_ListRational, 2);
  FuncObj->SetParamInfo(GSM_Lcp_ListRational, 0, "a", 
			PortionSpec(porRATIONAL,2),
			NO_DEFAULT_VALUE, PASS_BY_VALUE);
  FuncObj->SetParamInfo(GSM_Lcp_ListRational, 1, "b", 
			PortionSpec(porRATIONAL,1),
			NO_DEFAULT_VALUE, PASS_BY_VALUE);

  gsm->AddFunction(FuncObj);




  FuncObj = new FuncDescObj("LiapSolve");
  FuncObj->SetFuncInfo(GSM_Liap_EfgFloat, 12);
  FuncObj->SetParamInfo(GSM_Liap_EfgFloat, 0, "efg", porEFG_FLOAT, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_Liap_EfgFloat, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_Liap_EfgFloat, 2, "stopAfter", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_Liap_EfgFloat, 3, "nTries", porINTEGER,
		        new IntValPortion(10));
  FuncObj->SetParamInfo(GSM_Liap_EfgFloat, 4, "maxitsN", porINTEGER,
			new IntValPortion(20));
  FuncObj->SetParamInfo(GSM_Liap_EfgFloat, 5, "tolN", porFLOAT,
			new FloatValPortion(1.0e-10));
  FuncObj->SetParamInfo(GSM_Liap_EfgFloat, 6, "maxits1", porINTEGER,
			new IntValPortion(100));
  FuncObj->SetParamInfo(GSM_Liap_EfgFloat, 7, "tol1", porFLOAT,
			new FloatValPortion(2.0e-10));
  FuncObj->SetParamInfo(GSM_Liap_EfgFloat, 8, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Liap_EfgFloat, 9, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Liap_EfgFloat, 10, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Liap_EfgFloat, 11, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_Liap_BehavFloat, 12);
  FuncObj->SetParamInfo(GSM_Liap_BehavFloat, 0, "start", porBEHAV_FLOAT );
  FuncObj->SetParamInfo(GSM_Liap_BehavFloat, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_Liap_BehavFloat, 2, "stopAfter", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_Liap_BehavFloat, 3, "nTries", porINTEGER,
		        new IntValPortion(10));
  FuncObj->SetParamInfo(GSM_Liap_BehavFloat, 4, "maxitsN", porINTEGER,
			new IntValPortion(20));
  FuncObj->SetParamInfo(GSM_Liap_BehavFloat, 5, "tolN", porFLOAT,
			new FloatValPortion(1.0e-10));
  FuncObj->SetParamInfo(GSM_Liap_BehavFloat, 6, "maxits1", porINTEGER,
			new IntValPortion(100));
  FuncObj->SetParamInfo(GSM_Liap_BehavFloat, 7, "tol1", porFLOAT,
			new FloatValPortion(2.0e-10));
  FuncObj->SetParamInfo(GSM_Liap_BehavFloat, 8, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Liap_BehavFloat, 9, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Liap_BehavFloat, 10, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Liap_BehavFloat, 11, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_Liap_NfgFloat, 11);
  FuncObj->SetParamInfo(GSM_Liap_NfgFloat, 0, "nfg", porNFG_FLOAT, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_Liap_NfgFloat, 1, "stopAfter", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_Liap_NfgFloat, 2, "nTries", porINTEGER,
		        new IntValPortion(10));
  FuncObj->SetParamInfo(GSM_Liap_NfgFloat, 3, "maxitsN", porINTEGER,
			new IntValPortion(20));
  FuncObj->SetParamInfo(GSM_Liap_NfgFloat, 4, "tolN", porFLOAT,
			new FloatValPortion(1.0e-10));
  FuncObj->SetParamInfo(GSM_Liap_NfgFloat, 5, "maxits1", porINTEGER,
			new IntValPortion(100));
  FuncObj->SetParamInfo(GSM_Liap_NfgFloat, 6, "tol1", porFLOAT,
			new FloatValPortion(2.0e-10));
  FuncObj->SetParamInfo(GSM_Liap_NfgFloat, 7, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Liap_NfgFloat, 8, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Liap_NfgFloat, 9, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Liap_NfgFloat, 10, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_Liap_MixedFloat, 11);
  FuncObj->SetParamInfo(GSM_Liap_MixedFloat, 0, "start", porMIXED_FLOAT );
  FuncObj->SetParamInfo(GSM_Liap_MixedFloat, 1, "stopAfter", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_Liap_MixedFloat, 2, "nTries", porINTEGER,
		        new IntValPortion(10));
  FuncObj->SetParamInfo(GSM_Liap_MixedFloat, 3, "maxitsN", porINTEGER,
			new IntValPortion(20));
  FuncObj->SetParamInfo(GSM_Liap_MixedFloat, 4, "tolN", porFLOAT,
			new FloatValPortion(1.0e-10));
  FuncObj->SetParamInfo(GSM_Liap_MixedFloat, 5, "maxits1", porINTEGER,
			new IntValPortion(100));
  FuncObj->SetParamInfo(GSM_Liap_MixedFloat, 6, "tol1", porFLOAT,
			new FloatValPortion(2.0e-10));
  FuncObj->SetParamInfo(GSM_Liap_MixedFloat, 7, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Liap_MixedFloat, 8, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Liap_MixedFloat, 9, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Liap_MixedFloat, 10, "traceLevel", porINTEGER,
			new IntValPortion(0));

  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LpSolve");
  FuncObj->SetFuncInfo(GSM_Lp_NfgFloat, 5);
  FuncObj->SetParamInfo(GSM_Lp_NfgFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_NfgFloat, 1, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_NfgFloat, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_NfgFloat, 3, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_NfgFloat, 4, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_Lp_NfgRational, 5);
  FuncObj->SetParamInfo(GSM_Lp_NfgRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_NfgRational, 1, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_NfgRational, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_NfgRational, 3, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_NfgRational, 4, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_Lp_Support, 5);
  FuncObj->SetParamInfo(GSM_Lp_Support, 0, "support", porNF_SUPPORT );
  FuncObj->SetParamInfo(GSM_Lp_Support, 1, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_Support, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_Support, 3, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_Support, 4, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_Lp_EfgFloat, 6);
  FuncObj->SetParamInfo(GSM_Lp_EfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_EfgFloat, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_Lp_EfgFloat, 2, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_EfgFloat, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_EfgFloat, 4, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_EfgFloat, 5, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_Lp_EfgRational, 6);
  FuncObj->SetParamInfo(GSM_Lp_EfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_EfgRational, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_Lp_EfgRational, 2, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_EfgRational, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_EfgRational, 4, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_EfgRational, 5, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_Lp_ListFloat, 6);
  FuncObj->SetParamInfo(GSM_Lp_ListFloat, 0, "a", PortionSpec(porFLOAT,2),
			NO_DEFAULT_VALUE, PASS_BY_VALUE);
  FuncObj->SetParamInfo(GSM_Lp_ListFloat, 1, "b", PortionSpec(porFLOAT,1),
			NO_DEFAULT_VALUE, PASS_BY_VALUE);
  FuncObj->SetParamInfo(GSM_Lp_ListFloat, 2, "c", PortionSpec(porFLOAT,1),
			NO_DEFAULT_VALUE, PASS_BY_VALUE);
  FuncObj->SetParamInfo(GSM_Lp_ListFloat, 3, "nEqualities", porINTEGER);
  FuncObj->SetParamInfo(GSM_Lp_ListFloat, 4, "isFeasible", porBOOL,
			new BoolValPortion(false), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_ListFloat, 5, "isBounded", porBOOL,
			new BoolValPortion(false), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_Lp_ListRational, 6);
  FuncObj->SetParamInfo(GSM_Lp_ListRational, 0, "a", 
			PortionSpec(porRATIONAL,2),
			NO_DEFAULT_VALUE, PASS_BY_VALUE);
  FuncObj->SetParamInfo(GSM_Lp_ListRational, 1, "b", 
			PortionSpec(porRATIONAL,1),
			NO_DEFAULT_VALUE, PASS_BY_VALUE);
  FuncObj->SetParamInfo(GSM_Lp_ListRational, 2, "c", 
			PortionSpec(porRATIONAL,1),
			NO_DEFAULT_VALUE, PASS_BY_VALUE);
  FuncObj->SetParamInfo(GSM_Lp_ListRational, 3, "nEqualities", porINTEGER);
  FuncObj->SetParamInfo(GSM_Lp_ListRational, 4, "isFeasible", porBOOL,
			new BoolValPortion(false), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lp_ListRational, 5, "isBounded", porBOOL,
			new BoolValPortion(false), PASS_BY_REFERENCE);

  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("Nfg");
  FuncObj->SetFuncInfo(GSM_Nfg_Float, 2);
  FuncObj->SetParamInfo(GSM_Nfg_Float, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Nfg_Float, 1, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_Nfg_Rational, 2);
  FuncObj->SetParamInfo(GSM_Nfg_Rational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_Nfg_Rational, 1, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Payoff");
  FuncObj->SetFuncInfo(GSM_Payoff_BehavFloat, 1);
  FuncObj->SetParamInfo(GSM_Payoff_BehavFloat, 
			0, "strategy", porBEHAV_FLOAT);
  FuncObj->SetFuncInfo(GSM_Payoff_BehavRational, 1);
  FuncObj->SetParamInfo(GSM_Payoff_BehavRational, 
			0, "strategy", porBEHAV_RATIONAL);

  FuncObj->SetFuncInfo(GSM_Payoff_MixedFloat, 1);
  FuncObj->SetParamInfo(GSM_Payoff_MixedFloat, 
			0, "strategy", porMIXED_FLOAT);
  FuncObj->SetFuncInfo(GSM_Payoff_MixedRational, 1);
  FuncObj->SetParamInfo(GSM_Payoff_MixedRational, 
			0, "strategy", porMIXED_RATIONAL);

  FuncObj->SetFuncInfo(GSM_Payoff_NfgFloat, 2);
  FuncObj->SetParamInfo(GSM_Payoff_NfgFloat, 
			0, "nfg", porNFG_FLOAT, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Payoff_NfgFloat,
			1, "list", PortionSpec(porINTEGER,1) );
  FuncObj->SetFuncInfo(GSM_Payoff_NfgRational, 2);
  FuncObj->SetParamInfo(GSM_Payoff_NfgRational, 
			0, "nfg", porNFG_RATIONAL, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Payoff_NfgRational,
			1, "list", PortionSpec(porINTEGER,1) );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj( "SetPayoff" );
  FuncObj->SetFuncInfo(GSM_SetPayoff_NfgFloat, 3);
  FuncObj->SetParamInfo(GSM_SetPayoff_NfgFloat, 
			0, "nfg", porNFG_FLOAT, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SetPayoff_NfgFloat,
			1, "list", PortionSpec(porINTEGER,1) );
  FuncObj->SetParamInfo(GSM_SetPayoff_NfgFloat,
			2, "payoff", PortionSpec(porFLOAT,1) );

  FuncObj->SetFuncInfo(GSM_SetPayoff_NfgRational, 3);
  FuncObj->SetParamInfo(GSM_SetPayoff_NfgRational, 
			0, "nfg", porNFG_RATIONAL, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SetPayoff_NfgRational,
			1, "list", PortionSpec(porINTEGER,1) );
  FuncObj->SetParamInfo(GSM_SetPayoff_NfgRational,
			2, "payoff", PortionSpec(porRATIONAL,1) );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SimpDivSolve");
  FuncObj->SetFuncInfo(GSM_Simpdiv_NfgFloat, 8);
  FuncObj->SetParamInfo(GSM_Simpdiv_NfgFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_NfgFloat, 1, "stopAfter", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_Simpdiv_NfgFloat, 2, "nRestarts", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_Simpdiv_NfgFloat, 3, "leashLength", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_Simpdiv_NfgFloat, 4, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_NfgFloat, 5, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_NfgFloat, 6, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_NfgFloat, 7, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_Simpdiv_NfgRational, 8);
  FuncObj->SetParamInfo(GSM_Simpdiv_NfgRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_NfgRational, 1, "stopAfter", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_Simpdiv_NfgRational, 2, "nRestarts", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_Simpdiv_NfgRational, 3, "leashLength", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_Simpdiv_NfgRational, 4, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_NfgRational, 5, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_NfgRational, 6, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_NfgRational, 7, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_Simpdiv_Support, 8);
  FuncObj->SetParamInfo(GSM_Simpdiv_Support, 0, "support", porNF_SUPPORT );
  FuncObj->SetParamInfo(GSM_Simpdiv_Support, 1, "stopAfter", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_Simpdiv_Support, 2, "nRestarts", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_Simpdiv_Support, 3, "leashLength", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_Simpdiv_Support, 4, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_Support, 5, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_Support, 6, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_Support, 7, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_Simpdiv_EfgFloat, 9);
  FuncObj->SetParamInfo(GSM_Simpdiv_EfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_EfgFloat, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_Simpdiv_EfgFloat, 2, "stopAfter", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_Simpdiv_EfgFloat, 3, "nRestarts", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_Simpdiv_EfgFloat, 4, "leashLength", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_Simpdiv_EfgFloat, 5, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_EfgFloat, 6, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_EfgFloat, 7, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_EfgFloat, 8, "traceLevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_Simpdiv_EfgRational, 9);
  FuncObj->SetParamInfo(GSM_Simpdiv_EfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_EfgRational, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_Simpdiv_EfgRational, 2, "stopAfter", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_Simpdiv_EfgRational, 3, "nRestarts", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_Simpdiv_EfgRational, 4, "leashLength", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_Simpdiv_EfgRational, 5, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_EfgRational, 6, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_EfgRational, 7, "traceFile", porOUTPUT,
			new OutputRefPortion(gnull), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Simpdiv_EfgRational, 8, "traceLevel", porINTEGER,
			new IntValPortion(0));
  gsm->AddFunction(FuncObj);
}


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class Mixed_ListPortion<double>;
TEMPLATE class Mixed_ListPortion<gRational>;

TEMPLATE class Behav_ListPortion<double>;
TEMPLATE class Behav_ListPortion<gRational>;


