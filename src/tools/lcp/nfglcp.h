//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/lcp/nfglcp.h
// Compute Nash equilibria via Lemke-Howson algorithm
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

#ifndef LCP_NFGLCP_H
#define LCP_NFGLCP_H

#include "libgambit/nash.h"

using namespace Gambit;

template <class T> class LHTableau;

template <class T> class NashLcpStrategySolver : public NashStrategySolver<T> {
public:
  NashLcpStrategySolver(int p_stopAfter, int p_maxDepth,
			Gambit::shared_ptr<StrategyProfileRenderer<T> > p_onEquilibrium = 0)
    : NashStrategySolver<T>(p_onEquilibrium),
      m_stopAfter(p_stopAfter), m_maxDepth(p_maxDepth) { }
  virtual ~NashLcpStrategySolver()  { }

  virtual List<MixedStrategyProfile<T> > Solve(const Game &) const;

private:
  int m_stopAfter, m_maxDepth;

  class Solution;

  bool OnBFS(const Game &, LHTableau<T> &, Solution &) const;
  void AllLemke(const Game &, int j, LHTableau<T> &, Solution &, int) const;
};



#endif  // LCP_NFGLCP_H
