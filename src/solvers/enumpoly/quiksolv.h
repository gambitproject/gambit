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

/*
    The (optimistically named) class described in this file is a method
of finding the roots of a system of polynomials and inequalities, with
equal numbers of equations and unknowns, that lie inside a given
rectangle.  The general idea is to first ask whether the Taylor's
series information at the center of the rectangle precludes the
existence of roots, and if it does not, whether Newton's method leads
to a root, and if it does, whether the Taylor's series information at
the root precludes the existence of another root.  If the roots in the
rectangle are not resolved by these queries, the rectangle is
subdivided into 2^d subrectangles, and the process is repeated on
each.  This continues until it has been shown that all roots have been
found, or a predetermined search depth is reached.  The bound on depth
is necessary because the procedure will not terminate if there are
singular roots.
*/

/*
   The main constructor for this takes a gPolyList<T>.  The list must
be at least as long as the dimension Dmnsn() of the space of the
system.  The first Dmnsn() polynomials are interpreted as equations,
while remaining polynomials are interpreted as inequalities in the
sense that the polynomial is required to be nonnegative.
*/

/*
 * The original implementation of this used a custom floating-point
 * class as its parameter T, which implemented fuzzy comparisons.
 * This has since been rewritten such that it uses regular floating
 * point with explicit tolerances; this did introduce some subtle
 * bugs originally and it is possible some still remain.
 */

template <class T> class QuikSolv {
private:
  gPolyList<T> m_system;
  gPolyList<double> m_normalizedSystem;
  int NoEquations;
  int NoInequalities;
  ListOfPartialTrees<double> TreesOfPartials;
  bool HasBeenSolved;
  List<Vector<double>> Roots;
  bool isMultiaffine;
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

  // Recursive parts of recursive methods

  void FindRootsRecursion(List<Vector<double>> *, const Rectangle<double> &, const int &,
                          Array<int> &, int &iterations, int depth, const int &, int *) const;

public:
  explicit QuikSolv(const gPolyList<T> &p_system)
    : m_system(p_system), m_normalizedSystem(p_system.AmbientSpace(), p_system.NormalizedList()),
      NoEquations(std::min(m_system.Dmnsn(), m_system.Length())),
      NoInequalities(std::max(m_system.Length() - m_system.Dmnsn(), 0)),
      TreesOfPartials(m_normalizedSystem), HasBeenSolved(false),
      isMultiaffine(m_system.IsMultiaffine()), Equation_i_uses_var_j(Eq_i_Uses_j())
  {
  }
  QuikSolv(const QuikSolv<T> &) = delete;
  ~QuikSolv() = default;

  QuikSolv<T> &operator=(const QuikSolv<T> &) = delete;

  // Information
  int Dmnsn() const { return m_system.Dmnsn(); }
  const List<Vector<double>> &RootList() const { return Roots; }
  bool IsMultiaffine() const { return isMultiaffine; }

  // Refines the accuracy of roots obtained from other algorithms
  Vector<double> NewtonPolishOnce(const Vector<double> &) const;
  Vector<double> SlowNewtonPolishOnce(const Vector<double> &) const;

  // The grand calculation - returns true if successful
  bool FindCertainNumberOfRoots(const Rectangle<T> &, const int &, const int &);
};

#endif // QUIKSOLV_H
