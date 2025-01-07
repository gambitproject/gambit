//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/liblinear/ludecomp.h
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

#ifndef LUDECOMP_H
#define LUDECOMP_H

#include "gambit.h"
#include "basis.h"

namespace Gambit::linalg {

template <class T> class Tableau;

template <class T> class LUDecomposition {
private:
  struct EtaMatrix {
    int col;
    Vector<T> etadata;
  };

  Tableau<T> &tab;
  Basis &basis;

  std::list<EtaMatrix> U;
  std::list<EtaMatrix> E;
  std::vector<std::pair<int, EtaMatrix>> L;

  int refactor_number;
  int iterations;
  int total_operations;

  const LUDecomposition<T> *parent;
  mutable int copycount;

public:
  class BadPivot : public Exception {
  public:
    ~BadPivot() noexcept override = default;
    const char *what() const noexcept override { return "Bad pivot in LUdecomp"; }
  };
  class BadCount : public Exception {
  public:
    ~BadCount() noexcept override = default;
    const char *what() const noexcept override { return "Bad reference count in LUdecomp"; }
  };

  // ------------------------
  // Constructors, Destructor
  // ------------------------

  LUDecomposition(const LUDecomposition<T> &) = delete;

  // copy constructor
  // note:  Copying will fail an assertion if you try to update or delete
  //        the original before the copy has been deleted, refactored
  //        Or set to something else.
  LUDecomposition(const LUDecomposition<T> &, Tableau<T> &);

  // Decompose given matrix
  explicit LUDecomposition(Tableau<T> &, int rfac = 0);

  // Destructor
  ~LUDecomposition();

  // don't use the equals operator, use the Copy function instead
  LUDecomposition<T> &operator=(const LUDecomposition<T> &) = delete;

  // --------------------
  // Public Members
  // --------------------

  // copies the LUdecomp given (expect for the basis &).
  void Copy(const LUDecomposition<T> &, Tableau<T> &);

  // replace (update) the column given with the vector given.
  void update(int, int matcol); // matcol is the column number in the matrix

  // refactor
  void refactor();

  // solve: Bk d = a
  void solve(const Vector<T> &, Vector<T> &) const;

  // solve: y Bk = c
  void solveT(const Vector<T> &, Vector<T> &) const;

  // set number of etamatrices added before refactoring;
  // if number is set to zero, refactoring is done automatically.
  // if number is < 0, no refactoring is done
  void SetRefactor(int a) { refactor_number = a; }

private:
  void FactorBasis();

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

}; // end of class LUdecomp

} // end namespace Gambit::linalg

#endif // LUDECOMP_H
