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
  Vector<double> x = system.ComputeInitialPoint();

  const PathTracer tracer;
  double omega = 1.0;
  const double t_target = 1.0;
  const double tol = 1e-8;

  auto termination_condition = [t_target](const Vector<double> &point) {
    return point[1] >= t_target;
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
      x, omega, termination_condition,
      [&system](const Vector<double> &point) {
        std::cout << "[Path Tracer Step] t = " << point[1];
        std::cout << " | Alfas: ";
        for (size_t i = 2; i <= 5; ++i) {
          std::cout << point[i] << " ";
        }
        std::cout << "| Mu: " << point[6] << " " << point[7] << std::endl;

        std::cout << "Full point vector in probabilities: ";
        Vector<double> prob_vector = system.ExtractEquilibrium(point).GetProbVector();
        for (size_t i = 1; i <= prob_vector.size(); ++i) {
          std::cout << prob_vector[i] << " ";
        }
        std::cout << std::endl;
      },
      criterion_function);

  int polish_step = 1;
  const TracePathResult polishing_result = PolishPoint(
      [&system](const Vector<double> &point, Vector<double> &lhs) { system.GetValue(point, lhs); },
      [&system](const Vector<double> &point, Matrix<double> &jac) {
        system.GetJacobian(point, jac);
      },
      x, t_target, 1, polishing_termination_condition, 100,
      [&system, &polish_step](const Vector<double> &point) {
        const MixedStrategyProfile<double> profile = system.ExtractEquilibrium(point);
        const double regret = profile.GetMaxRegret();

        std::cout << "[Polish Step " << polish_step++ << "] ";
        std::cout << "Regret: " << regret << " | Probabilities: ";

        Vector<double> prob_vector = profile.GetProbVector();
        std::cout << std::fixed << std::setprecision(5);
        for (size_t i = 1; i <= prob_vector.size(); ++i) {
          std::cout << prob_vector[i] << " ";
        }
        std::cout << std::endl;
      });

  equilibria.push_back(system.ExtractEquilibrium(x));

  return equilibria;
}
} // namespace Gambit
