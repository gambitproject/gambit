//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
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

#ifndef LCP_LHTAB_H
#define LCP_LHTAB_H

#include "lemketab.h"

using namespace Gambit;

template <class T> Matrix<T> Make_A1(const Game &);
template <class T> Vector<T> Make_b1(const Game &);
template <class T> Matrix<T> Make_A2(const Game &);
template <class T> Vector<T> Make_b2(const Game &);

template <class T> class LHTableau : public BaseTableau<T> {
public:
  /// @name Lifecycle
  //@{
  LHTableau(const Matrix<T> &A1, const Matrix<T> &A2,
	    const Vector<T> &b1, const Vector<T> &b2);
  virtual ~LHTableau() { }
  
  LHTableau<T>& operator=(const LHTableau<T>&);
  //@}
  
  /// @name General information
  //@{
  int MinRow(void) const  { return T1.MinRow(); }
  int MaxRow(void) const  { return T2.MaxRow(); }
  int MinCol(void) const  { return T2.MinCol(); }
  int MaxCol(void) const  { return T1.MaxCol(); }
  T Epsilon(void) const   { return T1.Epsilon(); }
  
  bool Member(int i) const  { return T1.Member(i) || T2.Member(i); }
  /// Return variable in i'th position of Tableau
  int Label(int i) const;
  /// Return Tableau position of variable i
  int Find(int i) const;
  //@}
  
  /// @name Pivoting operations
  //@{
  bool CanPivot(int outgoing, int incoming) const;
  /// Perform apivot operation -- outgoing is row, incoming is column
  void Pivot(int outrow, int inlabel);
  long NumPivots(void) const { return T1.NumPivots() + T2.NumPivots(); }
  //@}

  /// @name Raw Tableau functions
  //@{
  void Refactor(void) { T1.Refactor(); T2.Refactor(); }
  //@}
  
  /// @name Miscellaneous functions
  //@{
  BFS<T> GetBFS(void);

  int PivotIn(int i);
  int ExitIndex(int i);
  /// Follow a path of ACBFS's from one CBFS to another
  int LemkePath(int dup);
  //@}

protected:
  LTableau<T> T1,T2;
  Vector<T> tmp1,tmp2; // temporary column vectors, to avoid allocation
  Vector<T> solution;
};

#endif  // LCP_LHTAB_H
