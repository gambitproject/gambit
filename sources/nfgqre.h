//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to computation of QRE correspondence for normal form games
//

#ifndef NFGQRE_H
#define NFGQRE_H

#include "base/base.h"
#include "corresp.h"

#include "nfg.h"
#include "mixedsol.h"

class QreNfg {
private:
  double m_maxLam, m_stepSize;
  bool m_fullGraph;

public:
  // LIFECYCLE
  QreNfg(void);

  // ACCESSING AND SETTING ALGORITHM PARAMETERS
  double GetMaxLambda(void) const { return m_maxLam; }
  void SetMaxLambda(double p_maxLam) { m_maxLam = p_maxLam; }

  double GetStepSize(void) const { return m_stepSize; }
  void SetStepSize(double p_stepSize) { m_stepSize = p_stepSize; }

  bool GetFullGraph(void) const { return m_fullGraph; }
  void SetFullGraph(bool p_fullGraph) { m_fullGraph = p_fullGraph; }

  // RUN THE ALGORITHM
  void Solve(const Nfg &p_nfg, gOutput &p_pxifile,
	     gStatus &p_status, 
	     Correspondence<double, MixedSolution> &p_solutions);
};  

#ifdef WITH_KQRE
void KQre(const Nfg &N, NFQreParams &params, gOutput &,
	    const MixedProfile<gNumber> &start,
	    gList<MixedSolution> &solutions, gStatus &,
	    long &nevals, long &nits);
#endif // WITH_KQRE

#endif  // NFGQRE_H



