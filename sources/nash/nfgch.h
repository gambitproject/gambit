//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Cognitive Hierarchy behavioral solution concept
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

#ifndef NFGCH_H
#define NFGCH_H

#include "base/base.h"
#include "base/gstatus.h"
#include "nfgalgorithm.h"

class gbtNfgBehavCH : public gbtNfgNashAlgorithm {
private:
  double m_minTau, m_maxTau, m_stepTau;

public:
  gbtNfgBehavCH(void) : m_minTau(0.0), m_maxTau(10.0), m_stepTau(0.1) { }
  virtual ~gbtNfgBehavCH() { } 

  double MinTau(void) const { return m_minTau; }
  void SetMinTau(double p_minTau) { m_minTau = p_minTau; }

  double MaxTau(void) const { return m_maxTau; }
  void SetMaxTau(double p_maxTau) { m_maxTau = p_maxTau; }

  double StepTau(void) const { return m_stepTau; }
  void SetStepTau(double p_stepTau) { m_stepTau = p_stepTau; }

  gText GetAlgorithm(void) const { return "CH"; }
  gList<MixedSolution> Solve(const gbtNfgSupport &, gStatus &);
};

#endif // NFGCH_H


