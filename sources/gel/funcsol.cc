//
// FILE: funcsol.cc -- Efg functions for GEL
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


//---------
// Behav
//---------

DECLARE_UNARY(gelfuncBehav, EFSupport *, BehavSolution *)

BehavSolution *gelfuncBehav::EvalItem(EFSupport *S) const
{
  return (S) ? new BehavSolution(BehavProfile<gNumber>(*S)) : 0;
}


gelExpr *GEL_Behav(const gArray<gelExpr *> &params)
{
  return new gelfuncBehav((gelExpression<EFSupport *> *) params[1]);
}



#include "match.h"

void gelSolInit(gelEnvironment *env)
{
  struct  { gelAdapter *func; char *sig; }  sigarray[] = {
    { GEL_Behav, "Behav[support->EFSUPPORT] =: BEHAV" },
    { 0, 0 } };

  for (int i = 0; sigarray[i].func; i++)
    env->Register(sigarray[i].func, sigarray[i].sig);
}












