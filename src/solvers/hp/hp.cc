//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/hp/hp.cc
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

#include "gambit.h"
#include "solvers/hp/hp.h"
#include "solvers/hp/hpsystem.h"
#include "solvers/logit/path.h"

namespace Gambit::Nash {
std::list<MixedStrategyProfile<double>>
HPStrategySolve(const MixedStrategyProfile<double> &p_prior,
                StrategyCallbackType<double> p_onEquilibrium, HPEventCallbackType p_onEvent,
                const CancelToken &p_cancel)
{
  std::list<MixedStrategyProfile<double>> equilibria;

  HPEquationSystem system(p_prior);
  Vector<double> x = system.ComputeInitialPoint();

  const PathTracer tracer;
  const PathTracer::TraceDirection direction = PathTracer::TraceDirection::Positive;
  const size_t tracking_index = 1; // Track the first variable (t) for orientation
  auto termination_condition = [](const Vector<double> &point) { return point[1] >= 1.5; };
  auto criterion_function = [](const Vector<double> &point,
                               const Vector<double> &tangent) -> double { return point[1] - 1.0; };

  tracer.TracePath(
      [&system](const Vector<double> &point, Vector<double> &lhs) { system.GetValue(point, lhs); },
      [&system](const Vector<double> &point, Matrix<double> &jac) {
        system.GetJacobian(point, jac);
      },
      x, direction, tracking_index, termination_condition,
      [&system, &p_onEvent](const Vector<double> &point) {
        const MixedStrategyProfile<double> profile = system.ExtractEquilibrium(point);
        p_onEvent(HPStepEvent{.profile = profile, .t = point[1]});
      },
      criterion_function, NullCriterionBracketFunction, p_cancel);

  const MixedStrategyProfile<double> equilibrium = system.ExtractEquilibrium(x);
  p_onEquilibrium(equilibrium);
  equilibria.push_back(equilibrium);
  return equilibria;
}
} // namespace Gambit::Nash
