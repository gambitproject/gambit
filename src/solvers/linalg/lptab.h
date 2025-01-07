//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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

namespace Gambit::linalg {

template <class T> class LPTableau : public Tableau<T> {
private:
  Vector<T> dual;
  Vector<T> unitcost;
  Vector<T> cost;

  void SolveDual();

public:
  LPTableau(const Matrix<T> &A, const Vector<T> &b);
  LPTableau(const Matrix<T> &A, const Array<int> &art, const Vector<T> &b);
  LPTableau(const LPTableau<T> &) = default;
  ~LPTableau() override = default;

  LPTableau<T> &operator=(const LPTableau<T> &) = default;

  // cost information
  void SetCost(const Vector<T> &); // unit column cost := 0
  const Vector<T> &GetCost() const { return cost; }
  const Vector<T> &GetUnitCost() const { return unitcost; }
  T TotalCost() const;       // cost of current solution
  T RelativeCost(int) const; // negative index convention
  const Vector<T> &GetDualVector() const { return dual; }

  void Refactor() override;
  void Pivot(int outrow, int col) override;
  std::list<Array<int>> ReversePivots();
  bool IsDualReversePivot(int i, int j);
  BFS<T> DualBFS() const;

  // returns the label of the index of the last artificial variable
  int GetLastLabel() { return this->artificial.last_index(); }

  // select Basis elements according to Tableau rows and cols
  void BasisSelect(const Array<T> &rowv, Vector<T> &colv) const;

  // as above, but unit column elements nonzero
  void BasisSelect(const Array<T> &unitv, const Array<T> &rowv, Vector<T> &colv) const;
};

} // end namespace Gambit::linalg

#endif // LPTAB_H
