//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Algorithm to enumerate mixed strategy equilibria on normal forms
//

//
// Enum implements the procedure described in 
// Mangasarian, O. L., "Equilibrium points of bimatrix games", 
// SIAM 12 (1964): 778-780 for finding all extreme points of 
// the Nash equilibrium components of a two person game. 
// 

#ifndef ENUM_H
#define ENUM_H

#include "nfgalgorithm.h"

template <class T> class nfgEnumMixed : public nfgNashAlgorithm {
private:
  int m_stopAfter;
  bool m_cliques;

  // Private auxiliary functions
  bool EqZero(const T &) const;

public:
  nfgEnumMixed(void);
  virtual ~nfgEnumMixed() { }
  
  int StopAfter(void) const { return m_stopAfter; }
  void SetStopAfter(int p_stopAfter) { m_stopAfter = p_stopAfter; }

  bool Cliques(void) const { return m_cliques; }
  void SetCliques(bool p_cliques) { m_cliques = p_cliques; }

  gText GetAlgorithm(void) const { return "EnumMixed"; }
  gList<MixedSolution> Solve(const NFSupport &, gStatus &);
};

#endif  // ENUM_H




