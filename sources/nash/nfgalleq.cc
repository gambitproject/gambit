//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Enumerates all Nash equilibria in a normal form game, via solving
// systems of polynomial equations
//

#include "game/nfgensup.h"
#include "nfgalleq.h"
#include "polenum.h"

nfgPolEnum::nfgPolEnum(void)
  : m_stopAfter(0)
{ }

gList<MixedSolution> nfgPolEnum::Solve(const NFSupport &p_support,
				       gStatus &p_status)
{
  p_status.SetProgress(0.0);
  p_status << "Step 1 of 2: Enumerating supports";
  gList<const NFSupport> supports = PossibleNashSubsupports(p_support,
							    p_status);

  p_status.SetProgress(0.0);
  p_status << "Step 2 of 2: Computing equilibria";

  gList<const NFSupport> singularSupports;
  gList<MixedSolution> solutions;

  for (int i = 1; (i <= supports.Length() &&
		   (m_stopAfter == 0 || m_stopAfter <= solutions.Length())); 
       i++) {
    p_status.Get();
    p_status.SetProgress((double) (i-1) / (double) supports.Length());
    long newevals = 0;
    double newtime = 0.0;
    gList<MixedSolution> newsolns;
    bool is_singular = false;
    
    PolEnumParams params;
    params.stopAfter = 0;
    PolEnum(supports[i], params, newsolns, p_status,
	    newevals, newtime, is_singular);

    for (int j = 1; j <= newsolns.Length(); j++) {
      if (newsolns[j].IsNash()) {
	solutions += newsolns[j];
      }
    }

    if (is_singular) { 
      singularSupports += supports[i];
    }
  }

  return solutions;
}







