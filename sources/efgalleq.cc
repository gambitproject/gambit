//
// FILE: efgalleq.cc -- All Nash Enum module for Efg's
//
// $Id$
//

//---------------------------------------------------------------------------
//                            AllEFNashSolve
//---------------------------------------------------------------------------

#include "efgalleq.h"

class AllEFNashSolveModule  {
private:
  const Efg::Game &EF;
  const EFSupport supersupport;
  gList<const EFSupport> possiblenashsubsupports;
  EfgPolEnumParams params;
  long count,nevals;
  double time;
  gList<BehavSolution> solutions;
  gList<const EFSupport> singular_supports;

public:
  AllEFNashSolveModule(const Efg::Game &, const EfgPolEnumParams &p,
		       gStatus &);
  AllEFNashSolveModule(const EFSupport &, const EfgPolEnumParams &p,
		       gStatus &);

  void NashEnum(gStatus &);
  
  long NumEvals(void) const;
  double Time(void) const;
  EfgPolEnumParams &Parameters(void);

  const gList<BehavSolution> &GetSolutions(void) const;

  const gList<const EFSupport> &GetSingularSupports(void) const;
};

//-------------------------------------------------------------------------
//                    AllEFNashSolveModule: Member functions
//-------------------------------------------------------------------------

AllEFNashSolveModule::AllEFNashSolveModule(const Efg::Game &E, 
					   const EfgPolEnumParams &p,
					   gStatus &p_status)
  : EF(E), 
    supersupport(E), 
    possiblenashsubsupports(), 
    params(p), 
    count(0), nevals(0),
    solutions(),
    singular_supports()
{ 
  possiblenashsubsupports += 
    PossibleNashSubsupports(supersupport,p_status);
}

AllEFNashSolveModule::AllEFNashSolveModule(const EFSupport &S, 
					   const EfgPolEnumParams &p,
					   gStatus &p_status)
  : EF(S.GetGame()), 
    supersupport(S), 
    possiblenashsubsupports(), 
    params(p), 
    count(0), nevals(0),
    solutions(),
    singular_supports()
{ 
  possiblenashsubsupports += 
    PossibleNashSubsupports(supersupport,p_status);
}


void AllEFNashSolveModule::NashEnum(gStatus &p_status)
{
  for (int i = 1; i <= possiblenashsubsupports.Length(); i++) {
    p_status.Get();
    p_status.SetProgress((double) (i-1) / (double) possiblenashsubsupports.Length());
    long newevals = 0;
    double newtime = 0.0;
    gList<BehavSolution> newsolns;
    bool is_singular = false;
    EfgPolEnum(possiblenashsubsupports[i], params, 
	       newsolns, p_status, 
	       newevals, newtime, is_singular);
    for (int j = 1; j <= newsolns.Length(); j++)
      if (newsolns[j].IsANFNash()) 
	solutions += newsolns[j];
    if (is_singular) 
      singular_supports += possiblenashsubsupports[i];
    nevals += newevals;
    time += newtime;
  }
}

long AllEFNashSolveModule::NumEvals(void) const
{
  return nevals;
}

double AllEFNashSolveModule::Time(void) const
{
  return time;
}

EfgPolEnumParams &AllEFNashSolveModule::Parameters(void)
{
  return params;
}

const gList<BehavSolution> &AllEFNashSolveModule::GetSolutions(void) const
{
  return solutions;
}

const gList<const EFSupport> &
AllEFNashSolveModule::GetSingularSupports(void) const
{
  return singular_supports;
}

int AllEFNashSolve(const EFSupport &S, const EfgPolEnumParams &params,
		   gList<BehavSolution> &solutions, gStatus &p_status,
		   long &nevals, double &time,
		   gList<const EFSupport> &singular_supports)
{
  p_status.SetProgress(0.0);
  p_status << "Step 1 of 2: Enumerating supports";
  AllEFNashSolveModule module(S, params, p_status);
  p_status.SetProgress(0.0); 
  p_status << "Step 2 of 2: Computing equilibria";
  module.NashEnum(p_status);
  nevals = module.NumEvals();
  time = module.Time();
  solutions = module.GetSolutions();
  singular_supports = module.GetSingularSupports();
  
  return 1;
}


void efgPolEnumSolve::SolveSubgame(const FullEfg &, const EFSupport &p_support,
				   gList<BehavSolution> &p_solutions,
				   gStatus &p_status)
{
  long nevals;
  double time;
  gList<const EFSupport> singularSupports;

  AllEFNashSolve(p_support, params, p_solutions, p_status,
		 nevals, time, singularSupports);
}

efgPolEnumSolve::efgPolEnumSolve(const EFSupport &,
				 const EfgPolEnumParams &p_params, int p_max)
  : SubgameSolver(p_max), params(p_params)
{ }

efgPolEnumSolve::~efgPolEnumSolve() { }










