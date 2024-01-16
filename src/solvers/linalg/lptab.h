//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/liblinear/lptab.h
// Interface to LP tableaus
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

#ifndef LPTAB_H
#define LPTAB_H

#include "tableau.h"

namespace Gambit {

namespace linalg {

template <class T> class LPTableau : public Tableau<T> {
private:
  Vector<T> dual;
  Array<T> unitcost;
  Array<T> cost;
  Array<bool> UB, LB; // does col have upper/lower bound?
  Array<T> ub, lb;    // upper/lower bound

  void SolveDual();

public:
  class BadPivot : public Exception {
  public:
    ~BadPivot() noexcept override = default;
    const char *what() const noexcept override { return "Bad pivot in LPTableau."; }
  };
  LPTableau(const Matrix<T> &A, const Vector<T> &b);
  LPTableau(const Matrix<T> &A, const Array<int> &art, const Vector<T> &b);
  LPTableau(const LPTableau<T> &);
  ~LPTableau() override = default;

  LPTableau<T> &operator=(const LPTableau<T> &);

  // cost information
  void SetCost(const Vector<T> &); // unit column cost := 0
  void SetCost(const Vector<T> &, const Vector<T> &);
  Vector<T> GetCost() const;
  Vector<T> GetUnitCost() const;
  T TotalCost();             // cost of current solution
  T RelativeCost(int) const; // negative index convention
  void RelativeCostVector(Vector<T> &, Vector<T> &);
  void DualVector(Vector<T> &) const; // column vector
                                      // Redefined functions
  void Refactor() override;
  void Pivot(int outrow, int col) override;
  void ReversePivots(List<Array<int>> &);
  bool IsReversePivot(int i, int j);
  void DualReversePivots(List<Array<int>> &);
  bool IsDualReversePivot(int i, int j);
  BFS<T> DualBFS() const;

  // returns the label of the index of the last artificial variable
  int LastLabel();

  // select Basis elements according to Tableau rows and cols
  void BasisSelect(const Array<T> &rowv, Vector<T> &colv) const;

  // as above, but unit column elements nonzero
  void BasisSelect(const Array<T> &unitv, const Array<T> &rowv, Vector<T> &colv) const;
};

} // namespace linalg

} // end namespace Gambit

#endif // LPTAB_H
