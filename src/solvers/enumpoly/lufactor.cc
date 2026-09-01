//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/lufactor.cc
// LU factorization of a square matrix of doubles
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

#include "lufactor.h"

namespace Gambit {

namespace {
constexpr double kSingularityTolerance = 1.0e-9;
} // end anonymous namespace

LUFactorization::LUFactorization(const Matrix<double> &m)
  : m_lu(m), m_pivot(m.MinRow(), m.MaxRow())
{
  if (!m.IsSquare()) {
    throw SingularMatrixException();
  }
  const int lo = m_lu.MinRow(), hi = m_lu.MaxRow();
  for (int i = lo; i <= hi; i++) {
    m_pivot[i] = i;
  }
  for (int k = lo; k <= hi; k++) {
    int pivotRow = k;
    double maxVal = std::abs(m_lu(k, k));
    for (int i = k + 1; i <= hi; i++) {
      if (std::abs(m_lu(i, k)) > maxVal) {
        maxVal = std::abs(m_lu(i, k));
        pivotRow = i;
      }
    }
    if (maxVal <= kSingularityTolerance) {
      throw SingularMatrixException();
    }
    if (pivotRow != k) {
      m_lu.SwitchRows(k, pivotRow);
      std::swap(m_pivot[k], m_pivot[pivotRow]);
      m_determinant = -m_determinant;
    }
    m_determinant *= m_lu(k, k);
    for (int i = k + 1; i <= hi; i++) {
      const double factor = m_lu(i, k) / m_lu(k, k);
      m_lu(i, k) = factor;
      for (int j = k + 1; j <= hi; j++) {
        m_lu(i, j) -= factor * m_lu(k, j);
      }
    }
  }
}

Vector<double> LUFactorization::Solve(const Vector<double> &b) const
{
  if (!m_lu.ConformsToColumn(b)) {
    throw DimensionException();
  }
  const int lo = m_lu.MinRow(), hi = m_lu.MaxRow();
  Vector<double> x(lo, hi);
  for (int i = lo; i <= hi; i++) {
    x[i] = b[m_pivot[i]];
  }
  // Forward substitution: L y = Pb (L has unit diagonal, stored below it in m_lu)
  for (int i = lo + 1; i <= hi; i++) {
    for (int j = lo; j < i; j++) {
      x[i] -= m_lu(i, j) * x[j];
    }
  }
  // Back substitution: U x = y
  for (int i = hi; i >= lo; i--) {
    for (int j = i + 1; j <= hi; j++) {
      x[i] -= m_lu(i, j) * x[j];
    }
    x[i] /= m_lu(i, i);
  }
  return x;
}

Matrix<double> LUFactorization::Inverse() const
{
  const int lo = m_lu.MinRow(), hi = m_lu.MaxRow();
  Matrix<double> inv(lo, hi, lo, hi);
  for (int j = lo; j <= hi; j++) {
    Vector<double> e(lo, hi);
    e = 0.0;
    e[j] = 1.0;
    const Vector<double> col = Solve(e);
    for (int i = lo; i <= hi; i++) {
      inv(i, j) = col[i];
    }
  }
  return inv;
}

} // end namespace Gambit
