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
#include "mixed.h"


//---------------------------------------------------------------
//                  default Nfg related functions
//---------------------------------------------------------------


extern GSM* _CurrentGSM;

Portion* GSM_DefaultNfg( Portion** param )
{
  return _CurrentGSM->DefaultNfg()->ValCopy();
}

Portion *GSM_ReadDefaultNfg(Portion **param)
{
  gInput &f = ((InputPortion *) param[0])->Value();
  
  if (f.IsValid())  
  {
    NormalForm<double> *N = new NormalForm<double>(f);
    delete _CurrentGSM->DefaultNfg();
    _CurrentGSM->DefaultNfg() = new NfgValPortion<double>(*N);
    return new BoolValPortion( true );
  }
  else
    return 0;
}

Portion* GSM_CopyDefaultNfg( Portion** param )
{
  delete _CurrentGSM->DefaultNfg();
  _CurrentGSM->DefaultNfg() = param[0]->ValCopy();
  return param[0]->ValCopy();
}


//----------------------------------------------------------------------


//
// These functions are added to the function list in efgfunc.cc along with
// their extensive form counterparts.
// What's a good way of dealing with these sorts of functions?
//
Portion *GSM_CentroidNfgFloat(Portion **param)
{
  NormalForm<double> &N = ((NfgPortion<double> *) param[0])->Value();
  MixedProfile<double> *P = new MixedProfile<double>(N);
  return new MixedValPortion<double>(*P);
}

Portion *GSM_CentroidNfgRational(Portion **param)
{
  NormalForm<gRational> &N = ((NfgPortion<gRational> *) param[0])->Value();
  MixedProfile<gRational> *P = new MixedProfile<gRational>(N);
  return new MixedValPortion<gRational>(*P);
}

Portion *GSM_NumPlayersNfg(Portion **param)
{
  BaseNormalForm &N = ((BaseNfgPortion *) param[0])->Value();
  return new IntValPortion(N.NumPlayers());
}

Portion *GSM_NameNfg(Portion **param)
{
  BaseNormalForm &N = ((BaseNfgPortion *) param[0])->Value();
  return new TextValPortion(N.GetTitle());
}

