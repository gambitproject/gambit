//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to (agent) QRE computation for extensive forms

#ifndef EFGQRE_H
#define EFGQRE_H

#include "base/base.h"
#include "algutils.h"

#include "efg.h"
#include "behavsol.h"

class EFQreParams : public FuncMinParams  {
public:
  bool m_homotopy;
  int powLam;
  double minLam, maxLam, delLam, m_stepSize;
  bool fullGraph;

  EFQreParams(void);
};

void Qre(const Efg::Game &, EFQreParams &, gOutput &,
	 const BehavProfile<gNumber> &, gList<BehavSolution > &,
	 gStatus &, long &nevals, long &nits);

void KQre(const Efg::Game &E, EFQreParams &params, gOutput &,
	  const BehavProfile<gNumber> &start, gList<BehavSolution> &solutions, 
	  gStatus &, long &nevals, long &nits);

#endif   // EFGQRE_H




