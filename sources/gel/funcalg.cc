//
// FILE: funcalg.cc -- Algorithm functions for GEL
//
// $Id$
//

#include "gmisc.h"
#include "exprtree.h"
#include "funcmisc.h"
#include "glist.h"
#include "tristate.h"

#include "efg.h"
#include "behavsol.h"
#include "nfg.h"
#include "nfplayer.h"
#include "mixedsol.h"

#include "enum.h"
#include "enumsub.h"
#include "nfgpure.h"
#include "efgpure.h"
#include "psnesub.h"
#include "grid.h"
#include "ngobit.h"
#include "egobit.h"
#include "lemke.h"
#include "seqform.h"
#include "lemkesub.h"
#include "nliap.h"
#include "eliap.h"
#include "liapsub.h"
#include "nfgcsum.h"
#include "csumsub.h"
#include "efgcsum.h"
#include "simpdiv.h"
#include "simpsub.h"




//------------
// AgentForm
//------------

DECLARE_UNARY(gelfuncAgentForm, Efg *, Nfg *)

Nfg *gelfuncAgentForm::EvalItem(Efg *E) const
{
  return (E) ? MakeAfg(*E) : 0;
}

//---------
// Behav
//---------

DECLARE_UNARY(gelfuncBehavMixed, MixedSolution *, BehavSolution *)

BehavSolution *gelfuncBehavMixed::EvalItem(MixedSolution *m) const
{
  if (!m) return 0;
  Nfg &N = m->Game();
  const Efg &E = *(const Efg *) N.AssociatedEfg();
  BehavProfile<gNumber> b = BehavProfile<gNumber>(EFSupport(E));
  MixedToBehav(N, *m, E, b);
  return new BehavSolution(b);
}


//------------------
// EnumMixedSolve
//------------------

DECLARE_UNARY(gelfuncEnumMixedSolveNfg, NFSupport *, MixedSolution *)

MixedSolution *gelfuncEnumMixedSolveNfg::EvalItem(NFSupport *S) const
{
  EnumParams EP;
  gList<MixedSolution> solutions;
  double time;
  int nPivots;
  Enum(*S, EP, solutions, nPivots, time);
  return new MixedSolution(solutions[1]);
}

DECLARE_BINARY(gelfuncEnumMixedSolveEfg, EFSupport *, gTriState *, BehavSolution *)

BehavSolution *gelfuncEnumMixedSolveEfg::EvalItem(EFSupport *S, gTriState *asNfg) const
{
  if (!S || !asNfg)  return 0;

  if (*asNfg)  {
    EnumParams EP;
    gList<BehavSolution> solutions;
    double time;
    int nPivots;
    Enum(*S, EP, solutions, nPivots, time);
    return new BehavSolution(solutions[1]);
  }
  else
    throw gelRuntimeError("EnumMixed not implemented for EFG");
}


//----------------
// EnumPureSolve
//----------------

DECLARE_UNARY(gelfuncEnumPureSolveNfg, NFSupport *, MixedSolution *)

MixedSolution *gelfuncEnumPureSolveNfg::EvalItem(NFSupport *S) const
{
  if (!S)  return 0;

  gList<MixedSolution> solutions;
  FindPureNash(S->Game(), *S, solutions);
  return new MixedSolution(solutions[1]);
}

DECLARE_BINARY(gelfuncEnumPureSolveEfg, EFSupport *, gTriState *, BehavSolution *)

BehavSolution *gelfuncEnumPureSolveEfg::EvalItem(EFSupport *S, gTriState *asNfg) const
{
  if (!S || !asNfg)  return 0;

  if (*asNfg)  {
    gList<BehavSolution> solutions;
    double time;
    EnumPureNfg(*S, solutions, time);
    return new BehavSolution(solutions[1]);
  }
  else  {
    gList<BehavSolution> solutions;
    double time;
    EnumPure(*S, solutions, time);
    return new BehavSolution(solutions[1]);
  }
}

