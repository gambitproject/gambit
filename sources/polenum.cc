//
// FILE: polenum.cc -- Polynomial Nash Enum module
//
// $Id$
//

#include "base/base.h"

#include "game/nfg.h"
#include "game/nfgiter.h"
#include "game/nfgciter.h"

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
  gPoly<gDouble> IndifferenceEquation(int i, int strat1, int strat2) const;
  gPolyList<gDouble>   IndifferenceEquations()                 const;
  gPolyList<gDouble>   LastActionProbPositiveInequalities()    const;
  gPolyList<gDouble>   NashOnSupportEquationsAndInequalities() const;
  gList<gVector<gDouble> > 
               NashOnSupportSolnVectors(const gPolyList<gDouble> &equations,
					const gRectangle<gDouble> &Cube,
					gWatch &timer,
					gStatus &p_status);

  int SaveSolutions(const gList<gVector<gDouble> > &list);
public:
  PolEnumModule(const NFSupport &, const PolEnumParams &p);
  
  int PolEnum(gStatus &);
  
  long NumEvals(void) const;
  double Time(void) const;
  
  PolEnumParams &Parameters(void);

  const gList<MixedSolution> &GetSolutions(void) const;
  gVector<gDouble> SolVarsFromMixedProfile(const MixedProfile<gNumber> &) 
                                                                     const;

  const int PolishKnownRoot(gVector<gDouble> &) const;

  MixedSolution ReturnPolishedSolution(const gVector<gDouble> &) const;

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


