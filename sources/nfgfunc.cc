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

Portion *GSM_Lemke(Portion **param)
{

  return new numerical_Portion<gInteger>(1);
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

  FuncObj = new FuncDescObj("Lemke", GSM_Lemke, 3);
  FuncObj->SetParamInfo(0, "N", porNFG, NO_DEFAULT_VALUE);
  FuncObj->SetParamInfo(1, "nequilib", porINTEGER,
			new numerical_Portion<gInteger>(0));
  FuncObj->SetParamInfo(2, "time", porDOUBLE, new numerical_Portion<double>(0),
			PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ReadNfg", GSM_ReadNfg, 1);
  FuncObj->SetParamInfo(0, "file", porSTRING, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

}

