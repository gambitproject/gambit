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

namespace Gambit {
std::list<MixedStrategyProfile<double>>
HPStrategySolve(const MixedStrategyProfile<double> &p_prior)
{

  std::list<MixedStrategyProfile<double>> equilibria;

  HPEquationSystem system(p_prior);
  Vector<double> x = system.ComputeInitialPoint();

  const PathTracer tracer;
  const PathTracer::TraceDirection direction = PathTracer::TraceDirection::Positive;
  const size_t tracking_index = 1; // Track the first variable (t) for orientation

  const double t_target = 1.0;
  const double tol = 1e-8;
  double last_t = 0.0;
  bool has_crossed = false;

  auto termination_condition = [t_target, &last_t, &has_crossed,
                                tol, system](const Vector<double> &point) {
    const double t = point[1];

    // Path tracer reaches tol
    if (system.ExtractEquilibrium(point).GetMaxRegret() <= tol && t >= t_target - tol) {
      return true;
    }

    if (t > t_target) {
      if (!has_crossed) {
        has_crossed = true;
      }
      else if (t > last_t + tol) { // Criterion function is not working; polish will do the job
        return true;
      }
    }
    else {
      // Criterion function might take t back to being less than t_target
      has_crossed = false;
    }

    last_t = t;
    return false;
  };

  auto criterion_function = [t_target](const Vector<double> &point,
                                       const Vector<double> &tangent) -> double {
    return point[1] - t_target;
  };

  auto polishing_termination_condition = [tol, &system](const Vector<double> &point) -> bool {
    return system.ExtractEquilibrium(point).GetMaxRegret() <= tol;
  };

  const TracePathResult result = tracer.TracePath(
      [&system](const Vector<double> &point, Vector<double> &lhs) { system.GetValue(point, lhs); },
      [&system](const Vector<double> &point, Matrix<double> &jac) {
        system.GetJacobian(point, jac);
      },
      x, direction, tracking_index, termination_condition, NullCallbackFunction,
      criterion_function);
  if (!result.status && system.ExtractEquilibrium(x).GetMaxRegret() > tol) {
    return {};
  }

  const PolishResult polishing_result = PolishPoint(
      [&system](const Vector<double> &point, Vector<double> &lhs) { system.GetValue(point, lhs); },
      [&system](const Vector<double> &point, Matrix<double> &jac) {
        system.GetJacobian(point, jac);
      },
      x, t_target, 1, polishing_termination_condition, 100, NullCallbackFunction);

  if (!polishing_result.status) {
    return {};
  }
  equilibria.push_back(system.ExtractEquilibrium(x));

  return equilibria;
}
} // namespace Gambit
