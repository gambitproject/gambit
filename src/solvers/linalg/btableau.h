//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/liblinear/btableau.h
// Interface to base tableau classes
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

#ifndef BTABLEAU_H
#define BTABLEAU_H

#include "gambit.h"

namespace Gambit::linalg {

template <class T> class BFS {
private:
  std::map<int, T> m_map;
  T m_default;

public:
  // Lifecycle
  BFS() : m_default(0) {}
  ~BFS() = default;

  // define two BFS's to be equal if their bases are equal
  bool operator==(const BFS &M) const
  {
    if (m_map.size() != M.m_map.size()) {
      return false;
    }

    for (auto iter = m_map.begin(); iter != m_map.end(); iter++) {
      if (M.m_map.count((*iter).first) == 0) {
        return false;
      }
    }
    return true;
  }
  bool operator!=(const BFS &M) const { return !(*this == M); }

  // Provide map-like operations
  int count(int key) const { return (m_map.count(key) > 0); }

  void insert(int key, const T &value)
  {
    m_map.erase(key);
    m_map.insert(std::pair<int, T>(key, value));
  }

  const T &operator[](int key) const
  {
    if (m_map.count(key) == 1) {
      return const_cast<std::map<int, T> &>(m_map)[key];
    }
    else {
      return m_default;
    }
  }
};

class Basis {
private:
  Array<int> basis;  // current members of basis (neg for slacks)
  Array<int> cols;   // location of col in basis (0 if not in basis)
  Array<int> slacks; // location of slacks in basis
  bool is_ident{true};

public:
  Basis(int first, int last, int firstlabel, int lastlabel)
    : basis(first, last), cols(firstlabel, lastlabel), slacks(first, last)
  {
    std::fill(cols.begin(), cols.end(), 0);
    std::iota(slacks.begin(), slacks.end(), slacks.front_index());
    std::generate(basis.begin(), basis.end(),
                  [n = -basis.front_index()]() mutable { return n--; });
  }
  Basis(const Basis &) = default;
  ~Basis() = default;

  Basis &operator=(const Basis &) = default;

  int First() const { return basis.front_index(); }
  int Last() const { return basis.back_index(); }
  int MinCol() const { return cols.front_index(); }
  int MaxCol() const { return cols.back_index(); }

  bool IsRegColumn(int col) const { return col >= cols.front_index() && col <= cols.back_index(); }
  bool IsSlackColumn(int col) const
  {
    return -col >= basis.front_index() && -col <= basis.back_index();
  }

  // remove outindex, insert label, return outlabel
  int Pivot(int outindex, int col)
  {
    const int outlabel = basis[outindex];

    if (IsSlackColumn(col)) {
      slacks[-col] = outindex;
    }
    else if (IsRegColumn(col)) {
      cols[col] = outindex;
    }
    else {
      throw std::out_of_range("Pivot in column out of range"); // not a valid column to pivot in.
    }

    if (IsSlackColumn(outlabel)) {
      slacks[-outlabel] = 0;
    }
    else if (IsRegColumn(outlabel)) {
      cols[outlabel] = 0;
    }
    else {
      // Note: here, should back out outindex.
      throw std::out_of_range("Pivot out column out of range"); // not a valid column to pivot out.
    }

    basis[outindex] = col;
    CheckBasis();

    return outlabel;
  }

  // return true iff label is a Basis member
  bool Member(int label) const
  {
    return ((IsSlackColumn(label) && slacks[-label] != 0) ||
            (IsRegColumn(label) && cols[label] != 0));
  }
  // finds Basis index corresponding to label number
  int Find(int label) const
  {
    if (IsSlackColumn(label)) {
      return slacks[-label];
    }
    if (IsRegColumn(label)) {
      return cols[label];
    }
    throw std::out_of_range("Column label index out of range");
  }

  // finds label of variable corresponding to Basis index
  int Label(int index) const { return basis[index]; }

