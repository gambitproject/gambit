//
// FILE: algfunc.cc -- Solution algorithm functions for GCL
//
// $Id$
//

#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "egobit.h"
#include "eliap.h"

#include "rational.h"

#include "gwatch.h"
#include "mixed.h"

#include "subsolve.h"


Portion *GSM_BehavFloat(Portion **param)
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

Portion *GSM_BehavRational(Portion **param)
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




//---------------------------- Payoff ----------------------------//


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
    assert( p->Type() == porINTEGER );
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
    assert( p->Type() == porINTEGER );
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



//-------------------------------- SetPayoff ---------------------------//

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
    assert( p->Type() == porINTEGER );
    Solution[ i ] = ( (IntPortion*) p )->Value();
    delete p;
  }
  for( i = 1; i <= nfg->NumPlayers(); i++ )
  {
    p = ( (ListPortion*) param[ 2 ] )->Subscript( i );
    assert( p->Type() == porFLOAT );
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
    assert( p->Type() == porINTEGER );
    Solution[ i ] = ( (IntPortion*) p )->Value();
    delete p;
  }
  for( i = 1; i <= nfg->NumPlayers(); i++ )
  {
    p = ( (ListPortion*) param[ 2 ] )->Subscript( i );
    assert( p->Type() == porRATIONAL );
    nfg->SetPayoff( i, Solution, ( (RationalPortion*) p )->Value() );
    delete p;
  }
  return param[ 1 ]->ValCopy();
}





Portion *GSM_NfgFloat(Portion **param)
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

Portion *GSM_NfgRational(Portion **param)
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


Portion *GSM_AfgFloat(Portion **param)
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

Portion *GSM_AfgRational(Portion **param)
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



//--------------------------- GobitSolve -----------------------------//

