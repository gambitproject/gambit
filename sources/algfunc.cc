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
  NormalForm<double> &N = *mp.BelongsTo(); 

  BehavProfile<double> *bp = new BehavProfile<double>(E);
  MixedToBehav(N, (gPVector<double> &) mp, E, (gDPVector<double> &) *bp);

  return new BehavValPortion(bp);
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

  NormalForm<double> *N = MakeReducedNfg(E);
  
  ((FloatPortion *) param[1])->Value() = watch.Elapsed();
  
  if (N)
    return new NfgValPortion(N);
  else
    return new ErrorPortion("Conversion to reduced nfg failed.\n");
}

Portion *GSM_NfgRational(Portion **param)
{
  Efg<gRational> &E = * (Efg<gRational>*) ((EfgPortion*) param[0])->Value();
  gWatch watch;

  NormalForm<gRational> *N = MakeReducedNfg(E);
  
  ((FloatPortion *) param[1])->Value() = watch.Elapsed();

  if (N)
    return new NfgValPortion(N);
  else
    return new ErrorPortion("Conversion to reduced nfg failed.\n");
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

  return new Behav_ListPortion<double>(M.GetSolutions());
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
  LM.Liap(1);

  ((IntPortion *) param[1])->Value() = LM.NumEvals();
  ((FloatPortion *) param[2])->Value() = LM.Time();

  return new Behav_ListPortion<double>(LM.GetSolutions());
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
  
  return new Behav_ListPortion<double>(SM.GetSolutions());
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

  return new Behav_ListPortion<gRational>(SM.GetSolutions());
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

  
void Init_algfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("GobitEfg");
  FuncObj->SetFuncInfo(GSM_GobitEfg, 11);
  FuncObj->SetParamInfo(GSM_GobitEfg, 0, "efg", porEFG_FLOAT, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE, DEFAULT_EFG );
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
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LiapEfg");
  FuncObj->SetFuncInfo(GSM_LiapEfg, 6);
  FuncObj->SetParamInfo(GSM_LiapEfg, 0, "efg", porEFG_FLOAT, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE, DEFAULT_EFG );
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
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LemkeEfg");
  FuncObj->SetFuncInfo(GSM_LemkeEfgFloat, 4);
  FuncObj->SetParamInfo(GSM_LemkeEfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE, DEFAULT_EFG);
  FuncObj->SetParamInfo(GSM_LemkeEfgFloat, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_LemkeEfgFloat, 2, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LemkeEfgFloat, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  

  FuncObj->SetFuncInfo(GSM_LemkeEfgRational, 4);
  FuncObj->SetParamInfo(GSM_LemkeEfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE, DEFAULT_EFG);
  FuncObj->SetParamInfo(GSM_LemkeEfgRational, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_LemkeEfgRational, 2, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LemkeEfgRational, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
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
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Nfg");
  FuncObj->SetFuncInfo(GSM_NfgFloat, 2);
  FuncObj->SetParamInfo(GSM_NfgFloat, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE, DEFAULT_EFG );
  FuncObj->SetParamInfo(GSM_NfgFloat, 1, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_NfgRational, 2);
  FuncObj->SetParamInfo(GSM_NfgRational, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE, DEFAULT_EFG );
  FuncObj->SetParamInfo(GSM_NfgRational, 1, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Payoff");
  FuncObj->SetFuncInfo(GSM_Payoff, 2);
  FuncObj->SetParamInfo(GSM_Payoff, 0, "behav", porBEHAV_FLOAT);
  FuncObj->SetParamInfo(GSM_Payoff, 1, "player", porINTEGER);
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
