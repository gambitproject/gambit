//#
//# FILE: listfunc.cc -- List and text oriented function implementations
//#
//# $Id$
//#

#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "gmisc.h"
#include "rational.h"

Portion *GSM_NthElement(Portion **param)
{
  return ((ListPortion *) param[0])->Subscript
    (((IntPortion *) param[1])->Value());
}

Portion *GSM_Remove(Portion **param)
{
  ListPortion *ret = (ListPortion *) param[0]->ValCopy();
  delete ret->Remove
    (((IntPortion *) param[1])->Value());
  return ret;
}
					    
Portion *GSM_Contains(Portion **param)
{
  return new BoolValPortion(((ListPortion *) param[0])->Contains(param[1]));
}


Portion *GSM_LengthList(Portion **param)
{
  return new IntValPortion(((ListPortion *) param[0])->Length());
}

Portion *GSM_LengthText(Portion **param)
{
  return new IntValPortion(((TextPortion *) param[0])->Value().length());
}

Portion *GSM_NthChar(Portion **param)
{
  gString text(((TextPortion *) param[0])->Value());
  int n = ((IntPortion *) param[1])->Value();
  if (n < 0 || n >= text.length())   return 0;
  return new TextValPortion(text[n-1]);
}
						   
Portion *GSM_TextInt(Portion **param)
{
  return new TextValPortion(ToString(((IntPortion *) param[0])->Value()));
}

Portion *GSM_TextFloat(Portion **param)
{
  return new TextValPortion(ToString(((FloatPortion *) param[0])->Value()));
}

Portion *GSM_TextRat(Portion **param)
{
  return new TextValPortion(ToString(((RationalPortion *) param[0])->Value()));
}

Portion *GSM_TextText(Portion **param)
{
  return param[0]->ValCopy();
}

Portion *GSM_FloatRational(Portion **param)
{
  return new FloatValPortion((double) ((RationalPortion *) param[0])->Value());
}

Portion *GSM_FloatInteger(Portion **param)
{
  return new FloatValPortion((double) ((IntPortion *) param[0])->Value());
}

Portion *GSM_FloatFloat(Portion **param)
{
  return param[0]->ValCopy();
}

Portion *GSM_RationalFloat(Portion **param)
{
  return new RationalValPortion(((FloatPortion *) param[0])->Value());
}

Portion *GSM_RationalInteger(Portion **param)
{
  return new RationalValPortion(((IntPortion *) param[0])->Value());
}

Portion *GSM_RationalRational(Portion **param)
{
  return param[0]->ValCopy();
}

#include "gwatch.h"

gWatch _gcl_watch(0);

Portion *GSM_StartWatch(Portion **param)
{
  _gcl_watch.Start();
  return new FloatValPortion(0.0);
}

Portion *GSM_StopWatch(Portion **param)
{
  _gcl_watch.Stop();
  return new FloatValPortion(_gcl_watch.Elapsed());
}

Portion *GSM_ElapsedTime(Portion **param)
{
  return new FloatValPortion(_gcl_watch.Elapsed());
}

Portion *GSM_IsWatchRunning(Portion **param)
{
  return new BoolValPortion(_gcl_watch.IsRunning());
}
					   

void Init_listfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;


  ParamInfoType x_Float[] =
  {
    ParamInfoType( "x", porFLOAT )
  };
  ParamInfoType x_Int[] =
  {
    ParamInfoType( "x", porINTEGER )
  };
  ParamInfoType x_Rational[] =
  {
    ParamInfoType( "x", porRATIONAL )
  };


  FuncObj = new FuncDescObj("Length");
  FuncObj->SetFuncInfo(GSM_LengthList, 1);
  FuncObj->SetParamInfo(GSM_LengthList, 0, "list", porALL | porLIST,
			NO_DEFAULT_VALUE);
  FuncObj->SetFuncInfo(GSM_LengthText, 1);
  FuncObj->SetParamInfo(GSM_LengthText, 0, "text", porTEXT);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Contains");
  FuncObj->SetFuncInfo(GSM_Contains, 2);
  FuncObj->SetParamInfo(GSM_Contains, 0, "list", porALL | porLIST);
  FuncObj->SetParamInfo(GSM_Contains, 1, "x", porVALUE);
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
  FuncObj->SetParamInfo(GSM_NthChar, 0, "text", porTEXT);
  FuncObj->SetParamInfo(GSM_NthChar, 1, "n", porINTEGER);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Text");
  FuncObj->SetFuncInfo(GSM_TextInt, 1, x_Int);
  // FuncObj->SetParamInfo(GSM_TextInt, 0, "x", porINTEGER);

  FuncObj->SetFuncInfo(GSM_TextFloat, 1, x_Float);
  // FuncObj->SetParamInfo(GSM_TextFloat, 0, "x", porFLOAT);

  FuncObj->SetFuncInfo(GSM_TextRat, 1, x_Rational);
  // FuncObj->SetParamInfo(GSM_TextRat, 0, "x", porRATIONAL);

  FuncObj->SetFuncInfo(GSM_TextText, 1);
  FuncObj->SetParamInfo(GSM_TextText, 0, "x", porTEXT);
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("Float");
  FuncObj->SetFuncInfo(GSM_FloatRational, 1, x_Rational);
  // FuncObj->SetParamInfo(GSM_FloatRational, 0, "x", porRATIONAL);

  FuncObj->SetFuncInfo(GSM_FloatInteger, 1, x_Int);
  // FuncObj->SetParamInfo(GSM_FloatInteger, 0, "x", porINTEGER);

  FuncObj->SetFuncInfo(GSM_FloatFloat, 1, x_Float);
  // FuncObj->SetParamInfo(GSM_FloatFloat, 0, "x", porFLOAT);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Rational");
  FuncObj->SetFuncInfo(GSM_RationalFloat, 1, x_Float);
  // FuncObj->SetParamInfo(GSM_RationalFloat, 0, "x", porFLOAT);
  
  FuncObj->SetFuncInfo(GSM_RationalInteger, 1, x_Int);
  // FuncObj->SetParamInfo(GSM_RationalInteger, 0, "x", porINTEGER);

  FuncObj->SetFuncInfo(GSM_RationalRational, 1, x_Rational);
  // FuncObj->SetParamInfo(GSM_RationalRational, 0, "x", porRATIONAL);
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