int PolEnumModule::PolEnum(gStatus &p_status)
{
  gWatch watch;
  gPolyList<gDouble> equations = NashOnSupportEquationsAndInequalities();

  /*
  // equations for equality of strat j to strat j+1
  for( i=1;i<=NF.NumPlayers();i++) 
    for(j=1;j<support.NumStrats(i);j++) 
      equations+=IndifferenceEquation(i,j,j+1);

  for( i=1;i<=NF.NumPlayers();i++)
    if(support.NumStrats(i)>2) 
      equations+=Prob(i,support.NumStrats(i));
  */

  // set up the rectangle of search
  gVector<gDouble> bottoms(num_vars), tops(num_vars);
  bottoms = (gDouble)0;
  tops = (gDouble)1;
 
  gRectangle<gDouble> Cube(bottoms, tops); 

  // start QuikSolv
  gWatch timer;
  timer.Start();

  gList<gVector<gDouble> > solutionlist = NashOnSupportSolnVectors(equations,
								   Cube,
								   timer,
								   p_status);

  timer.Stop();
  if(params.trace>0) {
    (*params.tracefile)  << "The QuikSolv computation of roots took " 
      << (int)timer.Elapsed() << " seconds.\n\n";
  }
  int index = SaveSolutions(solutionlist);
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
    gNumber eps = (gNumber)0.0;
    gEpsilon(eps);
    solutions[index].SetEpsilon(eps);
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
PolEnumModule::IndifferenceEquation(int i, int strat1, int strat2) const
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


gPolyList<gDouble>   PolEnumModule::IndifferenceEquations()  const
{
  gPolyList<gDouble> equations(&Space,&Lex);

  for(int pl=1;pl<=NF.NumPlayers();pl++) 
    for(int j=1;j<support.NumStrats(pl);j++) 
      equations+=IndifferenceEquation(pl,j,j+1);

  return equations;
}
 
gPolyList<gDouble> PolEnumModule::LastActionProbPositiveInequalities() const
{
  gPolyList<gDouble> equations(&Space,&Lex);

  for(int pl=1;pl<=NF.NumPlayers();pl++)
    if(support.NumStrats(pl)>2) 
      equations+=Prob(pl,support.NumStrats(pl));

  return equations;
}

gPolyList<gDouble> PolEnumModule::NashOnSupportEquationsAndInequalities() const
{
  gPolyList<gDouble> equations(&Space,&Lex);
  
  equations += IndifferenceEquations();
  equations += LastActionProbPositiveInequalities();

  return equations;
}


gList<gVector<gDouble> > 
PolEnumModule::NashOnSupportSolnVectors(const gPolyList<gDouble> &equations,
					      const gRectangle<gDouble> &Cube,
					      gWatch &timer,
					      gStatus &p_status)
{  
  QuikSolv<gDouble> quickie(equations, p_status);
  //  p_status.SetProgress(0);

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
  catch (gSquareMatrix<gDouble>::MatrixSingular) {
    is_singular = true;
  }

  return quickie.RootList();
}

bool PolEnumModule::IsSingular() const
{
  return is_singular;
}


//---------------------------------------------------------------------------
//                        PolEnumParams: member functions
//---------------------------------------------------------------------------

PolEnumParams::PolEnumParams(void)
{ }

int PolEnum(const NFSupport &support, const PolEnumParams &params,
	    gList<MixedSolution> &solutions, gStatus &p_status,
	    long &nevals, double &time, bool &is_singular)
{
  PolEnumModule module(support, params);
  module.PolEnum(p_status);
  nevals = module.NumEvals();
  time = module.Time();
  solutions = module.GetSolutions();
  if (module.IsSingular()) 
    is_singular = true;
  else 
    is_singular = false;
  return 1;
}


//---------------------------------------------------------------------------
//                        Polish Equilibrum for Nfg
//---------------------------------------------------------------------------

MixedSolution PolishEquilibrium(const NFSupport &support, 
				const MixedSolution &sol, 
				bool &is_singular)
{
  PolEnumParams params;
  PolEnumModule module(support, params);
  gVector<gDouble> vec = module.SolVarsFromMixedProfile(*(sol.Profile()));

  /* //DEBUG
  gPVector<double> xx = module.SeqFormProbsFromSolVars(vec);
  MixedProfile<gNumber> newsol = module.SequenceForm().ToMixed(xx);

  gout << "sol.Profile = " << *(sol.Profile()) << "\n";
  gout << "vec  = " << vec << "\n";
  gout << "xx   = " << xx << "\n";
  gout << "newsol   = " << newsol << "\n";

    exit(0);
  if ( newsol != *(sol.Profile()) ) {
    gout << "Failure of reversibility in PolishEquilibrium.\n";
    exit(0);
  }
  */

  //DEBUG
  //  gout << "Prior to Polishing vec is " << vec << ".\n";

  module.PolishKnownRoot(vec);

  //DEBUG
  //  gout << "After Polishing vec is " << vec << ".\n";

  return module.ReturnPolishedSolution(vec);
}


gVector<gDouble> 
PolEnumModule::SolVarsFromMixedProfile(const MixedProfile<gNumber> &sol) const
{
  int numvars(0);

  for (int pl = 1; pl <= NF.NumPlayers(); pl++) 
    numvars += support.NumStrats(pl) - 1;

  gVector<gDouble> answer(numvars);
  int count(0);

  for (int pl = 1; pl <= NF.NumPlayers(); pl++) 
    for (int j = 1; j < support.NumStrats(pl); j++) {
      count ++;
      answer[count] = (gDouble)sol(pl,j);
    }

  return answer;
}

#include "gnullstatus.h"

const int PolEnumModule::PolishKnownRoot(gVector<gDouble> &point) const
{
  //DEBUG
  //  gout << "Prior to Polishing point is " << point << ".\n";

  if (point.Length() > 0) {
    
    gWatch watch;
    
    // equations for equality of strat j to strat j+1
    gPolyList<gDouble> equations(&Space,&Lex);
    equations += IndifferenceEquations();

    //DEBUG
    //    gout << "We are about to construct quickie with Dmnsn() = "
    //  << Space->Dmnsn() << " and equations = \n"
    //	 << equations << "\n";
    
    // start QuikSolv
    gNullStatus gstatus;
    QuikSolv<gDouble> quickie(equations, gstatus);
    
    //DEBUG
    //    gout << "We constructed quickie.\n";
    
    try { 
      point = quickie.NewtonPolishedRoot(point);
    }
    catch (gSignalBreak &) { }
    catch (gSquareMatrix<gDouble>::MatrixSingular &) {
      return 0;
    }

    //DEBUG
    //    gout << "After Polishing point = " << point << ".\n";

  }

  return 1;	 
}

MixedSolution 
PolEnumModule::ReturnPolishedSolution(const gVector<gDouble> &root) const
{
  MixedProfile<double> profile(support);

  int j;
  int kk=0;
  for(int pl=1;pl<=NF.NumPlayers();pl++) {
    double sum=0;
    for(j=1;j<support.NumStrats(pl);j++) {
      profile(pl,j) = (root[j+kk]).ToDouble();
      sum+=profile(pl,j);
    }
    profile(pl,j) = (double)1.0 - sum;
    kk+=(support.NumStrats(pl)-1);
  }
       
  MixedSolution sol(profile, algorithmNfg_POLISH_NASH);
  return sol;
}

