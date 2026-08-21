//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/linalg/ludecomp.h
// Interface to LU decomposition classes
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

#ifndef GAMBIT_SOLVERS_LINALG_LUDECOMP_H
#define GAMBIT_SOLVERS_LINALG_LUDECOMP_H

#include <list>
#include <memory>

#include "core/core.h"

namespace Gambit::linalg {

template <class T> class Tableau;
class Basis;

template <class T> class LUDecomposition {
public:
  class BadPivot final : public std::runtime_error {
  public:
    BadPivot() : std::runtime_error("Bad pivot in LUDecomposition") {}
    ~BadPivot() noexcept override = default;
  };

  /// @name Constructors and destructor
  //@{
  LUDecomposition(const LUDecomposition<T> &) = delete;

  /// Copy constructor. The copy shares `a`'s factorization state as of the
  /// time of copying; subsequent updates to either are independent.
  LUDecomposition(const LUDecomposition<T> &a, Tableau<T> &t);

  /// Decompose given matrix
  explicit LUDecomposition(Tableau<T> &, int rfac = 0);

  /// Destructor
  ~LUDecomposition() = default;

  /// Don't use the equals operator, use the Copy function instead
  LUDecomposition<T> &operator=(const LUDecomposition<T> &) = delete;
  //@}

  /// @name Operations
  //@{
  /// Copies the LUDecomposition given (except for the basis reference)
  void Copy(const LUDecomposition<T> &, Tableau<T> &);

  /// Replace (update) the column given with the vector given
  void Update(int, int matcol); // matcol is the column number in the matrix

  /// Refactor
  void Refactor();

  /// Solve: Bk d = a
  void Solve(const Vector<T> &, Vector<T> &) const;

  /// Solve: y Bk = c
  void SolveT(const Vector<T> &, Vector<T> &) const;

  /// Set number of eta matrices added before refactoring;
  /// if number is set to zero, refactoring is done automatically;
  /// if number is < 0, no refactoring is done
  void SetRefactor(int a) { m_refactorInterval = a; }
  //@}

private:
  struct EtaMatrix {
    int col;
    Vector<T> etadata;
  };

  // Immutable node in a persistently-shared eta chain: forking is an O(1)
  // shared_ptr copy of the tail, and appending never disturbs it.
  struct EtaNode {
    EtaMatrix eta;
    std::shared_ptr<const EtaNode> prev;
  };

  // FactorBasis()'s output: expensive to compute, shared across copies.
  struct BaseFactorization {
    std::list<EtaMatrix> upperEtas;
    std::vector<std::pair<int, EtaMatrix>> lowerFactors;
  };

  Tableau<T> &m_tableau;
  Basis &m_basis;

  std::shared_ptr<const BaseFactorization> m_base;
  std::shared_ptr<const EtaNode> m_updateTail;

  int m_refactorInterval;
  int m_iterationsSinceRefactor;
  int m_totalOperations;

  BaseFactorization FactorBasis();

  void GaussElem(Matrix<T> &, int, int);

  bool RefactorCheck();

  void BTransE(Vector<T> &) const;
  void FTransE(Vector<T> &) const;
  void BTransU(Vector<T> &) const;
  void FTransU(Vector<T> &) const;
  void LPd_Trans(Vector<T> &) const;
  void yLP_Trans(Vector<T> &) const;

  void VectorEtaSolve(const EtaMatrix &, Vector<T> &y) const;
  void EtaVectorSolve(const EtaMatrix &, Vector<T> &d) const;

  void yLP_mult(const Vector<T> &y, int j, Vector<T> &) const;

  void LPd_mult(Vector<T> &d, int j, Vector<T> &) const;

}; // end of class LUDecomposition

} // end namespace Gambit::linalg

#endif // GAMBIT_SOLVERS_LINALG_LUDECOMP_H
