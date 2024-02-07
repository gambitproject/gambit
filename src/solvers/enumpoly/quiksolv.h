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
#include "gsolver.h"
#include "rectangl.h"
#include "gpoly.h"
#include "gpolylst.h"
#include "gpartltr.h"

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

// ***********************
//      class QuikSolv
// ***********************

template <class T> class QuikSolv {
private:
  const gPolyList<T> System;
  const gPolyList<double> gDoubleSystem;
  const int NoEquations;
  const int NoInequalities;
  const ListOfPartialTrees<double> TreesOfPartials;
  bool HasBeenSolved;
  Gambit::List<Gambit::Vector<double>> Roots;
  const bool isMultiaffine;
  const Gambit::RectArray<bool> Equation_i_uses_var_j;

  // Supporting routines for the constructors

  Gambit::RectArray<bool> Eq_i_Uses_j() const;

  // Check whether roots are impossible

  bool SystemHasNoRootsIn(const gRectangle<double> &r, Gambit::Array<int> &) const;

  // Ask whether Newton's method leads to a root

  bool NewtonRootInRectangle(const gRectangle<double> &, Gambit::Vector<double> &) const;
  bool NewtonRootNearRectangle(const gRectangle<double> &, Gambit::Vector<double> &) const;

  // Ask whether we can prove that there is no root other than
  // the one produced by the last step

  double
  MaxDistanceFromPointToVertexAfterTransformation(const gRectangle<double> &,
                                                  const Gambit::Vector<double> &,
                                                  const Gambit::SquareMatrix<double> &) const;

  bool HasNoOtherRootsIn(const gRectangle<double> &, const Gambit::Vector<double> &,
                         const Gambit::SquareMatrix<double> &) const;

  // Combine the last two steps into a single query

  bool NewtonRootIsOnlyInRct(const gRectangle<double> &, Gambit::Vector<double> &) const;

  // Recursive parts of recursive methods

  void FindRootsRecursion(Gambit::List<Gambit::Vector<double>> *, const gRectangle<double> &,
                          const int &, Gambit::Array<int> &, int &iterations, int depth,
                          const int &, int *) const;

  bool ARootExistsRecursion(const gRectangle<double> &, Gambit::Vector<double> &,
                            const gRectangle<double> &, Gambit::Array<int> &) const;

public:
  class NewtonError : public Gambit::Exception {
  public:
    ~NewtonError() noexcept override = default;
    const char *what() const noexcept override
    {
      return "Newton method failed to polish approximate root";
    }
  };
  explicit QuikSolv(const gPolyList<T> &);
  QuikSolv(const gPolyList<T> &, const int &);
  QuikSolv(const QuikSolv<T> &);
  ~QuikSolv();

  // Operators
  QuikSolv<T> &operator=(const QuikSolv<T> &);
  bool operator==(const QuikSolv<T> &) const;
  bool operator!=(const QuikSolv<T> &) const;

  // Information
  inline const gSpace *AmbientSpace() const { return System.AmbientSpace(); }
  inline const term_order *TermOrder() const { return System.TermOrder(); }
  inline int Dmnsn() const { return System.Dmnsn(); }
  inline const gPolyList<T> &UnderlyingEquations() const { return System; }
  inline bool WasSolved() const { return HasBeenSolved; }
  inline const Gambit::List<Gambit::Vector<double>> &RootList() const { return Roots; }
  inline bool IsMultiaffine() const { return isMultiaffine; }

  // Refines the accuracy of roots obtained from other algorithms
  Gambit::Vector<double> NewtonPolishOnce(const Gambit::Vector<double> &) const;
  Gambit::Vector<double> SlowNewtonPolishOnce(const Gambit::Vector<double> &) const;
  Gambit::Vector<double> NewtonPolishedRoot(const Gambit::Vector<double> &) const;

  // Checks for complex singular roots
  bool MightHaveSingularRoots() const;

  // The grand calculation - returns true if successful
  bool FindCertainNumberOfRoots(const gRectangle<T> &, const int &, const int &);
  bool FindRoots(const gRectangle<T> &, const int &);
  bool ARootExists(const gRectangle<T> &, Gambit::Vector<double> &) const;
};

#endif // QUIKSOLV_H
