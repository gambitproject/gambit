//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria by minimizing Liapunov function
//

#ifndef NLIAP_H
#define NLIAP_H

#include "nfgalgorithm.h"

class nfgLiap : public nfgNashAlgorithm {
private:
  int m_stopAfter, m_numTries, m_maxits1, m_maxitsN;
  double m_tol1, m_tolN;

public:
  nfgLiap(void);
  virtual ~nfgLiap() { }

  int StopAfter(void) const { return m_stopAfter; }
  void SetStopAfter(int p_stopAfter) { m_stopAfter = p_stopAfter; }

  int NumTries(void) const { return m_numTries; }
  void SetNumTries(int p_numTries) { m_numTries = p_numTries; }

  int Maxits1(void) const { return m_maxits1; }
  void SetMaxits1(int p_maxits1) { m_maxits1 = p_maxits1; }

  double Tol1(void) const { return m_tol1; }
  void SetTol1(double p_tol1) { m_tol1 = p_tol1; }

  int MaxitsN(void) const { return m_maxitsN; }
  void SetMaxitsN(int p_maxitsN) { m_maxitsN = p_maxitsN; }

  double TolN(void) const { return m_tolN; }
  void SetTolN(double p_tolN) { m_tolN = p_tolN; }

  gText GetAlgorithm(void) const { return "Liap"; }
  gList<MixedSolution> Solve(const NFSupport &, gStatus &);
};

#endif   // NLIAP_H

