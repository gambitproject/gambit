//
// FILE: polenum.cc -- Polynomial Nash Enum module
//
// $Id$
//

#include "gwatch.h"
#include "nfg.h"
#include "nfgiter.h"
#include "nfgciter.h"

#include "polenum.h"
#include "quiksolv.h"

class PolEnumModule  {
private:
  gDouble eps;
  const Nfg &NF;
  const NFSupport &support;
  PolEnumParams params;
  gSpace Space;
  term_order Lex;
  int num_vars;
  long count,nevals;
  double time;
  gList<MixedSolution> solutions;
  bool is_singular;

  bool EqZero(gDouble x) const;
  
  // p_i_j as a gPoly, with last prob in terms of previous probs
  gPoly<gDouble> Prob(int i,int j) const;

  // equation for when player i sets strat1 = strat2
  // with last probs for each player substituted out.  
  gPoly<gDouble> Equation(int i, int strat1, int strat2) const;

  int SaveSolutions(const gList<gVector<gDouble> > &list);
public:
  PolEnumModule(const NFSupport &, const PolEnumParams &p);
  
  int PolEnum(void);
  
  long NumEvals(void) const;
  double Time(void) const;
  
  PolEnumParams &Parameters(void);

  const gList<MixedSolution> &GetSolutions(void) const;

  bool IsSingular() const;
};

//-------------------------------------------------------------------------
//                    PolEnumModule: Member functions
//-------------------------------------------------------------------------

PolEnumModule::PolEnumModule(const NFSupport &S, const PolEnumParams &p)
  : NF(S.Game()), support(S), params(p), 
    Space(support.TotalNumStrats()-NF.NumPlayers()), 
    Lex(&Space, lex), num_vars(support.TotalNumStrats()-NF.NumPlayers()), 
    count(0), nevals(0), is_singular(false)
{ 
//  gEpsilon(eps,12);
}


int PolEnumModule::PolEnum(void)
{
  int i,j;

  gWatch watch;
  gPolyList<gDouble> equations(&Space,&Lex);

  // equations for equality of strat j to strat j+1
  for( i=1;i<=NF.NumPlayers();i++) 
    for(j=1;j<support.NumStrats(i);j++) 
      equations+=Equation(i,j,j+1);

  for( i=1;i<=NF.NumPlayers();i++)
    if(support.NumStrats(i)>2) 
      equations+=Prob(i,support.NumStrats(i));

  // set up the rectangle of search
  gVector<gDouble> bottoms(num_vars), tops(num_vars);
  bottoms = (gDouble)0;
  tops = (gDouble)1;
 
  gRectangle<gDouble> Cube(bottoms, tops); 

  // start QuikSolv
  gWatch timer;
  timer.Start();

  
  QuikSolv<gDouble> quickie(equations, params.status);
  params.status.SetProgress(50.0);

  if (params.trace>0) {
    (*params.tracefile) << "\nThe equilibrium equations are \n" 
      << quickie.UnderlyingEquations() ;
  }  

  try {
    if (quickie.FindCertainNumberOfRoots(Cube,2147483647,params.stopAfter)) {
      if (params.trace>0) {
	(*params.tracefile) << "\nThe system has the following roots in [0,1]^"
			    << num_vars << " :\n" << quickie.RootList();
      }
    }
    else
      if (params.trace>0) {
	(*params.tracefile) << "The system\n" << quickie.UnderlyingEquations()
			    << " could not be resolved by FindRoots.\n";
      }
  }
  catch (gSignalBreak &) { }
  catch (gSquareMatrix<gDouble>::MatrixSingular &) {
    is_singular = true;
  }
  timer.Stop();
  if(params.trace>0) {
    (*params.tracefile)  << "The QuikSolv computation of roots took " 
      << (int)timer.Elapsed() << " seconds.\n\n";
  }
  int index = SaveSolutions(quickie.RootList());
  time = watch.Elapsed();
  return index;	 

}

