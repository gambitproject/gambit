//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria by minimizing Liapunov function
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