//-----------------
// GobitGridSolve
//-----------------

DECLARE_UNARY(gelfuncGobitGridSolve, NFSupport *, MixedSolution *)

MixedSolution *gelfuncGobitGridSolve::EvalItem(NFSupport *S) const
{
  if (!S)  return 0;

  GridParams GP;
  gList<MixedSolution> solutions;
  GridSolve(*S, GP, solutions);
  return new MixedSolution(solutions[1]);
}

//-------------
// GobitSolve
//-------------

DECLARE_UNARY(gelfuncGobitSolveNfg, MixedSolution *, MixedSolution *)

MixedSolution *gelfuncGobitSolveNfg::EvalItem(MixedSolution *start) const
{
  if (!start)  return 0;

  NFGobitParams NP;
  int nEvals, nIters;
  gList<MixedSolution> solutions;
  Gobit(start->Game(), NP, *start, solutions, nEvals, nIters);
  return new MixedSolution(solutions[1]);
}

DECLARE_UNARY(gelfuncGobitSolveEfg, BehavSolution *, BehavSolution *)

BehavSolution *gelfuncGobitSolveEfg::EvalItem(BehavSolution *start) const
{
  if (!start)  return 0;

  EFGobitParams EP;
  int nEvals, nIters;
  gList<BehavSolution> solutions;
  Gobit(start->Game(), EP, *start, solutions, nEvals, nIters);
  return new BehavSolution(solutions[1]);
}

//-------------
// KGobitSolve
//-------------

DECLARE_UNARY(gelfuncKGobitSolveNfg, MixedSolution *, MixedSolution *)

MixedSolution *gelfuncKGobitSolveNfg::EvalItem(MixedSolution *start) const
{
  if (!start)  return 0;

  NFGobitParams NP;
  int nEvals, nIters;
  gList<MixedSolution> solutions;
  KGobit(start->Game(), NP, *start, solutions, nEvals, nIters);
  return new MixedSolution(solutions[1]);
}

DECLARE_UNARY(gelfuncKGobitSolveEfg, BehavSolution *, BehavSolution *)

BehavSolution *gelfuncKGobitSolveEfg::EvalItem(BehavSolution *start) const
{
  if (!start)  return 0;

  EFGobitParams EP;
  int nEvals, nIters;
  gList<BehavSolution> solutions;
  KGobit(start->Game(), EP, *start, solutions, nEvals, nIters);
  return new BehavSolution(solutions[1]);
}


//------------
// LcpSolve
//------------

DECLARE_UNARY(gelfuncLcpSolveNfg, NFSupport *, MixedSolution *)

MixedSolution *gelfuncLcpSolveNfg::EvalItem(NFSupport *S) const
{
  if (!S)  return 0;

  LemkeParams LP;
  gList<MixedSolution> solutions;
  double time;
  int nPivots;
  Lemke(*S, LP, solutions, nPivots, time);
  return new MixedSolution(solutions[1]);
}

DECLARE_BINARY(gelfuncLcpSolveEfg, EFSupport *, gTriState *, BehavSolution *)

BehavSolution *gelfuncLcpSolveEfg::EvalItem(EFSupport *S, gTriState *asNfg) const
{
  if (!S || !asNfg)  return 0;

  if (*asNfg)  {
    LemkeParams LP;
    gList<BehavSolution> solutions;
    double time;
    int nPivots;
    Lemke(*S, LP, solutions, nPivots, time);
    return new BehavSolution(solutions[1]);
  }
  else  {
    SeqFormParams SP;
    gList<BehavSolution> solutions;
    double time;
    int nPivots;
    SeqForm(*S, SP, solutions, nPivots, time);
    return new BehavSolution(solutions[1]);
  }
}

//------------
// LiapSolve
//------------

DECLARE_UNARY(gelfuncLiapSolveNfg, MixedSolution *, MixedSolution *)

