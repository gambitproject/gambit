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

namespace {

bool fuzzy_equals(double x, double y)
{
  const double epsilon = 0.000000001;

  if (x == 0.0) {
    return std::abs(y) < epsilon;
  }
  if (y == 0.0) {
    return std::abs(x) < epsilon;
  }
  return ((std::abs(x - y) / (std::abs(x) + std::abs(y)) < epsilon) ||
          (std::abs(x) < epsilon && std::abs(y) < epsilon));
}

inline bool fuzzy_zero(const Vector<double> &x)
{
  return std::all_of(x.begin(), x.end(), [](double v) { return std::abs(v) < 0.000000001; });
}

} // end anonymous namespace

bool PolynomialSystemSolver::SystemHasNoRootsIn(const Rectangle<double> &r) const
{
  return (
      std::any_of(m_derivatives.begin(), std::next(m_derivatives.begin(), NumEquations()),
                  [&](const PolynomialDerivatives<double> &p) { return p.PolyHasNoRootsIn(r); }) ||
      std::any_of(
          std::next(m_derivatives.begin(), NumEquations()), m_derivatives.end(),
          [&](const PolynomialDerivatives<double> &p) { return p.PolyEverywhereNegativeIn(r); }));
}

bool PolynomialSystemSolver::NewtonRootInRectangle(const Rectangle<double> &r,
                                                   Vector<double> &point) const
{
  auto evals = m_derivatives.ValuesOfRootPolys(point, NumEquations());
  if (fuzzy_zero(evals)) {
    return r.Contains(point);
  }

  Vector<double> newpoint(GetDimension());
  while (true) {
    try {
      newpoint = NewtonStep(point);
    }
    catch (SingularMatrixException &) {
      for (int i = 1; i <= point.size(); i++) {
        Vector<double> perturbed_point(point);
        if (r.Side(i).UpperBound() > point[i]) {
          perturbed_point[i] += (r.Side(i).UpperBound() - point[i]) / 10;
        }
        else {
          perturbed_point[i] += (r.Side(i).LowerBound() - point[i]) / 10;
        }
        try {
          newpoint = point + (NewtonStep(perturbed_point) - perturbed_point);
          break;
        }
        catch (SingularMatrixException &) {
        }
      }
    }

    if (!r.SameCenterDoubleLengths().Contains(newpoint)) {
      return false;
    }
    point = newpoint;
    auto newevals = m_derivatives.ValuesOfRootPolys(point, NumEquations());
    if (newevals.NormSquared() >= evals.NormSquared()) {
      return false;
    }
    if (fuzzy_zero(newevals)) {
      if (r.Contains(point)) {
        point = ImprovingNewtonStep(point);
        point = ImprovingNewtonStep(point);
        return true;
      }
      return false;
    }
    evals = newevals;
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
  Array<int> bottom(GetDimension()), top(GetDimension());
  std::fill(bottom.begin(), bottom.end(), 1);
  std::fill(top.begin(), top.end(), 2);
  double max_length = 0.0;
  for (const auto &topbottoms : CartesianIndexProduct(bottom, top)) {
    Vector<double> diffs(GetDimension());
    for (int i = 1; i <= GetDimension(); i++) {
      diffs[i] =
          (topbottoms[i] == 2) ? r.Side(i).UpperBound() - p[i] : p[i] - r.Side(i).LowerBound();
    }
    max_length = std::max(max_length, (M * diffs).NormSquared());
  }
  return std::sqrt(max_length);
}

bool PolynomialSystemSolver::HasNoOtherRootsIn(const Rectangle<double> &r,
                                               const Vector<double> &p) const
{
  auto M = m_derivatives.SquareDerivativeMatrix(p).Inverse();
  auto system2 = m_normalizedSystem.Translate(p).TransformCoords(M);
  double radius = MaxDistanceFromPointToVertexAfterTransformation(r, p, M);
  return (std::accumulate(system2.begin(), system2.end(), 0.0,
                          [&](double v, const Polynomial<double> &poly) {
                            return v + poly.MaximalValueOfNonlinearPart(radius);
                          }) < radius);
}

Vector<double> PolynomialSystemSolver::NewtonStep(const Vector<double> &point) const
{
  Vector<double> evals = m_derivatives.ValuesOfRootPolys(point, NumEquations());
  Matrix<double> deriv = m_derivatives.DerivativeMatrix(point, NumEquations());
  auto transpose = deriv.Transpose();
  auto sqmat = SquareMatrix<double>(deriv * transpose);
  if (std::abs(sqmat.Determinant()) <= 1.0e-9) {
    throw SingularMatrixException();
  }
  return point - (transpose * sqmat.Inverse()) * evals;
}

Vector<double> PolynomialSystemSolver::ImprovingNewtonStep(const Vector<double> &point) const
{
  Vector<double> evals = m_derivatives.ValuesOfRootPolys(point, NumEquations());
  Matrix<double> deriv = m_derivatives.DerivativeMatrix(point, NumEquations());
  auto transpose = deriv.Transpose();
  auto sqmat = SquareMatrix<double>(deriv * transpose);
  if (std::abs(sqmat.Determinant()) <= 1.0e-9) {
    throw SingularMatrixException();
  }
  auto step = -(transpose * sqmat.Inverse()) * evals;

  while (m_derivatives.ValuesOfRootPolys(point + step, NumEquations()).NormSquared() >
         evals.NormSquared()) {
    step /= 2;
  }
  return point + step;
}

std::list<Vector<double>> PolynomialSystemSolver::FindRoots(const Rectangle<double> &r,
                                                            const int max_roots)
{
  std::list<Vector<double>> roots;
  if (NumEquations() == 0) {
    roots.push_back(Vector<double>());
  }
  else {
    FindRoots(roots, r, max_roots);
  }
  return roots;
}

void PolynomialSystemSolver::FindRoots(std::list<Vector<double>> &rootlist,
                                       const Rectangle<double> &r, const int max_roots) const
{
  if (SystemHasNoRootsIn(r)) {
    return;
  }

  Vector<double> point = r.Center();
  if (NewtonRootInRectangle(r, point) && HasNoOtherRootsIn(r, point)) {
    // If all inequalities are satisfied and we haven't found the point before, add to the
    // list of roots
    if (std::all_of(std::next(m_derivatives.begin(), NumEquations()), m_derivatives.end(),
                    [&point](const PolynomialDerivatives<double> &d) {
                      return d.ValueOfRootPoly(point) >= 0.0;
                    }) &&
        !std::any_of(rootlist.begin(), rootlist.end(), [&point](const Vector<double> &root) {
          return std::equal(point.begin(), point.end(), root.begin(), fuzzy_equals);
        })) {
      rootlist.push_back(point);
    }
    return;
  }
  if (r.MaxSideLength() < 1.0e-8) {
    return;
  }
  for (const auto &cell : r.Orthants()) {
    FindRoots(rootlist, cell, max_roots);
    if (rootlist.size() >= max_roots) {
      return;
    }
  }
}

} // end namespace Gambit
