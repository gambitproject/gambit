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

template <class T> int BuildReducedNormal(const ExtForm<T> &,
					  NormalForm<T> *&);

Portion *GSM_EfgToNfg(Portion **param)
{
  ExtForm<double> &E = ((Efg_Portion<double> *) param[0])->Value();
  NormalForm<double> *N = 0;
  BuildReducedNormal(E, N);
  return new Nfg_Portion<double>(*N);
}

template <class T> class Behav_List_Portion : public List_Portion   {
  public:
    Behav_List_Portion(ExtForm<T> *, const gList<BehavProfile<T> > &);
    virtual ~Behav_List_Portion()   { }
};

Behav_List_Portion<double>::Behav_List_Portion(ExtForm<double> *E,
			       const gList<BehavProfile<double> > &list)
{
  _DataType = porBEHAV_DOUBLE;
  for (int i = 1; i <= list.Length(); i++)
    Append(new Behav_Portion<double>(list[i]));
}

Behav_List_Portion<gRational>::Behav_List_Portion(ExtForm<gRational> *E,
			    const gList<BehavProfile<gRational> > &list)
{
  _DataType = porBEHAV_RATIONAL;
  for (int i = 1; i <= list.Length(); i++)
    Append(new Behav_Portion<gRational>(list[i]));
}

Portion *GSM_GobitEfg(Portion **param)
{
  EFGobitParams<double> EP;
 
  EP.pxifile = &((Output_Portion *) param[1])->Value();
  EP.minLam = ((numerical_Portion<double> *) param[3])->Value();
  EP.maxLam = ((numerical_Portion<double> *) param[4])->Value();
  EP.delLam = ((numerical_Portion<double> *) param[5])->Value();
  EP.powLam = ((numerical_Portion<gInteger> *) param[6])->Value().as_long();
  
  EFGobitModule<double> M(((Efg_Portion<double> *) param[0])->Value(), EP);
  M.Gobit(1);

  ((numerical_Portion<double> *) param[2])->Value() = M.Time();

  return new numerical_Portion<gInteger>(1);
}

Portion *GSM_LiapEfg(Portion **param)
{
  ExtForm<double> &E = ((Efg_Portion<double> *) param[0])->Value();

  EFLiapParams<double> LP;
  EFLiapModule<double> LM(E, LP);
  LM.Liap(1);

  return new Behav_List_Portion<double>(&E, LM.GetSolutions());
}

#include "seqform.h"

Portion *GSM_SequenceD(Portion **param)
{
  ExtForm<double> &E = ((Efg_Portion<double> *) param[0])->Value();

  SeqFormParams SP;
  SeqFormModule<double> SM(E, SP);
  SM.Lemke();
  
  return new Behav_List_Portion<double>(&E, SM.GetSolutions());
}

Portion *GSM_SequenceR(Portion **param)
{
  ExtForm<gRational> &E = ((Efg_Portion<gRational> *) param[0])->Value();

  SeqFormParams SP;
  SeqFormModule<gRational> SM(E, SP);
  SM.Lemke();
  
  return new Behav_List_Portion<gRational>(&E, SM.GetSolutions());
}

Portion *GSM_SetOptions(Portion **param)
{
  gString alg = ((gString_Portion *) param[0])->Value();
  gString par = ((gString_Portion *) param[1])->Value();
  double value = ((numerical_Portion<double> *) param[2])->Value();
  
  if (alg == "Gobit")   {
    if (par == "minLam")           Gobit_default_minLam = value;
    else if (par == "maxLam")      Gobit_default_maxLam = value;
    else if (par == "delLam")      Gobit_default_delLam = value;
    else return 0;
    return new numerical_Portion<double>(value);
  }
  else
    return 0;
}
  
void Init_algfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("GobitEfg");
  FuncObj->SetFuncInfo(GSM_GobitEfg, 8);
  FuncObj->SetParamInfo(GSM_GobitEfg, 0, "efg", porEFG_DOUBLE);
  FuncObj->SetParamInfo(GSM_GobitEfg, 1, "pxifile", porOUTPUT,
			new Output_Portion(gnull));
  FuncObj->SetParamInfo(GSM_GobitEfg, 2, "time", porDOUBLE,
			new numerical_Portion<double>(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitEfg, 3, "minLam", porDOUBLE,
			new numerical_Portion<double>(Gobit_default_minLam, true));
  FuncObj->SetParamInfo(GSM_GobitEfg, 4, "maxLam", porDOUBLE,
			new numerical_Portion<double>(Gobit_default_maxLam, true));
  FuncObj->SetParamInfo(GSM_GobitEfg, 5, "delLam", porDOUBLE,
			new numerical_Portion<double>(Gobit_default_delLam, true));
  FuncObj->SetParamInfo(GSM_GobitEfg, 6, "powLam", porINTEGER,
		        new numerical_Portion<gInteger>(1));
  FuncObj->SetParamInfo(GSM_GobitEfg, 7, "start", porLIST | porDOUBLE,
		        new List_Portion);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LiapEfg");
  FuncObj->SetFuncInfo(GSM_LiapEfg, 1);
  FuncObj->SetParamInfo(GSM_LiapEfg, 0, "E", porEFG_DOUBLE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Sequence");
  FuncObj->SetFuncInfo(GSM_SequenceD, 1);
  FuncObj->SetParamInfo(GSM_SequenceD, 0, "E", porEFG_DOUBLE);

  FuncObj->SetFuncInfo(GSM_SequenceR, 1);
  FuncObj->SetParamInfo(GSM_SequenceR, 0, "E", porEFG_RATIONAL);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetOptions");
  FuncObj->SetFuncInfo(GSM_SetOptions, 3);
  FuncObj->SetParamInfo(GSM_SetOptions, 0, "alg", porSTRING);
  FuncObj->SetParamInfo(GSM_SetOptions, 1, "param", porSTRING);
  FuncObj->SetParamInfo(GSM_SetOptions, 2, "value", porDOUBLE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("EfgToNfg");
  FuncObj->SetFuncInfo(GSM_EfgToNfg, 1);
  FuncObj->SetParamInfo(GSM_EfgToNfg, 0, "E", porEFG_DOUBLE);
  gsm->AddFunction(FuncObj);

}

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class Behav_List_Portion<double>;
TEMPLATE class Behav_List_Portion<gRational>;
