//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to algorithm to solve extensive forms using linear
// complementarity program from sequence form
//

#ifndef SEQFORM_H
#define SEQFORM_H

#include "efgalgorithm.h"
#include "numerical/lemketab.h"

template <class T> class efgLcp : public efgNashAlgorithm {
private:
  int m_stopAfter, m_maxDepth;

  int ns1,ns2,ni1,ni2;
  T maxpay,eps;
  gList<BFS<T> > List;
  gList<Infoset *> isets1, isets2;

  void FillTableau(const EFSupport &, gMatrix<T> &, const Node *, T,
		   int, int, int, int);
  int Add_BFS(const LTableau<T> &tab);
  int All_Lemke(const EFSupport &, int dup, LTableau<T> &B,
		int depth, gMatrix<T> &, gList<BehavSolution> &, gStatus &);
  
  void GetProfile(const EFSupport &, const LTableau<T> &tab, 
		  gDPVector<T> &, const gVector<T> &, 
		  const Node *n, int,int);

public:
  efgLcp(void);
  virtual ~efgLcp();
  
  int StopAfter(void) const { return m_stopAfter; }
  void SetStopAfter(int p_stopAfter) { m_stopAfter = p_stopAfter; }

  int MaxDepth(void) const { return m_maxDepth; }
  void SetMaxDepth(int p_maxDepth) { m_maxDepth = p_maxDepth; }

  gText GetAlgorithm(void) const { return "Lcp"; }
  gList<BehavSolution> Solve(const EFSupport &, gStatus &);
};

#endif    // SEQFORM_H





