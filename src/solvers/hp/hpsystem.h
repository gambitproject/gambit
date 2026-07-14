//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/solvers/hp/hpsystem.h
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

#ifndef HPSYSTEM_H
#define HPSYSTEM_H

#include <functional>

namespace Gambit {

class HPEquationSystem {
public:
  HPEquationSystem(const MixedStrategyProfile<double> &prior);

  // Evaluates H(t, alpha, mu) = 0
  void GetValue(const Vector<double> &point, Vector<double> &lhs) const;

  void GetJacobian(const Vector<double> &point, Matrix<double> &jac) const;

  // Computes the initial point for the homotopy path tracing (t=0)
  Vector<double> ComputeInitialPoint() const;

  // Transforms the final vector into an equilibrium mixed strategy profile
  MixedStrategyProfile<double> ExtractEquilibrium(const Vector<double> &final_point) const;

private:
  const Game m_game;
  MixedStrategyProfile<double> m_prior;
  std::vector<double> m_payoffs_against_prior;
  int m_star;
  mutable MixedStrategyProfile<double> m_current_sigma;

  // Transforms alpha to sigma and lambda
  inline double AlphaToSigma(double alpha) const { return (alpha > 0.0) ? (alpha * alpha) : 0.0; }
  inline double AlphaToLambda(double alpha) const { return (alpha < 0.0) ? (alpha * alpha) : 0.0; }

  // d(sigma)/d(alpha)
  inline double AlphaToSigmaDeriv(double alpha) const { return (alpha > 0.0) ? 2.0 * alpha : 0.0; }
  // d(lambda)/d(alpha)
  inline double AlphaToLambdaDeriv(double alpha) const
  {
    return (alpha < 0.0) ? 2.0 * alpha : 0.0;
  }

  // v^i(t, s)
  double CalculateDynamicPayoff(int action_index, const GameStrategy &strategy,
                                const MixedStrategyProfile<double> &current_sigma, double t) const;
};

} // namespace Gambit
#endif // HPSYSTEM_H
