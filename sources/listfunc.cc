//#
//# FILE: listfunc.cc -- List and text oriented function implementations
//#
//# $Id$
//#

#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "gconvert.h"

Portion *GSM_NthElement(Portion **param)
{
  return ((List_Portion *) param[0])->GetSubscript
    (((numerical_Portion<gInteger> *) param[1])->Value().as_long())->Copy();
}

Portion *GSM_Remove(Portion **param)
{
  List_Portion *ret = (List_Portion *) param[0]->Copy();
  delete ret->Remove
    (((numerical_Portion<gInteger> *) param[1])->Value().as_long());
  return ret;
}
					    
Portion *GSM_LengthList(Portion **param)
{
  return new numerical_Portion<gInteger>(((List_Portion *) param[0])->Length());
}

Portion *GSM_LengthText(Portion **param)
{
  return new numerical_Portion<gInteger>(((gString_Portion *) param[0])->Value().length());
}

Portion *GSM_NthChar(Portion **param)
{
  gString text(((gString_Portion *) param[0])->Value());
  int n = ((numerical_Portion<gInteger> *) param[1])->Value().as_long();
  if (n < 0 || n >= text.length())   return 0;
  return new gString_Portion(text[n-1]);
}
						   
Portion *GSM_TextInt(Portion **param)
{
  return new gString_Portion(ToString(((numerical_Portion<gInteger> *) param[0])->Value()));
}

Portion *GSM_TextFloat(Portion **param)
{
  return new gString_Portion(ToString(((numerical_Portion<double> *) param[0])->Value()));
}

Portion *GSM_TextRat(Portion **param)
{
  return new gString_Portion(ToString(((numerical_Portion<gRational> *) param[0])->Value()));
}

Portion *GSM_TextText(Portion **param)
{
  return param[0]->Copy();
}

Portion *GSM_FloatRational(Portion **param)
{
  return new numerical_Portion<double>((double) ((numerical_Portion<gRational> *) param[0])->Value());
}

Portion *GSM_FloatInteger(Portion **param)
{
  return new numerical_Portion<double>((double) ((numerical_Portion<gInteger> *) param[0])->Value().as_long());
}

Portion *GSM_FloatFloat(Portion **param)
{
  return param[0]->Copy();
}

Portion *GSM_RationalFloat(Portion **param)
{
  return new numerical_Portion<gRational>(((numerical_Portion<double> *) param[0])->Value());
}

Portion *GSM_RationalInteger(Portion **param)
{
  return new numerical_Portion<gRational>(((numerical_Portion<gInteger> *) param[0])->Value());
}

Portion *GSM_RationalRational(Portion **param)
{
  return param[0]->Copy();
}

#include "gwatch.h"

gWatch _gcl_watch(0);

Portion *GSM_StartWatch(Portion **param)
{
  _gcl_watch.Start();
  return new numerical_Portion<double>(0.0);
}

Portion *GSM_StopWatch(Portion **param)
{
  _gcl_watch.Stop();
  return new numerical_Portion<double>(_gcl_watch.Elapsed());
}

Portion *GSM_ElapsedTime(Portion **param)
{
  return new numerical_Portion<double>(_gcl_watch.Elapsed());
}

Portion *GSM_IsWatchRunning(Portion **param)
{
  return new bool_Portion(_gcl_watch.IsRunning());
}
					   

void Init_listfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("Length");
  FuncObj->SetFuncInfo(GSM_LengthList, 1);
  FuncObj->SetParamInfo(GSM_LengthList, 0, "list", porALL | porLIST,
			NO_DEFAULT_VALUE);
  
  FuncObj->SetFuncInfo(GSM_LengthText, 1);
  FuncObj->SetParamInfo(GSM_LengthText, 0, "text", porSTRING);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NthElement");
  FuncObj->SetFuncInfo(GSM_NthElement, 2);
  FuncObj->SetParamInfo(GSM_NthElement, 0, "list", porALL | porLIST);
  FuncObj->SetParamInfo(GSM_NthElement, 1, "n", porINTEGER);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Remove");
  FuncObj->SetFuncInfo(GSM_Remove, 2);
  FuncObj->SetParamInfo(GSM_Remove, 0, "list", porALL | porLIST);
  FuncObj->SetParamInfo(GSM_Remove, 1, "n", porINTEGER);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NthChar");
  FuncObj->SetFuncInfo(GSM_NthChar, 2);
  FuncObj->SetParamInfo(GSM_NthChar, 0, "text", porSTRING);
  FuncObj->SetParamInfo(GSM_NthChar, 1, "n", porINTEGER);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Text");
  FuncObj->SetFuncInfo(GSM_TextInt, 1);
  FuncObj->SetParamInfo(GSM_TextInt, 0, "x", porINTEGER);

  FuncObj->SetFuncInfo(GSM_TextFloat, 1);
  FuncObj->SetParamInfo(GSM_TextFloat, 0, "x", porDOUBLE);

  FuncObj->SetFuncInfo(GSM_TextRat, 1);
  FuncObj->SetParamInfo(GSM_TextRat, 0, "x", porRATIONAL);

  FuncObj->SetFuncInfo(GSM_TextText, 1);
  FuncObj->SetParamInfo(GSM_TextText, 0, "x", porSTRING);
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("Float");
  FuncObj->SetFuncInfo(GSM_FloatRational, 1);
  FuncObj->SetParamInfo(GSM_FloatRational, 0, "x", porRATIONAL);

  FuncObj->SetFuncInfo(GSM_FloatInteger, 1);
  FuncObj->SetParamInfo(GSM_FloatInteger, 0, "x", porINTEGER);

  FuncObj->SetFuncInfo(GSM_FloatFloat, 1);
  FuncObj->SetParamInfo(GSM_FloatFloat, 0, "x", porDOUBLE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Rational");
  FuncObj->SetFuncInfo(GSM_RationalFloat, 1);
  FuncObj->SetParamInfo(GSM_RationalFloat, 0, "x", porDOUBLE);
  
  FuncObj->SetFuncInfo(GSM_RationalInteger, 1);
  FuncObj->SetParamInfo(GSM_RationalInteger, 0, "x", porINTEGER);

  FuncObj->SetFuncInfo(GSM_RationalRational, 1);
  FuncObj->SetParamInfo(GSM_RationalRational, 0, "x", porRATIONAL);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("StartWatch");
  FuncObj->SetFuncInfo(GSM_StartWatch, 0);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("StopWatch");
  FuncObj->SetFuncInfo(GSM_StopWatch, 0);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ElapsedTime");
  FuncObj->SetFuncInfo(GSM_ElapsedTime, 0);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsWatchRunning");
  FuncObj->SetFuncInfo(GSM_IsWatchRunning, 0);
  gsm->AddFunction(FuncObj);

}




