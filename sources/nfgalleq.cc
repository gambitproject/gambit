//
// FILE: nfgalleq.cc -- All Nash Enum module 
//
// $Id$
//

//---------------------------------------------------------------------------
//                            AllNashSolve
//---------------------------------------------------------------------------

#include "nfgalleq.h"

class AllNashSolveModule  {
private:
  const Nfg &NF;
  const NFSupport fullsupport;
  gList<const NFSupport> possiblenashsupports;
  PolEnumParams params;
  long count,nevals;
  double time;
  gList<MixedSolution> solutions;
  gList<const NFSupport> singular_supports;

public:
  AllNashSolveModule(const Nfg &, const PolEnumParams &p);

  void NashEnum(void);
  
  long NumEvals(void) const;
  double Time(void) const;
  PolEnumParams &Parameters(void);

  const gList<MixedSolution> &GetSolutions(void) const;

  const gList<const NFSupport> &GetSingularSupports(void) const;
};

//-------------------------------------------------------------------------
//                    AllNashSolveModule: Member functions
//-------------------------------------------------------------------------

AllNashSolveModule::AllNashSolveModule(const Nfg &N, const PolEnumParams &p)
  : NF(N), 
    fullsupport(N), 
    possiblenashsupports(), 
    params(p), 
    count(0), nevals(0),
    solutions(),
    singular_supports()
{ 
  possiblenashsupports += PossibleNashSubsupports(fullsupport,params.status);
}


void AllNashSolveModule::NashEnum(void)
{
  for (int i = 1; i <= possiblenashsupports.Length(); i++) {
    long newevals = 0;
    double newtime = 0.0;
    gList<MixedSolution> newsolns;
    bool is_singular = false;
    PolEnum(possiblenashsupports[i], params, newsolns, newevals, newtime,
	    is_singular);
    for (int j = 1; j <= newsolns.Length(); j++)
      if (newsolns[j].IsNash()) 
	solutions += newsolns[j];
    if (is_singular) 
      singular_supports += possiblenashsupports[i];
    nevals += newevals;
    time += newtime;
  }
}

long AllNashSolveModule::NumEvals(void) const
{
  return nevals;
}

double AllNashSolveModule::Time(void) const
{
  return time;
}

PolEnumParams &AllNashSolveModule::Parameters(void)
{
  return params;
}

const gList<MixedSolution> &AllNashSolveModule::GetSolutions(void) const
{
  return solutions;
}

const gList<const NFSupport> &
AllNashSolveModule::GetSingularSupports(void) const
{
  return singular_supports;
}

int AllNashSolve(const Nfg &N, const PolEnumParams &params,
		 gList<MixedSolution> &solutions, long &nevals, double &time,
		 gList<const NFSupport> &singular_supports)
{
  AllNashSolveModule module(N, params);
  module.NashEnum();
  nevals = module.NumEvals();
  time = module.Time();
  solutions = module.GetSolutions();
  singular_supports = module.GetSingularSupports();
  
  return 1;
}












