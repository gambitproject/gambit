//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to Lemke-Howson algorithm for computing equilibria in
// normal form games
//

#ifndef LEMKE_H
#define LEMKE_H

#include "lhtab.h"
#include "nfgalgorithm.h"

template <class T> class nfgLcp : public nfgNashAlgorithm  {
private:
  int m_stopAfter, m_maxDepth;

  int AddBfs(LHTableau<T> &, gList<BFS<T> > &);
  gList<MixedSolution> AddSolutions(const NFSupport &,
				    const gList<BFS<T> > &, const T &);
  void AllLemke(const NFSupport &, int, LHTableau<T> &B, gList<BFS<T> > &,
		int depth, gStatus &);
  
public:
  nfgLcp(void);
  virtual ~nfgLcp() { }
  
  int StopAfter(void) const { return m_stopAfter; }
  void SetStopAfter(int p_stopAfter) { m_stopAfter = p_stopAfter; }

  int MaxDepth(void) const { return m_maxDepth; }
  void SetMaxDepth(int p_maxDepth) { m_maxDepth = p_maxDepth; }

  gText GetAlgorithm(void) const { return "Lcp"; }
  gList<MixedSolution> Solve(const NFSupport &, gStatus &);
};


#endif    // LEMKE_H





