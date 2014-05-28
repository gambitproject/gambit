//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/lcp/efglcp.h
// Compute Nash equilibria via Lemke algorithm
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

#ifndef LCP_EFGLCP_H
#define LCP_EFGLCP_H

#include "libgambit/nash.h"

using namespace Gambit;

template <class T> class LTableau;

template <class T> class NashLcpBehavSolver : public NashBehavSolver<T> {
public:
  NashLcpBehavSolver(int p_stopAfter, int p_maxDepth,
	  	     shared_ptr<StrategyProfileRenderer<T> > p_onEquilibrium = 0)
    : NashBehavSolver<T>(p_onEquilibrium),
      m_stopAfter(p_stopAfter), m_maxDepth(p_maxDepth) { }
  virtual ~NashLcpBehavSolver()  { }

  virtual List<MixedBehavProfile<T> > Solve(const BehavSupport &) const;

private:
  int m_stopAfter, m_maxDepth;

  class Solution;

  void FillTableau(const BehavSupport &, Matrix<T> &, const GameNode &, T,
		   int, int, int, int, Solution &) const;
  void AllLemke(const BehavSupport &, int dup, LTableau<T> &B,
	       int depth, Matrix<T> &, Solution &) const; 
  void GetProfile(const BehavSupport &, const LTableau<T> &tab, 
		  MixedBehavProfile<T> &, const Vector<T> &, 
		  const GameNode &n, int, int,
		  Solution &) const;
};



#endif  // LCP_EFGLCP_H
