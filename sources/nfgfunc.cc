//#
//# FILE: nfgfunc.cc -- Normal form command language builtins
//#
//# $Id$
//#

#include "gsm.h"

Portion *GSM_DisplayNfg(Portion **param)
{
  NormalForm<double> *N = &((Nfg_Portion<double> *) param[0])->Value();

  N->DisplayNfg(gout);
  return new Nfg_Portion<double>(*N);
}


#include "lemke.h"

template <class T> class Mixed_List_Portion : public List_Portion   {
  public:
    Mixed_List_Portion(NormalForm<double> *, const gList<gPVector<T> > &);
};

Mixed_List_Portion<double>::Mixed_List_Portion(NormalForm<double> *N,
			       const gList<gPVector<double> > &list)
{
  _DataType = porMIXED_DOUBLE;
  for (int i = 1; i <= list.Length(); i++)
    Append(new Mixed_Portion<double>(MixedProfile<double>(*N, list[i])));
}

Portion *GSM_Lemke(Portion **param)
{
  NormalForm<double> *N = &((Nfg_Portion<double> *) param[0])->Value();

  LemkeParams LP;
  LemkeModule<double> LS(*N, LP);
  LS.Lemke();

  ((numerical_Portion<double> *) param[2])->Value() = (double) LS.Time();
  ((numerical_Portion<gInteger> *) param[3])->Value() = LS.NumPivots();

  return new Mixed_List_Portion<double>(N, LS.GetSolutions());
}

#include "enum.h"

Portion *GSM_Enum(Portion **param)
{
  NormalForm<double> *N = &((Nfg_Portion<double> *) param[0])->Value();

  EnumParams EP;
  EnumModule<double> EM(*N, EP);
  EM.Enum();

  return new Mixed_List_Portion<double>(N, EM.GetSolutions());
}

Portion *GSM_ReadNfg(Portion **param)
{
  gFileInput f(((gString_Portion *) param[0])->Value());
  
  if (f.IsValid())  {
    NormalForm<double> *N = new NormalForm<double>(f);
    return new Nfg_Portion<double>(*N);
  }
  else
    return 0;
}

void Init_nfgfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("DisplayNfg", GSM_DisplayNfg, 1);
  FuncObj->SetParamInfo(0, "N", porNFG, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Enum", GSM_Enum, 1);
  FuncObj->SetParamInfo(0, "N", porNFG, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Lemke", GSM_Lemke, 4);
  FuncObj->SetParamInfo(0, "N", porNFG, NO_DEFAULT_VALUE);
  FuncObj->SetParamInfo(1, "nequilib", porINTEGER,
			new numerical_Portion<gInteger>(0));
  FuncObj->SetParamInfo(2, "time", porDOUBLE, new numerical_Portion<double>(0),
			PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(3, "npivots", porINTEGER, new numerical_Portion<gInteger>(0));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ReadNfg", GSM_ReadNfg, 1);
  FuncObj->SetParamInfo(0, "file", porSTRING, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

}

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class Mixed_List_Portion<double>;
