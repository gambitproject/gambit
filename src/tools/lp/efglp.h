//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/lcp/efglp.h
// Compute Nash equilibria via linear programming
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

#ifndef LP_EFGLP_H
#define LP_EFGLP_H

#include "libgambit/nash.h"

using namespace Gambit;

template <class T> class NashLpBehavSolver : public NashBehavSolver<T> {
public:
  NashLpBehavSolver(Gambit::shared_ptr<StrategyProfileRenderer<T> > p_onEquilibrium = 0)
    : NashBehavSolver<T>(p_onEquilibrium) { }
  virtual ~NashLpBehavSolver() { }

  virtual List<MixedBehaviorProfile<T> > Solve(const BehaviorSupportProfile &) const;

private:
  class GameData;

  virtual bool SolveLP(const Matrix<T> &, const Vector<T> &, const Vector<T> &,
		       int, Array<T> &, Array<T> &) const;
};


#endif // LP_NFGLP_H