MixedSolution *gelfuncLiapSolveNfg::EvalItem(MixedSolution *start) const
{
  if (!start)  return 0;

  NFLiapParams LP;
  long nEvals, nIters;
  gList<MixedSolution> solutions;
  Liap(start->Game(), LP, *start, solutions, nEvals, nIters);
  return new MixedSolution(solutions[1]);
}

DECLARE_BINARY(gelfuncLiapSolveEfg, BehavSolution *, gTriState *, BehavSolution *)

BehavSolution *gelfuncLiapSolveEfg::EvalItem(BehavSolution *start,
                                             gTriState *asNfg) const
{
  if (!start || !asNfg)  return 0;

  if (*asNfg)   {
    NFLiapParams LP;
    NFLiapBySubgame M(start->Game(), LP, *start);
    M.Solve();
    return new BehavSolution(M.GetSolutions()[1]);
  }
  else  {
    EFLiapParams LP;
    EFLiapBySubgame M(start->Game(), LP, *start);
    M.Solve();
    return new BehavSolution(M.GetSolutions()[1]);
  }
}

//-----------
// LpSolve
//-----------

DECLARE_UNARY(gelfuncLpSolveNfg, NFSupport *, MixedSolution *)

MixedSolution *gelfuncLpSolveNfg::EvalItem(NFSupport *S) const
{
  if (!S)  return 0;

  ZSumParams ZP;
  gList<MixedSolution> solutions;
  double time;
  int nPivots;
  ZSum(*S, ZP, solutions, nPivots, time);
  return new MixedSolution(solutions[1]);
}

DECLARE_BINARY(gelfuncLpSolveEfg, EFSupport *, gTriState *, BehavSolution *)

BehavSolution *gelfuncLpSolveEfg::EvalItem(EFSupport *S, gTriState *asNfg) const
{
  if (!S || !asNfg)  return 0;

  if (*asNfg)   {
    ZSumParams ZP;
    gList<BehavSolution> solutions;
    double time;
    int nPivots;
    ZSum(*S, ZP, solutions, nPivots, time);
    return new BehavSolution(solutions[1]);
  }
  else  {
    CSSeqFormParams SP;
    gList<BehavSolution> solutions;
    double time;
    int nPivots;
    CSSeqForm(*S, SP, solutions, nPivots, time);
    return new BehavSolution(solutions[1]);
  }
}

//-------
// Nfg
//-------

DECLARE_UNARY(gelfuncNfg, Efg *, Nfg *)

Nfg *gelfuncNfg::EvalItem(Efg *E) const
{
  return (E) ? MakeReducedNfg(*E, EFSupport(*E)) : 0;
}

//---------
// Payoff
//---------

DECLARE_BINARY(gelfuncPayoffBehav, BehavSolution *, EFPlayer *, gNumber *)

gNumber *gelfuncPayoffBehav::EvalItem(BehavSolution *b, EFPlayer *p) const
{
  if (!b || !p)  return 0;
  if (&b->Game() != p->Game())
    throw gelGameMismatchError("Payoff");

  return new gNumber(b->Payoff(p->GetNumber()));
}

DECLARE_BINARY(gelfuncPayoffMixed, MixedSolution *, NFPlayer *, gNumber *)

gNumber *gelfuncPayoffMixed::EvalItem(MixedSolution *m, NFPlayer *p) const
{
  if (!m || !p)  return 0;
  if (&m->Game() != &p->Game())
    throw gelGameMismatchError("Payoff");

  return new gNumber(m->Payoff(p->GetNumber()));
}

//-----------
// Simpdiv
//-----------

DECLARE_UNARY(gelfuncSimpdivSolveNfg, NFSupport *, MixedSolution *)

MixedSolution *gelfuncSimpdivSolveNfg::EvalItem(NFSupport *S) const
{
  if (!S)  return 0;

  SimpdivParams SP;
  SimpdivModule<double> M(*S, SP);
  M.Simpdiv();
  return new MixedSolution(M.GetSolutions()[1]);
}

