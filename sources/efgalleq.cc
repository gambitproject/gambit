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
  const Efg &EF;
  const EFSupport supersupport;
  gList<const EFSupport> possiblenashsubsupports;
  EfgPolEnumParams params;
  long count,nevals;
  double time;
  gList<BehavSolution> solutions;
  gList<const EFSupport> singular_supports;

public:
  AllEFNashSolveModule(const Efg &, const EfgPolEnumParams &p);
  AllEFNashSolveModule(const EFSupport &, const EfgPolEnumParams &p);

  void NashEnum(void);
  
  long NumEvals(void) const;
  double Time(void) const;
  EfgPolEnumParams &Parameters(void);

  const gList<BehavSolution> &GetSolutions(void) const;

  const gList<const EFSupport> &GetSingularSupports(void) const;
};

//-------------------------------------------------------------------------
//                    AllEFNashSolveModule: Member functions
//-------------------------------------------------------------------------

AllEFNashSolveModule::AllEFNashSolveModule(const Efg &E, 
					   const EfgPolEnumParams &p)
  : EF(E), 
    supersupport(E), 
    possiblenashsubsupports(), 
    params(p), 
    count(0), nevals(0),
    solutions(),
    singular_supports()
{ 
  possiblenashsubsupports += 
    PossibleNashSubsupports(supersupport,params.status);
}

AllEFNashSolveModule::AllEFNashSolveModule(const EFSupport &S, 
					   const EfgPolEnumParams &p)
  : EF(S.Game()), 
    supersupport(S), 
    possiblenashsubsupports(), 
    params(p), 
    count(0), nevals(0),
    solutions(),
    singular_supports()
{ 
  possiblenashsubsupports += 
    PossibleNashSubsupports(supersupport,params.status);
}


void AllEFNashSolveModule::NashEnum(void)
{
  for (int i = 1; i <= possiblenashsubsupports.Length(); i++) {
    long newevals = 0;
    double newtime = 0.0;
    gList<BehavSolution> newsolns;
    bool is_singular = false;
    EfgPolEnum(possiblenashsubsupports[i], params, newsolns, 
	       newevals, newtime, is_singular);
    //    EfgPolEnum(possiblenashsubsupports[i], params, newsolns, newevals, 
    //	       newtime);
    for (int j = 1; j <= newsolns.Length(); j++)
      if (newsolns[j].IsNash()) 
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
		   gList<BehavSolution> &solutions, long &nevals, double &time,
		   gList<const EFSupport> &singular_supports)
{
  AllEFNashSolveModule module(S, params);
  module.NashEnum();
  nevals = module.NumEvals();
  time = module.Time();
  solutions = module.GetSolutions();
  singular_supports = module.GetSingularSupports();
  
  return 1;
}












