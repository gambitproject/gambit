//#
//# FILE: efgfunc.cc -- Extensive form command language builtins
//#
//# $Id$
//#

#include "gsm.h"
#include "extform.h"

#include "egobit.h"
#include "eliap.h"

template <class T> class Behav_List_Portion : public List_Portion   {
  public:
    Behav_List_Portion(ExtForm<double> *, const gList<BehavProfile<T> > &);
};

Behav_List_Portion<double>::Behav_List_Portion(ExtForm<double> *E,
			       const gList<BehavProfile<double> > &list)
{
  _DataType = porBEHAV_DOUBLE;
  for (int i = 1; i <= list.Length(); i++)
    Append(new Behav_Portion<double>(list[i]));
}

//
// GobitEfg: Parameter assignments:
// 0   E            EFG 
// 1   pxifile      TEXT
// 2   minLam       *DOUBLE
// 3   maxLam       *DOUBLE
// 4   delLam       *DOUBLE
// 5   maxitsOpt    *INTEGER
// 6   maxitsBrent  *INTEGER
// 7   tolOpt       *DOUBLE
// 8   tolBrent     *DOUBLE
// 9   time         *REF(DOUBLE)
// 
Portion *GSM_GobitEfg(Portion **param)
{
  EFGobitParams<double> EP;
 
  gFileOutput f(((gString_Portion *) param[1])->Value());

  if (f.IsValid())    EP.pxifile = &f;
  
  EFGobitModule<double> M(((Efg_Portion<double> *) param[0])->Value(), EP);
  M.Gobit(1);

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

Portion *GSM_ReadEfg(Portion **param)
{
  gFileInput f(((gString_Portion *) param[0])->Value());
  
  if (f.IsValid())   {
    ExtForm<double> *E = new ExtForm<double>;
    ReadEfgFile(f, E);
    return new Efg_Portion<double>(*E);
  }
  else
    return 0;
}

Portion *GSM_WriteEfg(Portion **param)
{
  ExtForm<double> *E = &((Efg_Portion<double> *) param[0])->Value();
  gFileOutput f(((gString_Portion *) param[1])->Value());
  
  if (f.IsValid())   {
    E->WriteEfgFile(f);
    return new Efg_Portion<double>(*E);
  }
  else
    return 0;
}

void Init_efgfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("ReadEfg");
  FuncObj->SetFuncInfo(GSM_ReadEfg, 1);
  FuncObj->SetParamInfo(GSM_ReadEfg, 0, "file", porSTRING, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("WriteEfg");
  FuncObj->SetFuncInfo(GSM_WriteEfg, 2);
  FuncObj->SetParamInfo(GSM_WriteEfg, 0, "E", porEFG_DOUBLE, NO_DEFAULT_VALUE);
  FuncObj->SetParamInfo(GSM_WriteEfg, 1, "file", porSTRING, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("GobitEfg");
  FuncObj->SetFuncInfo(GSM_GobitEfg, 2);
  FuncObj->SetParamInfo(GSM_GobitEfg, 0, "E", porEFG_DOUBLE, NO_DEFAULT_VALUE);
  FuncObj->SetParamInfo(GSM_GobitEfg, 1, "pxifile", porSTRING, NO_DEFAULT_VALUE);
/*
  FuncObj->SetParamInfo(GSM_GobitEfg, 2, "minLam", porDOUBLE,
		        new numerical_Portion<double>(.01));
  FuncObj->SetParamInfo(GSM_GobitEfg, 3, "maxLam", porDOUBLE,
		        new numerical_Portion<double>(30));
  FuncObj->SetParamInfo(GSM_GobitEfg, 4, "delLam", porDOUBLE,
		        new numerical_Portion<double>(.01));
  FuncObj->SetParamInfo(GSM_GobitEfg, 5, "maxitsOpt", porINTEGER,
		        new numerical_Portion<gInteger>(200));
  FuncObj->SetParamInfo(GSM_GobitEfg, 6, "maxitsBrent", porINTEGER,
		        new numerical_Portion<gInteger>(100));
  FuncObj->SetParamInfo(GSM_GobitEfg, 7, "tolOpt", porDOUBLE,
		        new numerical_Portion<double>(1.0e-10));
  FuncObj->SetParamInfo(GSM_GobitEfg, 8, "tolBrent", porDOUBLE,
		        new numerical_Portion<double>(2.0e-10));
  FuncObj->SetParamInfo(GSM_GobitEfg, 9, "time", porDOUBLE,
			new numerical_Portion<double>(0), PASS_BY_REFERENCE);
  */
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LiapEfg");
  FuncObj->SetFuncInfo(GSM_LiapEfg, 1);
  FuncObj->SetParamInfo(GSM_LiapEfg, 0, "E", porEFG_DOUBLE, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);
}



#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class Behav_List_Portion<double>;

