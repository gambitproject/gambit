//#
//# FILE: nfgfunc.cc -- Normal form command language builtins
//#
//# $Id$
//#

#include "gsm.h"


Portion *GSM_ReadNfgFile(Portion **param)
{
  gFileInput f(((gString_Portion *) param[0])->Value());
  
  if (f.IsValid())  {
    BaseNormalForm *N = new NormalForm<double>(f);
    return new Nfg_Portion(*N);
  }
  else
    return 0;
}

void Init_nfgfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("ReadNfgFile", GSM_ReadNfgFile, 1);
  FuncObj->SetParamInfo(0, "file", porSTRING, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);
}
