//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/lcp/lhtab.h
// Tableau class for Lemke-Howson algorithm
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

#ifndef GAMBIT_LINALG_LHTAB_H
#define GAMBIT_LINALG_LHTAB_H

#include "lemketab.h"

namespace Gambit::linalg {

template <class T> class LHTableau {
public:
  /// @name Lifecycle
  //@{
  LHTableau(const Matrix<T> &A1, const Matrix<T> &A2, const Vector<T> &b1, const Vector<T> &b2)
    : T1(A1, b1), T2(A2, b2), tmp1(b1.front_index(), b1.back_index()),
      tmp2(b2.front_index(), b2.back_index()), solution(b1.front_index(), b2.back_index())
  {
  }
  LHTableau(const LHTableau<T> &) = default;
  ~LHTableau() = default;

  LHTableau<T> &operator=(const LHTableau<T> &) = delete;
  //@}

  /// @name General information
  //@{

  int MinRow() const { return T1.MinRow(); }
  int MaxRow() const { return T2.MaxRow(); }
  int MinCol() const { return T2.MinCol(); }
  int MaxCol() const { return T1.MaxCol(); }

  bool ColIndex(int x) const { return MinCol() <= x && x <= MaxCol(); }
  bool RowIndex(int x) const { return MinRow() <= x && x <= MaxRow(); }

  bool Member(int i) const { return T1.Member(i) || T2.Member(i); }
  /// Return variable in i'th position of Tableau
  int Label(int i) const;
  /// Return Tableau position of variable i
  int Find(int i) const;
  //@}

  /// @name Pivoting operations
  //@{
  /// Perform apivot operation -- outgoing is row, incoming is column
  void Pivot(int outrow, int inlabel);
  //@}

  /// @name Miscellaneous functions
  //@{
  BFS<T> GetBFS();

  int PivotIn(int i);
  int ExitIndex(int i);
  /// Follow a path of ACBFS's from one CBFS to another
  int LemkePath(int dup);
  //@}

protected:
  LemkeTableau<T> T1, T2;
  Vector<T> tmp1, tmp2; // temporary column vectors, to avoid allocation
  Vector<T> solution;
};

} // end namespace Gambit::linalg

#endif // GAMBIT_LINALG_LHTAB_H
