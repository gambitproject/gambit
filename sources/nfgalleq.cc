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
  const NFSupport supersupport;
  gList<const NFSupport> possiblenashsubsupports;
  PolEnumParams params;
  long count,nevals;
  double time;
  gList<MixedSolution> solutions;
  gList<const NFSupport> singular_supports;

public:
  AllNashSolveModule(const Nfg &, const PolEnumParams &p, gStatus &);
  AllNashSolveModule(const NFSupport &, const PolEnumParams &p, gStatus &);

  void NashEnum(gStatus &);
  
  long NumEvals(void) const;
  double Time(void) const;
  PolEnumParams &Parameters(void);

  const gList<MixedSolution> &GetSolutions(void) const;

  const gList<const NFSupport> &GetSingularSupports(void) const;
};

//-------------------------------------------------------------------------
//                    AllNashSolveModule: Member functions
//-------------------------------------------------------------------------

AllNashSolveModule::AllNashSolveModule(const Nfg &N, const PolEnumParams &p,
				       gStatus &p_status)
  : NF(N), 
    supersupport(N), 
    possiblenashsubsupports(), 
    params(p), 
    count(0), nevals(0),
    solutions(),
    singular_supports()
{ 
  possiblenashsubsupports += PossibleNashSubsupports(supersupport,p_status);
}

AllNashSolveModule::AllNashSolveModule(const NFSupport &S, 
				       const PolEnumParams &p,
				       gStatus &p_status)
  : NF(S.Game()), 
    supersupport(S), 
    possiblenashsubsupports(), 
    params(p), 
    count(0), nevals(0),
    solutions(),
    singular_supports()
{ 
  possiblenashsubsupports += 
    PossibleNashSubsupports(supersupport, p_status);
}


void AllNashSolveModule::NashEnum(gStatus &p_status)
{
  for (int i = 1; i <= possiblenashsubsupports.Length(); i++) {
    p_status.Get();
    p_status.SetProgress((double) (i-1) / (double) possiblenashsubsupports.Length());
    long newevals = 0;
    double newtime = 0.0;
    gList<MixedSolution> newsolns;
    bool is_singular = false;
    PolEnum(possiblenashsubsupports[i], params, newsolns, p_status,
	    newevals, newtime, is_singular);
    for (int j = 1; j <= newsolns.Length(); j++)
      if (newsolns[j].IsNash()) 
	solutions += newsolns[j];
    if (is_singular) 
      singular_supports += possiblenashsubsupports[i];
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

int AllNashSolve(const NFSupport &S, const PolEnumParams &params,
		 gList<MixedSolution> &solutions, gStatus &p_status,
		 long &nevals, double &time,
		 gList<const NFSupport> &singular_supports)
{
  p_status.SetProgress(0.0);
  p_status << "Step 1 of 2: Enumerating supports";
  AllNashSolveModule module(S, params, p_status);
  p_status.SetProgress(0.0);
  p_status << "Step 2 of 2: Computing equilibria";
  module.NashEnum(p_status);
  nevals = module.NumEvals();
  time = module.Time();
  solutions = module.GetSolutions();
  singular_supports = module.GetSingularSupports();
  
  return 1;
}












