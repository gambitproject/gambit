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

Portion *GSM_BehavFloat(Portion **param)
{
  MixedProfile<double> &mp = * (MixedProfile<double>*) ((MixedPortion*) param[0])->Value();
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[1])->Value();
  Nfg<double> &N = *mp.BelongsTo(); 

  BehavProfile<double> *bp = new BehavProfile<double>(E);
  MixedToBehav(N, mp, E, *bp);

  Portion* por = new BehavValPortion(bp);
  por->SetOwner( param[ 1 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_BehavRational(Portion **param)
{
  MixedProfile<gRational> &mp = * (MixedProfile<gRational>*) ((MixedPortion*) param[0])->Value();
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[1])->Value();
  Nfg<gRational> &N = *mp.BelongsTo(); 

  BehavProfile<gRational> *bp = new BehavProfile<gRational>(E);
  MixedToBehav(N, mp, E, *bp);

  Portion* por = new BehavValPortion(bp);
  por->SetOwner( param[ 1 ]->Original() );
  por->AddDependency();
  return por;
}



Portion *GSM_Payoff(Portion **param)
{
  BehavProfile<double> bp = * (BehavProfile<double>*) ((BehavPortion*) param[0])->Value();
  int pl = ((IntPortion *) param[1])->Value();

  return new FloatValPortion(bp.Payoff(pl));
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

Portion *GSM_GobitEfg(Portion **param)
{
  Efg<double> &E = *(Efg<double> *) ((EfgPortion *) param[0])->Value();
  
  EFGobitParams<double> EP;
 
  EP.pxifile = &((OutputPortion *) param[1])->Value();
  EP.fullGraph = ((BoolPortion *) param[5])->Value();
  EP.minLam = ((FloatPortion *) param[6])->Value();
  EP.maxLam = ((FloatPortion *) param[7])->Value();
  EP.delLam = ((FloatPortion *) param[8])->Value();
  EP.powLam = ((IntPortion *) param[9])->Value();
  
  BehavProfile<double> *foo = (BehavProfile<double> *) ((BehavPortion *) param[10])->Value();
  BehavProfile<double> start(E);
  if (foo)   start = *foo;

  EFGobitModule<double> M(E, EP, start);
  M.Gobit(1);

  ((FloatPortion *) param[2])->Value() = M.Time();
  ((IntPortion *) param[3])->Value() = M.NumEvals();
  ((IntPortion *) param[4])->Value() = M.NumIters();

  Portion* por = new Behav_ListPortion<double>(M.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_LiapEfg(Portion **param)
{
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();

  EFLiapParams<double> LP;

  LP.stopAfter = ((IntPortion *) param[3])->Value();
  LP.nTries = ((IntPortion *) param[4])->Value();
 
  BehavProfile<double> *foo = (BehavProfile<double> *) ((BehavPortion *) param[5])->Value();
  BehavProfile<double> start(E);

  EFLiapModule<double> LM(E, LP, (foo) ? *foo : start);
  LM.Liap();

  ((IntPortion *) param[1])->Value() = LM.NumEvals();
  ((FloatPortion *) param[2])->Value() = LM.Time();

  Portion* por = new Behav_ListPortion<double>(LM.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

#include "seqform.h"

Portion *GSM_LemkeEfgFloat(Portion **param)
{
  Efg<double> &E = * (Efg<double>*) ((EfgPortion*) param[0])->Value();

  SeqFormParams SP;
  SP.nequilib = ((IntPortion *) param[1])->Value();

  SeqFormModule<double> SM(E, SP);
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
  SeqFormModule<gRational> SM(E, SP);
  SM.Lemke();
  
  ((IntPortion *) param[2])->Value() = SM.NumPivots();
  ((FloatPortion *) param[3])->Value() = SM.Time();

  Portion* por = new Behav_ListPortion<gRational>(SM.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

extern double Funct_tolBrent, Funct_tolN;
extern int Funct_maxitsBrent, Funct_maxitsN;

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

  E->CondPayoff(*bp, values, probs);
  
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

  E->CondPayoff(*bp, values, probs);
  
  gVector<gRational> ret(s->NumActions());
  for (int i = 1; i <= s->NumActions(); i++)
    ret[i] = values(s->GetPlayer()->GetNumber(), s->GetNumber(), i);

  return ArrayToList(ret);
}

Portion *GSM_BeliefsFloat(Portion **param)
{
  BehavProfile<double> *bp = (BehavProfile<double> *) ((BehavPortion *) param[0])->Value();

  return ArrayToList(bp->BelongsTo()->Beliefs(*bp));
}

Portion *GSM_BeliefsRational(Portion **param)
{
  BehavProfile<gRational> *bp = (BehavProfile<gRational> *) ((BehavPortion *) param[0])->Value();

  return ArrayToList(bp->BelongsTo()->Beliefs(*bp));
}

Portion *GSM_InfosetProbsFloat(Portion **param)
{
  BehavProfile<double> *bp = (BehavProfile<double> *) ((BehavPortion *) param[0])->Value();

  Efg<double> *E = bp->BelongsTo();

  gDPVector<double> values(E->Dimensionality());
  gPVector<double> probs(E->Dimensionality().Lengths());

  E->CondPayoff(*bp, values, probs);

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

  E->CondPayoff(*bp, values, probs);

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

  return ArrayToList(bp->BelongsTo()->NodeValues(p->GetNumber(), *bp));
}

Portion *GSM_NodeValuesRational(Portion **param)
{
  BehavProfile<gRational> *bp = (BehavProfile<gRational> *) ((BehavPortion *) param[0])->Value();
  EFPlayer *p = ((EfPlayerPortion *) param[1])->Value();

  if (bp->BelongsTo() != p->BelongsTo())
    return new ErrorPortion("Profile and player are from different games");

  return ArrayToList(bp->BelongsTo()->NodeValues(p->GetNumber(), *bp));
}
 
Portion *GSM_RealizProbsFloat(Portion **param)
{
  BehavProfile<double> *bp = (BehavProfile<double> *) ((BehavPortion *) param[0])->Value();
  
  return ArrayToList(bp->BelongsTo()->NodeRealizProbs(*bp));
}  
  
Portion *GSM_RealizProbsRational(Portion **param)
{
  BehavProfile<gRational> *bp = (BehavProfile<gRational> *) ((BehavPortion *) param[0])->Value();
  
  return ArrayToList(bp->BelongsTo()->NodeRealizProbs(*bp));
}
  
  
extern Portion *GSM_GobitNfg(Portion **param);
extern Portion *GSM_LiapNfg(Portion **param);
extern Portion *GSM_LemkeNfgFloat(Portion **param);
extern Portion *GSM_LemkeNfgRational(Portion **param);

void Init_algfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("GobitSolve");
  FuncObj->SetFuncInfo(GSM_GobitEfg, 11);
  FuncObj->SetParamInfo(GSM_GobitEfg, 0, "efg", porEFG_FLOAT, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_GobitEfg, 1, "pxifile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_GobitEfg, 2, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitEfg, 3, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitEfg, 4, "nIters", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitEfg, 5, "fullGraph", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_GobitEfg, 6, "minLam", porFLOAT,
			new FloatRefPortion(Gobit_default_minLam));
  FuncObj->SetParamInfo(GSM_GobitEfg, 7, "maxLam", porFLOAT,
			new FloatRefPortion(Gobit_default_maxLam));
  FuncObj->SetParamInfo(GSM_GobitEfg, 8, "delLam", porFLOAT,
			new FloatRefPortion(Gobit_default_delLam));
  FuncObj->SetParamInfo(GSM_GobitEfg, 9, "powLam", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_GobitEfg, 10, "start", porBEHAV_FLOAT,
			new BehavValPortion(0));

  FuncObj->SetFuncInfo(GSM_GobitNfg, 11);
  FuncObj->SetParamInfo(GSM_GobitNfg, 0, "nfg", 
			porNFG_FLOAT, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_GobitNfg, 1, "pxifile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_GobitNfg, 2, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitNfg, 3, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitNfg, 4, "nIters", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitNfg, 5, "fullGraph", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_GobitNfg, 6, "minLam", porFLOAT,
			new FloatRefPortion(Gobit_default_minLam));
  FuncObj->SetParamInfo(GSM_GobitNfg, 7, "maxLam", porFLOAT,
			new FloatRefPortion(Gobit_default_maxLam));
  FuncObj->SetParamInfo(GSM_GobitNfg, 8, "delLam", porFLOAT,
			new FloatRefPortion(Gobit_default_delLam));
  FuncObj->SetParamInfo(GSM_GobitNfg, 9, "powLam", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_GobitNfg, 10, "start", porMIXED_FLOAT,
			new MixedValPortion(0));

  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LiapSolve");
  FuncObj->SetFuncInfo(GSM_LiapEfg, 6);
  FuncObj->SetParamInfo(GSM_LiapEfg, 0, "efg", porEFG_FLOAT, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_LiapEfg, 1, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapEfg, 2, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapEfg, 3, "stopAfter", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_LiapEfg, 4, "nTries", porINTEGER,
		        new IntValPortion(10));
  FuncObj->SetParamInfo(GSM_LiapEfg, 5, "start", porBEHAV_FLOAT,
			new BehavValPortion(0));

  FuncObj->SetFuncInfo(GSM_LiapNfg, 6);
  FuncObj->SetParamInfo(GSM_LiapNfg, 0, "nfg", porNFG_FLOAT, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapNfg, 1, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapNfg, 2, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapNfg, 3, "stopAfter", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_LiapNfg, 4, "nTries", porINTEGER,
			new IntValPortion(10));
  FuncObj->SetParamInfo(GSM_LiapNfg, 5, "start", porMIXED_FLOAT,
			new MixedValPortion(0));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LCPSolve");
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

  FuncObj = new FuncDescObj("Payoff");
  FuncObj->SetFuncInfo(GSM_Payoff, 2);
  FuncObj->SetParamInfo(GSM_Payoff, 0, "behav", porBEHAV_FLOAT);
  FuncObj->SetParamInfo(GSM_Payoff, 1, "player", porINTEGER);
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