Portion *GSM_SetNameNfg(Portion **param)
{
  BaseNormalForm &N = ((BaseNfgPortion *) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  N.SetTitle(name);
  return param[0]->ValCopy();
}


#include "gwatch.h"

Portion *GSM_ElimAllDom(Portion **param)
{
  BaseNormalForm &N = ((BaseNfgPortion *) param[0])->Value();
  bool strong = ((BoolPortion *) param[1])->Value();

  gWatch watch;

  N.FindAllDominated((strong) ? STRONG : WEAK, &gout);

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();

  return new IntValPortion(N.NumStratSets());
}

Portion *GSM_ElimNDom(Portion **param)
{
  int maxiter = ((IntPortion *) param[0])->Value();
  BaseNormalForm &N = ((BaseNfgPortion *) param[1])->Value();
  bool strong = ((BoolPortion *) param[2])->Value();
  
  gWatch watch;
  gBlock<int> players(N.NumPlayers());
  int i;
  for (i = 1; i <= players.Length(); i++)   players[i] = i;

  for (i = 1; i <= maxiter && 
       N.FindDominated(players, (strong) ? STRONG : WEAK, &gout);
       i++);

  ((FloatPortion *) param[3])->Value() = watch.Elapsed();
 
  return new IntValPortion(N.NumStratSets());
}



template <class T> class Mixed_ListPortion : public ListValPortion   {
  public:
    Mixed_ListPortion(NormalForm<T> *, const gList<gPVector<T> > &);
};

Mixed_ListPortion<double>::Mixed_ListPortion(NormalForm<double> *N,
			       const gList<gPVector<double> > &list)
{
  _DataType = porMIXED_FLOAT;
  for (int i = 1; i <= list.Length(); i++)
    Append(new MixedValPortion<double>(* new MixedProfile<double>(*N, list[i])));
}

Mixed_ListPortion<gRational>::Mixed_ListPortion(NormalForm<gRational> *N,
						const gList<gPVector<gRational> > &list)
{
  _DataType = porMIXED_RATIONAL;
  for (int i = 1; i <= list.Length(); i++)
    Append(new MixedValPortion<gRational>(* new MixedProfile<gRational>(*N, list[i])));
}

#include "enum.h"

Portion *GSM_EnumFloat(Portion **param)
{
  NormalForm<double> &N = ((NfgPortion<double> *) param[0])->Value();

  EnumParams EP;

  EP.stopAfter = ((IntPortion *) param[1])->Value();
  
  EnumModule<double> EM(N, EP);
  EM.Enum();

  ((FloatPortion *) param[2])->Value() = EM.Time();

  return new Mixed_ListPortion<double>(&N, EM.GetSolutions());
}

Portion *GSM_EnumRational(Portion **param)
{
  NormalForm<gRational> &N = ((NfgPortion<gRational> *) param[0])->Value();

  EnumParams EP;

  EP.stopAfter = ((IntPortion *) param[1])->Value();
  
  EnumModule<gRational> EM(N, EP);
  EM.Enum();

  ((FloatPortion *) param[2])->Value() = EM.Time();

  return new Mixed_ListPortion<gRational>(&N, EM.GetSolutions());
}

#include "ngobit.h"

Portion *GSM_GobitNfg(Portion **param)
{
  NFGobitParams<double> NP;

  NP.pxifile = &((OutputPortion *) param[1])->Value();
  NP.minLam = ((FloatPortion *) param[3])->Value();
  NP.maxLam = ((FloatPortion *) param[4])->Value();
  NP.delLam = ((FloatPortion *) param[5])->Value();
  NP.powLam = ((IntPortion *) param[6])->Value();

  NFGobitModule<double> M(((NfgPortion<double> *) param[0])->Value(), NP);
  M.Gobit(1);

  ((FloatPortion *) param[2])->Value() = M.Time();

  return new IntValPortion(1);
}

#include "grid.h"

Portion *GSM_GridSolveFloat(Portion **param)
{
  NormalForm<double> &N = ((NfgPortion<double> *) param[0])->Value();
  
  GridParams<double> GP;
  GP.pxifile = &((OutputPortion *) param[1])->Value();
  GP.minLam = ((FloatPortion *) param[2])->Value();
  GP.maxLam = ((FloatPortion *) param[3])->Value();
  GP.delLam = ((FloatPortion *) param[4])->Value();
  GP.type = ((IntPortion *) param[5])->Value();
  GP.delp = ((FloatPortion *) param[6])->Value();
  GP.tol = ((FloatPortion *) param[7])->Value();

  GridSolveModule<double> GM(N, GP);
  GM.GridSolve();

//  ((IntPortion *) param[8])->Value() = GM.NumEvals();
//  ((FloatPortion *) param[9])->Value() = GM.Time();

  gList<gPVector<double> > solns;
  return new Mixed_ListPortion<double>(&N, solns);
}

Portion *GSM_GridSolveRational(Portion **param)
{
  NormalForm<gRational> &N = ((NfgPortion<gRational> *) param[0])->Value();
  
  GridParams<gRational> GP;
  GP.pxifile = &((OutputPortion *) param[1])->Value();
  GP.minLam = ((RationalPortion *) param[2])->Value();
  GP.maxLam = ((RationalPortion *) param[3])->Value();
  GP.delLam = ((RationalPortion *) param[4])->Value();
  GP.type = ((IntPortion *) param[5])->Value();
  GP.delp = ((RationalPortion *) param[6])->Value();
  GP.tol = ((RationalPortion *) param[7])->Value();

  GridSolveModule<gRational> GM(N, GP);
  GM.GridSolve();

//  ((IntPortion *) param[8])->Value() = GM.NumEvals();
//  ((FloatPortion *) param[9])->Value() = GM.Time();

  gList<gPVector<gRational> > solns;
  return new Mixed_ListPortion<gRational>(&N, solns);
}

#include "lemke.h"

Portion *GSM_Lemke(Portion **param)
{
  NormalForm<double> &N = ((NfgPortion<double> *) param[0])->Value();

  LemkeParams LP;
  LemkeModule<double> LS(N, LP);
  LS.Lemke();

  ((FloatPortion *) param[2])->Value() = (double) LS.Time();
  ((IntPortion *) param[3])->Value() = LS.NumPivots();

  return new Mixed_ListPortion<double>(&N, LS.GetSolutions());
}

#include "nliap.h"

Portion *GSM_LiapNfg(Portion **param)
{
  NormalForm<double> &N = ((NfgPortion<double> *) param[0])->Value();

  NFLiapParams<double> LP;
  LP.nequilib = ((IntPortion *) param[3])->Value();
  LP.ntries = ((IntPortion *) param[4])->Value();
  
  NFLiapModule<double> LM(N, LP);
  LM.Liap(1);

  ((FloatPortion *) param[1])->Value() = LM.Time();
  ((IntPortion *) param[2])->Value() = LM.NumEvals();

  return new Mixed_ListPortion<double>(&N, LM.GetSolutions());
}

#include "simpdiv.h"

Portion *GSM_SimpdivFloat(Portion **param)
{
  NormalForm<double> &N = ((NfgPortion<double> *) param[0])->Value();
  
  SimpdivParams SP;
  SP.number = ((IntPortion *) param[1])->Value();
  SP.leash = ((IntPortion *) param[3])->Value();

  SimpdivModule<double> SM(N, SP);
  SM.Simpdiv();

  ((IntPortion *) param[4])->Value() = SM.NumEvals();
  ((FloatPortion *) param[5])->Value() = SM.Time();
  
  return new Mixed_ListPortion<double>(&N, SM.GetSolutions());
}

Portion *GSM_SimpdivRational(Portion **param)
{
  NormalForm<gRational> &N = ((NfgPortion<gRational> *) param[0])->Value();
  
  SimpdivParams SP;
  SP.number = ((IntPortion *) param[1])->Value();
  SP.leash = ((IntPortion *) param[3])->Value();

  SimpdivModule<gRational> SM(N, SP);
  SM.Simpdiv();

  ((IntPortion *) param[4])->Value() = SM.NumEvals();
  ((FloatPortion *) param[5])->Value() = SM.Time();
  
  return new Mixed_ListPortion<gRational>(&N, SM.GetSolutions());
}

#include "purenash.h"

Portion *GSM_PureNashFloat(Portion **param)
{
  NormalForm<double> &N = ((NfgPortion<double> *) param[0])->Value();

  gList<gPVector<double> > solns;

  gWatch watch;

  FindPureNash(N, solns);

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  return new Mixed_ListPortion<double>(&N, solns);
}

Portion *GSM_PureNashRational(Portion **param)
{
  NormalForm<gRational> &N = ((NfgPortion<gRational> *) param[0])->Value();

  gList<gPVector<gRational> > solns;

  gWatch watch;

  FindPureNash(N, solns);

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  return new Mixed_ListPortion<gRational>(&N, solns);
}
 
Portion *GSM_ReadNfg(Portion **param)
{
  gInput &f = ((InputPortion *) param[0])->Value();
  
  if (f.IsValid())  {
    NormalForm<double> *N = new NormalForm<double>(f);
    return new NfgValPortion<double>(*N);
  }
  else
    return new ErrorPortion("ReadNfg: Couldn't open file for reading\n");
}

Portion *GSM_NewNfg(Portion **param)
{
  ListPortion *dim = ((ListPortion *) param[0]);
  gArray<int> d(dim->Length());
  
  for (int i = 1; i <= dim->Length(); i++)
    d[i] = ((IntPortion *) dim->Subscript(i))->Value();

  bool random = ((BoolPortion *) param[1])->Value();
  int seed = ((BoolPortion *) param[2])->Value();

  NormalForm<double> *N = new NormalForm<double>(d, random, seed);

  return new NfgValPortion<double>(*N);
}

Portion *GSM_WriteNfg(Portion **param)
{
  gOutput &f = ((OutputPortion *) param[0])->Value();
  BaseNormalForm &N = ((BaseNfgPortion *) param[1])->Value();
  int sset = ((IntPortion *) param[2])->Value();

  N.WriteNfgFile(f, sset);
  return new OutputRefPortion(f);
}

//---------------------------------------------------------------------


void Init_nfgfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("ElimDom");
  FuncObj->SetFuncInfo(GSM_ElimAllDom, 3);
  FuncObj->SetParamInfo(GSM_ElimAllDom, 0, "nfg", porNFG, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE, DEFAULT_NFG );
  FuncObj->SetParamInfo(GSM_ElimAllDom, 1, "strong", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_ElimAllDom, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Enum");
  FuncObj->SetFuncInfo(GSM_EnumFloat, 3);
  FuncObj->SetParamInfo(GSM_EnumFloat, 0, "nfg", 
			porNFG_FLOAT, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE, DEFAULT_NFG );
  FuncObj->SetParamInfo(GSM_EnumFloat, 1, "stopAfter", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_EnumFloat, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_EnumRational, 3);
  FuncObj->SetParamInfo(GSM_EnumRational, 0, "nfg", 
			porNFG_RATIONAL, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE, DEFAULT_NFG );
  FuncObj->SetParamInfo(GSM_EnumRational, 1, "stopAfter", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_EnumRational, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("GobitNfg");
  FuncObj->SetFuncInfo(GSM_GobitNfg, 7);
  FuncObj->SetParamInfo(GSM_GobitNfg, 0, "nfg", 
			porNFG_FLOAT, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE, DEFAULT_NFG );
  FuncObj->SetParamInfo(GSM_GobitNfg, 1, "pxifile", porOUTPUT,
			new OutputRefPortion(gnull));
  FuncObj->SetParamInfo(GSM_GobitNfg, 2, "time", porFLOAT,
			new FloatValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GobitNfg, 3, "minLam", porFLOAT,
			new FloatRefPortion(Gobit_default_minLam));
  FuncObj->SetParamInfo(GSM_GobitNfg, 4, "maxLam", porFLOAT,
			new FloatRefPortion(Gobit_default_maxLam));
  FuncObj->SetParamInfo(GSM_GobitNfg, 5, "delLam", porFLOAT,
			new FloatRefPortion(Gobit_default_delLam));
  FuncObj->SetParamInfo(GSM_GobitNfg, 6, "powLam", porINTEGER,
			new IntValPortion(1));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("GridSolve");
  FuncObj->SetFuncInfo(GSM_GridSolveFloat, 10);
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE, DEFAULT_NFG);
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 1, "pxifile", porOUTPUT,
			new OutputValPortion(gnull));
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 2, "minLam", porFLOAT,
			new FloatValPortion(.01));
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 3, "maxLam", porFLOAT,
			new FloatValPortion(30));
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 4, "delLam", porFLOAT,
			new FloatValPortion(.01));
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 5, "powLam", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 6, "delp", porFLOAT,
			new FloatValPortion(.01));
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 7, "tol", porFLOAT,
			new FloatValPortion(.01));
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 8, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 9, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_GridSolveRational, 10);
  FuncObj->SetParamInfo(GSM_GridSolveRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE, DEFAULT_NFG);
  FuncObj->SetParamInfo(GSM_GridSolveRational, 1, "pxifile", porOUTPUT,
			new OutputValPortion(gnull));
  FuncObj->SetParamInfo(GSM_GridSolveRational, 2, "minLam", porRATIONAL,
			new FloatValPortion(.01));
  FuncObj->SetParamInfo(GSM_GridSolveRational, 3, "maxLam", porRATIONAL,
			new FloatValPortion(30));
  FuncObj->SetParamInfo(GSM_GridSolveRational, 4, "delLam", porRATIONAL,
			new FloatValPortion(.01));
  FuncObj->SetParamInfo(GSM_GridSolveRational, 5, "powLam", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_GridSolveRational, 6, "delp", porRATIONAL,
			new FloatValPortion(.01));
  FuncObj->SetParamInfo(GSM_GridSolveRational, 7, "tol", porRATIONAL,
			new FloatValPortion(.01));
  FuncObj->SetParamInfo(GSM_GridSolveRational, 8, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_GridSolveRational, 9, "time", porRATIONAL,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Lemke");
  FuncObj->SetFuncInfo(GSM_Lemke, 4);
  FuncObj->SetParamInfo(GSM_Lemke, 0, "nfg",
			porNFG_FLOAT, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE, DEFAULT_NFG );
  FuncObj->SetParamInfo(GSM_Lemke, 1, "stopAfter", 
			porINTEGER, new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_Lemke, 2, "nPivots", 
			porINTEGER, new IntValPortion(0),
		        PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_Lemke, 3, "time", 
			porFLOAT, new FloatValPortion(0),
			PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LiapNfg");
  FuncObj->SetFuncInfo(GSM_LiapNfg, 5);
  FuncObj->SetParamInfo(GSM_LiapNfg, 0, "nfg", porNFG_FLOAT, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE, DEFAULT_NFG);
  FuncObj->SetParamInfo(GSM_LiapNfg, 1, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapNfg, 2, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_LiapNfg, 3, "stopAfter", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_LiapNfg, 4, "nTries", porINTEGER,
			new IntValPortion(10));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("PureNash");
  FuncObj->SetFuncInfo(GSM_PureNashFloat, 3);
  FuncObj->SetParamInfo(GSM_PureNashFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE, DEFAULT_NFG);
  FuncObj->SetParamInfo(GSM_PureNashFloat, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_PureNashFloat, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_PureNashRational, 3);
  FuncObj->SetParamInfo(GSM_PureNashRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE, DEFAULT_NFG);
  FuncObj->SetParamInfo(GSM_PureNashRational, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_PureNashRational, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SimpDiv");
  FuncObj->SetFuncInfo(GSM_SimpdivFloat, 6);
  FuncObj->SetParamInfo(GSM_SimpdivFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE, DEFAULT_NFG);
  FuncObj->SetParamInfo(GSM_SimpdivFloat, 1, "stopAfter", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_SimpdivFloat, 2, "nRestarts", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_SimpdivFloat, 3, "leashLength", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_SimpdivFloat, 4, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SimpdivFloat, 5, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_SimpdivRational, 6);
  FuncObj->SetParamInfo(GSM_SimpdivRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE, DEFAULT_NFG);
  FuncObj->SetParamInfo(GSM_SimpdivRational, 1, "stopAfter", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_SimpdivRational, 2, "nRestarts", porINTEGER,
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_SimpdivRational, 3, "leashLength", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_SimpdivRational, 4, "nEvals", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SimpdivRational, 5, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NewNfg");
  FuncObj->SetFuncInfo(GSM_NewNfg, 3);
  FuncObj->SetParamInfo(GSM_NewNfg, 0, "dim", porLIST | porINTEGER);
  FuncObj->SetParamInfo(GSM_NewNfg, 1, "random", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_NewNfg, 2, "seed", porINTEGER,
			new IntValPortion(0));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ReadNfg");
  FuncObj->SetFuncInfo(GSM_ReadNfg, 1);
  FuncObj->SetParamInfo(GSM_ReadNfg, 0, "file",	porINPUT);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("WriteNfg");
  FuncObj->SetFuncInfo(GSM_WriteNfg, 3);
  FuncObj->SetParamInfo(GSM_WriteNfg, 0, "output", porOUTPUT);
  FuncObj->SetParamInfo(GSM_WriteNfg, 1, "nfg", porNFG, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE, DEFAULT_NFG );
  FuncObj->SetParamInfo(GSM_WriteNfg, 2, "sset", porINTEGER,
		        new IntValPortion(1));
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
  FuncObj->SetParamInfo(GSM_CopyDefaultNfg, 0, "nfg", porNFG, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );
  gsm->AddFunction(FuncObj);

}



#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class Mixed_ListPortion<double>;
TEMPLATE class Mixed_ListPortion<gRational>;
