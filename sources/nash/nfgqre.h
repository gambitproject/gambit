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

#include "nfgalgorithm.h"

//
// A useful function for QRE stability analysis
//
void QreJacobian(const Nfg &p_nfg,
		 const MixedProfile<double> &p_profile,
		 const double &p_nu, gMatrix<double> &p_matrix);

class nfgQre : public nfgNashAlgorithm {
private:
  double m_maxLam, m_stepSize;
  bool m_fullGraph;

  // Execute one step of the homotopy
  void SolveStep(MixedProfile<double> &p_profile, double &p_nu,
		 double, double) const;
		 
public:
  nfgQre(void);
  virtual ~nfgQre() { }

  double GetMaxLambda(void) const { return m_maxLam; }
  void SetMaxLambda(double p_maxLam) { m_maxLam = p_maxLam; }

  double GetStepSize(void) const { return m_stepSize; }
  void SetStepSize(double p_stepSize) { m_stepSize = p_stepSize; }

  bool GetFullGraph(void) const { return m_fullGraph; }
  void SetFullGraph(bool p_fullGraph) { m_fullGraph = p_fullGraph; }

  gText GetAlgorithm(void) const { return "Qre"; }
  gList<MixedSolution> Solve(const NFSupport &, gStatus &);
};  

#endif  // NFGQRE_H



