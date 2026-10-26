//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/lp/lp.h
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

#ifndef GAMBIT_SOLVERS_LP_LP_H
#define GAMBIT_SOLVERS_LP_LP_H

#include <optional>

#include "solvers/nash.h"

namespace Gambit::Nash {

/// @brief The result of computing a Nash equilibrium of a strategic game via linear programming
template <class T> struct LpStrategyResult {
  std::optional<MixedStrategyProfile<T>> equilibrium;
  bool success{true};
};

template <class T>
LpStrategyResult<T>
LpStrategySolve(const Game &p_game,
                StrategyCallbackType<T> p_onEquilibrium = NullStrategyCallback<T>,
                const CancelToken &p_cancel = CancelToken());

/// @brief The result of computing a Nash equilibrium of an extensive game via linear programming
template <class T> struct LpBehaviorResult {
  std::optional<MixedBehaviorProfile<T>> equilibrium;
  bool success{true};
};

template <class T>
LpBehaviorResult<T>
LpBehaviorSolve(const Game &p_game,
                BehaviorCallbackType<T> p_onEquilibrium = NullBehaviorCallback<T>,
                const CancelToken &p_cancel = CancelToken());

} // namespace Gambit::Nash

#endif // GAMBIT_SOLVERS_LP_LP_H
