//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/quiksolv.h
// Interface to quick-solver classes
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

#ifndef QUIKSOLV_H
#define QUIKSOLV_H

#include "gambit.h"
#include "odometer.h"
#include "rectangle.h"
#include "gpoly.h"
#include "gpolylst.h"
#include "gpartltr.h"

using namespace Gambit;

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
/// be at least as long as the dimension Dmnsn() of the space of the
/// system.  The first Dmnsn() polynomials are interpreted as equations,
/// while remaining polynomials are interpreted as inequalities in the
/// sense that the polynomial is required to be non-negative.
class QuickSolver {
private:
  gPolyList<double> m_system, m_normalizedSystem;
  ListOfPartialTrees<double> TreesOfPartials;
  List<Vector<double>> Roots;
  RectArray<bool> Equation_i_uses_var_j;

  // Supporting routines for the constructors

  RectArray<bool> Eq_i_Uses_j() const;

  // Check whether roots are impossible

  bool SystemHasNoRootsIn(const Rectangle<double> &r, Array<int> &) const;

  // Ask whether Newton's method leads to a root

  bool NewtonRootInRectangle(const Rectangle<double> &, Vector<double> &) const;

  // Ask whether we can prove that there is no root other than
  // the one produced by the last step

  double MaxDistanceFromPointToVertexAfterTransformation(const Rectangle<double> &,
                                                         const Vector<double> &,
                                                         const SquareMatrix<double> &) const;

  bool HasNoOtherRootsIn(const Rectangle<double> &, const Vector<double> &,
                         const SquareMatrix<double> &) const;

  // Combine the last two steps into a single query
  bool NewtonRootIsOnlyInRct(const Rectangle<double> &, Vector<double> &) const;

  void FindRoots(List<Vector<double>> &, const Rectangle<double> &, int, Array<int> &,
                 int &iterations, int depth, int) const;

public:
  explicit QuickSolver(const gPolyList<double> &p_system)
    : m_system(p_system), m_normalizedSystem(p_system.Normalize()),
      TreesOfPartials(m_normalizedSystem), Equation_i_uses_var_j(Eq_i_Uses_j())
  {
  }
  QuickSolver(const QuickSolver &) = delete;
  ~QuickSolver() = default;

  QuickSolver &operator=(const QuickSolver &) = delete;

  // Information
  int Dmnsn() const { return m_system.Dmnsn(); }
  int NumEquations() const { return std::min(m_system.Dmnsn(), m_system.Length()); }
  const List<Vector<double>> &RootList() const { return Roots; }
  bool IsMultiaffine() const { return m_system.IsMultiaffine(); }

  // Refines the accuracy of roots obtained from other algorithms
  Vector<double> NewtonPolishOnce(const Vector<double> &) const;
  Vector<double> SlowNewtonPolishOnce(const Vector<double> &) const;

  // Find up to `max_roots` roots inside rectangle `r`
  bool FindRoots(const Rectangle<double> &r, int max_roots);
};

#endif // QUIKSOLV_H
