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

  auto criterion_function = [](const Vector<double> &point,
                               const Vector<double> &tangent) -> double { return point[1] - 1.0; };

  for (int attemp = 0; attemp < 2; ++attemp) {
    x = system.ComputeInitialPoint();
    wrong_orientation = false;

    auto termination_condition = [&wrong_orientation](const Vector<double> &point) {
      const double wrong_orientation_tol = -1.0e-4;
      if (point[1] < wrong_orientation_tol) {
        wrong_orientation = true;
        return true;
      }
      return point[1] >= 1.5;
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

  equilibria.push_back(system.ExtractEquilibrium(x));
  return equilibria;
}
} // namespace Gambit