Portion *GSM_GobitEfg_EfgFloat(Portion **param)
{
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


Portion *GSM_GobitEfg_BehavFloat(Portion **param)
{
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

//------------------------- GobitLambda, GobitValue -----------------//

Portion* GSM_GobitLambda_BehavFloat(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new FloatValPortion( bs->GobitLambda() );
}

Portion* GSM_GobitLambda_BehavRational(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new RationalValPortion( bs->GobitLambda() );
}

Portion* GSM_GobitValue_BehavFloat(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new FloatValPortion( bs->GobitValue() );
}

Portion* GSM_GobitValue_BehavRational(Portion** param)
{
  BehavSolution<double>* bs = 
    (BehavSolution<double>*) ((BehavPortion*) param[0])->Value();
  return new RationalValPortion( bs->GobitValue() );
}




//-------------------------- LiapSolve ---------------------------//

Portion *GSM_LiapEfg_EfgFloat(Portion **param)
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

Portion *GSM_LiapEfg_BehavFloat(Portion **param)
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




//-------------------------------- LcpSolve -------------------------//

#include "seqform.h"

Portion *GSM_LcpSolveEfgFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();

  gList<BehavSolution<double> > solns;
  
  if (((BoolPortion *) param[1])->Value())  {
    LemkeParams LP;
    LP.stopAfter = ((IntPortion *) param[2])->Value();
/*
    LP.tracefile = &((OutputPortion *) param[5])->Value();
    LP.trace = ((IntPortion *) param[6])->Value();
    */
    LemkeBySubgame<double> LM(E, LP);

    LM.Solve();

    solns = LM.GetSolutions();

    ((IntPortion *) param[3])->Value() = LM.NumPivots();
    ((FloatPortion *) param[4])->Value() = LM.Time();
  }
  else   {
    SeqFormParams SP;
    SP.stopAfter = ((IntPortion *) param[2])->Value();
/*
    SP.tracefile = &((OutputPortion *) param[5])->Value();
    SP.trace = ((IntPortion *) param[6])->Value();
    */
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

Portion *GSM_LcpSolveEfgRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();

  gList<BehavSolution<gRational> > solns;
  
  if (((BoolPortion *) param[1])->Value())  {
    LemkeParams LP;
    LP.stopAfter = ((IntPortion *) param[2])->Value();
/*
    LP.tracefile = &((OutputPortion *) param[5])->Value();
    LP.trace = ((IntPortion *) param[6])->Value();
    */
    LemkeBySubgame<gRational> LM(E, LP);
    
    LM.Solve();

    solns = LM.GetSolutions();

    ((IntPortion *) param[3])->Value() = LM.NumPivots();
    ((FloatPortion *) param[4])->Value() = LM.Time();
  }
  else   {
    SeqFormParams SP;
    SP.stopAfter = ((IntPortion *) param[2])->Value();
/*
    SP.tracefile = &((OutputPortion *) param[5])->Value();
    SP.trace = ((IntPortion *) param[6])->Value();
    */
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


/* NOTE: LcpSolve[EFSUPPORT] is currently disabled as it doesn't do
         anything all that useful anyway and isn't too well-defined. --TLT

Portion *GSM_LemkeEfgSupport(Portion **param)
{
  EFSupport& SUP = * ( (EfSupportPortion*) param[ 0 ] )->Value();
  BaseEfg* E = (BaseEfg*) &( SUP.BelongsTo() );
  Portion* por = 0;

  SeqFormParams SP;
  SP.nequilib = ((IntPortion *) param[1])->Value();
      
  switch( E->Type() )
  {
  case DOUBLE:
    {
      SeqFormModule<double> SM( * (Efg<double>*) E, SP, SUP);
      SM.Lemke();
      ((IntPortion *) param[2])->Value() = SM.NumPivots();
      ((FloatPortion *) param[3])->Value() = SM.Time();
      por = new Behav_ListPortion<double>(SM.GetSolutions());
    }
    break;
  case RATIONAL:
    {
      SeqFormModule<gRational> SM( * (Efg<gRational>*) E, SP, SUP);
      SM.Lemke();
      ((IntPortion *) param[2])->Value() = SM.NumPivots();
      ((FloatPortion *) param[3])->Value() = SM.Time();
      por = new Behav_ListPortion<gRational>(SM.GetSolutions());
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

*/


//------------------ Simpdiv -------------------------

Portion *GSM_SimpdivEfgFloat(Portion **param)
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

Portion *GSM_SimpdivEfgRational(Portion **param)
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


//------------------ LpSolve -------------------------

Portion *GSM_LpSolveEfgFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double> *) ((EfgPortion*) param[0])->Value();
  
  if (E.NumPlayers() > 2 || !E.IsConstSum())
    return new ErrorPortion("Only valid for two-person zero-sum games");

  ZSumParams ZP;

  ZP.tracefile = &((OutputPortion *) param[3])->Value();
  ZP.trace = ((IntPortion *) param[4])->Value();

  ZSumBySubgame<double> ZM(E, ZP);

  ZM.Solve();

  gList<BehavSolution<double> > solns(ZM.GetSolutions());


  ((IntPortion *) param[1])->Value() = ZM.NumPivots();
  ((FloatPortion *) param[2])->Value() = ZM.Time();
  
  Portion* por = new Behav_ListPortion<double>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_LpSolveEfgRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational> *) ((EfgPortion*) param[0])->Value();
  
  if (E.NumPlayers() > 2 || !E.IsConstSum())
    return new ErrorPortion("Only valid for two-person zero-sum games");

  ZSumParams ZP;

  ZP.tracefile = &((OutputPortion *) param[3])->Value();
  ZP.trace = ((IntPortion *) param[4])->Value();

  ZSumBySubgame<gRational> ZM(E, ZP);

  ZM.Solve();

  gList<BehavSolution<gRational> > solns(ZM.GetSolutions());

  ((IntPortion *) param[1])->Value() = ZM.NumPivots();
  ((FloatPortion *) param[2])->Value() = ZM.Time();
  
  Portion* por = new Behav_ListPortion<gRational>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}


//----------------------- EnumPureSolve --------------------------//

Portion *GSM_EnumPureEfgFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();

  if (!((BoolPortion *) param[1])->Value())
    return new ErrorPortion("algorithm not implemented for extensive forms");

  PureNashBySubgame<double> M(E);

  M.Solve();

  gList<BehavSolution<double> > solns(M.GetSolutions());

  ((FloatPortion *) param[3])->Value() = M.Time();
  
  Portion* por = new Behav_ListPortion<double>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_EnumPureEfgRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();

  if (!((BoolPortion *) param[1])->Value())
    return new ErrorPortion("algorithm not implemented for extensive forms");

  PureNashBySubgame<gRational> M(E);

  M.Solve();

  gList<BehavSolution<gRational> > solns(M.GetSolutions());

  ((FloatPortion *) param[3])->Value() = M.Time();
  
  Portion* por = new Behav_ListPortion<gRational>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

  //-----------------------EnumMixedSolve------------------------

Portion *GSM_EnumMixedEfgFloat(Portion **param)
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

Portion *GSM_EnumMixedEfgRational(Portion **param)
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



extern Portion *ArrayToList(const gArray<double> &A);
extern Portion *ArrayToList(const gArray<gRational> &A);

Portion* gDPVectorToList(const gDPVector<double>& A)
{
  ListPortion* p;
  ListPortion* s1;
  ListPortion* s2;
  p = new ListValPortion();
  int l = 1;
  for (int i = 1; i <= A.DPLengths().Length(); i++)  {
    s1 = new ListValPortion();
    for (int j = 1; j <= A.DPLengths()[i]; j++)  {
      s2 = new ListValPortion();
      for (int k = 1; k <= A.Lengths()[l]; k++)
	s2->Append(new FloatValPortion(A(i, j, k)));
      l++;
      s1->Append(s2);
    }
    p->Append(s1);
  }
  return p;
}

Portion* gDPVectorToList(const gDPVector<gRational>& A)
{
  ListPortion* p;
  ListPortion* s1;
  ListPortion* s2;
  p = new ListValPortion();
  int l = 1;
  for (int i = 1; i <= A.DPLengths().Length(); i++)  {
    s1 = new ListValPortion();
    for (int j = 1; j <= A.DPLengths()[i]; j++)  {
      s2 = new ListValPortion();
      for (int k = 1; k <= A.Lengths()[l]; k++)
	s2->Append(new RationalValPortion(A(i, j, k)));
      l++;
      s1->Append(s2);
    }
    p->Append(s1);
  }
  return p;
}



Portion *GSM_ActionValuesFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  Infoset *s = ((InfosetPortion *) param[1])->Value();

  if (s->BelongsTo() != bp->BelongsTo())
    return new ErrorPortion("Solution and infoset must belong to same game");
  
  if (s->GetPlayer()->IsChance())
    return new ErrorPortion("Infoset must belong to personal player");

  Efg<double> *E = bp->BelongsTo();

  gDPVector<double> values(E->Dimensionality());
  gPVector<double> probs(E->Dimensionality().Lengths());

  // E->CondPayoff(*bp, values, probs);
  bp->CondPayoff(values, probs);
  
  gVector<double> ret(s->NumActions());
  for (int i = 1; i <= s->NumActions(); i++)
    ret[i] = values(s->GetPlayer()->GetNumber(), s->GetNumber(), i);

  return ArrayToList(ret);
}

