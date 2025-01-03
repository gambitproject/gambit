//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/gpartltr.h
// Interface to TreeOfPartials
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

#ifndef GPARTLTR_H
#define GPARTLTR_H

#include "gambit.h"
#include "rectangle.h"
#include "gpoly.h"
#include "gpolylst.h"

using namespace Gambit;

template <class T> class TreeOfPartials {
private:
  struct Node {
  public:
    gPoly<T> data;
    std::list<Node> children;

    Node(const gPoly<T> &p_data) : data(p_data) {}
    Node(const Node &) = default;
    ~Node() = default;
  };

  Node m_treeroot;

  void TreeOfPartialsRECURSIVE(Node &);
  T MaximalNonconstantContributionRECURSIVE(const Node &, const Vector<T> &, const Vector<T> &,
                                            Vector<int> &) const;

public:
  explicit TreeOfPartials(const gPoly<T> &given) : m_treeroot(given)
  {
    TreeOfPartialsRECURSIVE(m_treeroot);
  }
  TreeOfPartials(const TreeOfPartials<T> &) = default;
  ~TreeOfPartials() = default;

  int Dmnsn() const { return m_treeroot.data.Dmnsn(); }

  T MaximalNonconstantContribution(const Vector<T> &, const Vector<T> &) const;

  const gPoly<T> &RootPoly() const { return m_treeroot.data; }
  T ValueOfRootPoly(const Vector<T> &point) const { return m_treeroot.data.Evaluate(point); }
  T ValueOfPartialOfRootPoly(int, const Vector<T> &) const;
  bool PolyHasNoRootsIn(const Rectangle<T> &) const;
  bool MultiaffinePolyHasNoRootsIn(const Rectangle<T> &) const;
  bool PolyEverywhereNegativeIn(const Rectangle<T> &) const;
  bool MultiaffinePolyEverywhereNegativeIn(const Rectangle<T> &) const;
};

template <class T> class ListOfPartialTrees {
private:
  std::list<TreeOfPartials<T>> m_list;

public:
  explicit ListOfPartialTrees(const gPolyList<T> &given)
  {
    for (const auto &p : given) {
      m_list.push_back(TreeOfPartials<T>(p));
    }
  }
  ListOfPartialTrees(const ListOfPartialTrees<T> &) = default;
  ~ListOfPartialTrees() = default;
  ListOfPartialTrees<T> &operator=(const ListOfPartialTrees<T> &) = delete;

  const TreeOfPartials<T> &operator[](const int index) const
  {
    return *std::next(m_list.begin(), index - 1);
  }
  int Dmnsn() const { return m_list.front().Dmnsn(); }
  Matrix<T> DerivativeMatrix(const Vector<T> &, int) const;
  SquareMatrix<T> SquareDerivativeMatrix(const Vector<T> &) const;
  Vector<T> ValuesOfRootPolys(const Vector<T> &, int) const;
};

#endif // GPARTLTR_H
