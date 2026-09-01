//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/solvers/hp/hp.h
// Computation of a Nash equilibria using a differentiable homotopy
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

#ifndef GAMBIT_SOLVERS_HP_HP_H
#define GAMBIT_SOLVERS_HP_HP_H

#include <functional>
#include <list>
#include <variant>

#include "solvers/nash.h"

namespace Gambit::Nash {

/// @brief Reports a point traced along the HP homotopy path, at homotopy parameter \p t
struct HPStepEvent {
  const MixedStrategyProfile<double> &profile;
  double t;
};

using HPEvent = std::variant<HPStepEvent>;
using HPEventCallbackType = std::function<void(const HPEvent &)>;

inline void NullHPEventCallback(const HPEvent &) {}

/// @brief Compute a Nash equilibrium of a game using the homotopy method of
/// Herings and Peeters (2001)
std::list<MixedStrategyProfile<double>>
HPStrategySolve(const MixedStrategyProfile<double> &p_prior,
                StrategyCallbackType<double> p_onEquilibrium = NullStrategyCallback<double>,
                HPEventCallbackType p_onEvent = NullHPEventCallback,
                const CancelToken &p_cancel = CancelToken());

} // namespace Gambit::Nash

#endif // GAMBIT_SOLVERS_HP_HP_H