Portion *GSM_ActionValuesRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  Infoset *s = ((InfosetPortion *) param[1])->Value();

  if (s->BelongsTo() != bp->BelongsTo())
    return new ErrorPortion("Solution and infoset must belong to same game");
  
  if (s->GetPlayer()->IsChance())
    return new ErrorPortion("Infoset must belong to personal player");

  Efg<gRational> *E = bp->BelongsTo();

  gDPVector<gRational> values(E->Dimensionality());
  gPVector<gRational> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);
  
  gVector<gRational> ret(s->NumActions());
  for (int i = 1; i <= s->NumActions(); i++)
    ret[i] = values(s->GetPlayer()->GetNumber(), s->GetNumber(), i);

  return ArrayToList(ret);
}

Portion *GSM_BeliefsFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  return gDPVectorToList(bp->Beliefs());
}

Portion *GSM_BeliefsRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  return gDPVectorToList(bp->Beliefs());
}

Portion *GSM_InfosetProbsFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();

  Efg<double> *E = bp->BelongsTo();

  gDPVector<double> values(E->Dimensionality());
  gPVector<double> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);

  ListPortion *ret = new ListValPortion;

  for (int i = 1; i <= E->NumPlayers(); i++)
    ret->Append(ArrayToList(probs.GetRow(i)));

  return ret;
}

Portion *GSM_InfosetProbsRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();

  Efg<gRational> *E = bp->BelongsTo();

  gDPVector<gRational> values(E->Dimensionality());
  gPVector<gRational> probs(E->Dimensionality().Lengths());

  bp->CondPayoff(values, probs);

  ListPortion *ret = new ListValPortion;

  for (int i = 1; i <= E->NumPlayers(); i++)
    ret->Append(ArrayToList(probs.GetRow(i)));

  return ret;
}

Portion *GSM_NodeValuesFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();

  if (bp->BelongsTo() != p->BelongsTo())
    return new ErrorPortion("Solution and player are from different games");

  return ArrayToList(bp->NodeValues(p->GetNumber()));
}

Portion *GSM_NodeValuesRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();

  if (bp->BelongsTo() != p->BelongsTo())
    return new ErrorPortion("Solution and player are from different games");

  return ArrayToList(bp->NodeValues(p->GetNumber()));
}
 
