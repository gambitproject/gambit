//#
//# FILE: algfunc.cc -- Solution algorithm functions for GCL
//#
//# $Id$
//#

#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "egobit.h"
#include "eliap.h"

#include "rational.h"

#include "gwatch.h"
#include "mixed.h"




extern double Funct_tolBrent;
extern double Funct_tolN;
extern int Funct_maxitsBrent;
extern int Funct_maxitsN;



Portion *GSM_BehavFloat(Portion **param)
{
  MixedProfile<double> &mp = * (MixedProfile<double>*) ((MixedPortion*) param[0])->Value();
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[1])->Value();
  Nfg<double> &N = *mp.BelongsTo(); 

  BehavProfile<double>* bp;

  if( AssociatedNfg( &E ) != &N )  
    return new ErrorPortion("Normal and extensive form games not associated");
  else
  {
    bp = new BehavProfile<double>(E);
    MixedToBehav(N, mp, E, *bp);
  }

  Portion* por = new BehavValPortion(bp);
  por->SetOwner( param[ 1 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_BehavRational(Portion **param)
{
  MixedProfile<gRational> &mp = 
    * (MixedProfile<gRational>*) ((MixedPortion*) param[0])->Value();
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[1])->Value();
  Nfg<gRational> &N = *mp.BelongsTo(); 

  BehavProfile<gRational>* bp;

  if( AssociatedNfg( &E ) != &N )  
    return new ErrorPortion("Normal and extensive form games not associated");
  else
  {
    bp = new BehavProfile<gRational>(E);
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
  BehavProfile<double>* bp = 
    (BehavProfile<double>*) ((BehavPortion*) param[0])->Value();
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
  BehavProfile<gRational>* bp = 
    (BehavProfile<gRational>*) ((BehavPortion*) param[0])->Value();
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
  MixedProfile<double>* bp = 
    (MixedProfile<double>*) ((MixedPortion*) param[0])->Value();
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
  MixedProfile<gRational>* bp = 
    (MixedProfile<gRational>*) ((MixedPortion*) param[0])->Value();
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
  gArray<int> profile( ( (ListPortion*) param[ 1 ] )->Length() );
  
  if( ( (ListPortion*) param[ 1 ] )->Length() != nfg->NumPlayers() )
    return new ErrorPortion("Invalid number of players specified in \"list\"");
  
  for( i = 1; i <= nfg->NumPlayers() ; i++ )
  {
    p = ( (ListPortion*) param[ 1 ] )->Subscript( i );
    assert( p->Type() == porINTEGER );
    profile[ i ] = ( (IntPortion*) p )->Value();
    delete p;
  }
  for( i = 1; i <= nfg->NumPlayers(); i++ )
  {
    ( (ListValPortion*) por )->
      Append( new FloatValPortion( nfg->Payoff( i, profile ) ) );
  }
  return por;
}

Portion* GSM_Payoff_NfgRational( Portion** param )
{
  int i;
  Portion* p;
  Portion* por = new ListValPortion;
  Nfg<gRational>* nfg = (Nfg<gRational>*) ( (NfgPortion*) param[0] )->Value();
  gArray<int> profile( ( (ListPortion*) param[ 1 ] )->Length() );
  
  if( ( (ListPortion*) param[ 1 ] )->Length() != nfg->NumPlayers() )
    return new ErrorPortion("Invalid number of players specified in \"list\"");
  
  for( i = 1; i <= nfg->NumPlayers() ; i++ )
  {
    p = ( (ListPortion*) param[ 1 ] )->Subscript( i );
    assert( p->Type() == porINTEGER );
    profile[ i ] = ( (IntPortion*) p )->Value();
    delete p;
  }
  for( i = 1; i <= nfg->NumPlayers(); i++ )
  {
    ( (ListValPortion*) por )->
      Append( new RationalValPortion( nfg->Payoff( i, profile ) ) );
  }
  return por;
}



//-------------------------------- SetPayoff ---------------------------//

Portion* GSM_SetPayoff_NfgFloat( Portion** param )
{
  int i;
  Portion* p;
  Nfg<double>* nfg = (Nfg<double>*) ( (NfgPortion*) param[0] )->Value();
  gArray<int> profile( ( (ListPortion*) param[ 1 ] )->Length() );
  
  if( ( (ListPortion*) param[ 1 ] )->Length() != nfg->NumPlayers() )
    return new ErrorPortion("Invalid number of players specified in \"list\"");
  if( ( (ListPortion*) param[ 2 ] )->Length() != nfg->NumPlayers() )
    return new ErrorPortion("Invalid number of players specified in \"payoff\"");
  
  for( i = 1; i <= nfg->NumPlayers() ; i++ )
  {
    p = ( (ListPortion*) param[ 1 ] )->Subscript( i );
    assert( p->Type() == porINTEGER );
    profile[ i ] = ( (IntPortion*) p )->Value();
    delete p;
  }
  for( i = 1; i <= nfg->NumPlayers(); i++ )
  {
    p = ( (ListPortion*) param[ 2 ] )->Subscript( i );
    assert( p->Type() == porFLOAT );
    nfg->SetPayoff( i, profile, ( (FloatPortion*) p )->Value() );
    delete p;
  }
  return param[ 1 ]->ValCopy();
}

Portion* GSM_SetPayoff_NfgRational( Portion** param )
{
  int i;
  Portion* p;
  Nfg<gRational>* nfg = (Nfg<gRational>*) ( (NfgPortion*) param[0] )->Value();
  gArray<int> profile( ( (ListPortion*) param[ 1 ] )->Length() );
  
  if( ( (ListPortion*) param[ 1 ] )->Length() != nfg->NumPlayers() )
    return new ErrorPortion("Invalid number of players specified in \"list\"");
  if( ( (ListPortion*) param[ 2 ] )->Length() != nfg->NumPlayers() )
    return new ErrorPortion("Invalid number of players specified in \"payoff\"");
  
  for( i = 1; i <= nfg->NumPlayers() ; i++ )
  {
    p = ( (ListPortion*) param[ 1 ] )->Subscript( i );
    assert( p->Type() == porINTEGER );
    profile[ i ] = ( (IntPortion*) p )->Value();
    delete p;
  }
  for( i = 1; i <= nfg->NumPlayers(); i++ )
  {
    p = ( (ListPortion*) param[ 2 ] )->Subscript( i );
    assert( p->Type() == porRATIONAL );
    nfg->SetPayoff( i, profile, ( (RationalPortion*) p )->Value() );
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

template <class T> class Behav_ListPortion : public ListValPortion   {
  public:
    Behav_ListPortion(const gList<BehavProfile<T> > &);
    virtual ~Behav_ListPortion()   { }
};

Behav_ListPortion<double>::Behav_ListPortion(
			   const gList<BehavProfile<double> > &list)
{
  _DataType = porBEHAV_FLOAT;
  for (int i = 1; i <= list.Length(); i++)
    Append( new BehavValPortion( new BehavProfile<double>(list[i])));
}

Behav_ListPortion<gRational>::Behav_ListPortion(
			      const gList<BehavProfile<gRational> > &list)
{
  _DataType = porBEHAV_RATIONAL;
  for (int i = 1; i <= list.Length(); i++)
    Append( new BehavValPortion( new BehavProfile<gRational>(list[i])));
}



//--------------------------- GobitSolve -----------------------------//

Portion *GSM_GobitEfg_EfgFloat(Portion **param)
{
  int old_Funct_maxitsN = Funct_maxitsN;
  double old_Funct_tolN = Funct_tolN;
  int old_Funct_maxitsBrent = Funct_maxitsBrent;
  double old_Funct_tolBrent = Funct_tolBrent;

  Efg<double> &E = *(Efg<double> *) ((EfgPortion *) param[0])->Value();
  BehavProfile<double> start(E);
  
  EFGobitParams<double> EP;
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
  
  EFGobitModule<double> M(E, EP, start);
  M.Gobit(1);

  ((FloatPortion *) param[11])->Value() = M.Time();
  ((IntPortion *) param[12])->Value() = M.NumEvals();
  ((IntPortion *) param[13])->Value() = M.NumIters();

  Funct_maxitsN = old_Funct_maxitsN;
  Funct_tolN = old_Funct_tolN;
  Funct_maxitsBrent = old_Funct_maxitsBrent;
  Funct_tolBrent = old_Funct_tolBrent;

  Portion* por = new Behav_ListPortion<double>(M.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}


Portion *GSM_GobitEfg_BehavFloat(Portion **param)
{
  int old_Funct_maxitsN = Funct_maxitsN;
  double old_Funct_tolN = Funct_tolN;
  int old_Funct_maxitsBrent = Funct_maxitsBrent;
  double old_Funct_tolBrent = Funct_tolBrent;

  BehavProfile<double>& start = 
    * (BehavProfile<double> *) ((BehavPortion *) param[0])->Value();
  Efg<double> &E = *( start.BelongsTo() );
  
  EFGobitParams<double> EP;
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
  
  EFGobitModule<double> M(E, EP, start);
  M.Gobit(1);

  ((FloatPortion *) param[11])->Value() = M.Time();
  ((IntPortion *) param[12])->Value() = M.NumEvals();
  ((IntPortion *) param[13])->Value() = M.NumIters();

  Funct_maxitsN = old_Funct_maxitsN;
  Funct_tolN = old_Funct_tolN;
  Funct_maxitsBrent = old_Funct_maxitsBrent;
  Funct_tolBrent = old_Funct_tolBrent;

  Portion* por = new Behav_ListPortion<double>(M.GetSolutions());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}









//-------------------------- LiapSolve ---------------------------//

Portion *GSM_LiapEfg_EfgFloat(Portion **param)
{
  int old_Funct_maxitsN = Funct_maxitsN;
  double old_Funct_tolN = Funct_tolN;
  int old_Funct_maxitsBrent = Funct_maxitsBrent;
  double old_Funct_tolBrent = Funct_tolBrent;

  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();
  BehavProfile<double> start(E);

  EFLiapParams<double> LP;

  LP.stopAfter = ((IntPortion *) param[1])->Value();
  LP.nTries = ((IntPortion *) param[2])->Value();
 
  Funct_maxitsN = ( (IntPortion*) param[3] )->Value();
  Funct_tolN = ( (FloatPortion*) param[4] )->Value();
  Funct_maxitsBrent = ( (IntPortion*) param[5] )->Value();
  Funct_tolBrent = ( (FloatPortion*) param[6] )->Value();

  EFLiapModule<double> LM(E, LP, start);
  LM.Liap();

  ((FloatPortion *) param[7])->Value() = LM.Time();
  ((IntPortion *) param[8])->Value() = LM.NumEvals();

  Funct_maxitsN = old_Funct_maxitsN;
  Funct_tolN = old_Funct_tolN;
  Funct_maxitsBrent = old_Funct_maxitsBrent;
  Funct_tolBrent = old_Funct_tolBrent;

  Portion* por = new Behav_ListPortion<double>(LM.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_LiapEfg_BehavFloat(Portion **param)
{
  int old_Funct_maxitsN = Funct_maxitsN;
  double old_Funct_tolN = Funct_tolN;
  int old_Funct_maxitsBrent = Funct_maxitsBrent;
  double old_Funct_tolBrent = Funct_tolBrent;

  BehavProfile<double> &start = 
    * (BehavProfile<double> *) ((BehavPortion *) param[0])->Value();
  Efg<double> &E = *( start.BelongsTo() );

  EFLiapParams<double> LP;

  LP.stopAfter = ((IntPortion *) param[1])->Value();
  LP.nTries = ((IntPortion *) param[2])->Value();
 
  Funct_maxitsN = ( (IntPortion*) param[3] )->Value();
  Funct_tolN = ( (FloatPortion*) param[4] )->Value();
  Funct_maxitsBrent = ( (IntPortion*) param[5] )->Value();
  Funct_tolBrent = ( (FloatPortion*) param[6] )->Value();

  EFLiapModule<double> LM(E, LP, start);
  LM.Liap();

  ((FloatPortion *) param[7])->Value() = LM.Time();
  ((IntPortion *) param[8])->Value() = LM.NumEvals();

  Funct_maxitsN = old_Funct_maxitsN;
  Funct_tolN = old_Funct_tolN;
  Funct_maxitsBrent = old_Funct_maxitsBrent;
  Funct_tolBrent = old_Funct_tolBrent;

  Portion* por = new Behav_ListPortion<double>(LM.GetSolutions());
  por->SetOwner( param[ 0 ]->Owner() );
  por->AddDependency();
  return por;
}




//-------------------------------- LcpSolve -------------------------//

#include "seqform.h"

Portion *GSM_LemkeEfgFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();

  SeqFormParams SP;
  SP.nequilib = ((IntPortion *) param[1])->Value();

  EFSupport SUP(E);
  SeqFormModule<double> SM(E, SP, SUP);
  SM.Lemke();

  ((IntPortion *) param[2])->Value() = SM.NumPivots();
  ((FloatPortion *) param[3])->Value() = SM.Time();
  
  Portion* por = new Behav_ListPortion<double>(SM.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_LemkeEfgRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();

  SeqFormParams SP;
  SP.nequilib = ((IntPortion *) param[1])->Value();
  EFSupport SUP(E);
  SeqFormModule<gRational> SM(E, SP, SUP);
  SM.Lemke();
  
  ((IntPortion *) param[2])->Value() = SM.NumPivots();
  ((FloatPortion *) param[3])->Value() = SM.Time();

  Portion* por = new Behav_ListPortion<gRational>(SM.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}


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








Portion *GSM_SetFloatOptions(Portion **param)
{
  gString alg = ((TextPortion *) param[0])->Value();
  gString par = ((TextPortion *) param[1])->Value();
  double value = ((FloatPortion *) param[2])->Value();
  
  if (alg == "Gobit")   {
    if (par == "minLam")           Gobit_default_minLam = value;
    else if (par == "maxLam")      Gobit_default_maxLam = value;
    else if (par == "delLam")      Gobit_default_delLam = value;
    else return 0;
    return new FloatValPortion(value);
  }
  else if (alg == "FuncMin")  {
    if (par == "tolBrent")         Funct_tolBrent = value;
    else if (par == "tolN")        Funct_tolN = value;
    else return 0;
    return new FloatValPortion(value);
  }
  else
    return 0;
}

Portion *GSM_SetIntegerOptions(Portion **param)
{
  gString alg = ((TextPortion *) param[0])->Value();
  gString par = ((TextPortion *) param[1])->Value();
  int value = ((IntPortion *) param[2])->Value();
  
  if (alg == "Gobit")   {
    if (par == "powLam")           Gobit_default_powLam = value;
    else return 0;
    return new IntValPortion(value);
  }
  else if (alg == "FuncMin")  {
    if (par == "maxitsBrent")      Funct_maxitsBrent = value;
    else if (par == "maxitsN")     Funct_maxitsN = value;
    else return 0;
    return new IntValPortion(value);
  }
  else
    return 0;
}

extern Portion *ArrayToList(const gArray<double> &A);
extern Portion *ArrayToList(const gArray<gRational> &A);

Portion *GSM_ActionValuesFloat(Portion **param)
{
  BehavProfile<double> *bp = (BehavProfile<double> *) ((BehavPortion *) param[0])->Value();
  Infoset *s = ((InfosetPortion *) param[1])->Value();

  if (s->BelongsTo() != bp->BelongsTo())
    return new ErrorPortion("Profile and infoset must belong to same game");
  
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
  BehavProfile<gRational> *bp = (BehavProfile<gRational> *) ((BehavPortion *) param[0])->Value();
  Infoset *s = ((InfosetPortion *) param[1])->Value();

  if (s->BelongsTo() != bp->BelongsTo())
    return new ErrorPortion("Profile and infoset must belong to same game");
  
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
  BehavProfile<double> *bp = (BehavProfile<double> *) ((BehavPortion *) param[0])->Value();

  return ArrayToList(bp->Beliefs());
}

Portion *GSM_BeliefsRational(Portion **param)
{
  BehavProfile<gRational> *bp = (BehavProfile<gRational> *) ((BehavPortion *) param[0])->Value();

  return ArrayToList(bp->Beliefs());
}

Portion *GSM_InfosetProbsFloat(Portion **param)
{
  BehavProfile<double> *bp = (BehavProfile<double> *) ((BehavPortion *) param[0])->Value();

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
  BehavProfile<gRational> *bp = (BehavProfile<gRational> *) ((BehavPortion *) param[0])->Value();

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
  BehavProfile<double> *bp = (BehavProfile<double> *) ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();

  if (bp->BelongsTo() != p->BelongsTo())
    return new ErrorPortion("Profile and player are from different games");

  return ArrayToList(bp->NodeValues(p->GetNumber()));
}

Portion *GSM_NodeValuesRational(Portion **param)
{
  BehavProfile<gRational> *bp = (BehavProfile<gRational> *) ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();

  if (bp->BelongsTo() != p->BelongsTo())
    return new ErrorPortion("Profile and player are from different games");

  return ArrayToList(bp->NodeValues(p->GetNumber()));
}
 
Portion *GSM_RealizProbsFloat(Portion **param)
{
  BehavProfile<double> *bp = (BehavProfile<double> *) ((BehavPortion *) param[0])->Value();
  
  return ArrayToList(bp->NodeRealizProbs());
}  
  
Portion *GSM_RealizProbsRational(Portion **param)
{
  BehavProfile<gRational> *bp = (BehavProfile<gRational> *) ((BehavPortion *) param[0])->Value();
  
  return ArrayToList(bp->NodeRealizProbs());
}
  
  
extern Portion *GSM_GobitNfg_NfgFloat(Portion **param);
extern Portion *GSM_GobitNfg_MixedFloat(Portion **param);
extern Portion *GSM_LiapNfg_NfgFloat(Portion **param);
extern Portion *GSM_LiapNfg_MixedFloat(Portion **param);
extern Portion *GSM_LemkeNfgFloat(Portion **param);
extern Portion *GSM_LemkeNfgRational(Portion **param);
extern Portion *GSM_LemkeNfgSupport(Portion **param);

void Init_algfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("GobitSolve");
  FuncObj->SetFuncInfo(GSM_GobitEfg_EfgFloat, 14);
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 0, "efg", porEFG_FLOAT, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 1, "pxifile", porTEXT,
			new TextValPortion("") );
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 2, "minLam", porFLOAT,
			new FloatRefPortion(Gobit_default_minLam));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 3, "maxLam", porFLOAT,
			new FloatRefPortion(Gobit_default_maxLam));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 4, "delLam", porFLOAT,
			new FloatRefPortion(Gobit_default_delLam));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 5, "powLam", porINTEGER,
		        new IntRefPortion( (long&) Gobit_default_powLam));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 6, "fullGraph", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 7, "maxitsN", porINTEGER,
			new IntRefPortion( (long&) Funct_maxitsN ));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 8, "tolN", porFLOAT,
			new FloatRefPortion( Funct_tolN ));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 9, "maxits1", porINTEGER,
			new IntRefPortion( (long&) Funct_maxitsBrent ));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 10, "tol1", porFLOAT,
			new FloatRefPortion( Funct_tolBrent ));
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 11, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 12, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitEfg_EfgFloat, 13, "nIters", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_GobitEfg_BehavFloat, 14);
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 0, "start", porBEHAV_FLOAT);
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 1, "pxifile", porTEXT,
			new TextValPortion("") );
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 2, "minLam", porFLOAT,
			new FloatRefPortion(Gobit_default_minLam));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 3, "maxLam", porFLOAT,
			new FloatRefPortion(Gobit_default_maxLam));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 4, "delLam", porFLOAT,
			new FloatRefPortion(Gobit_default_delLam));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 5, "powLam", porINTEGER,
		        new IntRefPortion( (long&) Gobit_default_powLam));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 6, "fullGraph", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 7, "maxitsN", porINTEGER,
			new IntRefPortion( (long&) Funct_maxitsN ));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 8, "tolN", porFLOAT,
			new FloatRefPortion( Funct_tolN ));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 9, "maxits1", porINTEGER,
			new IntRefPortion( (long&) Funct_maxitsBrent ));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 10, "tol1", porFLOAT,
			new FloatRefPortion( Funct_tolBrent ));
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 11, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 12, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitEfg_BehavFloat, 13, "nIters", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);


  FuncObj->SetFuncInfo(GSM_GobitNfg_NfgFloat, 14);
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 0, "nfg", porNFG_FLOAT, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 1, "pxifile", porTEXT,
			new TextValPortion("") );
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 2, "minLam", porFLOAT,
			new FloatRefPortion(Gobit_default_minLam));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 3, "maxLam", porFLOAT,
			new FloatRefPortion(Gobit_default_maxLam));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 4, "delLam", porFLOAT,
			new FloatRefPortion(Gobit_default_delLam));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 5, "powLam", porINTEGER,
		        new IntRefPortion( (long&) Gobit_default_powLam));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 6, "fullGraph", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 7, "maxitsN", porINTEGER,
			new IntRefPortion( (long&) Funct_maxitsN ));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 8, "tolN", porFLOAT,
			new FloatRefPortion( Funct_tolN ));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 9, "maxits1", porINTEGER,
			new IntRefPortion( (long&) Funct_maxitsBrent ));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 10, "tol1", porFLOAT,
			new FloatRefPortion( Funct_tolBrent ));
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 11, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 12, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitNfg_NfgFloat, 13, "nIters", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);


  FuncObj->SetFuncInfo(GSM_GobitNfg_MixedFloat, 14);
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 0, "start", porMIXED_FLOAT);
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 1, "pxifile", porTEXT,
			new TextValPortion("") );
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 2, "minLam", porFLOAT,
			new FloatRefPortion(Gobit_default_minLam));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 3, "maxLam", porFLOAT,
			new FloatRefPortion(Gobit_default_maxLam));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 4, "delLam", porFLOAT,
			new FloatRefPortion(Gobit_default_delLam));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 5, "powLam", porINTEGER,
		        new IntRefPortion( (long&) Gobit_default_powLam));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 6, "fullGraph", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 7, "maxitsN", porINTEGER,
			new IntRefPortion( (long&) Funct_maxitsN ));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 8, "tolN", porFLOAT,
			new FloatRefPortion( Funct_tolN ));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 9, "maxits1", porINTEGER,
			new IntRefPortion( (long&) Funct_maxitsBrent ));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 10, "tol1", porFLOAT,
			new FloatRefPortion( Funct_tolBrent ));
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 11, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 12, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitNfg_MixedFloat, 13, "nIters", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);

  gsm->AddFunction(FuncObj);


  //------------------------- LiapSolve ------------------------------//

  FuncObj = new FuncDescObj("LiapSolve");
  FuncObj->SetFuncInfo(GSM_LiapEfg_EfgFloat, 9);
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 0, "efg", porEFG_FLOAT, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 1, "stopAfter", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 2, "nTries", porINTEGER,
		        new IntValPortion(10));
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 3, "maxitsN", porINTEGER,
			new IntRefPortion( (long&) Funct_maxitsN ));
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 4, "tolN", porFLOAT,
			new FloatRefPortion( Funct_tolN ));
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 5, "maxits1", porINTEGER,
			new IntRefPortion( (long&) Funct_maxitsBrent ));
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 6, "tol1", porFLOAT,
			new FloatRefPortion( Funct_tolBrent ));
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 7, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapEfg_EfgFloat, 8, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_LiapEfg_BehavFloat, 9);
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 0, "start", porBEHAV_FLOAT );
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 1, "stopAfter", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 2, "nTries", porINTEGER,
		        new IntValPortion(10));
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 3, "maxitsN", porINTEGER,
			new IntRefPortion( (long&) Funct_maxitsN ));
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 4, "tolN", porFLOAT,
			new FloatRefPortion( Funct_tolN ));
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 5, "maxits1", porINTEGER,
			new IntRefPortion( (long&) Funct_maxitsBrent ));
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 6, "tol1", porFLOAT,
			new FloatRefPortion( Funct_tolBrent ));
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 7, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapEfg_BehavFloat, 8, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);


  FuncObj->SetFuncInfo(GSM_LiapNfg_NfgFloat, 9);
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 0, "nfg", porNFG_FLOAT, 
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 1, "stopAfter", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 2, "nTries", porINTEGER,
		        new IntValPortion(10));
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 3, "maxitsN", porINTEGER,
			new IntRefPortion( (long&) Funct_maxitsN ));
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 4, "tolN", porFLOAT,
			new FloatRefPortion( Funct_tolN ));
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 5, "maxits1", porINTEGER,
			new IntRefPortion( (long&) Funct_maxitsBrent ));
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 6, "tol1", porFLOAT,
			new FloatRefPortion( Funct_tolBrent ));
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 7, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapNfg_NfgFloat, 8, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_LiapNfg_MixedFloat, 9);
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 0, "start", porMIXED_FLOAT );
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 1, "stopAfter", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 2, "nTries", porINTEGER,
		        new IntValPortion(10));
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 3, "maxitsN", porINTEGER,
			new IntRefPortion( (long&) Funct_maxitsN ));
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 4, "tolN", porFLOAT,
			new FloatRefPortion( Funct_tolN ));
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 5, "maxits1", porINTEGER,
			new IntRefPortion( (long&) Funct_maxitsBrent ));
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 6, "tol1", porFLOAT,
			new FloatRefPortion( Funct_tolBrent ));
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 7, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapNfg_MixedFloat, 8, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);

  gsm->AddFunction(FuncObj);


  //------------------------- LcpSolve -------------------------//

  FuncObj = new FuncDescObj("LcpSolve");
  FuncObj->SetFuncInfo(GSM_LemkeEfgFloat, 4);
  FuncObj->SetParamInfo(GSM_LemkeEfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LemkeEfgFloat, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_LemkeEfgFloat, 2, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LemkeEfgFloat, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  
  FuncObj->SetFuncInfo(GSM_LemkeEfgRational, 4);
  FuncObj->SetParamInfo(GSM_LemkeEfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LemkeEfgRational, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_LemkeEfgRational, 2, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LemkeEfgRational, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_LemkeEfgSupport, 4);
  FuncObj->SetParamInfo(GSM_LemkeEfgSupport, 0, "support", porEF_SUPPORT );
  FuncObj->SetParamInfo(GSM_LemkeEfgSupport, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_LemkeEfgSupport, 2, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LemkeEfgSupport, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_LemkeNfgFloat, 4);
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

  FuncObj->SetFuncInfo(GSM_LemkeNfgRational, 4);
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

  FuncObj->SetFuncInfo(GSM_LemkeNfgSupport, 4);
  FuncObj->SetParamInfo(GSM_LemkeNfgSupport, 0, "support", porNF_SUPPORT );
  FuncObj->SetParamInfo(GSM_LemkeNfgSupport, 1, "stopAfter", 
			porINTEGER, new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_LemkeNfgSupport, 2, "nPivots", 
			porINTEGER, new IntValPortion(0),
		        PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LemkeNfgSupport, 3, "time", 
			porFLOAT, new FloatValPortion(0),
			PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetOptions");
  FuncObj->SetFuncInfo(GSM_SetFloatOptions, 3);
  FuncObj->SetParamInfo(GSM_SetFloatOptions, 0, "alg", porTEXT);
  FuncObj->SetParamInfo(GSM_SetFloatOptions, 1, "param", porTEXT);
  FuncObj->SetParamInfo(GSM_SetFloatOptions, 2, "value", porFLOAT);

  FuncObj->SetFuncInfo(GSM_SetIntegerOptions, 3);
  FuncObj->SetParamInfo(GSM_SetIntegerOptions, 0, "alg", porTEXT);
  FuncObj->SetParamInfo(GSM_SetIntegerOptions, 1, "param", porTEXT);
  FuncObj->SetParamInfo(GSM_SetIntegerOptions, 2, "value", porINTEGER);
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
  FuncObj->SetParamInfo(GSM_BeliefsFloat, 0, "strategy", porBEHAV_FLOAT);

  FuncObj->SetFuncInfo(GSM_BeliefsRational, 1);
  FuncObj->SetParamInfo(GSM_BeliefsRational, 0, "strategy",
			porBEHAV_RATIONAL);
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


