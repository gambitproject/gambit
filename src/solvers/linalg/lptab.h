//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/linalg/lptab.h
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

#ifndef GAMBIT_SOLVERS_LINALG_LPTAB_H
#define GAMBIT_SOLVERS_LINALG_LPTAB_H

#include <list>

#include "tableau.h"

namespace Gambit::linalg {

template <class T> class LPTableau final : public Tableau<T> {
public:
  LPTableau(const Matrix<T> &A, const Vector<T> &b);
  LPTableau(const Matrix<T> &A, const Array<int> &art, const Vector<T> &b);
  LPTableau(const LPTableau<T> &) = default;
  ~LPTableau() = default;

  LPTableau<T> &operator=(const LPTableau<T> &) = default;

  /// @name Cost information
  //@{
  /// Set cost vector; unit column cost := 0
  void SetCost(const Vector<T> &);
  /// Cost of current solution
  T ComputeTotalCost() const;
  /// Relative cost of a column; negative index convention
  T ComputeRelativeCost(int) const;
  const Vector<T> &GetDualVector() const { return m_dual; }
  //@}

  void Refactor();
  void SetConst(const Vector<T> &bnew);
  void Pivot(int outrow, int col);
  std::list<Array<int>> ComputeReversePivots();
  bool IsDualReversePivot(int i, int j);
  BFS<T> GetDualBFS() const;

private:
  Vector<T> m_dual;
  Vector<T> m_unitCost;
  Vector<T> m_cost;

  void SolveDual();

  /// Same unit-column convention as ComputeRelativeCost, but selects the
  /// vector entries themselves rather than a single cost
  void BasisSelect(const Vector<T> &unitv, const Vector<T> &rowv, Vector<T> &colv) const;
};

} // end namespace Gambit::linalg

#endif // GAMBIT_SOLVERS_LINALG_LPTAB_H
