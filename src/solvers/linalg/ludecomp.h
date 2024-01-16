//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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

namespace Gambit {

namespace linalg {

template <class T> class Tableau;

// ---------------------------------------------------------------------------
// Class EtaMatrix
// ---------------------------------------------------------------------------

template <class T> class EtaMatrix {
public:
  int col;
  Vector<T> etadata;

  EtaMatrix(int c, Vector<T> &v) : col(c), etadata(v) {}

  // required for list class
  bool operator==(const EtaMatrix<T> &) const;
  bool operator!=(const EtaMatrix<T> &) const;
};

// ---------------------------------------------------------------------------
// Class LUdecomp
// ---------------------------------------------------------------------------

template <class T> class LUdecomp {
private:
  Tableau<T> &tab;
  Basis &basis;

  List<EtaMatrix<T>> L;
  List<EtaMatrix<T>> U;
  List<EtaMatrix<T>> E;
  List<int> P;

  Vector<T> scratch1; // scratch vectors so we don't reallocate them
  Vector<T> scratch2; // everytime we do something.

  int refactor_number;
  int iterations;
  int total_operations;

  const LUdecomp<T> *parent;
  int copycount;

  // don't use this copy constructor
  LUdecomp(const LUdecomp<T> &a);
  // don't use the equals operator, use the Copy function instead
  LUdecomp<T> &operator=(const LUdecomp<T> &);

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

  // copy constructor
  // note:  Copying will fail an assertion if you try to update or delete
  //        the original before the copy has been deleted, refactored
  //        Or set to something else.
  LUdecomp(const LUdecomp<T> &, Tableau<T> &);

  // Decompose given matrix
  explicit LUdecomp(Tableau<T> &, int rfac = 0);

  // Destructor
  ~LUdecomp();

  // --------------------
  // Public Members
  // --------------------

  // copies the LUdecomp given (expect for the basis &).
  void Copy(const LUdecomp<T> &, Tableau<T> &);

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
  // if number is < 0, no refactoring is done;
  void SetRefactor(int);

  //-------------------
  // Private Members
  //-------------------

private:
  void FactorBasis();

  void GaussElem(Matrix<T> &, int, int);

  bool CheckBasis();
  bool RefactorCheck();

  void BTransE(Vector<T> &) const;
  void FTransE(Vector<T> &) const;
  void BTransU(Vector<T> &) const;
  void FTransU(Vector<T> &) const;
  void LPd_Trans(Vector<T> &) const;
  void yLP_Trans(Vector<T> &) const;

  void VectorEtaSolve(const Vector<T> &v, const EtaMatrix<T> &, Vector<T> &y) const;

  void EtaVectorSolve(const Vector<T> &v, const EtaMatrix<T> &, Vector<T> &d) const;

  void yLP_mult(const Vector<T> &y, int j, Vector<T> &) const;

  void LPd_mult(Vector<T> &d, int j, Vector<T> &) const;

}; // end of class LUdecomp

} // namespace linalg

} // end namespace Gambit

#endif // LUDECOMP_H
