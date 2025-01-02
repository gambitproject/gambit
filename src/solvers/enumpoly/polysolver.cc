//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/polysolver.cc
// Implementation of polynomial system solver
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

#include "polysolver.h"
#include "indexproduct.h"

namespace Gambit {

RectArray<bool> PolynomialSystemSolver::BuildEquationVariableMatrix() const
{
  RectArray<bool> answer(m_system.size(), GetDimension());
  for (int i = 1; i <= m_system.size(); i++) {
    for (int j = 1; j <= GetDimension(); j++) {
      if (m_system[i].DegreeOfVar(j) > 0) {
        answer(i, j) = true;
      }
      else {
        answer(i, j) = false;
      }
    }
  }
  return answer;
}

bool PolynomialSystemSolver::SystemHasNoRootsIn(const Rectangle<double> &r,
                                                Array<int> &precedence) const
{
  for (int i = 1; i <= m_system.size(); i++) {
    if ((precedence[i] <= NumEquations() && m_derivatives[precedence[i]].PolyHasNoRootsIn(r)) ||
        (precedence[i] > NumEquations() &&
         m_derivatives[precedence[i]].PolyEverywhereNegativeIn(r))) {
      if (i != 1) { // We have found a new "most likely to never vanish"
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

//--------------------------------------
// Does Newton's method lead to a root?
//--------------------------------------

//
// In NewtonRootInRectangle(), problems with infinite looping will occur
// due to roundoff in trying to compare to the zero vector.
// The fuzzy_equals() functions below alleviate this by introducing
// an epsilon fudge-factor.  This fudge-factor, and indeed the entire
// implementation, is largely ad-hoc, but appears to work for most
// applications.
//

namespace {

static bool fuzzy_equals(double x, double y)
{
  const double epsilon = 0.000000001;

  if (x == 0.0) {
    return fabs(y) < epsilon;
  }
  if (y == 0.0) {
    return fabs(x) < epsilon;
  }
  return ((fabs(x - y) / (fabs(x) + fabs(y)) < epsilon) ||
          (fabs(x) < epsilon && fabs(y) < epsilon));
}

} // end anonymous namespace

bool PolynomialSystemSolver::NewtonRootInRectangle(const Rectangle<double> &r,
                                                   Vector<double> &point) const
{
  Vector<double> zero(GetDimension());
  zero = 0;

  Vector<double> oldevals = m_derivatives.ValuesOfRootPolys(point, NumEquations());
  if (std::equal(oldevals.begin(), oldevals.end(), zero.begin(), fuzzy_equals)) {
    return r.Contains(point);
  }

  Rectangle<double> bigr = r.SameCenterDoubleLengths();

  Vector<double> newpoint(GetDimension());

  while (true) {
    try {
      newpoint = NewtonPolishOnce(point);
    }
    catch (SingularMatrixException &) {
      bool nonsingular = false;
      int direction = 1;
      while (direction < GetDimension() && !nonsingular) {
        Vector<double> perturbed_point(point);
        if (r.Side(direction).UpperBound() > point[direction]) {
          perturbed_point[direction] += (r.Side(direction).UpperBound() - point[direction]) / 10;
        }
        else {
          perturbed_point[direction] += (r.Side(direction).LowerBound() - point[direction]) / 10;
        }
        nonsingular = true;

        try {
          newpoint = point + (NewtonPolishOnce(perturbed_point) - perturbed_point);
        }
        catch (SingularMatrixException &) {
          nonsingular = false;
        }
        direction++;
      }
      if (!nonsingular) {
        Vector<double> perturbed_point(point);
        if (r.Side(direction).UpperBound() > point[direction]) {
          perturbed_point[direction] += (r.Side(direction).UpperBound() - point[direction]) / 10;
        }
        else {
          perturbed_point[direction] += (r.Side(direction).LowerBound() - point[direction]) / 10;
        }
        newpoint = point + (NewtonPolishOnce(perturbed_point) - perturbed_point);
      }
    }

    if (!bigr.Contains(newpoint)) {
      return false;
    }
    point = newpoint;

    Vector<double> newevals = m_derivatives.ValuesOfRootPolys(point, NumEquations());
    if (newevals * newevals >= oldevals * oldevals) {
      return false;
    }
    if (std::equal(newevals.begin(), newevals.end(), zero.begin(), fuzzy_equals)) {
      if (r.Contains(point)) {
        point = SlowNewtonPolishOnce(point);
        point = SlowNewtonPolishOnce(point);
        return true;
      }
      return false;
    }
    oldevals = newevals;
  }
}

double PolynomialSystemSolver::MaxDistanceFromPointToVertexAfterTransformation(
    const Rectangle<double> &r, const Vector<double> &p, const SquareMatrix<double> &M) const
{
  // A very early implementation of this method used a type gDouble which
  // implemented fuzzy comparisons.  Adding the epsilon parameter here is
  // important for the case when a solution may be found on the boundary
  // of the rectangle but be slightly outside due to numerical error.
  if (!r.Contains(p, 1.0e-8)) {
    throw AssertionException(
        "Point not in rectangle in MaxDistanceFromPointToVertexAfterTransformation.");
  }
  double max_length = 0.0;

  Array<int> bottom(GetDimension()), top(GetDimension());
  std::fill(bottom.begin(), bottom.end(), 1);
  std::fill(top.begin(), top.end(), 2);
  for (const auto &topbottoms : CartesianIndexProduct(bottom, top)) {
    Vector<double> diffs(GetDimension());
    for (int i = 1; i <= GetDimension(); i++) {
      if (topbottoms[i] == 2) {
        diffs[i] = r.Side(i).UpperBound() - p[i];
      }
      else {
        diffs[i] = p[i] - r.Side(i).LowerBound();
      }
    }
    Vector<double> new_diffs = M * diffs;
    max_length = std::max(max_length, new_diffs * new_diffs);
  }
  return std::sqrt(max_length);
}

bool PolynomialSystemSolver::HasNoOtherRootsIn(const Rectangle<double> &r, const Vector<double> &p,
                                               const SquareMatrix<double> &M) const
{
  auto system2 = m_normalizedSystem.Translate(p).TransformCoords(M);
  double radius = MaxDistanceFromPointToVertexAfterTransformation(r, p, M);
  return (std::accumulate(system2.begin(), system2.end(), 0.0,
                          [&](double v, const Polynomial<double> &poly) {
                            return v + poly.MaximalValueOfNonlinearPart(radius);
                          }) < radius);
}

/// Does Newton's method yield a unique root?
bool PolynomialSystemSolver::NewtonRootIsOnlyInRct(const Rectangle<double> &r,
                                                   Vector<double> &point) const
{
  return (NewtonRootInRectangle(r, point) &&
          HasNoOtherRootsIn(r, point, m_derivatives.SquareDerivativeMatrix(point).Inverse()));
}

Vector<double> PolynomialSystemSolver::NewtonPolishOnce(const Vector<double> &point) const
{
  Vector<double> oldevals = m_derivatives.ValuesOfRootPolys(point, NumEquations());
  Matrix<double> Df = m_derivatives.DerivativeMatrix(point, NumEquations());
  return point - (Df.Transpose() * SquareMatrix<double>(Df * Df.Transpose()).Inverse()) * oldevals;
}

Vector<double> PolynomialSystemSolver::SlowNewtonPolishOnce(const Vector<double> &point) const
{
  Vector<double> oldevals = m_derivatives.ValuesOfRootPolys(point, NumEquations());
  Matrix<double> Df = m_derivatives.DerivativeMatrix(point, NumEquations());
  Vector<double> Del =
      -(Df.Transpose() * SquareMatrix<double>(Df * Df.Transpose()).Inverse()) * oldevals;

  while (true) {
    Vector<double> newevals(m_derivatives.ValuesOfRootPolys(point + Del, NumEquations()));
    if (newevals * newevals <= oldevals * oldevals) {
      return point + Del;
    }
    Del /= 2;
  }
}

bool PolynomialSystemSolver::FindRoots(const Rectangle<double> &r, const int max_roots)
{
  const int MAX_ITER = 100000;
  m_roots = List<Vector<double>>();

  if (NumEquations() == 0) {
    m_roots.push_back(Vector<double>(0));
    return true;
  }

  Array<int> precedence(m_system.size());
  // Orders search for nonvanishing poly
  std::iota(precedence.begin(), precedence.end(), 1);

  int iterations = 0;
  FindRoots(m_roots, r, MAX_ITER, precedence, iterations, 1, max_roots);
  return iterations < MAX_ITER;
}

void PolynomialSystemSolver::FindRoots(List<Vector<double>> &rootlist, const Rectangle<double> &r,
                                       int max_iterations, Array<int> &precedence, int &iterations,
                                       int depth, int max_roots) const
{
  //
  // TLT: In some cases, this recursive process apparently goes into an
  // infinite regress.  I'm not able to identify just why this occurs,
  // but as at least a temporary safeguard, we will limit the maximum depth
  // of this recursive search.
  //
  // This limit has been chosen only because it doesn't look like any
  // "serious" search (i.e., one that actually terminates with a result)
  // will take more than a depth of 32.
  //
  const int MAX_DEPTH = 32;

  if (SystemHasNoRootsIn(r, precedence)) {
    return;
  }

  Vector<double> point = r.Center();
  if (NewtonRootIsOnlyInRct(r, point)) {
    for (int i = NumEquations() + 1; i <= m_system.size(); i++) {
      if (m_derivatives[i].ValueOfRootPoly(point) < 0.0) {
        return;
      }
    }

    bool already_found = false;
    for (size_t i = 1; i <= rootlist.size(); i++) {
      if (std::equal(point.begin(), point.end(), rootlist[i].begin(), fuzzy_equals)) {
        already_found = true;
      }
    }
    if (!already_found) {
      rootlist.push_back(point);
    }
    return;
  }

  for (const auto &cell : r.Orthants()) {
    if (max_roots == 0 || rootlist.size() < max_roots) {
      if (iterations >= max_iterations || depth == MAX_DEPTH) {
        return;
      }
      iterations++;
      FindRoots(rootlist, cell, max_iterations, precedence, iterations, depth + 1, max_roots);
    }
  }
}

} // end namespace Gambit
