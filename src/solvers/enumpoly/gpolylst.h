//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/gpolylist.h
// Declaration of polynomial list type
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

#ifndef GPOLYLST_H
#define GPOLYLST_H

#include "odometer.h"
#include "core/sqmatrix.h"
#include "gpoly.h"

using namespace Gambit;

template <class T> class gPolyList {
private:
  const VariableSpace *m_space;
  std::list<Polynomial<T>> m_list;

public:
  using iterator = typename List<Polynomial<T>>::iterator;
  using const_iterator = typename List<Polynomial<T>>::const_iterator;

  gPolyList(const VariableSpace *p_space) : m_space(p_space) {}
  gPolyList(const gPolyList<T> &) = default;
  ~gPolyList() = default;
  gPolyList<T> &operator=(const gPolyList<T> &) = default;

  iterator begin() { return m_list.begin(); }
  const_iterator begin() const { return m_list.begin(); }
  iterator end() { return m_list.end(); }
  const_iterator end() const { return m_list.end(); }
  const_iterator cbegin() const { return m_list.cbegin(); }
  const_iterator cend() const { return m_list.cend(); }

  bool operator==(const gPolyList<T> &rhs) const
  {
    return m_space == rhs.m_space && m_list == rhs.m_list;
  }
  bool operator!=(const gPolyList<T> &rhs) const
  {
    return m_space != rhs.m_space || m_list != rhs.m_list;
  }
  void push_back(const Polynomial<T> &x) { m_list.push_back(x); }
  void push_back(const gPolyList<T> &x)
  {
    for (const auto &p : x.m_list) {
      m_list.push_back(p);
    }
  }

  const Polynomial<T> &operator[](const int index) const
  {
    return *std::next(m_list.begin(), index - 1);
  }

  const VariableSpace *AmbientSpace() const { return m_space; }
  int size() const { return m_list.size(); }
  int Dmnsn() const { return m_space->GetDimension(); }
  bool IsMultiaffine() const
  {
    return std::all_of(m_list.begin(), m_list.end(),
                       [](const Polynomial<T> &v) { return v.IsMultiaffine(); });
  }
  /// Evaluate system at a point
  Vector<T> Evaluate(const Vector<T> &v) const
  {
    Vector<T> answer(m_list.size());
    std::transform(m_list.begin(), m_list.end(), answer.begin(),
                   [&](const Polynomial<T> &p) { return p.Evaluate(v); });
    return answer;
  }
  /// Translate system to new origin
  gPolyList<T> Translate(const Vector<T> &new_origin) const
  {
    gPolyList<T> ret(m_space);
    for (const auto &v : m_list) {
      ret.m_list.push_back(v.TranslateOfPoly(new_origin));
    }
    return ret;
  }
  /// Transform system to new coordinates
  gPolyList<T> TransformCoords(const SquareMatrix<T> &M) const
  {
    gPolyList<T> ret(m_space);
    for (const auto &v : m_list) {
      ret.m_list.push_back(v.PolyInNewCoordinates(M));
    }
    return ret;
  }
  /// Normalize system
  gPolyList<T> Normalize() const
  {
    gPolyList<T> ret(m_space);
    for (const auto &v : m_list) {
      ret.m_list.push_back(v.Normalize());
    }
    return ret;
  }
};

#endif // GPOLYLST_H
