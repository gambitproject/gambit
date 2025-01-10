//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/polypartial.h
// Efficient representation of partial derivatives of polynomials
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

#ifndef POLYPARTIAL_H
#define POLYPARTIAL_H

#include "gambit.h"
#include "rectangle.h"
#include "polysystem.h"

namespace Gambit {

template <class T> class PolynomialDerivatives {
private:
  struct Node {
  public:
    Polynomial<T> data;
    std::list<Node> children;

    Node(const Polynomial<T> &p_data) : data(p_data) {}
    Node(const Node &) = default;
    ~Node() = default;
  };

  Node m_treeroot;

  void BuildTree(Node &);
  T MaximalNonconstantContribution(const Node &, const Vector<T> &, const Vector<T> &,
                                   Vector<int> &) const;

public:
  explicit PolynomialDerivatives(const Polynomial<T> &given) : m_treeroot(given)
  {
    BuildTree(m_treeroot);
  }
  PolynomialDerivatives(const PolynomialDerivatives<T> &) = default;
  ~PolynomialDerivatives() = default;

  int GetDimension() const { return m_treeroot.data.GetDimension(); }

  T MaximalNonconstantContribution(const Vector<T> &, const Vector<T> &) const;

  T ValueOfRootPoly(const Vector<T> &point) const { return m_treeroot.data.Evaluate(point); }
  T ValueOfPartialOfRootPoly(int, const Vector<T> &) const;
  bool PolyHasNoRootsIn(const Rectangle<T> &) const;
  bool MultiaffinePolyHasNoRootsIn(const Rectangle<T> &) const;
  bool PolyEverywhereNegativeIn(const Rectangle<T> &) const;
  bool MultiaffinePolyEverywhereNegativeIn(const Rectangle<T> &) const;
};

template <class T> class PolynomialSystemDerivatives {
private:
  std::list<PolynomialDerivatives<T>> m_system;

public:
  using iterator = typename std::list<PolynomialDerivatives<T>>::iterator;
  using const_iterator = typename std::list<PolynomialDerivatives<T>>::const_iterator;

  explicit PolynomialSystemDerivatives(const PolynomialSystem<T> &given)
  {
    for (const auto &p : given) {
      m_system.push_back(PolynomialDerivatives<T>(p));
    }
  }
  PolynomialSystemDerivatives(const PolynomialSystemDerivatives<T> &) = default;
  ~PolynomialSystemDerivatives() = default;
  PolynomialSystemDerivatives<T> &operator=(const PolynomialSystemDerivatives<T> &) = delete;

  iterator begin() { return m_system.begin(); }
  const_iterator begin() const { return m_system.begin(); }
  iterator end() { return m_system.end(); }
  const_iterator end() const { return m_system.end(); }
  const_iterator cbegin() const { return m_system.cbegin(); }
  const_iterator cend() const { return m_system.cend(); }

  const PolynomialDerivatives<T> &operator[](const int index) const
  {
    return *std::next(m_system.begin(), index - 1);
  }
  int GetDimension() const { return m_system.front().GetDimension(); }
  Matrix<T> DerivativeMatrix(const Vector<T> &, int) const;
  SquareMatrix<T> SquareDerivativeMatrix(const Vector<T> &) const;
  Vector<T> ValuesOfRootPolys(const Vector<T> &, int) const;
};

} // namespace Gambit

#endif // POLYPARTIAL_H