Portion *GSM_RealizProbsFloat(Portion **param)
{
  BehavSolution<double> *bp = (BehavSolution<double> *) ((BehavPortion *) param[0])->Value();
  
  return ArrayToList(bp->NodeRealizProbs());
}  
  
Portion *GSM_RealizProbsRational(Portion **param)
{
  BehavSolution<gRational> *bp = (BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value();
  
  return ArrayToList(bp->NodeRealizProbs());
}
  
  
extern Portion *GSM_GobitNfg_NfgFloat(Portion **param);
extern Portion *GSM_GobitNfg_MixedFloat(Portion **param);
extern Portion *GSM_GobitLambda_MixedFloat(Portion** param);
extern Portion *GSM_GobitLambda_MixedRational(Portion** param);
extern Portion *GSM_GobitValue_MixedFloat(Portion** param);
extern Portion *GSM_GobitValue_MixedRational(Portion** param);
extern Portion *GSM_LiapNfg_NfgFloat(Portion **param);
extern Portion *GSM_LiapNfg_MixedFloat(Portion **param);
extern Portion *GSM_LemkeNfgFloat(Portion **param);
extern Portion *GSM_LemkeNfgRational(Portion **param);
extern Portion *GSM_LemkeNfgSupport(Portion **param);

void Init_algfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("GobitSolve");
  FuncObj->SetFuncInfo(GSM_GobitEfg_EfgFloat, 16);
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 0, "efg", porEFG_FLOAT, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 1, "pxifile", porTEXT,
			new TextValPortion("") );
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 2, "minLam", porFLOAT,
			new FloatValPortion(0.01));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 3, "maxLam", porFLOAT,
			new FloatValPortion(30.0));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 4, "delLam", porFLOAT,
			new FloatValPortion(0.01));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 5, "powLam", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 6, "fullGraph", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 7, "maxitsN", porINTEGER,
			new IntValPortion(20));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 8, "tolN", porFLOAT,
			new FloatValPortion(1.0e-10));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 9, "maxits1", porINTEGER,
			new IntValPortion(100));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 10, "tol1", porFLOAT,
			new FloatValPortion(2.0e-10));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 11, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 12, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 13, "nIters", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 14, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 15, "tracelevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_GobitEfg_BehavFloat, 16);
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 0, "start", porBEHAV_FLOAT);
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 1, "pxifile", porTEXT,
			new TextValPortion("") );
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 2, "minLam", porFLOAT,
			new FloatValPortion(0.01));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 3, "maxLam", porFLOAT,
			new FloatValPortion(30.0));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 4, "delLam", porFLOAT,
			new FloatValPortion(0.01));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 5, "powLam", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 6, "fullGraph", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 7, "maxitsN", porINTEGER,
			new IntValPortion(20));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 8, "tolN", porFLOAT,
			new FloatValPortion(1.0e-10));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 9, "maxits1", porINTEGER,
			new IntValPortion(100));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 10, "tol1", porFLOAT,
			new FloatValPortion(2.0e-10));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 11, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 12, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 13, "nIters", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 14, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 15, "tracelevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_GobitNfg_NfgFloat, 16);
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 0, "nfg", porNFG_FLOAT, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 1, "pxifile", porTEXT,
			new TextValPortion("") );
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 2, "minLam", porFLOAT,
			new FloatValPortion(0.01));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 3, "maxLam", porFLOAT,
			new FloatValPortion(30.0));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 4, "delLam", porFLOAT,
			new FloatValPortion(0.01));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 5, "powLam", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 6, "fullGraph", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 7, "maxitsN", porINTEGER,
			new IntValPortion(20));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 8, "tolN", porFLOAT,
			new FloatValPortion(1.0e-10));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 9, "maxits1", porINTEGER,
			new IntValPortion(100));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 10, "tol1", porFLOAT,
			new FloatValPortion(2.0e-10));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 11, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 12, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 13, "nIters", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 14, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 15, "tracelevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_GobitNfg_MixedFloat, 16);
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 0, "start", porMIXED_FLOAT);
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 1, "pxifile", porTEXT,
			new TextValPortion("") );
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 2, "minLam", porFLOAT,
			new FloatValPortion(0.01));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 3, "maxLam", porFLOAT,
			new FloatValPortion(30.0));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 4, "delLam", porFLOAT,
			new FloatValPortion(0.01));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 5, "powLam", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 6, "fullGraph", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 7, "maxitsN", porINTEGER,
			new IntValPortion(20));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 8, "tolN", porFLOAT,
			new FloatValPortion(1.0e-10));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 9, "maxits1", porINTEGER,
			new IntValPortion(100));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 10, "tol1", porFLOAT,
			new FloatValPortion(2.0e-10));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 11, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 12, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 13, "nIters", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 14, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 15, "tracelevel", porINTEGER,
			new IntValPortion(0));

  gsm->AddFunction(FuncObj);


  //------------------------- GobitLambda ------------------------------//

  FuncObj = new FuncDescObj("GobitLambda");
  FuncObj->SetFuncInfo(GSM_GobitLambda_MixedFloat, 1);
  FuncObj->SetParamInfo(GSM_GobitLambda_MixedFloat, 0, "x", porMIXED_FLOAT);
  FuncObj->SetFuncInfo(GSM_GobitLambda_MixedRational, 1);
  FuncObj->SetParamInfo(GSM_GobitLambda_MixedRational, 0, "x", 
			porMIXED_RATIONAL);
  FuncObj->SetFuncInfo(GSM_GobitLambda_BehavFloat, 1);
  FuncObj->SetParamInfo(GSM_GobitLambda_BehavFloat, 0, "x", porBEHAV_FLOAT);
  FuncObj->SetFuncInfo(GSM_GobitLambda_BehavRational, 1);
  FuncObj->SetParamInfo(GSM_GobitLambda_BehavRational, 0, "x", 
			porBEHAV_RATIONAL);
  gsm->AddFunction(FuncObj);

  //------------------------- GobitLambda ------------------------------//

  FuncObj = new FuncDescObj("GobitValue");
  FuncObj->SetFuncInfo(GSM_GobitValue_MixedFloat, 1);
  FuncObj->SetParamInfo(GSM_GobitValue_MixedFloat, 0, "x", porMIXED_FLOAT);
  FuncObj->SetFuncInfo(GSM_GobitValue_MixedRational, 1);
  FuncObj->SetParamInfo(GSM_GobitValue_MixedRational, 0, "x", 
			porMIXED_RATIONAL);
  FuncObj->SetFuncInfo(GSM_GobitValue_BehavFloat, 1);
  FuncObj->SetParamInfo(GSM_GobitValue_BehavFloat, 0, "x", porBEHAV_FLOAT);
  FuncObj->SetFuncInfo(GSM_GobitValue_BehavRational, 1);
  FuncObj->SetParamInfo(GSM_GobitValue_BehavRational, 0, "x", 
			porBEHAV_RATIONAL);
  gsm->AddFunction(FuncObj);


  //------------------------- LiapSolve ------------------------------//

  FuncObj = new FuncDescObj("LiapSolve");
  FuncObj->SetFuncInfo(GSM_LiapEfg_EfgFloat, 12);
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 0, "efg", porEFG_FLOAT, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 2, "stopAfter", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 3, "nTries", porINTEGER,
		        new IntValPortion(10));
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 4, "maxitsN", porINTEGER,
			new IntValPortion(20));
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 5, "tolN", porFLOAT,
			new FloatValPortion(1.0e-10));
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 6, "maxits1", porINTEGER,
			new IntValPortion(100));
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 7, "tol1", porFLOAT,
			new FloatValPortion(2.0e-10));
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 8, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 9, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 10, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 11, "tracelevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_LiapEfg_BehavFloat, 12);
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 0, "start", porBEHAV_FLOAT );
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 2, "stopAfter", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 3, "nTries", porINTEGER,
		        new IntValPortion(10));
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 4, "maxitsN", porINTEGER,
			new IntValPortion(20));
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 5, "tolN", porFLOAT,
			new FloatValPortion(1.0e-10));
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 6, "maxits1", porINTEGER,
			new IntValPortion(100));
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 7, "tol1", porFLOAT,
			new FloatValPortion(2.0e-10));
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 8, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 9, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 10, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 11, "tracelevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_LiapNfg_NfgFloat, 11);
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 0, "nfg", porNFG_FLOAT, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 1, "stopAfter", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 2, "nTries", porINTEGER,
		        new IntValPortion(10));
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 3, "maxitsN", porINTEGER,
			new IntValPortion(20));
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 4, "tolN", porFLOAT,
			new FloatValPortion(1.0e-10));
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 5, "maxits1", porINTEGER,
			new IntValPortion(100));
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 6, "tol1", porFLOAT,
			new FloatValPortion(2.0e-10));
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 7, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 8, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 9, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 10, "tracelevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_LiapNfg_MixedFloat, 11);
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 0, "start", porMIXED_FLOAT );
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 1, "stopAfter", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 2, "nTries", porINTEGER,
		        new IntValPortion(10));
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 3, "maxitsN", porINTEGER,
			new IntValPortion(20));
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 4, "tolN", porFLOAT,
			new FloatValPortion(1.0e-10));
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 5, "maxits1", porINTEGER,
			new IntValPortion(100));
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 6, "tol1", porFLOAT,
			new FloatValPortion(2.0e-10));
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 7, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 8, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 9, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 10, "tracelevel", porINTEGER,
			new IntValPortion(0));

  gsm->AddFunction(FuncObj);


  //------------------------- LcpSolve -------------------------//

  FuncObj = new FuncDescObj("LcpSolve");
  FuncObj->SetFuncInfo(GSM_LcpSolveEfgFloat, 7);
  FuncObj->SetParamInfo(GSM_LcpSolveEfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LcpSolveEfgFloat, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_LcpSolveEfgFloat, 2, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_LcpSolveEfgFloat, 3, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LcpSolveEfgFloat, 4, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LcpSolveEfgFloat, 5, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_LcpSolveEfgFloat, 6, "tracelevel", porINTEGER,
			new IntValPortion(0));
  
  FuncObj->SetFuncInfo(GSM_LcpSolveEfgRational, 7);
  FuncObj->SetParamInfo(GSM_LcpSolveEfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LcpSolveEfgRational, 1, "asNFG", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_LcpSolveEfgRational, 2, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_LcpSolveEfgRational, 3, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LcpSolveEfgRational, 4, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LcpSolveEfgRational, 5, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_LcpSolveEfgRational, 6, "tracelevel", porINTEGER,
			new IntValPortion(0));

