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

class gelfuncLiapSolveEfg : public gelExpression<BehavSolution *>
{                                                     
private:                                              
  gelExpression<BehavSolution *> *op1;
  gelExpression<gTriState *> *op2;
                                                      
  BehavSolution *EvalItem(BehavSolution *item1, gTriState *item2) const;
                                                      
public:                                               
  gelfuncLiapSolveEfg(gelExpression<BehavSolution *> *x1,
                   gelExpression<gTriState *> *x2)
    : op1(x1), op2(x2) {}                             
  virtual ~gelfuncLiapSolveEfg() { delete op1; delete op2; }
  gNestedList<BehavSolution *> Evaluate(gelVariableTable *vt) const;
};                                                                 
                                                                   
gNestedList<BehavSolution *> gelfuncLiapSolveEfg::Evaluate(gelVariableTable *vt) const
{                                                                  
  gNestedList<BehavSolution *> arg1 = op1->Evaluate(vt);
  gNestedList<gTriState *> arg2 = op2->Evaluate(vt);
								       
  if ((arg1.Dim()[1] == 0) != (arg2.Dim()[1] == 0))   {
    /* only one of the arguments is a scalar */
                                                                       
    if( arg1.Dim()[1] == 0 )  {  /* the first argument is a scalar */
      gNestedList<BehavSolution *> ret(arg2.Dim());
      for (int i = 1; i <= arg2.Data().Length(); i++)                  
	ret[i] = EvalItem(arg1.Data()[1], arg2.Data()[i]);   
      return ret;                                                      
    }                                                                  
    else   { /* the second argument is a scalar */
      gNestedList<BehavSolution *> ret(arg1.Dim());
      for (int i = 1; i <= arg1.Data().Length(); i++)                  
	ret[i] = EvalItem(arg1.Data()[i], arg2.Data()[1]);   
      return ret;                                                      
    }                                                                  
                                                                       
  }                                                                    
  else  {
    gNestedList<BehavSolution *> ret( arg1.Dim() );
    if( arg1.Data().Length() != arg2.Data().Length() )                 
      throw gDimMismatchException();                                   
    if( arg1.Dim() != arg2.Dim() )                                     
      throw gDimMismatchException();                                   
    for (int i = 1; i <= arg1.Data().Length(); i++)                    
      ret[i] = EvalItem(arg1.Data()[i], arg2.Data()[i]);     
    return ret;                                                        
  }                                                                    
}

BehavSolution *gelfuncLiapSolveEfg::EvalItem(BehavSolution *start,
                                             gTriState *) const
{
  if (!start)  return 0;

  EFLiapParams EP;
  EFLiapBySubgame M(start->Game(), EP, *start);
  M.Solve();

  return new BehavSolution(M.GetSolutions()[1]);
}


gelExpr *GEL_LiapSolveEfg(const gArray<gelExpr *> &params)
{
  return new gelfuncLiapSolveEfg((gelExpression<BehavSolution *> *) params[1],
                                 (gelExpression<gTriState *> *) params[2]);
}



#include "match.h"

void gelAlgInit(gelEnvironment *env)
{
  struct  { gelAdapter *func; char *sig; }  sigarray[] = {
    { GEL_LiapSolveEfg, "LiapSolve[start->BEHAV, {asNfg->False}] =: BEHAV" },
    { 0, 0 } };

  for (int i = 0; sigarray[i].func; i++)
    env->Register(sigarray[i].func, sigarray[i].sig);
}












