//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/polyfeasible.h
// Check feasibility of a system of polynomial inequalities
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

#ifndef POLYFEASIBLE_H
#define POLYFEASIBLE_H

#include "gambit.h"

#include "rectangle.h"
#include "polysystem.h"
#include "polypartial.h"

namespace Gambit {

/// Determine whether a
/// system of weak inequalities has a solution (a point where all are satisfied)
/// in a given rectangle.  Ir is modeled on PolynomialSystemSolver, but simpler.  There is
/// no Newton search, only repeated subdivision, queries at the center, and
/// tests against whether one of the inequalities is provably everywhere
/// negative in the rectangle.
/// The constructor for this takes a list of polynomials, interpreted as
/// inequalities in the sense that, at a solution, all the polynomials
/// are required to be non-negative.
class PolynomialFeasibilitySolver {
private:
  PolynomialSystem<double> m_system;
  PolynomialSystemDerivatives<double> m_systemDerivs;
  double m_epsilon;

  bool IsASolution(const Vector<double> &v) const
  {
    return std::all_of(m_system.begin(), m_system.end(),
                       [&](const Polynomial<double> &p) { return p.Evaluate(v) > -m_epsilon; });
  }
  bool SystemHasNoSolutionIn(const Rectangle<double> &r, Array<int> &precedence) const
  {
    for (int i = 1; i <= m_system.size(); i++) {
      if (m_systemDerivs[precedence[i]].PolyEverywhereNegativeIn(r)) {
        if (i != 1) { // We have found a new "most likely to never be positive"
          int tmp = precedence[i];
          for (int j = 1; j <= i - 1; j++) {
            precedence[i - j + 1] = precedence[i - j];
          }
          precedence[1] = tmp;
        }
        return true;
      }
    }
    return false;
  }

  bool SolutionExists(const Rectangle<double> &r, Array<int> &precedence) const
  {
    if (IsASolution(r.Center())) {
      return true;
    }
    if (SystemHasNoSolutionIn(r, precedence)) {
      return false;
    }
    for (const auto &cell : r.Orthants()) {
      if (SolutionExists(cell, precedence)) {
        return true;
      }
    }
    return false;
  }

public:
  explicit PolynomialFeasibilitySolver(const PolynomialSystem<double> &given)
    : m_system(given), m_systemDerivs(given), m_epsilon(1.0e-6)
  {
  }
  PolynomialFeasibilitySolver(const PolynomialFeasibilitySolver &) = delete;
  ~PolynomialFeasibilitySolver() = default;
  PolynomialFeasibilitySolver &operator=(const PolynomialFeasibilitySolver &) = delete;

  /// Does a solution exist in the specified rectangle?
  bool HasSolution(const Rectangle<double> &r)
  {
    Array<int> precedence(m_system.size());
    std::iota(precedence.begin(), precedence.end(), 1);
    return SolutionExists(r, precedence);
  }
};

} // namespace Gambit

#endif // POLYFEASIBLE_H
