//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Approximation of Nash equilibria via logit equilibria
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef NFGLOGIT_H
#define NFGLOGIT_H

#include "nfgalgorithm.h"

//
// A useful function for QRE stability analysis
//
void QreJacobian(const Nfg &p_nfg,
		 const MixedProfile<double> &p_profile,
		 const double &p_nu, gMatrix<double> &p_matrix);

class gbtNfgNashLogit : public gbtNfgNashAlgorithm {
private:
  double m_maxLam, m_stepSize;
  bool m_fullGraph;

public:
  gbtNfgNashLogit(void);
  virtual ~gbtNfgNashLogit() { }

  double GetMaxLambda(void) const { return m_maxLam; }
  void SetMaxLambda(double p_maxLam) { m_maxLam = p_maxLam; }

  double GetStepSize(void) const { return m_stepSize; }
  void SetStepSize(double p_stepSize) { m_stepSize = p_stepSize; }

  bool GetFullGraph(void) const { return m_fullGraph; }
  void SetFullGraph(bool p_fullGraph) { m_fullGraph = p_fullGraph; }

  gText GetAlgorithm(void) const { return "Qre"; }
  gList<MixedSolution> Solve(const gbtNfgSupport &, gStatus &);
};  

#endif  // NFGLOGIT_H



