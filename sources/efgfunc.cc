//#
//# FILE: efgfunc.cc -- Extensive form command language builtins
//#
//# $Id$
//#


#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

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
// 1   pxifile      STREAM
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
 
  EP.pxifile = &((Stream_Portion *) param[1])->Value();
  EP.minLam = ((numerical_Portion<double> *) param[2])->Value();
  EP.maxLam = ((numerical_Portion<double> *) param[3])->Value();
  EP.delLam = ((numerical_Portion<double> *) param[4])->Value();
  EP.maxitsOpt = ((numerical_Portion<gInteger> *) param[5])->Value().as_long();
  EP.maxitsBrent = ((numerical_Portion<gInteger> *) param[6])->Value().as_long();
  EP.tolOpt = ((numerical_Portion<double> *) param[7])->Value();
  EP.tolBrent = ((numerical_Portion<double> *) param[8])->Value();
  
  EFGobitModule<double> M(((Efg_Portion<double> *) param[0])->Value(), EP);
  M.Gobit(1);

  ((numerical_Portion<double> *) param[9])->Value() = (double) M.Time();

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
    ExtForm<double> *E = 0;
    ReadEfgFile((gInput &) f, E);

    if (E)
      return new Efg_Portion<double>(*E);
    else
      return 0;
  }
  else
    return 0;
}

Portion *GSM_WriteEfg(Portion **param)
{
  ExtForm<double> *E = &((Efg_Portion<double> *) param[0])->Value();
  gOutput &f = ((Stream_Portion *) param[1])->Value();
  
  E->WriteEfgFile(f);
  return new Efg_Portion<double>(*E);
}

template <class T> int BuildReducedNormal(const ExtForm<T> &,
					  NormalForm<T> *&);

Portion *GSM_EfgToNfg(Portion **param)
{
  ExtForm<double> &E = ((Efg_Portion<double> *) param[0])->Value();
  NormalForm<double> *N = 0;
  BuildReducedNormal(E, N);
  return new Nfg_Portion<double>(*N);
}

//
// A few temporary functions for modifying game parameters.  Don't document!
// :-)
//

#include "infoset.h"
#include "player.h"

Portion *GSM_SetOutcome(Portion **param)
{
  ExtForm<double> *E = &((Efg_Portion<double> *) param[0])->Value();
  int outc = (((numerical_Portion<gInteger> *) param[1])->Value()).as_long();
  int pl   = (((numerical_Portion<gInteger> *) param[2])->Value()).as_long();
  double value = (((numerical_Portion<double> *) param[3])->Value());

  ((OutcomeVector<double> &) *E->OutcomeList()[outc])[pl] = value;
  return new numerical_Portion<double>(value);
}

Portion *GSM_SetActionProb(Portion **param)
{
  ExtForm<double> *E = &((Efg_Portion<double> *) param[0])->Value();
  int iset = (((numerical_Portion<gInteger> *) param[1])->Value()).as_long();
  int act  = (((numerical_Portion<gInteger> *) param[2])->Value()).as_long();
  double value = (((numerical_Portion<double> *) param[3])->Value());

  ((ChanceInfoset<double> &) *E->GetChance()->InfosetList()[iset]).SetActionProb(act, value);
  return new numerical_Portion<double>(value);
}


//
// These appear here only temporarily until I figure out the best place
// for them
//

#include "gconvert.h"

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

