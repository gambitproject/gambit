//#
//# FILE: nfgfunc.cc -- Normal form command language builtins
//#
//# $Id$
//#


#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "normal.h"

#include "glist.h"


//---------------------------------------------------------------
//                  default Nfg related functions
//---------------------------------------------------------------


extern Portion* _DefaultNfgShadow;

Portion* GSM_DefaultNfg( Portion** param )
{
  return _DefaultNfgShadow->ShadowOf()->Copy();
}

Portion *GSM_ReadDefaultNfg(Portion **param)
{
  gInput &f = ((Input_Portion *) param[0])->Value();
  
  if (f.IsValid())  
  {
    NormalForm<double> *N = new NormalForm<double>(f);
    delete _DefaultNfgShadow->ShadowOf();
    _DefaultNfgShadow->ShadowOf() = new Nfg_Portion<double>(*N);
    return new bool_Portion( true );
  }
  else
    return 0;
}

Portion* GSM_CopyDefaultNfg( Portion** param )
{
  delete _DefaultNfgShadow->ShadowOf();
  _DefaultNfgShadow->ShadowOf() = param[0]->Copy();
  return param[0]->Copy();
}


//----------------------------------------------------------------------


Portion *GSM_ElimDom(Portion **param)
{
  NormalForm<double> *N = &((Nfg_Portion<double> *) param[0])->Value();
  
  N->FindAllDominated(STRONG, &gout);

  return new numerical_Portion<gInteger>(1);
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

Portion *GSM_GobitNfg(Portion **param)
{
  NFGobitParams<double> NP;

  NP.pxifile = &((Output_Portion *) param[1])->Value();
  NP.minLam = ((numerical_Portion<double> *) param[3])->Value();
  NP.maxLam = ((numerical_Portion<double> *) param[4])->Value();
  NP.delLam = ((numerical_Portion<double> *) param[5])->Value();
  NP.powLam = ((numerical_Portion<gInteger> *) param[6])->Value().as_long();

  NFGobitModule<double> M(((Nfg_Portion<double> *) param[0])->Value(), NP);
  M.Gobit(1);

  ((numerical_Portion<double> *) param[2])->Value() = M.Time();

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

#include "purenash.h"

Portion *GSM_PureNash(Portion **param)
{
  NormalForm<double> &N = ((Nfg_Portion<double> *) param[0])->Value();

  gList<gPVector<double> > solns;
  FindPureNash(N, solns);
  return new Mixed_List_Portion<double>(&N, solns);
}
 
Portion *GSM_ReadNfg(Portion **param)
{
  gInput &f = ((Input_Portion *) param[0])->Value();
  
  if (f.IsValid())  {
    NormalForm<double> *N = new NormalForm<double>(f);
    return new Nfg_Portion<double>(*N);
  }
  else
    return 0;
}

Portion *GSM_WriteNfg(Portion **param)
{
  gOutput &f = ((Output_Portion *) param[0])->Value();
  BaseNormalForm &N = ((BaseNfg_Portion *) param[1])->Value();
  int sset = ((numerical_Portion<gInteger> *) param[2])->Value().as_long();

  N.WriteNfgFile(f, sset);
  return new Output_Portion(f);
}

/*
Portion *GSM_NumPlayers(Portion **param)
{
  NormalForm<double> *N = &((Nfg_Portion<double> *) param[0])->Value();
  return new numerical_Portion<gInteger>(N->NumPlayers());
}
*/



//---------------------------------------------------------------------


void Init_nfgfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("ElimDom");
  FuncObj->SetFuncInfo(GSM_ElimDom, 1);
  FuncObj->SetParamInfo(GSM_ElimDom, 0, "nfg", porNFG_DOUBLE,
			_DefaultNfgShadow);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Enum");
  FuncObj->SetFuncInfo(GSM_Enum, 1);
  FuncObj->SetParamInfo(GSM_Enum, 0, "nfg", porNFG_DOUBLE,
		        _DefaultNfgShadow);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("GobitNfg");
  FuncObj->SetFuncInfo(GSM_GobitNfg, 8);
  FuncObj->SetParamInfo(GSM_GobitNfg, 0, "nfg", porNFG_DOUBLE,
		        _DefaultNfgShadow);
  FuncObj->SetParamInfo(GSM_GobitNfg, 1, "pxifile", porOUTPUT,
			new Output_Portion(gnull));
  FuncObj->SetParamInfo(GSM_GobitNfg, 2, "time", porDOUBLE,
			new numerical_Portion<double>(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitNfg, 3, "minLam", porDOUBLE,
			new numerical_Portion<double>(Gobit_default_minLam, true));
  FuncObj->SetParamInfo(GSM_GobitNfg, 4, "maxLam", porDOUBLE,
			new numerical_Portion<double>(Gobit_default_maxLam, true));
  FuncObj->SetParamInfo(GSM_GobitNfg, 5, "delLam", porDOUBLE,
			new numerical_Portion<double>(Gobit_default_delLam, true));
  FuncObj->SetParamInfo(GSM_GobitNfg, 6, "powLam", porINTEGER,
			new numerical_Portion<gInteger>(1));
  FuncObj->SetParamInfo(GSM_GobitNfg, 7, "start", porLIST | porDOUBLE,
			new List_Portion);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Lemke");
  FuncObj->SetFuncInfo(GSM_Lemke, 4);
  FuncObj->SetParamInfo(GSM_Lemke, 0, "nfg", porNFG_DOUBLE,
		        _DefaultNfgShadow);
  FuncObj->SetParamInfo(GSM_Lemke, 1, "stopAfter", 
			porINTEGER, new numerical_Portion<gInteger>(0));
  FuncObj->SetParamInfo(GSM_Lemke, 2, "nPivots", 
			porINTEGER, new numerical_Portion<gInteger>(0),
		        PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lemke, 3, "time", 
			porDOUBLE, new numerical_Portion<double>(0),
			PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LiapNfg");
  FuncObj->SetFuncInfo(GSM_LiapNfg, 1);
  FuncObj->SetParamInfo(GSM_LiapNfg, 0, "N", porNFG_DOUBLE);
  gsm->AddFunction(FuncObj);

/*
  FuncObj = new FuncDescObj("NumPlayers");
  FuncObj->SetFuncInfo(GSM_NumPlayers, 1);
  FuncObj->SetParamInfo(GSM_NumPlayers, 0, "N", porNFG_DOUBLE);
  gsm->AddFunction(FuncObj);
  */
  FuncObj = new FuncDescObj("PureNash");
  FuncObj->SetFuncInfo(GSM_PureNash, 1);
  FuncObj->SetParamInfo(GSM_PureNash, 0, "N", porNFG_DOUBLE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Simpdiv");
  FuncObj->SetFuncInfo(GSM_Simpdiv, 1);
  FuncObj->SetParamInfo(GSM_Simpdiv, 0, "N", porNFG_DOUBLE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ReadNfg");
  FuncObj->SetFuncInfo(GSM_ReadNfg, 1);
  FuncObj->SetParamInfo(GSM_ReadNfg, 0, "file",	porINPUT);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("WriteNfg");
  FuncObj->SetFuncInfo(GSM_WriteNfg, 3);
  FuncObj->SetParamInfo(GSM_WriteNfg, 0, "output", porOUTPUT);
  FuncObj->SetParamInfo(GSM_WriteNfg, 1, "nfg", porNFG);
  FuncObj->SetParamInfo(GSM_WriteNfg, 2, "sset", porINTEGER);
  gsm->AddFunction(FuncObj);


  //---------- default Nfg functions ---------------
  FuncObj = new FuncDescObj("DefaultNfg");
  FuncObj->SetFuncInfo(GSM_DefaultNfg, 0);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ReadDefaultNfg");
  FuncObj->SetFuncInfo(GSM_ReadDefaultNfg, 1);
  FuncObj->SetParamInfo(GSM_ReadDefaultNfg, 0, "file", porINPUT);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("CopyDefaultNfg");
  FuncObj->SetFuncInfo(GSM_CopyDefaultNfg, 1);
  FuncObj->SetParamInfo(GSM_CopyDefaultNfg, 0, "nfg", porNFG);
  gsm->AddFunction(FuncObj);

}



#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class Mixed_List_Portion<double>;
