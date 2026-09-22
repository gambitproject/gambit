//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/liap/liap.h
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

#ifndef GAMBIT_SOLVERS_LIAP_LIAP_H
#define GAMBIT_SOLVERS_LIAP_LIAP_H

#include <optional>
#include <variant>
#include "solvers/nash.h"

namespace Gambit::Nash {

template <class Profile> struct LiapStartEvent {
  const Profile &profile;
};

template <class Profile> struct LiapEndEvent {
  const Profile &profile;
  double regret;
};

template <class Profile>
using LiapEvent = std::variant<LiapStartEvent<Profile>, LiapEndEvent<Profile>>;

template <class Profile>
using LiapEventCallbackType = std::function<void(const LiapEvent<Profile> &)>;

template <class Profile> void NullLiapEventCallback(const LiapEvent<Profile> &) {}

/// @brief Why a call to LiapStrategySolve()/LiapAgentSolve() did not return an accepted
///        equilibrium
enum class LiapTerminationReason {
  Converged,              // the accepted point satisfies maxregret
  MinimizerFailed,        // the underlying line search encountered a numerical breakdown
  RegretTargetNotReached, // the iteration limit was reached without regret falling below
                          // maxregret
};

/// @brief The result of minimizing the Lyapunov function on an extensive game
struct LiapAgentResult {
  std::optional<MixedBehaviorProfile<double>> equilibrium;
  bool success{false};
  LiapTerminationReason reason{LiapTerminationReason::RegretTargetNotReached};
};

LiapAgentResult
LiapAgentSolve(const MixedBehaviorProfile<double> &p_start, double p_maxregret, int p_maxitsN,
               BehaviorCallbackType<double> p_onEquilibrium = NullBehaviorCallback<double>,
               LiapEventCallbackType<MixedBehaviorProfile<double>> p_onEvent =
                   NullLiapEventCallback<MixedBehaviorProfile<double>>,
               const CancelToken &p_cancel = CancelToken());

/// @brief The result of minimizing the Lyapunov function on a strategic game
struct LiapStrategyResult {
  std::optional<MixedStrategyProfile<double>> equilibrium;
  bool success{false};
  LiapTerminationReason reason{LiapTerminationReason::RegretTargetNotReached};
};

LiapStrategyResult
LiapStrategySolve(const MixedStrategyProfile<double> &p_start, double p_maxregret, int p_maxitsN,
                  StrategyCallbackType<double> p_onEquilibrium = NullStrategyCallback<double>,
                  LiapEventCallbackType<MixedStrategyProfile<double>> p_onEvent =
                      NullLiapEventCallback<MixedStrategyProfile<double>>,
                  const CancelToken &p_cancel = CancelToken());

} // namespace Gambit::Nash

#endif // GAMBIT_SOLVERS_LIAP_LIAP_H