void Init_efgfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("ReadEfg");
  FuncObj->SetFuncInfo(GSM_ReadEfg, 1);
  FuncObj->SetParamInfo(GSM_ReadEfg, 0, "f", porSTRING, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("WriteEfg");
  FuncObj->SetFuncInfo(GSM_WriteEfg, 2);
  FuncObj->SetParamInfo(GSM_WriteEfg, 0, "E", porEFG_DOUBLE, NO_DEFAULT_VALUE);
  FuncObj->SetParamInfo(GSM_WriteEfg, 1, "f", porSTREAM, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("GobitEfg");
  FuncObj->SetFuncInfo(GSM_GobitEfg, 10);
  FuncObj->SetParamInfo(GSM_GobitEfg, 0, "E", porEFG_DOUBLE, NO_DEFAULT_VALUE);
  FuncObj->SetParamInfo(GSM_GobitEfg, 1, "pxifile", porSTREAM, NO_DEFAULT_VALUE);
  FuncObj->SetParamInfo(GSM_GobitEfg, 2, "minLam", porDOUBLE,
		        new numerical_Portion<double>(.01));
  FuncObj->SetParamInfo(GSM_GobitEfg, 3, "maxLam", porDOUBLE,
		        new numerical_Portion<double>(30));
  FuncObj->SetParamInfo(GSM_GobitEfg, 4, "delLam", porDOUBLE,
		        new numerical_Portion<double>(.01));
  FuncObj->SetParamInfo(GSM_GobitEfg, 5, "maxitsOpt", porINTEGER,
		        new numerical_Portion<gInteger>(20));
  FuncObj->SetParamInfo(GSM_GobitEfg, 6, "maxitsBrent", porINTEGER,
		        new numerical_Portion<gInteger>(100));
  FuncObj->SetParamInfo(GSM_GobitEfg, 7, "tolOpt", porDOUBLE,
		        new numerical_Portion<double>(1.0e-10));
  FuncObj->SetParamInfo(GSM_GobitEfg, 8, "tolBrent", porDOUBLE,
		        new numerical_Portion<double>(2.0e-10));
  FuncObj->SetParamInfo(GSM_GobitEfg, 9, "time", porDOUBLE,
			new numerical_Portion<double>(0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LiapEfg");
  FuncObj->SetFuncInfo(GSM_LiapEfg, 1);
  FuncObj->SetParamInfo(GSM_LiapEfg, 0, "E", porEFG_DOUBLE, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("EfgToNfg");
  FuncObj->SetFuncInfo(GSM_EfgToNfg, 1);
  FuncObj->SetParamInfo(GSM_EfgToNfg, 0, "E", porEFG_DOUBLE, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetOutcome");
  FuncObj->SetFuncInfo(GSM_SetOutcome, 4);
  FuncObj->SetParamInfo(GSM_SetOutcome, 0, "E", porEFG_DOUBLE, NO_DEFAULT_VALUE);
  FuncObj->SetParamInfo(GSM_SetOutcome, 1, "outc", porINTEGER, NO_DEFAULT_VALUE);
  FuncObj->SetParamInfo(GSM_SetOutcome, 2, "pl", porINTEGER, NO_DEFAULT_VALUE);
  FuncObj->SetParamInfo(GSM_SetOutcome, 3, "value", porDOUBLE, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetActionProb");
  FuncObj->SetFuncInfo(GSM_SetActionProb, 4);
  FuncObj->SetParamInfo(GSM_SetActionProb, 0, "E", porEFG_DOUBLE, NO_DEFAULT_VALUE);
  FuncObj->SetParamInfo(GSM_SetActionProb, 1, "iset", porINTEGER, NO_DEFAULT_VALUE);
  FuncObj->SetParamInfo(GSM_SetActionProb, 2, "act", porINTEGER, NO_DEFAULT_VALUE);
  FuncObj->SetParamInfo(GSM_SetActionProb, 3, "value", porDOUBLE, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Text");
  FuncObj->SetFuncInfo(GSM_TextInt, 1);
  FuncObj->SetParamInfo(GSM_TextInt, 0, "v", porINTEGER, NO_DEFAULT_VALUE);

  FuncObj->SetFuncInfo(GSM_TextFloat, 1);
  FuncObj->SetParamInfo(GSM_TextFloat, 0, "v", porDOUBLE, NO_DEFAULT_VALUE);

  FuncObj->SetFuncInfo(GSM_TextRat, 1);
  FuncObj->SetParamInfo(GSM_TextRat, 0, "v", porRATIONAL, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);
}



#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class Behav_List_Portion<double>;

