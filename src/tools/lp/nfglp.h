//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/lcp/nfglp.h
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

#ifndef LP_NFGLP_H
#define LP_NFGLP_H

#include "games/nash.h"

using namespace Gambit;
using namespace Gambit::Nash;

template <class T> class NashLpStrategySolver : public StrategySolver<T> {
public:
  NashLpStrategySolver(std::shared_ptr<StrategyProfileRenderer<T> > p_onEquilibrium = nullptr)
    : StrategySolver<T>(p_onEquilibrium) { }
  ~NashLpStrategySolver() override = default;

  List<MixedStrategyProfile<T> > Solve(const Game &) const override;

private:
  virtual bool SolveLP(const Matrix<T> &, const Vector<T> &, const Vector<T> &,
		       int, Array<T> &, Array<T> &) const;
};

inline List<MixedStrategyProfile<double> > LpStrategySolveDouble(const Game &p_game)
{
  return NashLpStrategySolver<double>().Solve(p_game);
}

inline List<MixedStrategyProfile<Rational> > LpStrategySolveRational(const Game &p_game)
{
  return NashLpStrategySolver<Rational>().Solve(p_game);
}

#endif // LP_NFGLP_H
