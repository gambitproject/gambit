//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/linalg/tableau.h
// Interface to tableau class and shared tableau infrastructure
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

#ifndef GAMBIT_SOLVERS_LINALG_TABLEAU_H
#define GAMBIT_SOLVERS_LINALG_TABLEAU_H

#include <set>

#include "core/core.h"
#include "ludecomp.h"

namespace Gambit::linalg {

template <class T> class BFS {
public:
  /// @name Lifecycle
  //@{
  BFS() = default;
  ~BFS() = default;
  //@}

  /// Returns the set of basic indices (positive for structural variables, negative for
  /// slacks).
  std::set<int> Keys() const
  {
    std::set<int> keys;
    for (const auto &[key, value] : m_map) {
      keys.insert(key);
    }
    return keys;
  }

  /// @name Map-like operations
  //@{
  int count(int key) const { return m_map.contains(key); }

  void insert(int key, const T &value)
  {
    m_map.erase(key);
    m_map.insert(std::pair<int, T>(key, value));
  }

  const T &operator[](int key) const
  {
    if (m_map.contains(key)) {
      return const_cast<std::map<int, T> &>(m_map)[key];
    }
    else {
      return m_default;
    }
  }
  //@}

private:
  std::map<int, T> m_map;
  T m_default{0};
};

class Basis {
public:
  Basis(int first, int last, int firstlabel, int lastlabel)
    : m_basisLabels(first, last), m_columnPositions(firstlabel, lastlabel),
      m_slackPositions(first, last)
  {
    std::fill(m_columnPositions.begin(), m_columnPositions.end(), 0);
    std::iota(m_slackPositions.begin(), m_slackPositions.end(), m_slackPositions.front_index());
    std::generate(m_basisLabels.begin(), m_basisLabels.end(),
                  [n = -m_basisLabels.front_index()]() mutable { return n--; });
  }
  Basis(const Basis &) = default;
  ~Basis() = default;

  Basis &operator=(const Basis &) = default;

  int GetFirst() const { return m_basisLabels.front_index(); }
  int GetLast() const { return m_basisLabels.back_index(); }
  int MinCol() const { return m_columnPositions.front_index(); }
  int MaxCol() const { return m_columnPositions.back_index(); }

  bool IsRegColumn(int col) const
  {
    return col >= m_columnPositions.front_index() && col <= m_columnPositions.back_index();
  }
  bool IsSlackColumn(int col) const
  {
    return -col >= m_basisLabels.front_index() && -col <= m_basisLabels.back_index();
  }

  /// Remove outindex, insert label, return outlabel
  int Pivot(int outindex, int col)
  {
    const int outlabel = m_basisLabels[outindex];

    if (IsSlackColumn(col)) {
      m_slackPositions[-col] = outindex;
    }
    else if (IsRegColumn(col)) {
      m_columnPositions[col] = outindex;
    }
    else {
      throw std::out_of_range("Pivot in column out of range"); // not a valid column to pivot in.
    }

    if (IsSlackColumn(outlabel)) {
      m_slackPositions[-outlabel] = 0;
    }
    else if (IsRegColumn(outlabel)) {
      m_columnPositions[outlabel] = 0;
    }
    else {
      // Note: here, should back out outindex.
      throw std::out_of_range("Pivot out column out of range"); // not a valid column to pivot out.
    }

    m_basisLabels[outindex] = col;
    RefreshIsIdentity();

    return outlabel;
  }

  /// Return true iff label is a Basis member
  bool IsMember(int label) const
  {
    return ((IsSlackColumn(label) && m_slackPositions[-label] != 0) ||
            (IsRegColumn(label) && m_columnPositions[label] != 0));
  }
  /// Finds Basis index corresponding to label number
  int GetPosition(int label) const
  {
    if (IsSlackColumn(label)) {
      return m_slackPositions[-label];
    }
    if (IsRegColumn(label)) {
      return m_columnPositions[label];
    }
    throw std::out_of_range("Column label index out of range");
  }

  /// Finds label of variable corresponding to Basis index
  int GetLabel(int index) const { return m_basisLabels[index]; }

  /// Returns whether the basis is the identity matrix
  bool IsIdent() const { return m_isIdentity; }

private:
  Array<int> m_basisLabels;     // current members of basis (neg for slacks)
  Array<int> m_columnPositions; // location of col in basis (0 if not in basis)
  Array<int> m_slackPositions;  // location of slacks in basis
  bool m_isIdentity{true};

