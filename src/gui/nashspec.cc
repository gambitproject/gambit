//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// Builds in-process, cancellable solver functions for Nash equilibrium
// computations initiated by the GUI.
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

#include "nashspec.h"

#include "solvers/enummixed/enummixed.h"
#include "solvers/enumpure/enumpure.h"
#include "solvers/lcp/lcp.h"
#include "solvers/logit/logit.h"
#include "solvers/lp/lp.h"

namespace Gambit::GUI {

std::optional<SolverFunction> EnumPureNashSpec::MakeSolver(NashRepresentation) const
{
  return [](const Game &p_game, const ProfileFoundCallback &p_callback,
            const CancelToken &p_cancel) {
    Nash::EnumPureStrategySolve(
        p_game,
        [&p_callback](const MixedStrategyProfile<Rational> &p) { p_callback(ComputedProfile(p)); },
        p_cancel);
  };
}

std::optional<SolverFunction> EnumMixedNashSpec::MakeSolver(NashRepresentation) const
{
  return [](const Game &p_game, const ProfileFoundCallback &p_callback,
            const CancelToken &p_cancel) {
    Nash::EnumMixedStrategySolve<Rational>(
        p_game,
        [&p_callback](const MixedStrategyProfile<Rational> &p) { p_callback(ComputedProfile(p)); },
        p_cancel);
  };
}

std::optional<SolverFunction> LPNashSpec::MakeSolver(NashRepresentation p_representation) const
{
  if (p_representation == NashRepresentation::Behavior) {
    return [](const Game &p_game, const ProfileFoundCallback &p_callback,
              const CancelToken &p_cancel) {
      Nash::LpBehaviorSolve<Rational>(
          p_game,
          [&p_callback](const MixedBehaviorProfile<Rational> &p) {
            p_callback(ComputedProfile(p));
          },
          p_cancel);
    };
  }
  return [](const Game &p_game, const ProfileFoundCallback &p_callback,
            const CancelToken &p_cancel) {
    Nash::LpStrategySolve<Rational>(
        p_game,
        [&p_callback](const MixedStrategyProfile<Rational> &p) { p_callback(ComputedProfile(p)); },
        p_cancel);
  };
}

std::optional<SolverFunction> LCPNashSpec::MakeSolver(NashRepresentation p_representation) const
{
  if (p_representation == NashRepresentation::Behavior) {
    return [](const Game &p_game, const ProfileFoundCallback &p_callback,
              const CancelToken &p_cancel) {
      Nash::LcpBehaviorSolve<Rational>(
          p_game,
          [&p_callback](const MixedBehaviorProfile<Rational> &p) {
            p_callback(ComputedProfile(p));
          },
          p_cancel);
    };
  }
  const LCPNashSpec spec = *this;
  return [spec](const Game &p_game, const ProfileFoundCallback &p_callback,
                const CancelToken &p_cancel) {
    Nash::LcpStrategySolve<Rational>(
        p_game, spec.stopAfter, spec.maxDepth,
        [&p_callback](const MixedStrategyProfile<Rational> &p) { p_callback(ComputedProfile(p)); },
        p_cancel);
  };
}

std::optional<SolverFunction> LogitNashSpec::MakeSolver(NashRepresentation p_representation) const
{
  const LogitNashSpec spec = *this;
  if (p_representation == NashRepresentation::Behavior) {
    return [spec](const Game &p_game, const ProfileFoundCallback &p_callback,
                  const CancelToken &p_cancel) {
      const LogitQREMixedBehaviorProfile start(p_game);
      LogitBehaviorSolve(
          start, spec.maxRegret, spec.omega, spec.firstStep, spec.maxAcceleration,
          [&p_callback](const MixedBehaviorProfile<double> &p) { p_callback(ComputedProfile(p)); },
          NullLogitEventCallback<LogitQREMixedBehaviorProfile>, p_cancel);
    };
  }
  return [spec](const Game &p_game, const ProfileFoundCallback &p_callback,
                const CancelToken &p_cancel) {
    const LogitQREMixedStrategyProfile start(p_game);
    LogitStrategySolve(
        start, spec.maxRegret, spec.omega, spec.firstStep, spec.maxAcceleration,
        [&p_callback](const MixedStrategyProfile<double> &p) { p_callback(ComputedProfile(p)); },
        NullLogitEventCallback<LogitQREMixedStrategyProfile>, p_cancel);
  };
}

} // namespace Gambit::GUI
