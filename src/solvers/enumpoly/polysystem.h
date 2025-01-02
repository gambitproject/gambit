//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/polysystem.h
// Representation of a system (collection) of polynomials
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

#ifndef POLYSYSTEM_H
#define POLYSYSTEM_H

#include "poly.h"

namespace Gambit {

template <class T> class PolynomialSystem {
private:
  std::shared_ptr<VariableSpace> m_space;
  std::list<Polynomial<T>> m_system;

public:
  using iterator = typename List<Polynomial<T>>::iterator;
  using const_iterator = typename List<Polynomial<T>>::const_iterator;

  PolynomialSystem(std::shared_ptr<VariableSpace> p_space) : m_space(p_space) {}
  PolynomialSystem(const PolynomialSystem<T> &) = default;
  ~PolynomialSystem() = default;
  PolynomialSystem<T> &operator=(const PolynomialSystem<T> &) = default;

  iterator begin() { return m_system.begin(); }
  const_iterator begin() const { return m_system.begin(); }
  iterator end() { return m_system.end(); }
  const_iterator end() const { return m_system.end(); }
  const_iterator cbegin() const { return m_system.cbegin(); }
  const_iterator cend() const { return m_system.cend(); }

  bool operator==(const PolynomialSystem<T> &rhs) const
  {
    return m_space == rhs.m_space && m_system == rhs.m_system;
  }
  bool operator!=(const PolynomialSystem<T> &rhs) const
  {
    return m_space != rhs.m_space || m_system != rhs.m_system;
  }
  void push_back(const Polynomial<T> &x) { m_system.push_back(x); }
  void push_back(const PolynomialSystem<T> &x)
  {
    for (const auto &p : x.m_system) {
      m_system.push_back(p);
    }
  }

  const Polynomial<T> &operator[](const int index) const
  {
    return *std::next(m_system.begin(), index - 1);
  }

  int size() const { return m_system.size(); }
  int GetDimension() const { return m_space->GetDimension(); }
  /// Evaluate system at a point
  Vector<T> Evaluate(const Vector<T> &v) const
  {
    Vector<T> answer(m_system.size());
    std::transform(m_system.begin(), m_system.end(), answer.begin(),
                   [&](const Polynomial<T> &p) { return p.Evaluate(v); });
    return answer;
  }
  /// Translate system to new origin
  PolynomialSystem<T> Translate(const Vector<T> &new_origin) const
  {
    PolynomialSystem<T> ret(m_space);
    for (const auto &v : m_system) {
      ret.m_system.push_back(v.TranslateOfPoly(new_origin));
    }
    return ret;
  }
  /// Transform system to new coordinates
  PolynomialSystem<T> TransformCoords(const SquareMatrix<T> &M) const
  {
    PolynomialSystem<T> ret(m_space);
    for (const auto &v : m_system) {
      ret.m_system.push_back(v.PolyInNewCoordinates(M));
    }
    return ret;
  }
  /// Normalize system
  PolynomialSystem<T> Normalize() const
  {
    PolynomialSystem<T> ret(m_space);
    for (const auto &v : m_system) {
      ret.m_system.push_back(v.Normalize());
    }
    return ret;
  }
};

} // end namespace Gambit

#endif // POLYSYSTEM_H
