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


template <class T> int BuildReducedNormal(const ExtForm<T> &,
					  NormalForm<T> *&);
template <class T>
void MixedToBehav(const NormalForm<T> &N, const gPVector<T> &mp,
		  const ExtForm<T> &E, gDPVector<T> &bp);

#include "gwatch.h"
#include "mixed.h"

Portion *GSM_BehavFloat(Portion **param)
{
  MixedProfile<double> &mp = ((MixedPortion<double> *) param[0])->Value();
  ExtForm<double> &E = ((EfgPortion<double> *) param[1])->Value();
  NormalForm<double> &N = *mp.BelongsTo(); 

  BehavProfile<double> *bp = new BehavProfile<double>(E);
  MixedToBehav(N, (gPVector<double> &) mp, E, (gDPVector<double> &) *bp);

  return new BehavValPortion<double>(*bp);
}

Portion *GSM_Nfg(Portion **param)
{
  ExtForm<double> &E = ((EfgPortion<double> *) param[0])->Value();
  NormalForm<double> *N = 0;
  gWatch watch;
  BuildReducedNormal(E, (NormalForm<double>*&) N);
  
  ((FloatPortion *) param[1])->Value() = watch.Elapsed();
  return new NfgValPortion<double>(*N);
}

template <class T> class Behav_ListPortion : public ListValPortion   {
  public:
    Behav_ListPortion(ExtForm<T> *, const gList<BehavProfile<T> > &);
    virtual ~Behav_ListPortion()   { }
};

Behav_ListPortion<double>::Behav_ListPortion(ExtForm<double> *E,
			   const gList<BehavProfile<double> > &list)
{
  _DataType = porBEHAV_FLOAT;
  for (int i = 1; i <= list.Length(); i++)
    Append( new BehavValPortion<double>( * new BehavProfile<double>(list[i])));
}

Behav_ListPortion<gRational>::Behav_ListPortion(ExtForm<gRational> *E,
			      const gList<BehavProfile<gRational> > &list)
{
  _DataType = porBEHAV_RATIONAL;
  for (int i = 1; i <= list.Length(); i++)
    Append( new BehavValPortion<gRational>( * new BehavProfile<gRational>(list[i])));
}

Portion *GSM_GobitEfg(Portion **param)
{
  EFGobitParams<double> EP;
 
  EP.pxifile = &((OutputPortion *) param[1])->Value();
  EP.minLam = ((FloatPortion *) param[3])->Value();
  EP.maxLam = ((FloatPortion *) param[4])->Value();
  EP.delLam = ((FloatPortion *) param[5])->Value();
  EP.powLam = ((IntPortion *) param[6])->Value();
  
  EFGobitModule<double> M(((EfgPortion<double> *) param[0])->Value(), EP);
  M.Gobit(1);

  ((FloatPortion *) param[2])->Value() = M.Time();

  return new IntValPortion(1);
}

Portion *GSM_LiapEfg(Portion **param)
{
  ExtForm<double> &E = ((EfgPortion<double> *) param[0])->Value();

  EFLiapParams<double> LP;

  LP.nequilib = ((IntPortion *) param[3])->Value();
  LP.ntries = ((IntPortion *) param[4])->Value();
 
  EFLiapModule<double> LM(E, LP);
  LM.Liap(1);

  ((IntPortion *) param[1])->Value() = LM.NumEvals();
  ((FloatPortion *) param[2])->Value() = LM.Time();

  return new Behav_ListPortion<double>(&E, LM.GetSolutions());
}

#include "seqform.h"

Portion *GSM_SequenceD(Portion **param)
{
  ExtForm<double> &E = ((EfgPortion<double> *) param[0])->Value();

  SeqFormParams SP;
  SeqFormModule<double> SM(E, SP);
  SM.Lemke();
  
  return new Behav_ListPortion<double>(&E, SM.GetSolutions());
}

Portion *GSM_SequenceR(Portion **param)
{
  ExtForm<gRational> &E = ((EfgPortion<gRational> *) param[0])->Value();

  SeqFormParams SP;
  SeqFormModule<gRational> SM(E, SP);
  SM.Lemke();
  
  return new Behav_ListPortion<gRational>(&E, SM.GetSolutions());
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
  FuncObj->SetFuncInfo(GSM_GobitEfg, 8);
  FuncObj->SetParamInfo(GSM_GobitEfg, 0, "efg", porEFG_FLOAT, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE, DEFAULT_EFG );
  FuncObj->SetParamInfo(GSM_GobitEfg, 1, "pxifile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_GobitEfg, 2, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitEfg, 3, "minLam", porFLOAT,
			new FloatRefPortion(Gobit_default_minLam));
  FuncObj->SetParamInfo(GSM_GobitEfg, 4, "maxLam", porFLOAT,
			new FloatRefPortion(Gobit_default_maxLam));
  FuncObj->SetParamInfo(GSM_GobitEfg, 5, "delLam", porFLOAT,
			new FloatRefPortion(Gobit_default_delLam));
  FuncObj->SetParamInfo(GSM_GobitEfg, 6, "powLam", porINTEGER,
		        new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_GobitEfg, 7, "start", porLIST | porFLOAT,
		        new ListValPortion);
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
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Sequence");
  FuncObj->SetFuncInfo(GSM_SequenceD, 1);
  FuncObj->SetParamInfo(GSM_SequenceD, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE, 
			PASS_BY_REFERENCE, DEFAULT_EFG );

  FuncObj->SetFuncInfo(GSM_SequenceR, 1);
  FuncObj->SetParamInfo(GSM_SequenceR, 0, "efg", porEFG_RATIONAL,
			NO_DEFAULT_VALUE, 
			PASS_BY_REFERENCE, DEFAULT_EFG );
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
  FuncObj->SetParamInfo(GSM_BehavFloat, 1, "efg", porEFG_FLOAT);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Nfg");
  FuncObj->SetFuncInfo(GSM_Nfg, 2);
  FuncObj->SetParamInfo(GSM_Nfg, 0, "efg", porEFG_FLOAT,
			NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE, DEFAULT_EFG );
  FuncObj->SetParamInfo(GSM_Nfg, 1, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
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
