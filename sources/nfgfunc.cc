//#
//# FILE: nfgfunc.cc -- Normal form command language builtins
//#
//# $Id$
//#


#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "nfg.h"

#include "glist.h"
#include "mixed.h"

Portion *ArrayToList(const gArray<NFPlayer *> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NfPlayerValPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<Strategy *> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new StrategyValPortion(A[i]));
  return ret;
}

//
// These functions are added to the function list in efgfunc.cc along with
// their extensive form counterparts.
// What's a good way of dealing with these sorts of functions?
//
Portion *GSM_CentroidNfgFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();
  MixedProfile<double> *P = new MixedProfile<double>(N);

  Portion* por = new MixedValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_CentroidNfgRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();
  MixedProfile<gRational> *P = new MixedProfile<gRational>(N);

  Portion* por = new MixedValPortion(P);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_CentroidNFSupport(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  BaseMixedProfile *P;

  if (S->BelongsTo().Type() == DOUBLE)
    P = new MixedProfile<double>((Nfg<double> &) S->BelongsTo(), *S);
  else
    P = new MixedProfile<gRational>((Nfg<gRational> &) S->BelongsTo(), *S);

  Portion *por = new MixedValPortion(P);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

Portion *GSM_NumPlayersNfg(Portion **param)
{
  BaseNfg &N = * ((NfgPortion*) param[0])->Value();
  return new IntValPortion(N.NumPlayers());
}

Portion *GSM_NameNfg(Portion **param)
{
  BaseNfg &N = * ((NfgPortion*) param[0])->Value();
  return new TextValPortion(N.GetTitle());
}

Portion *GSM_PlayersNfg(Portion **param)
{
  BaseNfg &N = *((NfgPortion*) param[0])->Value();

  Portion* p = ArrayToList(N.PlayerList());
  p->SetOwner( param[ 0 ]->Original() );
  p->AddDependency();
  return p;
}

Portion *GSM_AddStrategy(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  Strategy *s = ((StrategyPortion *) param[1])->Value();

  S->GetNFStrategySet(s->nfp->GetNumber())->AddStrategy(s);

  Portion* por = new StrategyValPortion(s);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

Portion *GSM_RemoveStrategy(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  Strategy *s = ((StrategyPortion *) param[1])->Value();
  
  S->GetNFStrategySet(s->nfp->GetNumber())->RemoveStrategy(s);

  Portion* por = new StrategyValPortion(s);
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}

Portion *GSM_SetNameNfg(Portion **param)
{
  BaseNfg &N = * ((NfgPortion*) param[0])->Value();
  gString name = ((TextPortion *) param[1])->Value();
  N.SetTitle(name);
  return param[0]->ValCopy();
}

Portion *GSM_Strategies(Portion **param)
{
  NFPlayer *P = (NFPlayer *) ((NfPlayerPortion *) param[0])->Value();

  Portion *por = ArrayToList(P->StrategyList());
  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}


#include "gwatch.h"

extern NFSupport *ComputeDominated(NFSupport &S, bool strong, 
				   const gArray<int> &players,
				   gOutput &tracefile);


Portion *GSM_ElimDom(Portion **param)
{
  NFSupport *S = ((NfSupportPortion *) param[0])->Value();
  bool strong = ((BoolPortion *) param[1])->Value();
  
  gWatch watch;
  gBlock<int> players(S->BelongsTo().NumPlayers());
  int i;
  for (i = 1; i <= players.Length(); i++)   players[i] = i;

  NFSupport *T = ComputeDominated(*S, strong, players, gout);

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  Portion *por = (T) ? new NfSupportValPortion(T) : new NfSupportValPortion(new NFSupport(*S));

  por->SetOwner(param[0]->Owner());
  por->AddDependency();
  return por;
}



template <class T> class Mixed_ListPortion : public ListValPortion   {
  public:
    Mixed_ListPortion(const gList<MixedProfile<T> > &);
};

Mixed_ListPortion<double>::Mixed_ListPortion(const gList<MixedProfile<double> > &list)
{
  _DataType = porMIXED_FLOAT;
  for (int i = 1; i <= list.Length(); i++)
    Append(new MixedValPortion( new MixedProfile<double>(list[i])));
}

Mixed_ListPortion<gRational>::Mixed_ListPortion(const gList<MixedProfile<gRational> > &list)
{
  _DataType = porMIXED_RATIONAL;
  for (int i = 1; i <= list.Length(); i++)
    Append(new MixedValPortion( new MixedProfile<gRational>(list[i])));
}

#include "csum.h"

Portion *GSM_ConstSumFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();

  ZSumParams ZP;

  NFSupport S(N);
  ZSumModule<double> ZM(N, ZP, S);
  ZM.ZSum();

  ((IntPortion *) param[1])->Value() = ZM.NumPivots();
  ((FloatPortion *) param[2])->Value() = ZM.Time();

  gList<MixedProfile<double> > solns;
  ZM.GetSolutions(solns);

  Portion* por = new Mixed_ListPortion<double>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_ConstSumRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational> *) ((NfgPortion *) param[0])->Value();

  ZSumParams ZP;
  
  NFSupport S(N);
  ZSumModule<gRational> ZM(N, ZP, S);
  ZM.ZSum();

  ((IntPortion *) param[1])->Value() = ZM.NumPivots();
  ((FloatPortion *) param[2])->Value() = ZM.Time();

  gList<MixedProfile<gRational> > solns;
  ZM.GetSolutions(solns);

  Portion* por = new Mixed_ListPortion<gRational>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

#include "enum.h"

Portion *GSM_EnumFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();

  EnumParams EP;

  EP.stopAfter = ((IntPortion *) param[1])->Value();
  
  NFSupport S(N);
  EnumModule<double> EM(N, EP, S);
  EM.Enum();

  ((IntPortion *) param[2])->Value() = EM.NumPivots();
  ((FloatPortion *) param[3])->Value() = EM.Time();

  Portion* por = new Mixed_ListPortion<double>(EM.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_EnumRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();

  EnumParams EP;

  EP.stopAfter = ((IntPortion *) param[1])->Value();
  
  NFSupport S(N);
  EnumModule<gRational> EM(N, EP, S);
  EM.Enum();

  ((IntPortion *) param[2])->Value() = EM.NumPivots();
  ((FloatPortion *) param[3])->Value() = EM.Time();

  Portion* por = new Mixed_ListPortion<gRational>(EM.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

#include "ngobit.h"

Portion *GSM_GobitNfg(Portion **param)
{
  Nfg<double> &N = *(Nfg<double> *) ((NfgPortion *) param[0])->Value();

  NFGobitParams<double> NP;

  NP.pxifile = &((OutputPortion *) param[1])->Value();
  NP.fullGraph = ((BoolPortion *) param[5])->Value();
  NP.minLam = ((FloatPortion *) param[6])->Value();
  NP.maxLam = ((FloatPortion *) param[7])->Value();
  NP.delLam = ((FloatPortion *) param[8])->Value();
  NP.powLam = ((IntPortion *) param[9])->Value();

  MixedProfile<double> *foo = (MixedProfile<double> *)
    ((MixedPortion *) param[10])->Value();
  MixedProfile<double> start(N);

  NFGobitModule<double> M(N, NP, (foo) ? *foo : start);
  M.Gobit(1);

  ((FloatPortion *) param[2])->Value() = M.Time();
  ((IntPortion *) param[3])->Value() = M.NumEvals();
  ((IntPortion *) param[4])->Value() = M.NumIters();

  Portion* por = new Mixed_ListPortion<double>(M.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

#include "grid.h"

Portion *GSM_GridSolveFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();
  
  GridParams<double> GP;
  GP.pxifile = &((OutputPortion *) param[1])->Value();
  GP.minLam = ((FloatPortion *) param[2])->Value();
  GP.maxLam = ((FloatPortion *) param[3])->Value();
  GP.delLam = ((FloatPortion *) param[4])->Value();
  GP.powLam = ((IntPortion *) param[5])->Value();
  GP.delp = ((FloatPortion *) param[6])->Value();
  GP.tol = ((FloatPortion *) param[7])->Value();

  NFSupport S(N);
  GridSolveModule<double> GM(N, GP, S);
  GM.GridSolve();

//  ((IntPortion *) param[8])->Value() = GM.NumEvals();
//  ((FloatPortion *) param[9])->Value() = GM.Time();

  gList<MixedProfile<double> > solns;

  Portion* por = new Mixed_ListPortion<double>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_GridSolveRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();
  
  GridParams<gRational> GP;
  GP.pxifile = &((OutputPortion *) param[1])->Value();
  GP.minLam = ((RationalPortion *) param[2])->Value();
  GP.maxLam = ((RationalPortion *) param[3])->Value();
  GP.delLam = ((RationalPortion *) param[4])->Value();
  GP.powLam = ((IntPortion *) param[5])->Value();
  GP.delp = ((RationalPortion *) param[6])->Value();
  GP.tol = ((RationalPortion *) param[7])->Value();

  NFSupport S(N);
  GridSolveModule<gRational> GM(N, GP, S);
  GM.GridSolve();

//  ((IntPortion *) param[8])->Value() = GM.NumEvals();
//  ((FloatPortion *) param[9])->Value() = GM.Time();

  gList<MixedProfile<gRational> > solns;

  Portion* por = new Mixed_ListPortion<gRational>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

#include "lemke.h"

Portion *GSM_LemkeNfgFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();

  LemkeParams LP;
  LP.stopAfter = ((IntPortion *) param[1])->Value();
  
  NFSupport S(N);
  LemkeModule<double> LS(N, LP, S);
  LS.Lemke();

  ((IntPortion *) param[2])->Value() = LS.NumPivots();
  ((FloatPortion *) param[3])->Value() = LS.Time();

  Portion* por = new Mixed_ListPortion<double>(LS.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_LemkeNfgRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();

  LemkeParams LP;
  LP.stopAfter = ((IntPortion *) param[1])->Value();

  NFSupport S(N);
  LemkeModule<gRational> LS(N, LP, S);
  LS.Lemke();

  ((IntPortion *) param[2])->Value() = LS.NumPivots();
  ((FloatPortion *) param[3])->Value() = LS.Time();

  Portion* por = new Mixed_ListPortion<gRational>(LS.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

#include "nliap.h"

Portion *GSM_LiapNfg(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();

  NFLiapParams<double> LP;
  LP.stopAfter = ((IntPortion *) param[3])->Value();
  LP.nTries = ((IntPortion *) param[4])->Value();
  
  MixedProfile<double> *foo = (MixedProfile<double> *) ((MixedPortion *) param[5])->Value();
  MixedProfile<double> start(N);

  NFLiapModule<double> LM(N, LP, (foo) ? *foo : start);
  LM.Liap();

  ((FloatPortion *) param[1])->Value() = LM.Time();
  ((IntPortion *) param[2])->Value() = LM.NumEvals();

  Portion* por = new Mixed_ListPortion<double>(LM.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

#include "simpdiv.h"

Portion *GSM_SimpdivFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();
  
  SimpdivParams SP;
  SP.stopAfter = ((IntPortion *) param[1])->Value();
  SP.nRestarts = ((IntPortion *) param[2])->Value();
  SP.leashLength = ((IntPortion *) param[3])->Value();

  NFSupport S(N);
  SimpdivModule<double> SM(N, SP, S);
  SM.Simpdiv();

  ((IntPortion *) param[4])->Value() = SM.NumEvals();
  ((FloatPortion *) param[5])->Value() = SM.Time();
  
  Portion* por = new Mixed_ListPortion<double>(SM.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_SimpdivRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();
  
  SimpdivParams SP;
  SP.stopAfter = ((IntPortion *) param[1])->Value();
  SP.nRestarts = ((IntPortion *) param[2])->Value();
  SP.leashLength = ((IntPortion *) param[3])->Value();

  NFSupport S(N);
  SimpdivModule<gRational> SM(N, SP, S);
  SM.Simpdiv();

  ((IntPortion *) param[4])->Value() = SM.NumEvals();
  ((FloatPortion *) param[5])->Value() = SM.Time();
  
  Portion* por = new Mixed_ListPortion<gRational>(SM.GetSolutions());
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

#include "purenash.h"

Portion *GSM_PureNashFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double>*) ((NfgPortion*) param[0])->Value();

  gList<MixedProfile<double> > solns;

  gWatch watch;

  FindPureNash(N, solns);

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  Portion* por = new Mixed_ListPortion<double>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}

Portion *GSM_PureNashRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational>*) ((NfgPortion*) param[0])->Value();

  gList<MixedProfile<gRational> > solns;

  gWatch watch;

  FindPureNash(N, solns);

  ((FloatPortion *) param[2])->Value() = watch.Elapsed();
  
  Portion* por = new Mixed_ListPortion<gRational>(solns);
  por->SetOwner( param[ 0 ]->Original() );
  por->AddDependency();
  return por;
}
 
Portion *GSM_LoadNfg(Portion **param)
{
  gString file = ((TextPortion *) param[0])->Value();

  gFileInput f(file);

  if (f.IsValid())   {
    DataType type;
    bool valid;

    NfgFileType(f, valid, type);
    if (!valid)   return new ErrorPortion("Not a valid .nfg file");
    
    switch (type)   {
      case DOUBLE:  {
	Nfg<double> *N = 0;
	ReadNfgFile((gInput &) f, N);

	if (N)
	  return new NfgValPortion(N);
	else
	  return new ErrorPortion("Not a valid .nfg file");
      }
      case RATIONAL:   {
	Nfg<gRational> *N = 0;
	ReadNfgFile((gInput &) f, N);
	
	if (N)
	  return new NfgValPortion(N);
	else
	  return new ErrorPortion("Not a valid .nfg file");
      }
      default:
	assert(0);
	return 0;
    }
  }
  else
    return new ErrorPortion("Unable to open file for reading");

}

Portion *GSM_CompressNfg(Portion **param)
{
  Nfg<double> &N = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();
  Nfg<double> *M = new Nfg<double>(N);
  return new NfgValPortion(M);
}

Portion *GSM_NewNfg(Portion **param)
{
  ListPortion *dim = ((ListPortion *) param[0]);
  gArray<int> d(dim->Length());
  
  for (int i = 1; i <= dim->Length(); i++)
    d[i] = ((IntPortion *) dim->Subscript(i))->Value();

  Nfg<double> *N = new Nfg<double>(d);

  return new NfgValPortion(N);
}

Portion *GSM_RandomNfgFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();
  
  RandomNfg(N);
  return param[0]->RefCopy();
}

Portion *GSM_RandomNfgRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational> *) ((NfgPortion *) param[0])->Value();
  
  RandomNfg(N);
  return param[0]->RefCopy();
}

Portion *GSM_RandomNfgSeedFloat(Portion **param)
{
  Nfg<double> &N = * (Nfg<double> *) ((NfgPortion *) param[0])->Value();
  int seed = ((IntPortion *) param[1])->Value();

  SetSeed(seed);
  RandomNfg(N);
  return param[0]->RefCopy();
}

Portion *GSM_RandomNfgSeedRational(Portion **param)
{
  Nfg<gRational> &N = * (Nfg<gRational> *) ((NfgPortion *) param[0])->Value();
  int seed = ((IntPortion *) param[1])->Value();

  SetSeed(seed);
  RandomNfg(N);
  return param[0]->RefCopy();
}  

Portion *GSM_NewSupport(Portion **param)
{
  BaseNfg &N = * ((NfgPortion *) param[0])->Value();
  Portion *p = new NfSupportValPortion(new NFSupport(N));

  p->SetOwner( param[ 0 ]->Original() );
  p->AddDependency();
  return p;
}

Portion *GSM_SaveNfg(Portion **param)
{
  gString file = ((TextPortion *) param[1])->Value();
  BaseNfg &N = * ((NfgPortion*) param[1])->Value();

  gFileOutput f(file);

  if (!f.IsValid())
    return new ErrorPortion("Unable to open file for output");

  N.WriteNfgFile(f);
  return new OutputRefPortion(f);
}

//---------------------------------------------------------------------


void Init_nfgfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;

  FuncObj = new FuncDescObj("LPSolve");
  FuncObj->SetFuncInfo(GSM_ConstSumFloat, 3);
  FuncObj->SetParamInfo(GSM_ConstSumFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ConstSumFloat, 1, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ConstSumFloat, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_ConstSumRational, 3);
  FuncObj->SetParamInfo(GSM_ConstSumRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ConstSumRational, 1, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_ConstSumRational, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ElimDom");
  FuncObj->SetFuncInfo(GSM_ElimDom, 3);
  FuncObj->SetParamInfo(GSM_ElimDom, 0, "support", porNF_SUPPORT);
  FuncObj->SetParamInfo(GSM_ElimDom, 1, "strong", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_ElimDom, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("EnumMixedSolve");
  FuncObj->SetFuncInfo(GSM_EnumFloat, 4);
  FuncObj->SetParamInfo(GSM_EnumFloat, 0, "nfg", 
			porNFG_FLOAT, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_EnumFloat, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumFloat, 2, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumFloat, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_EnumRational, 4);
  FuncObj->SetParamInfo(GSM_EnumRational, 0, "nfg", 
			porNFG_RATIONAL, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_EnumRational, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_EnumRational, 2, "nPivots", porINTEGER,
			new IntValPortion(0), PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_EnumRational, 3, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LqreGridSolve");
  FuncObj->SetFuncInfo(GSM_GridSolveFloat, 10);
  FuncObj->SetParamInfo(GSM_GridSolveFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
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
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
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

  FuncObj = new FuncDescObj("EnumPureSolve");
  FuncObj->SetFuncInfo(GSM_PureNashFloat, 3);
  FuncObj->SetParamInfo(GSM_PureNashFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_PureNashFloat, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_PureNashFloat, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_PureNashRational, 3);
  FuncObj->SetParamInfo(GSM_PureNashRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_PureNashRational, 1, "stopAfter", porINTEGER,
			new IntValPortion(0));
  FuncObj->SetParamInfo(GSM_PureNashRational, 2, "time", porFLOAT,
			new FloatValPortion(0.0), PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SimpDivSolve");
  FuncObj->SetFuncInfo(GSM_SimpdivFloat, 6);
  FuncObj->SetParamInfo(GSM_SimpdivFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
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
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
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

  FuncObj = new FuncDescObj("CompressNfg");
  FuncObj->SetFuncInfo(GSM_CompressNfg, 1);
  FuncObj->SetParamInfo(GSM_CompressNfg, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NewNfg");
  FuncObj->SetFuncInfo(GSM_NewNfg, 3);
  FuncObj->SetParamInfo(GSM_NewNfg, 0, "dim", porLIST | porINTEGER);
  FuncObj->SetParamInfo(GSM_NewNfg, 1, "random", porBOOL,
			new BoolValPortion(false));
  FuncObj->SetParamInfo(GSM_NewNfg, 2, "seed", porINTEGER,
			new IntValPortion(0));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RandomNfg");
  FuncObj->SetFuncInfo(GSM_RandomNfgFloat, 1);
  FuncObj->SetParamInfo(GSM_RandomNfgFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_RandomNfgRational, 1);
  FuncObj->SetParamInfo(GSM_RandomNfgRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);

  FuncObj->SetFuncInfo(GSM_RandomNfgSeedFloat, 2);
  FuncObj->SetParamInfo(GSM_RandomNfgSeedFloat, 0, "nfg", porNFG_FLOAT,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_RandomNfgSeedFloat, 1, "seed", porINTEGER);

  FuncObj->SetFuncInfo(GSM_RandomNfgSeedRational, 2);
  FuncObj->SetParamInfo(GSM_RandomNfgSeedRational, 0, "nfg", porNFG_RATIONAL,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_RandomNfgSeedRational, 1, "seed", porINTEGER);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NewSupport");
  FuncObj->SetFuncInfo(GSM_NewSupport, 1);
  FuncObj->SetParamInfo(GSM_NewSupport, 0, "nfg", porNFG,
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("AddStrategy");
  FuncObj->SetFuncInfo(GSM_AddStrategy, 2);
  FuncObj->SetParamInfo(GSM_AddStrategy, 0, "support", porNF_SUPPORT);
  FuncObj->SetParamInfo(GSM_AddStrategy, 1, "strategy", porSTRATEGY);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("RemoveStrategy");
  FuncObj->SetFuncInfo(GSM_RemoveStrategy, 2);
  FuncObj->SetParamInfo(GSM_RemoveStrategy, 0, "support", porNF_SUPPORT);
  FuncObj->SetParamInfo(GSM_RemoveStrategy, 1, "strategy", porSTRATEGY);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Strategies");
  FuncObj->SetFuncInfo(GSM_Strategies, 1);
  FuncObj->SetParamInfo(GSM_Strategies, 0, "player", porPLAYER_NFG);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("LoadNfg");
  FuncObj->SetFuncInfo(GSM_LoadNfg, 1);
  FuncObj->SetParamInfo(GSM_LoadNfg, 0, "file", porTEXT);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SaveNfg");
  FuncObj->SetFuncInfo(GSM_SaveNfg, 2);
  FuncObj->SetParamInfo(GSM_SaveNfg, 0, "nfg", porNFG, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE);
  FuncObj->SetParamInfo(GSM_SaveNfg, 1, "file", porTEXT);
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
