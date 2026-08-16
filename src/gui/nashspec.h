//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// Typed specifications for Nash equilibrium computations initiated by the GUI.
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

#ifndef GAMBIT_GUI_NASHSPEC_H
#define GAMBIT_GUI_NASHSPEC_H

#include <functional>
#include <optional>
#include <variant>

#include "core/cancel.h"
#include "games/nash.h"
#include "core/rational.h"

namespace Gambit::GUI {

enum class NashRepresentation { Strategic, Behavior };
enum class NashEquilibriumTarget { One, Some, All };

/// A profile found by an in-process computation, in whichever of the four
/// shapes (strategy/behavior, double/Rational) the method in question
/// produces.
using ComputedProfile = std::variant<MixedStrategyProfile<double>, MixedStrategyProfile<Rational>,
                                     MixedBehaviorProfile<double>, MixedBehaviorProfile<Rational>>;

/// Reports one profile found by an in-process computation.
using ProfileFoundCallback = std::function<void(const ComputedProfile &)>;

/// A runnable in-process Nash computation: given the game, a callback to
/// report each equilibrium found, and a token to cooperatively cancel it.
using SolverFunction =
    std::function<void(const Game &, const ProfileFoundCallback &, const CancelToken &)>;

//
// Each method spec below provides its own MakeSolver(), returning a
// SolverFunction that runs it in-process, or std::nullopt if its solver
// hasn't been instrumented with a CancelToken yet (the default for
// everything not yet touched). This is the only thing NashComputationSpec
// needs from whichever alternative its `method` variant holds -- adding or
// instrumenting a method never requires touching any other method's spec,
// NashComputationSpec itself, or any of its callers.
//
// The non-trivial ones (those returning more than std::nullopt) are
// implemented out of line, in nashspec.cc, which is the only place that
// needs to depend on the solvers' own headers.
//

struct EnumPureNashSpec {
  std::optional<SolverFunction> MakeSolver(NashRepresentation p_representation) const;
};

struct EnumMixedNashSpec {
  std::optional<SolverFunction> MakeSolver(NashRepresentation p_representation) const;
};

struct EnumPolyNashSpec {
  int stopAfter{0};
  double maxRegret{1.0e-4};

  std::optional<SolverFunction> MakeSolver(NashRepresentation p_representation) const;
};

struct GNMNashSpec {
  int perturbations{1};
  double lambdaEnd{-10.0};
  int steps{100};
  int localNewtonInterval{3};
  int localNewtonMaxIterations{10};

  std::optional<SolverFunction> MakeSolver(NashRepresentation) const { return std::nullopt; }
};

struct IPANashSpec {
  int perturbations{1};

  std::optional<SolverFunction> MakeSolver(NashRepresentation) const { return std::nullopt; }
};

struct LPNashSpec {
  std::optional<SolverFunction> MakeSolver(NashRepresentation p_representation) const;
};

struct LCPNashSpec {
  int stopAfter{0};
  int maxDepth{0};

  std::optional<SolverFunction> MakeSolver(NashRepresentation p_representation) const;
};

struct LiapNashSpec {
  int startingPoints{10};
  double maxRegret{1.0e-4};
  int maxIterations{1000};

  std::optional<SolverFunction> MakeSolver(NashRepresentation) const { return std::nullopt; }
};

struct LogitNashSpec {
  double maxRegret{1.0e-8};
  double omega{1.0};
  double firstStep{0.03};
  double maxAcceleration{1.1};

  std::optional<SolverFunction> MakeSolver(NashRepresentation p_representation) const;
};

struct SimpdivNashSpec {
  int startingPoints{20};
  int randomDenominator{100};
  int gridResize{2};
  int leashLength{0};
  Rational maxRegret{1, 10000000};

  std::optional<SolverFunction> MakeSolver(NashRepresentation) const;
};

using NashMethodSpec =
    std::variant<EnumPureNashSpec, EnumMixedNashSpec, EnumPolyNashSpec, GNMNashSpec, IPANashSpec,
                 LPNashSpec, LCPNashSpec, LiapNashSpec, LogitNashSpec, SimpdivNashSpec>;

struct NashComputationSpec {
  NashRepresentation representation;
  NashEquilibriumTarget target;
  NashMethodSpec method;
  bool selectedByRecommendation{false};

  /// Build a function that runs this computation in-process on a worker
  /// thread, or std::nullopt if this method/representation combination
  /// has not been instrumented with cooperative cancellation -- callers
  /// should fall back to running the external gambit-<method> process in
  /// that case. Just forwards to whichever method's own MakeSolver().
  std::optional<SolverFunction> MakeSolver() const
  {
    return std::visit([this](const auto &m) { return m.MakeSolver(representation); }, method);
  }
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_NASHSPEC_H
