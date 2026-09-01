//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/lufactor.h
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

#ifndef GAMBIT_SOLVERS_ENUMPOLY_LUFACTOR_H
#define GAMBIT_SOLVERS_ENUMPOLY_LUFACTOR_H

#include "gambit.h"

namespace Gambit {

/// LU factorization (with partial pivoting) of a square matrix, computed once
/// and reused for Solve()/Determinant()/Inverse() without repeating the
/// O(n^3) elimination.  Throws SingularMatrixException if the matrix is
/// singular, or any pivot is too small to trust the factorization.
class LUFactorization {
public:
  explicit LUFactorization(const Matrix<double> &m);

  double Determinant() const { return m_determinant; }

  /// @throws DimensionException if `b` does not conform to the factored matrix
  Vector<double> Solve(const Vector<double> &b) const;
  Matrix<double> Inverse() const;

private:
  Matrix<double> m_lu;
  Array<int> m_pivot;
  double m_determinant{1.0};
};

} // end namespace Gambit

#endif // GAMBIT_SOLVERS_ENUMPOLY_LUFACTOR_H
