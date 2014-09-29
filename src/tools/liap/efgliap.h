//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/liap/efgliap.h
// Compute Nash equilibria by minimizing Liapunov function on extensive game
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

#ifndef EFGLIAP_H
#define EFGLIAP_H

#include "libgambit/nash.h"

using namespace Gambit;

class NashLiapBehavSolver : public NashBehavSolver<double> {
public:
  NashLiapBehavSolver(int p_maxitsN, bool p_verbose = false,
		      shared_ptr<StrategyProfileRenderer<double> > p_onEquilibrium = 0)
    : NashBehavSolver<double>(p_onEquilibrium),
      m_maxitsN(p_maxitsN), m_verbose(p_verbose)
  { }
  virtual ~NashLiapBehavSolver() { }

  List<MixedBehaviorProfile<double> > Solve(const MixedBehaviorProfile<double> &p_start) const;
  List<MixedBehaviorProfile<double> > Solve(const BehaviorSupportProfile &p_support) const
    { return Solve(MixedBehaviorProfile<double>(p_support)); }

private:
  int m_maxitsN;
  bool m_verbose;
};

#endif  // EFGLIAP_H