DECLARE_BINARY(gelfuncSimpdivSolveEfg, EFSupport *, gTriState *, BehavSolution *)

BehavSolution *gelfuncSimpdivSolveEfg::EvalItem(EFSupport *S, gTriState *asNfg) const
{
  if (!S || !asNfg)  return 0;

  if (*asNfg)   {
    SimpdivParams SP;
    gList<BehavSolution> solutions;
    int nEvals, nIters;
    double time;
    Simpdiv(*S, SP, solutions, nEvals, nIters, time);
    return new BehavSolution(solutions[1]);
  }
  else
    throw gelRuntimeError("Simpdiv not implemented for EFG");
}


gelExpr *GEL_AgentForm(const gArray<gelExpr *> &params)
{
  return new gelfuncAgentForm((gelExpression<Efg *> *) params[1]);
}

gelExpr *GEL_BehavMixed(const gArray<gelExpr *> &params)
{
  return new gelfuncBehavMixed((gelExpression<MixedSolution *> *) params[1]);
}

gelExpr *GEL_EnumMixedSolveNfg(const gArray<gelExpr *> &params)
{
  return new gelfuncEnumMixedSolveNfg((gelExpression<NFSupport *> *) params[1]);
}

gelExpr *GEL_EnumMixedSolveEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncEnumMixedSolveEfg((gelExpression<EFSupport *> *) params[1],
                                      (gelExpression<gTriState *> *) params[2]);
}

gelExpr *GEL_EnumPureSolveNfg(const gArray<gelExpr *> &params)
{
  return new gelfuncEnumPureSolveNfg((gelExpression<NFSupport *> *) params[1]);
}

gelExpr *GEL_EnumPureSolveEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncEnumPureSolveEfg((gelExpression<EFSupport *> *) params[1],
                                     (gelExpression<gTriState *> *) params[2]);
}

gelExpr *GEL_GobitGridSolve(const gArray<gelExpr *> &params)
{
  return new gelfuncGobitGridSolve((gelExpression<NFSupport *> *) params[1]);
}

gelExpr *GEL_GobitSolveNfg(const gArray<gelExpr *> &params)
{
  return new gelfuncGobitSolveNfg((gelExpression<MixedSolution *> *) params[1]);
}

gelExpr *GEL_GobitSolveEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncGobitSolveEfg((gelExpression<BehavSolution *> *) params[1]);
}

gelExpr *GEL_KGobitSolveNfg(const gArray<gelExpr *> &params)
{
  return new gelfuncKGobitSolveNfg((gelExpression<MixedSolution *> *) params[1]);
}

gelExpr *GEL_KGobitSolveEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncKGobitSolveEfg((gelExpression<BehavSolution *> *) params[1]);
}

gelExpr *GEL_LcpSolveNfg(const gArray<gelExpr *> &params)
{
  return new gelfuncLcpSolveNfg((gelExpression<NFSupport *> *) params[1]);
}

gelExpr *GEL_LcpSolveEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncLcpSolveEfg((gelExpression<EFSupport *> *) params[1],
                                (gelExpression<gTriState *> *) params[2]);
}

gelExpr *GEL_LiapSolveNfg(const gArray<gelExpr *> &params)
{
  return new gelfuncLiapSolveNfg((gelExpression<MixedSolution *> *) params[1]);
}

gelExpr *GEL_LiapSolveEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncLiapSolveEfg((gelExpression<BehavSolution *> *) params[1],
                                 (gelExpression<gTriState *> *) params[2]);
}

gelExpr *GEL_LpSolveNfg(const gArray<gelExpr *> &params)
{
  return new gelfuncLpSolveNfg((gelExpression<NFSupport *> *) params[1]);
}

gelExpr *GEL_LpSolveEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncLpSolveEfg((gelExpression<EFSupport *> *) params[1],
                               (gelExpression<gTriState *> *) params[2]);
}

