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


#include "eliap.h"

//------------
// LiapSolve
//------------

DECLARE_UNARY(gelfuncLiapSolveEfg, BehavSolution *, BehavSolution *)

BehavSolution *gelfuncLiapSolveEfg::EvalItem(BehavSolution *start) const
{
  if (!start)  return 0;

  EFLiapParams EP;
  EFLiapBySubgame M(start->Game(), EP, *start);
  M.Solve();

  return new BehavSolution(M.GetSolutions()[1]);
}


gelExpr *GEL_LiapSolveEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncLiapSolveEfg((gelExpression<BehavSolution *> *) params[1]);
}



#include "match.h"

void gelAlgInit(gelEnvironment *env)
{
  struct  { gelAdapter *func; char *sig; }  sigarray[] = {
    { GEL_LiapSolveEfg, "LiapSolve[start->BEHAV] =: BEHAV" },
    { 0, 0 } };

  for (int i = 0; sigarray[i].func; i++)
    env->Register(sigarray[i].func, sigarray[i].sig);
}












