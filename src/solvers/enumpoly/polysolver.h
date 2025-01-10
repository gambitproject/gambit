//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/polysolver.h
// Interface to polynomial system solver
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

#ifndef POLYSOLVER_H
#define POLYSOLVER_H

#include "gambit.h"
#include "rectangle.h"
#include "polysystem.h"
#include "polypartial.h"

namespace Gambit {

/// @brief Find the roots of a system of polynomials and inequalities, with
///        equal numbers of equations and unknowns, that lie inside a given
///        rectangle.
///
/// The general idea is to first ask whether the Taylor's
/// series information at the center of the rectangle precludes the
/// existence of roots, and if it does not, whether Newton's method leads
/// to a root, and if it does, whether the Taylor's series information at
/// the root precludes the existence of another root.  If the roots in the
/// rectangle are not resolved by these queries, the rectangle is
/// subdivided into 2^d subrectangles, and the process is repeated on
/// each.  This continues until it has been shown that all roots have been
/// found, or a predetermined search depth is reached.  The bound on depth
/// is necessary because the procedure will not terminate if there are
/// singular roots.

/// The list of polynomials must
/// be at least as long as the dimension GetDimension() of the space of the
/// system.  The first GetDimension() polynomials are interpreted as equations,
/// while remaining polynomials are interpreted as inequalities in the
/// sense that the polynomial is required to be non-negative.
class PolynomialSystemSolver {
private:
  PolynomialSystem<double> m_system, m_normalizedSystem;
  PolynomialSystemDerivatives<double> m_derivatives;

  // Check whether roots are impossible
  bool SystemHasNoRootsIn(const Rectangle<double> &r) const;

  // Ask whether Newton's method leads to a root
  bool NewtonRootInRectangle(const Rectangle<double> &, Vector<double> &) const;

  // Ask whether we can prove that there is no root other than
  // the one produced by the last step
  double MaxDistanceFromPointToVertexAfterTransformation(const Rectangle<double> &,
                                                         const Vector<double> &,
                                                         const SquareMatrix<double> &) const;

  bool HasNoOtherRootsIn(const Rectangle<double> &, const Vector<double> &) const;

  void FindRoots(std::list<Vector<double>> &, const Rectangle<double> &, int) const;

public:
  explicit PolynomialSystemSolver(const PolynomialSystem<double> &p_system)
    : m_system(p_system), m_normalizedSystem(p_system.Normalize()),
      m_derivatives(m_normalizedSystem)
  {
  }
  PolynomialSystemSolver(const PolynomialSystemSolver &) = delete;
  ~PolynomialSystemSolver() = default;

  PolynomialSystemSolver &operator=(const PolynomialSystemSolver &) = delete;

  // Information
  int GetDimension() const { return m_system.GetDimension(); }
  int NumEquations() const { return std::min(m_system.GetDimension(), m_system.size()); }

  /// Take a single Newton step, irrespective of whether the norm of the function at the
  /// resulting point is smaller than at the original point
  Vector<double> NewtonStep(const Vector<double> &) const;
  /// Take a Newton step, sized such that the norm of the function at the resulting point
  /// is smaller than at the original point
  Vector<double> ImprovingNewtonStep(const Vector<double> &) const;

  // Find up to `max_roots` roots inside rectangle `r`
  std::list<Vector<double>> FindRoots(const Rectangle<double> &r, int max_roots);
};

} // end namespace Gambit

#endif // POLYSOLVER_H