  void RefreshIsIdentity()
  {
    int i = m_basisLabels.front_index();
    m_isIdentity = std::all_of(m_basisLabels.begin(), m_basisLabels.end(),
                               [&i](int label) { return label == -(i++); });
  }
};

class BadPivot final : public std::runtime_error {
public:
  BadPivot() : std::runtime_error("Bad pivot") {}
  ~BadPivot() noexcept override = default;
};

template <class T> class TableauBase {
public:
  TableauBase(const Matrix<T> &A, const Vector<T> &b)
    : m_A(A), m_b(b), m_basis(A.MinRow(), A.MaxRow(), A.MinCol(), A.MaxCol()),
      m_solution(A.MinRow(), A.MaxRow()), m_feasibilityTolerance(ComputeEpsilon(5)),
      m_zeroTolerance(ComputeEpsilon()), m_artificialColumns(A.MaxCol() + 1, A.MaxCol())
  {
  }
  TableauBase(const Matrix<T> &A, const Array<int> &art, const Vector<T> &b)
    : m_A(A), m_b(b), m_basis(A.MinRow(), A.MaxRow(), A.MinCol(), A.MaxCol() + art.size()),
      m_solution(A.MinRow(), A.MaxRow()), m_feasibilityTolerance(ComputeEpsilon(5)),
      m_zeroTolerance(ComputeEpsilon()),
      m_artificialColumns(A.MaxCol() + 1, A.MaxCol() + art.size())
  {
    for (size_t i = 0; i < art.size(); i++) {
      m_artificialColumns[A.MaxCol() + 1 + i] = art[art.front_index() + i];
    }
  }
  TableauBase(const TableauBase<T> &) = default;
  ~TableauBase() = default;

  TableauBase<T> &operator=(const TableauBase<T> &) = default;

  /// @name Information
  //@{
  int MinRow() const { return m_A.MinRow(); }
  int MaxRow() const { return m_A.MaxRow(); }
  int MinCol() const { return m_basis.MinCol(); }
  int MaxCol() const { return m_basis.MaxCol(); }

  bool IsColIndex(int x) const { return MinCol() <= x && x <= MaxCol(); }
  bool IsRowIndex(int x) const { return MinRow() <= x && x <= MaxRow(); }
  bool IsValidIndex(int x) const { return IsColIndex(x) || IsRowIndex(-x); }

  Basis &GetBasis() { return m_basis; }

  bool IsMember(int i) const { return m_basis.IsMember(i); }
  /// Return variable in i'th position of Tableau
  int GetLabel(int i) const { return m_basis.GetLabel(i); }
  /// Return Tableau position of variable i
  int GetPosition(int i) const { return m_basis.GetPosition(i); }

  void GetColumn(int col, Vector<T> &ret) const
  {
    if (IsArtifColumn(col)) {
      ret = T{0};
      ret[m_artificialColumns[col]] = T{1};
    }
    else if (m_basis.IsRegColumn(col)) {
      m_A.GetColumn(col, ret);
    }
    else if (m_basis.IsSlackColumn(col)) {
      ret = T{0};
      ret[-col] = T{1};
    }
  }
  //@}

  /// @name Miscellaneous functions
  //@{
  bool IsEqZero(const T &x) const { return (IsLeZero(x) && IsGeZero(x)); }
  bool IsLtZero(const T &x) const { return !IsGeZero(x); }
  bool IsGtZero(const T &x) const { return !IsLeZero(x); }
  bool IsLeZero(const T &x) const { return (x <= m_zeroTolerance); }
  bool IsGeZero(const T &x) const { return (x >= -m_zeroTolerance); }

  T GetZeroTolerance() const { return m_zeroTolerance; }

  bool IsArtifColumn(int col) const
  {
    return (col >= m_artificialColumns.front_index() && col <= m_artificialColumns.back_index());
  }
  //@}

protected:
  Matrix<T> m_A;
  Vector<T> m_b;
  Basis m_basis;
  /// Current solution vector
  Vector<T> m_solution;
  T m_feasibilityTolerance, m_zeroTolerance;
  /// Column indices of artificial variables
  Array<int> m_artificialColumns;

private:
  // Values recommended by Murtagh (1981) for 15-digit accuracy in LP problems.
  // For Rational, both tolerances resolve to exact zero.
  static T ComputeEpsilon(int i = 8);
};

template <> inline double TableauBase<double>::ComputeEpsilon(int i)
{
  return std::pow(10.0, -static_cast<double>(i));
}

template <> inline Rational TableauBase<Rational>::ComputeEpsilon(int /*i*/)
{
  return Rational(0);
}

template <> class Tableau<double> : public TableauBase<double> {
public:
  /// @name Constructors and destructor
  //@{
  Tableau(const Matrix<double> &A, const Vector<double> &b);
  Tableau(const Matrix<double> &A, const Array<int> &art, const Vector<double> &b);
  Tableau(const Tableau<double> &);
  ~Tableau() = default;

  Tableau<double> &operator=(const Tableau<double> &);
  //@}

  /// @name Pivoting
  //@{
  /// Pivot -- outgoing is row, incoming is column
  void Pivot(int outrow, int col);
  /// Solve M x = (*b)
  void GetBasisVector(Vector<double> &x) const;
  /// Column in new basis
  void SolveColumn(int, Vector<double> &);
  /// Solve M x = b
  void Solve(const Vector<double> &b, Vector<double> &x);
  /// Solve y M = c
  void SolveT(const Vector<double> &c, Vector<double> &y);
  //@}

  /// @name Raw Tableau functions
  //@{
  void Refactor();
  void SetConst(const Vector<double> &bnew);
  bool IsLexMin();
  /// Basic feasible solution over both row-indexed (slack) and column-indexed variables
  BFS<double> GetFullBFS() const;
  /// Basic feasible solution restricted to column-indexed (structural) variables
  BFS<double> GetColumnBFS() const;
  //@}

private:
  // The LU decomposition of the tableau
  LUDecomposition<double> m_luDecomposition;
  // A temporary column vector, to avoid frequent allocation
  mutable Vector<double> m_scratchColumn;
};

template <> class Tableau<Rational> : public TableauBase<Rational> {
public:
  class BadDenom final : public std::runtime_error {
  public:
    BadDenom() : std::runtime_error("Bad denominator in Tableau") {}
    ~BadDenom() noexcept override = default;
  };

  Tableau(const Matrix<Rational> &A, const Vector<Rational> &b);
  Tableau(const Matrix<Rational> &A, const Array<int> &art, const Vector<Rational> &b);
  Tableau(const Tableau<Rational> &) = default;
  ~Tableau() = default;

  Tableau<Rational> &operator=(const Tableau<Rational> &) = default;

  /// @name Pivoting
  //@{
  /// Pivot -- outgoing is row, incoming is column
  void Pivot(int outrow, int col);
  /// Column in new basis
  void SolveColumn(int, Vector<Rational> &);
  /// Column in new basis
  void GetColumn(int, Vector<Rational> &) const;
  //@}

  /// @name Raw Tableau functions
  //@{
  void Refactor();
  void SetConst(const Vector<Rational> &bnew);
  /// Solve M x = b
  void Solve(const Vector<Rational> &b, Vector<Rational> &x);
  /// Solve y M = c
  void SolveT(const Vector<Rational> &c, Vector<Rational> &y);

  bool IsLexMin();
  void GetBasisVector(Vector<Rational> &out) const;
  /// Basic feasible solution over both row-indexed (slack) and column-indexed variables
  BFS<Rational> GetFullBFS() const;
  /// Basic feasible solution restricted to column-indexed (structural) variables
  BFS<Rational> GetColumnBFS() const;
  //@}

protected:
  Array<int> m_nonbasicLabels; //** nonbasic variables -- should be moved to Basis

  Integer GetTotalDenominator() const { return m_totalDenominator; }

private:
  Matrix<Integer> m_tableauData; // This carries the full tableau
  Vector<Integer> m_scaledRHS;   // integer-scaled right-hand side
  Integer m_totalDenominator;    // This carries the denominator for Q data or 1 for Z
  Integer m_pivotDenominator;    // This is the denominator for the simplex

  mutable Vector<Rational> m_scratchColumn; // temporary column vector, to avoid allocation

  int GetNonbasicPosition(int col_index) const; // aligns the column indexes
  Matrix<Rational> ComputeInverse();

  void MySolveColumn(int, Vector<Rational> &); // column in new basis
};

} // end namespace Gambit::linalg

#endif // GAMBIT_SOLVERS_LINALG_TABLEAU_H
