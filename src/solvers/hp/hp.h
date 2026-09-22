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
#include <optional>
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

/// @brief Why a call to HPStrategySolve() did not return an accepted equilibrium
enum class HPTerminationReason {
  Converged,    // the accepted equilibrium satisfies maxRegret
  TraceFailed,  // the homotopy path tracer could not make progress towards t=1
  PolishFailed, // the tracer reached t=1, but polishing to reduce regret there did not converge
  RegretTargetNotReached, // polishing converged, but the resulting point still exceeds maxRegret
};

/// @brief The result of computing a Nash equilibrium via the homotopy method of
///        Herings and Peeters (2001)
struct HPStrategyResult {
  std::optional<MixedStrategyProfile<double>> equilibrium;
  bool success{false};
  HPTerminationReason reason{HPTerminationReason::TraceFailed};
};

/// @brief Compute a Nash equilibrium of a game using the homotopy method of
/// Herings and Peeters (2001)
HPStrategyResult
HPStrategySolve(const MixedStrategyProfile<double> &p_prior, double p_maxRegret = 1.0e-8,
                StrategyCallbackType<double> p_onEquilibrium = NullStrategyCallback<double>,
                HPEventCallbackType p_onEvent = NullHPEventCallback,
                const CancelToken &p_cancel = CancelToken());

} // namespace Gambit::Nash

#endif // GAMBIT_SOLVERS_HP_HP_H