int PolEnumModule::SaveSolutions(const gList<gVector<gDouble> > &list)
{
  MixedProfile<double> profile(support);
  int i,j,k,kk,index=0;
  double sum;

  for(k=1;k<=list.Length();k++) {
    kk=0;
    for(i=1;i<=NF.NumPlayers();i++) {
      sum=0;
      for(j=1;j<support.NumStrats(i);j++) {
	profile(i,j) = (list[k][j+kk]).ToDouble();
	sum+=profile(i,j);
      }
      profile(i,j) = (double)1.0 - sum;
      kk+=(support.NumStrats(i)-1);
    }
    index = solutions.Append(MixedSolution(profile, algorithmNfg_POLENUM));
    solutions[index].SetEpsilon(0);
    //    solutions[index].SetIsNash(triTRUE);
    // At this point we do not know that it is Nash, since there could
    // be a beneficial deviation to a nonsupport strategy.
  }
  return index;
}

bool PolEnumModule::EqZero(gDouble x) const
{
  if(x <= eps && x >= -eps) return 1;
  return 0;
}     

long PolEnumModule::NumEvals(void) const
{
  return nevals;
}

double PolEnumModule::Time(void) const
{
  return time;
}

PolEnumParams &PolEnumModule::Parameters(void)
{
  return params;
}

const gList<MixedSolution> &PolEnumModule::GetSolutions(void) const
{
  return solutions;
}

gPoly<gDouble> PolEnumModule::Prob(int p, int strat) const
{
  gPoly<gDouble> equation(&Space,&Lex);
  gVector<int> exps(num_vars);
  int i,j,kk = 0;
  
  for(i=1;i<p;i++) 
    kk+=(support.NumStrats(i)-1);

  if(strat<support.NumStrats(p)) {
    exps=0;
    exps[strat+kk]=1;
    exp_vect const_exp(&Space,exps);
    gMono<gDouble> const_term((gDouble)1,const_exp);
    gPoly<gDouble> new_term(&Space,const_term,&Lex);
    equation+=new_term;
  }
  else {
    for(j=1;j<support.NumStrats(p);j++) {
      exps=0;
      exps[j+kk]=1;
      exp_vect exponent(&Space,exps);
      gMono<gDouble> term((gDouble)(-1),exponent);
      gPoly<gDouble> new_term(&Space,term,&Lex);
      equation+=new_term;
    }
    exps=0;
    exp_vect const_exp(&Space,exps);
    gMono<gDouble> const_term((gDouble)1,const_exp);
    gPoly<gDouble> new_term(&Space,const_term,&Lex);
    equation+=new_term;
  }
  return equation;
}

gPoly<gDouble> 
PolEnumModule::Equation(int i, int strat1, int strat2) const
{
  StrategyProfile profile(NF);

  NfgContIter A(support), B(support);
  A.Freeze(i);
  A.Set(i, strat1);
  B.Freeze(i);
  B.Set(i, strat2);
  gPoly<gDouble> equation(&Space,&Lex);
  do {
    gPoly<gDouble> term(&Space,(gDouble)1,&Lex);
    profile = A.Profile();
    int k;
    for(k=1;k<=NF.NumPlayers();k++) 
      if(i!=k) 
	term*=Prob(k,support.Find(profile[k]));
    gDouble coeff,ap,bp;
    ap = (double)NF.Payoff(A.GetOutcome(), i);
    bp = (double)NF.Payoff(B.GetOutcome(), i);
    coeff = ap - bp;
    term*=coeff;
    equation+=term;
    A.NextContingency();
  } while (B.NextContingency());
  return equation;
}

bool PolEnumModule::IsSingular() const
{
  return is_singular;
}


//---------------------------------------------------------------------------
//                        PolEnumParams: member functions
//---------------------------------------------------------------------------

PolEnumParams::PolEnumParams(gStatus &status_)
  : trace(0), stopAfter(0), tracefile(&gnull), status(status_)
{ }

int PolEnum(const NFSupport &support, const PolEnumParams &params,
	    gList<MixedSolution> &solutions, long &nevals, double &time,
	    bool &is_singular)
{
  PolEnumModule module(support, params);
  module.PolEnum();
  nevals = module.NumEvals();
  time = module.Time();
  solutions = module.GetSolutions();
  if (module.IsSingular()) 
    is_singular = true;
  else 
    is_singular = false;
  return 1;
}


