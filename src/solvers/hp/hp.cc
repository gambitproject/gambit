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

#include <iostream>
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

  const PathTracer tracer;
  Vector<double> x;
  double omega = 1.0;
  bool wrong_orientation = false;

  const double t_target = 1.0;
  const double t_tol = 0.5;

  auto criterion_function = [t_target](const Vector<double> &point,
                                       const Vector<double> &tangent) -> double {
    return point[1] - t_target;
  };

  for (int attempt = 0; attempt < 2; ++attempt) {
    x = system.ComputeInitialPoint();
    wrong_orientation = false;

    auto termination_condition = [&wrong_orientation, t_target,
                                  t_tol](const Vector<double> &point) {
      const double wrong_orientation_tol = -1.0e-4;
      if (point[1] < wrong_orientation_tol) {
        wrong_orientation = true;
        return true;
      }
      return (point[1] >= t_target + t_tol);
    };

    const TracePathResult result =
        tracer.TracePath([&system](const Vector<double> &point,
                                   Vector<double> &lhs) { system.GetValue(point, lhs); },
                         [&system](const Vector<double> &point, Matrix<double> &jac) {
                           system.GetJacobian(point, jac);
                         },
                         x, omega, termination_condition,
                         [&system](const Vector<double> &point) {
                           std::cout << "[Path Tracer Step] t = " << point[1] << std::endl;
                           std::cout << "Full point vector in probabilities: ";
                           Vector<double> prob_vector =
                               system.ExtractEquilibrium(point).GetProbVector();
                           for (size_t i = 1; i <= prob_vector.size(); ++i) {
                             std::cout << prob_vector[i] << " ";
                           }
                           std::cout << std::endl;
                         },
                         criterion_function);
    if (!wrong_orientation) {
      break;
    }

    // Direction was wrong, flip the orientation and try again
    omega = -1.0;
  }
  const MixedStrategyProfile<double> eq_profile = system.ExtractEquilibrium(x);

  equilibria.push_back(eq_profile);

  // Check that the profile is an equilibrium taking into account how far we are from the target t
  // value
  const double delta = std::abs(t_target - x[1]);
  const double scale = p_prior.GetGame()->GetMaxPayoff() - p_prior.GetGame()->GetMinPayoff();
  const double max_regret = eq_profile.GetMaxRegret();
  // error tolerance <= delta * scale
  const double error_tolerance =
      (delta * scale) + 1e-6; // Add a small epsilon to account for numerical errors

  // If it is not an equilibrium, return an empty list
  if (max_regret > error_tolerance) {
    return {};
  }
  return equilibria;
}
} // namespace Gambit