gelExpr *GEL_Nfg(const gArray<gelExpr *> &params)
{
  return new gelfuncNfg((gelExpression<Efg *> *) params[1]);
}

gelExpr *GEL_PayoffBehav(const gArray<gelExpr *> &params)
{
  return new gelfuncPayoffBehav((gelExpression<BehavSolution *> *) params[1],
                                (gelExpression<EFPlayer *> *) params[2]);
}

gelExpr *GEL_PayoffMixed(const gArray<gelExpr *> &params)
{
  return new gelfuncPayoffMixed((gelExpression<MixedSolution *> *) params[1],
                                (gelExpression<NFPlayer *> *) params[2]);
}

gelExpr *GEL_SimpdivSolveNfg(const gArray<gelExpr *> &params)
{
  return new gelfuncSimpdivSolveNfg((gelExpression<NFSupport *> *) params[1]);
}

gelExpr *GEL_SimpdivSolveEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncSimpdivSolveEfg((gelExpression<EFSupport *> *) params[1],
                                    (gelExpression<gTriState *> *) params[2]);
}

#include "match.h"

void gelAlgInit(gelEnvironment *env)
{
  struct  { gelAdapter *func; char *sig; }  sigarray[] = {
    { GEL_AgentForm, "AgentForm[efg->EFG] =: NFG" },
    { GEL_BehavMixed, "Behav[profile->MIXED] =: BEHAV" },
    { GEL_EnumMixedSolveNfg, "EnumMixedSolve[support->NFSUPPORT] =: MIXED" },
    { GEL_EnumMixedSolveEfg, "EnumMixedSolve[support->EFSUPPORT, {asNfg->False}] =: BEHAV" },
    { GEL_EnumPureSolveNfg, "EnumPureSolve[support->NFSUPPORT] =: MIXED" },
    { GEL_EnumPureSolveEfg, "EnumPureSolve[support->EFSUPPORT, {asNfg->False}] =: BEHAV" },
    { GEL_GobitGridSolve, "GobitGridSolve[support->NFSUPPORT] =: MIXED" },
    { GEL_GobitSolveNfg, "GobitSolve[support->NFSUPPORT] =: MIXED" },
    { GEL_GobitSolveEfg, "GobitSolve[support->EFSUPPORT] =: BEHAV" },
    { GEL_KGobitSolveNfg, "KGobitSolve[support->NFSUPPORT] =: MIXED" },
    { GEL_KGobitSolveEfg, "KGobitSolve[support->EFSUPPORT] =: BEHAV" },
    { GEL_LcpSolveNfg, "LcpSolve[support->NFSUPPORT] =: MIXED" },
    { GEL_LcpSolveEfg, "LcpSolve[support->NFSUPPORT, {asNfg->False}] =: BEHAV" },
    { GEL_LiapSolveNfg, "LiapSolve[start->MIXED] =: MIXED" },
    { GEL_LiapSolveEfg, "LiapSolve[start->BEHAV, {asNfg->False}] =: BEHAV" },
    { GEL_LpSolveNfg, "LpSolve[support->NFSUPPORT] =: MIXED" },
    { GEL_LpSolveEfg, "LpSolve[support->EFSUPPORT, {asNfg->False}] =: BEHAV" },
    { GEL_Nfg, "Nfg[efg->EFG] =: NFG" },
    { GEL_PayoffBehav, "Payoff[profile->BEHAV, player->EFPLAYER] =: NUMBER" },
    { GEL_PayoffMixed, "Payoff[profile->MIXED, player->NFPLAYER] =: NUMBER" },
    { GEL_SimpdivSolveNfg, "Simpdiv[support->NFSUPPORT] =: MIXED" },
    { GEL_SimpdivSolveEfg, "Simpdiv[support->EFSUPPORT, {asNfg->False}] =: BEHAV" },
    { 0, 0 } };

  for (int i = 0; sigarray[i].func; i++)
    env->Register(sigarray[i].func, sigarray[i].sig);
}