  // Check if Basis is Ident
  void CheckBasis()
  {
    is_ident = true;
    for (int i = basis.front_index(); i <= basis.back_index(); i++) {
      if (basis[i] != -i) {
        is_ident = false;
        return;
      }
    }
  }
  // returns whether the basis is the identity matrix
  bool IsIdent() const { return is_ident; }
};

inline void epsilon(double &v, int i = 8) { v = std::pow(10.0, (double)-i); }

inline void epsilon(Rational &v, int /*i*/ = 8) { v = Rational(0); }

class BadPivot final : public std::runtime_error {
public:
  BadPivot() : std::runtime_error("Bad pivot") {}
  ~BadPivot() noexcept override = default;
};

template <class T> class TableauInterface {
protected:
  Matrix<T> A; // should this be private?
  Vector<T> b; // should this be private?
  Basis basis;
  Vector<T> solution; // current solution vector. should this be private?
  T eps1, eps2;
  Array<int> artificial; // artificial variables

public:
  TableauInterface(const Matrix<T> &A, const Vector<T> &b)
    : A(A), b(b), basis(A.MinRow(), A.MaxRow(), A.MinCol(), A.MaxCol()),
      solution(A.MinRow(), A.MaxRow()), artificial(A.MaxCol() + 1, A.MaxCol())

  {
    // These are the values recommended by Murtagh (1981) for 15 digit
    // accuracy in LP problems
    // Note: for Rational, eps1 and eps2 resolve to 0
    linalg::epsilon(eps1, 5);
    linalg::epsilon(eps2);
  }
  TableauInterface(const Matrix<T> &A, const Array<int> &art, const Vector<T> &b)
    : A(A), b(b), basis(A.MinRow(), A.MaxRow(), A.MinCol(), A.MaxCol() + art.size()),
      solution(A.MinRow(), A.MaxRow()), artificial(A.MaxCol() + 1, A.MaxCol() + art.size())
  {
    linalg::epsilon(eps1, 5);
    linalg::epsilon(eps2);
    for (size_t i = 0; i < art.size(); i++) {
      artificial[A.MaxCol() + 1 + i] = art[art.front_index() + i];
    }
  }
  TableauInterface(const TableauInterface<T> &) = default;
  virtual ~TableauInterface() = default;

  TableauInterface<T> &operator=(const TableauInterface<T> &) = default;

  // information

  int MinRow() const { return A.MinRow(); }
  int MaxRow() const { return A.MaxRow(); }
  int MinCol() const { return basis.MinCol(); }
  int MaxCol() const { return basis.MaxCol(); }

  bool ColIndex(int x) const { return MinCol() <= x && x <= MaxCol(); }
  bool RowIndex(int x) const { return MinRow() <= x && x <= MaxRow(); }
  bool ValidIndex(int x) const { return ColIndex(x) || RowIndex(-x); }

  Basis &GetBasis() { return basis; }

  bool Member(int i) const { return basis.Member(i); }
  // return variable in i'th position of Tableau
  int Label(int i) const { return basis.Label(i); }
  // return Tableau position of variable i
  int Find(int i) const { return basis.Find(i); }

  virtual void BasisVector(Vector<T> &x) const = 0; // solve M x = (*b)
  virtual void GetColumn(int col, Vector<T> &ret) const
  {
    if (IsArtifColumn(col)) {
      ret = (T)0;
      ret[artificial[col]] = (T)1;
    }
    else if (basis.IsRegColumn(col)) {
      A.GetColumn(col, ret);
    }
    else if (basis.IsSlackColumn(col)) {
      ret = (T)0;
      ret[-col] = (T)1;
    }
  }

  BFS<T> GetBFS1() const
  {
    Vector<T> sol(basis.First(), basis.Last());
    BasisVector(sol);

    BFS<T> cbfs;
    for (int i = -MaxRow(); i <= -MinRow(); i++) {
      if (Member(i)) {
        cbfs.insert(i, sol[basis.Find(i)]);
      }
    }
    for (int i = MinCol(); i <= MaxCol(); i++) {
      if (Member(i)) {
        cbfs.insert(i, sol[basis.Find(i)]);
      }
    }
    return cbfs;
  }

  BFS<T> GetBFS() const
  {
    Vector<T> sol(basis.First(), basis.Last());
    BasisVector(sol);

    BFS<T> cbfs;
    for (int i = MinCol(); i <= MaxCol(); i++) {
      if (Member(i)) {
        cbfs.insert(i, sol[basis.Find(i)]);
      }
    }
    return cbfs;
  }

  virtual void Pivot(int outrow, int col) = 0;    // pivot -- outgoing is row, incoming is column
  virtual void SolveColumn(int, Vector<T> &) = 0; // column in new basis

  // miscellaneous functions
  bool EqZero(const T &x) const { return (LeZero(x) && GeZero(x)); }
  bool LtZero(const T &x) const { return !GeZero(x); }
  bool GtZero(const T &x) const { return !LeZero(x); }
  bool LeZero(const T &x) const { return (x <= eps2); }
  bool GeZero(const T &x) const { return (x >= -eps2); }

  T Epsilon(int i = 2) const
  {
    if (i != 1 && i != 2) {
      throw Gambit::DimensionException();
    }
    return (i == 1) ? eps1 : eps2;
  }

  bool IsArtifColumn(int col) const
  {
    return (col >= artificial.front_index() && col <= artificial.back_index());
  }
};

} // end namespace Gambit::linalg

#endif // BTABLEAU_H
