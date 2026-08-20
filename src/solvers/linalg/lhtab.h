//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/linalg/lhtab.h
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

#ifndef GAMBIT_SOLVERS_LINALG_LHTAB_H
#define GAMBIT_SOLVERS_LINALG_LHTAB_H

#include "core/cancel.h"
#include "lemketab.h"

namespace Gambit::linalg {

template <class T> class LHTableau final {
public:
  /// @name Lifecycle
  //@{
  LHTableau(const Matrix<T> &A1, const Matrix<T> &A2, const Vector<T> &b1, const Vector<T> &b2)
    : m_tableau1(A1, b1), m_tableau2(A2, b2), m_scratch1(b1.front_index(), b1.back_index()),
      m_scratch2(b2.front_index(), b2.back_index()), m_solution(b1.front_index(), b2.back_index())
  {
  }
  LHTableau(const LHTableau<T> &) = default;
  ~LHTableau() = default;

  LHTableau<T> &operator=(const LHTableau<T> &) = delete;
  //@}

  /// @name General information
  //@{

  int MinRow() const { return m_tableau1.MinRow(); }
  int MaxRow() const { return m_tableau2.MaxRow(); }
  int MinCol() const { return m_tableau2.MinCol(); }
  int MaxCol() const { return m_tableau1.MaxCol(); }

  bool IsColIndex(int x) const { return MinCol() <= x && x <= MaxCol(); }
  bool IsRowIndex(int x) const { return MinRow() <= x && x <= MaxRow(); }

  bool IsMember(int i) const { return m_tableau1.IsMember(i) || m_tableau2.IsMember(i); }
  /// Return variable in i'th position of Tableau
  int GetLabel(int i) const;
  /// Return Tableau position of variable i
  int GetPosition(int i) const;
  //@}

  /// @name Pivoting operations
  //@{
  /// Perform apivot operation -- outgoing is row, incoming is column
  void Pivot(int outrow, int inlabel);
  //@}

  /// @name Miscellaneous functions
  //@{
  /// Basic feasible solution restricted to column-indexed (structural) variables
  BFS<T> GetColumnBFS();

  int PivotIn(int i);

  /// Determines, for the current tableau and variable to be added to the
  /// basis, which element should leave the basis. The choice is the one
  /// specified by Eaves, which is guaranteed not to cycle, even if the
  /// problem is degenerate.
  int ExitIndex(int i);

  /// Follow a path of ACBFS's from one CBFS to another
  int LemkePath(int dup, const CancelToken &p_cancel = CancelToken());
  //@}

private:
  LemkeTableau<T> m_tableau1, m_tableau2;
  Vector<T> m_scratch1, m_scratch2; // temporary column vectors, to avoid allocation
  Vector<T> m_solution;
};

} // end namespace Gambit::linalg

#endif // GAMBIT_SOLVERS_LINALG_LHTAB_H
