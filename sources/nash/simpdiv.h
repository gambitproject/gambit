//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria via simplicial subdivision on the normal form
//

#ifndef SIMPDIV_H
#define SIMPDIV_H

#include "nfgalgorithm.h"

template <class T> class nfgSimpdiv : public nfgNashAlgorithm {
private:
  int m_nRestarts, m_leashLength;

  int t, ibar;
  T pay,d,maxz,bestz,mingrid;

  T Simplex(MixedProfile<T> &);
  T getlabel(MixedProfile<T> &yy, gArray<int> &, gPVector<T> &);
  void update(gRectArray<int> &, gRectArray<int> &, gPVector<T> &,
	      const gPVector<int> &, int j, int i);
  void getY(MixedProfile<T> &x, gPVector<T> &, 
	    const gPVector<int> &, const gPVector<int> &, 
	    const gPVector<T> &, const gRectArray<int> &, int k);
  void getnexty(MixedProfile<T> &x, const gRectArray<int> &,
		const gPVector<int> &, int i);
  int get_c(int j, int h, int nstrats, const gPVector<int> &);
  int get_b(int j, int h, int nstrats, const gPVector<int> &);
  
public:
  nfgSimpdiv(void);
  virtual ~nfgSimpdiv();

  int NumRestarts(void) const { return m_nRestarts; }
  void SetNumRestarts(int p_nRestarts) { m_nRestarts = p_nRestarts; }

  int LeashLength(void) const { return m_leashLength; }
  void SetLeashLength(int p_leashLength) { m_leashLength = p_leashLength; }

  gText GetAlgorithm(void) const { return "Simpdiv"; }
  gList<MixedSolution> Solve(const NFSupport &, gStatus &);
};

#endif  // SIMPDIV_H



