//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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

namespace Gambit {
namespace linalg {

template <class T> class LHTableau : public BaseTableau<T> {
public:
  /// @name Lifecycle
  //@{
  LHTableau(const Matrix<T> &A1, const Matrix<T> &A2, const Vector<T> &b1, const Vector<T> &b2);
  ~LHTableau() override = default;

  LHTableau<T> &operator=(const LHTableau<T> &);
  //@}

  /// @name General information
  //@{
  int MinRow() const override { return T1.MinRow(); }
  int MaxRow() const override { return T2.MaxRow(); }
  int MinCol() const override { return T2.MinCol(); }
  int MaxCol() const override { return T1.MaxCol(); }
  T Epsilon() const { return T1.Epsilon(); }

  bool Member(int i) const override { return T1.Member(i) || T2.Member(i); }
  /// Return variable in i'th position of Tableau
  int Label(int i) const override;
  /// Return Tableau position of variable i
  int Find(int i) const override;
  //@}

  /// @name Pivoting operations
  //@{
  bool CanPivot(int outgoing, int incoming) const override;
  /// Perform apivot operation -- outgoing is row, incoming is column
  void Pivot(int outrow, int inlabel) override;
  long NumPivots() const override { return T1.NumPivots() + T2.NumPivots(); }
  //@}

  /// @name Raw Tableau functions
  //@{
  void Refactor() override
  {
    T1.Refactor();
    T2.Refactor();
  }
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

} // namespace linalg
} // end namespace Gambit

#endif // GAMBIT_LINALG_LHTAB_H