/*
  FuncObj->SetFuncInfo(GSM_LemkeEfgSupport, 4);
  FuncObj->SetParamInfo(GSM_LemkeEfgSupport, 0, "support", porEF_SUPPORT );
  FuncObj->SetParamInfo(GSM_LemkeEfgSupport, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_LemkeEfgSupport, 2, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LemkeEfgSupport, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  */


  FuncObj->SetFuncInfo(GSM_LemkeNfgFloat, 6);
  FuncObj->SetParamInfo(GSM_LemkeNfgFloat, 0, "nfg",
			porNFG_FLOAT, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_LemkeNfgFloat, 1, "stopAfter", 
			porINTEGER, new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_LemkeNfgFloat, 2, "nPivots", 
			porINTEGER, new IntValPortion(0),
		        PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LemkeNfgFloat, 3, "time", 
			porFLOAT, new FloatValPortion(0),
			PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LemkeNfgFloat, 4, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_LemkeNfgFloat, 5, "tracelevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_LemkeNfgRational, 6);
  FuncObj->SetParamInfo(GSM_LemkeNfgRational, 0, "nfg",
			porNFG_RATIONAL, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_LemkeNfgRational, 1, "stopAfter", 
			porINTEGER, new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_LemkeNfgRational, 2, "nPivots", 
			porINTEGER, new IntValPortion(0),
		        PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LemkeNfgRational, 3, "time", 
			porFLOAT, new FloatValPortion(0),
			PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LemkeNfgRational, 4, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_LemkeNfgRational, 5, "tracelevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_LemkeNfgSupport, 6);
  FuncObj->SetParamInfo(GSM_LemkeNfgSupport, 0, "support", porNF_SUPPORT );
  FuncObj->SetParamInfo(GSM_LemkeNfgSupport, 1, "stopAfter", 
			porINTEGER, new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_LemkeNfgSupport, 2, "nPivots", 
			porINTEGER, new IntValPortion(0),
		        PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LemkeNfgSupport, 3, "time", 
			porFLOAT, new FloatValPortion(0),
			PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LemkeNfgSupport, 4, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_LemkeNfgSupport, 5, "tracelevel", porINTEGER,
			new IntValPortion(0));
  gsm->AddFunction(FuncObj);

  //------------------------- SimpDivSolve -------------------------//

  FuncObj = new FuncDescObj("SimpDivSolve");
  FuncObj->SetFuncInfo(GSM_SimpdivEfgFloat, 9);
  FuncObj->SetParamInfo(GSM_SimpdivEfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SimpdivEfgFloat, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_SimpdivEfgFloat, 2, "stopAfter", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_SimpdivEfgFloat, 3, "nRestarts", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_SimpdivEfgFloat, 4, "leashLength", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_SimpdivEfgFloat, 5, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SimpdivEfgFloat, 6, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SimpdivEfgFloat, 7, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_SimpdivEfgFloat, 8, "tracelevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_SimpdivEfgRational, 9);
  FuncObj->SetParamInfo(GSM_SimpdivEfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SimpdivEfgRational, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_SimpdivEfgRational, 2, "stopAfter", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_SimpdivEfgRational, 3, "nRestarts", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_SimpdivEfgRational, 4, "leashLength", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_SimpdivEfgRational, 5, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SimpdivEfgRational, 6, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SimpdivEfgRational, 7, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_SimpdivEfgRational, 8, "tracelevel", porINTEGER,
			new IntValPortion(0));
  gsm->AddFunction(FuncObj);

  //-------------------------- LpSolve ------------------------------//

  FuncObj = new FuncDescObj("LpSolve");
  FuncObj->SetFuncInfo(GSM_LpSolveEfgFloat, 5);
  FuncObj->SetParamInfo(GSM_LpSolveEfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LpSolveEfgFloat, 1, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LpSolveEfgFloat, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LpSolveEfgFloat, 3, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_LpSolveEfgFloat, 4, "tracelevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_LpSolveEfgRational, 5);
  FuncObj->SetParamInfo(GSM_LpSolveEfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LpSolveEfgRational, 1, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LpSolveEfgRational, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LpSolveEfgRational, 3, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_LpSolveEfgRational, 4, "tracelevel", porINTEGER,
			new IntValPortion(0));
  gsm->AddFunction(FuncObj);

  //-------------------------EnumMixedSolve-------------------------

  FuncObj = new FuncDescObj("EnumMixedSolve");
  FuncObj->SetFuncInfo(GSM_EnumMixedEfgFloat, 7);
  FuncObj->SetParamInfo(GSM_EnumMixedEfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_EnumMixedEfgFloat, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_EnumMixedEfgFloat, 2, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumMixedEfgFloat, 3, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixedEfgFloat, 4, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixedEfgFloat, 5, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_EnumMixedEfgFloat, 6, "tracelevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_EnumMixedEfgRational, 7);
  FuncObj->SetParamInfo(GSM_EnumMixedEfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_EnumMixedEfgRational, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_EnumMixedEfgRational, 2, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumMixedEfgRational, 3, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixedEfgRational, 4, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumMixedEfgRational, 5, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_EnumMixedEfgRational, 6, "tracelevel", porINTEGER,
			new IntValPortion(0));
  gsm->AddFunction(FuncObj);

  //-------------------------- EnumPureSolve ------------------------//

  FuncObj = new FuncDescObj("EnumPureSolve");
  FuncObj->SetFuncInfo(GSM_EnumPureEfgFloat, 6);
  FuncObj->SetParamInfo(GSM_EnumPureEfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumPureEfgFloat, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_EnumPureEfgFloat, 2, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumPureEfgFloat, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumPureEfgFloat, 4, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_EnumPureEfgFloat, 5, "tracelevel", porINTEGER,
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_EnumPureEfgRational, 6);
  FuncObj->SetParamInfo(GSM_EnumPureEfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumPureEfgRational, 1, "asNfg", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_EnumPureEfgRational, 2, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumPureEfgRational, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumPureEfgRational, 4, "tracefile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_EnumPureEfgRational, 5, "tracelevel", porINTEGER,
			new IntValPortion(0));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Behav");
  FuncObj->SetFuncInfo(GSM_BehavFloat, 2);
  FuncObj->SetParamInfo(GSM_BehavFloat, 0, "mixed", porMIXED_FLOAT);
  FuncObj->SetParamInfo(GSM_BehavFloat, 1, "efg", porEFG_FLOAT,
		        NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_BehavRational, 2);
  FuncObj->SetParamInfo(GSM_BehavRational, 0, "mixed", porMIXED_RATIONAL);
  FuncObj->SetParamInfo(GSM_BehavRational, 1, "efg", porEFG_RATIONAL,
		        NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Afg");
  FuncObj->SetFuncInfo(GSM_AfgFloat, 2);
  FuncObj->SetParamInfo(GSM_AfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_AfgFloat, 1, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  
  FuncObj->SetFuncInfo(GSM_AfgRational, 2);
  FuncObj->SetParamInfo(GSM_AfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_AfgRational, 1, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Nfg");
  FuncObj->SetFuncInfo(GSM_NfgFloat, 2);
  FuncObj->SetParamInfo(GSM_NfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_NfgFloat, 1, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_NfgRational, 2);
  FuncObj->SetParamInfo(GSM_NfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_NfgRational, 1, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);


  //------------------------- Payoff -----------------------------//

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
			1, "list", porLIST | porINTEGER );
  FuncObj->SetFuncInfo(GSM_Payoff_NfgRational, 2);
  FuncObj->SetParamInfo(GSM_Payoff_NfgRational, 
			0, "nfg", porNFG_RATIONAL, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Payoff_NfgRational,
			1, "list", porLIST | porINTEGER );
  gsm->AddFunction(FuncObj);


  //-------------------------- SetPayoff ---------------------------//

  FuncObj = new FuncDescObj( "SetPayoff" );
  FuncObj->SetFuncInfo(GSM_SetPayoff_NfgFloat, 3);
  FuncObj->SetParamInfo(GSM_SetPayoff_NfgFloat, 
			0, "nfg", porNFG_FLOAT, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SetPayoff_NfgFloat,
			1, "list", porLIST | porINTEGER );
  FuncObj->SetParamInfo(GSM_SetPayoff_NfgFloat,
			2, "payoff", porLIST | porFLOAT );

  FuncObj->SetFuncInfo(GSM_SetPayoff_NfgRational, 3);
  FuncObj->SetParamInfo(GSM_SetPayoff_NfgRational, 
			0, "nfg", porNFG_RATIONAL, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SetPayoff_NfgRational,
			1, "list", porLIST | porINTEGER );
  FuncObj->SetParamInfo(GSM_SetPayoff_NfgRational,
			2, "payoff", porLIST | porRATIONAL );
  gsm->AddFunction(FuncObj);




  FuncObj = new FuncDescObj("ActionValues");
  FuncObj->SetFuncInfo(GSM_ActionValuesFloat, 2);
  FuncObj->SetParamInfo(GSM_ActionValuesFloat, 0, "strategy",
			porBEHAV_FLOAT);
  FuncObj->SetParamInfo(GSM_ActionValuesFloat, 1, "infoset", porINFOSET);

  FuncObj->SetFuncInfo(GSM_ActionValuesRational, 2);
  FuncObj->SetParamInfo(GSM_ActionValuesRational, 0, "strategy",
			porBEHAV_RATIONAL);
  FuncObj->SetParamInfo(GSM_ActionValuesRational, 1, "infoset", porINFOSET);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Beliefs");
  FuncObj->SetFuncInfo(GSM_BeliefsFloat, 1);
  FuncObj->SetParamInfo(GSM_BeliefsFloat, 0, "strategy", 
			porBEHAV_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_BeliefsRational, 1);
  FuncObj->SetParamInfo(GSM_BeliefsRational, 0, "strategy",
			porBEHAV_RATIONAL, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("InfosetProbs");
  FuncObj->SetFuncInfo(GSM_InfosetProbsFloat, 1);
  FuncObj->SetParamInfo(GSM_InfosetProbsFloat, 0, "strategy", porBEHAV_FLOAT);

  FuncObj->SetFuncInfo(GSM_InfosetProbsRational, 1);
  FuncObj->SetParamInfo(GSM_InfosetProbsRational, 0, "strategy",
			porBEHAV_RATIONAL);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NodeValues");
  FuncObj->SetFuncInfo(GSM_NodeValuesFloat, 2);
  FuncObj->SetParamInfo(GSM_NodeValuesFloat, 0, "strategy", porBEHAV_FLOAT);
  FuncObj->SetParamInfo(GSM_NodeValuesFloat, 1, "player", porPLAYER_EFG);

  FuncObj->SetFuncInfo(GSM_NodeValuesRational, 2);
  FuncObj->SetParamInfo(GSM_NodeValuesRational, 0, "strategy",
			porBEHAV_RATIONAL);
  FuncObj->SetParamInfo(GSM_NodeValuesRational, 1, "player", porPLAYER_EFG);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RealizProbs");
  FuncObj->SetFuncInfo(GSM_RealizProbsFloat, 1);
  FuncObj->SetParamInfo(GSM_RealizProbsFloat, 0, "strategy", porBEHAV_FLOAT);

  FuncObj->SetFuncInfo(GSM_RealizProbsRational, 1);
  FuncObj->SetParamInfo(GSM_RealizProbsRational, 0, "strategy",
			porBEHAV_RATIONAL);
  gsm->AddFunction(FuncObj);

}

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class Behav_ListPortion<double>;
TEMPLATE class Behav_ListPortion<gRational>;


