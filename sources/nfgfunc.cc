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

#include "enum.h"

Portion *GSM_Enum(Portion **param)
{
  NormalForm<double> *N = &((Nfg_Portion<double> *) param[0])->Value();

  EnumParams EP;
  EnumModule<double> EM(*N, EP);
  EM.Enum();

  return new Mixed_List_Portion<double>(N, EM.GetSolutions());
}

#include "ngobit.h"

//
// GobitNfg: Parameter assignments:
//
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
Portion *GSM_GobitNfg(Portion **param)
{
  NFGobitParams<double> EP;

  EP.pxifile = &((Stream_Portion *) param[1])->Value();
  EP.minLam = ((numerical_Portion<double> *) param[2])->Value();
  EP.maxLam = ((numerical_Portion<double> *) param[3])->Value();
  EP.delLam = ((numerical_Portion<double> *) param[4])->Value();
  EP.maxitsOpt = ((numerical_Portion<gInteger> *) param[5])->Value().as_long();
  EP.maxitsBrent = ((numerical_Portion<gInteger> *) param[6])->Value().as_long();
  EP.tolOpt = ((numerical_Portion<double> *) param[7])->Value();
  EP.tolBrent = ((numerical_Portion<double> *) param[8])->Value();

  NFGobitModule<double> M(((Nfg_Portion<double> *) param[0])->Value(), EP);
  M.Gobit(1);

  ((numerical_Portion<double> *) param[9])->Value() = M.Time();

  return new numerical_Portion<gInteger>(1);
}

#include "lemke.h"

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

#include "nliap.h"

Portion *GSM_LiapNfg(Portion **param)
{
  NormalForm<double> &N = ((Nfg_Portion<double> *) param[0])->Value();

  NFLiapParams<double> LP;
  NFLiapModule<double> LM(N, LP);
  LM.Liap(1);

  return new Mixed_List_Portion<double>(&N, LM.GetSolutions());
}

#include "simpdiv.h"

Portion *GSM_Simpdiv(Portion **param)
{
  NormalForm<double> &N = ((Nfg_Portion<double> *) param[0])->Value();

  SimpdivParams SP;
  SimpdivModule<double> SM(N, SP);
  SM.Simpdiv();

  return new Mixed_List_Portion<double>(&N, SM.GetSolutions());
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

  FuncObj = new FuncDescObj("DisplayNfg");
  FuncObj->SetFuncInfo(GSM_DisplayNfg, 1);
  FuncObj->SetParamInfo(GSM_DisplayNfg, 0, "N", porNFG, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Enum");
  FuncObj->SetFuncInfo(GSM_Enum, 1);
  FuncObj->SetParamInfo(GSM_Enum, 0, "N", porNFG, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("GobitNfg");
  FuncObj->SetFuncInfo(GSM_GobitNfg, 10);
  FuncObj->SetParamInfo(GSM_GobitNfg, 0, "N", porNFG_DOUBLE, NO_DEFAULT_VALUE);
  FuncObj->SetParamInfo(GSM_GobitNfg, 1, "pxifile", porSTREAM,
			NO_DEFAULT_VALUE);
  FuncObj->SetParamInfo(GSM_GobitNfg, 2, "minLam", porDOUBLE,
		        new numerical_Portion<double>(.01));
  FuncObj->SetParamInfo(GSM_GobitNfg, 3, "maxLam", porDOUBLE,
		        new numerical_Portion<double>(30));
  FuncObj->SetParamInfo(GSM_GobitNfg, 4, "delLam", porDOUBLE,
		        new numerical_Portion<double>(.01));
  FuncObj->SetParamInfo(GSM_GobitNfg, 5, "maxitsOpt", porINTEGER,
		        new numerical_Portion<gInteger>(20));
  FuncObj->SetParamInfo(GSM_GobitNfg, 6, "maxitsBrent", porINTEGER,
		        new numerical_Portion<gInteger>(100));
  FuncObj->SetParamInfo(GSM_GobitNfg, 7, "tolOpt", porDOUBLE,
		        new numerical_Portion<double>(1.0e-10));
  FuncObj->SetParamInfo(GSM_GobitNfg, 8, "tolBrent", porDOUBLE,
		        new numerical_Portion<double>(2.0e-10));
  FuncObj->SetParamInfo(GSM_GobitNfg, 9, "time", porDOUBLE,
			new numerical_Portion<double>(0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Lemke");
  FuncObj->SetFuncInfo(GSM_Lemke, 4);
  FuncObj->SetParamInfo(GSM_Lemke, 0, "N", porNFG_DOUBLE, NO_DEFAULT_VALUE);
  FuncObj->SetParamInfo(GSM_Lemke, 1, "nequilib", 
			porINTEGER, new numerical_Portion<gInteger>( 0 ) );
  FuncObj->SetParamInfo(GSM_Lemke, 2, "time", 
			porDOUBLE, new numerical_Portion<double>( 0 ),
			PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lemke, 3, "npivots", 
			porINTEGER, new numerical_Portion<gInteger>( 0 ) );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LiapNfg");
  FuncObj->SetFuncInfo(GSM_LiapNfg, 1);
  FuncObj->SetParamInfo(GSM_LiapNfg, 0, "N", porNFG_DOUBLE, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Simpdiv");
  FuncObj->SetFuncInfo(GSM_Simpdiv, 1);
  FuncObj->SetParamInfo(GSM_Simpdiv, 0, "N", porNFG_DOUBLE, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ReadNfg");
  FuncObj->SetFuncInfo(GSM_ReadNfg, 1);
  FuncObj->SetParamInfo(GSM_ReadNfg, 0, "file",	porSTRING, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

}


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class Mixed_List_Portion<double>;
