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
#include "gtree.h"
#include "rectangle.h"
#include "gpoly.h"
#include "gpolylst.h"

using namespace Gambit;

template <class T> class TreeOfPartials {
private:
  gTree<gPoly<T>> PartialTree;

  void TreeOfPartialsRECURSIVE(gTree<gPoly<T>> &, gTreeNode<gPoly<T>> *) const;

  T MaximalNonconstantContributionRECURSIVE(const gTreeNode<gPoly<T>> *, const Vector<T> &,
                                            const Vector<T> &, Vector<int> &) const;

public:
  explicit TreeOfPartials(const gPoly<T> &given) : PartialTree(given)
  {
    TreeOfPartialsRECURSIVE(PartialTree, PartialTree.RootNode());
  }
  TreeOfPartials(const TreeOfPartials<T> &) = default;
  ~TreeOfPartials() = default;

  bool operator==(const TreeOfPartials<T> &rhs) const { return PartialTree == rhs.PartialTree; }
  bool operator!=(const TreeOfPartials<T> &rhs) const { return PartialTree != rhs.PartialTree; }
  int Dmnsn() const { return RootNode()->GetData().Dmnsn(); }

  T MaximalNonconstantContribution(const Vector<T> &, const Vector<T> &) const;

  gTreeNode<gPoly<T>> *RootNode() const { return PartialTree.RootNode(); }
  const gPoly<T> &RootPoly() const { return RootNode()->GetData(); }
  T ValueOfRootPoly(const Vector<T> &point) const { return RootPoly().Evaluate(point); }
  T ValueOfPartialOfRootPoly(int, const Vector<T> &) const;
  bool PolyHasNoRootsIn(const Rectangle<T> &) const;
  bool MultiaffinePolyHasNoRootsIn(const Rectangle<T> &) const;
  bool PolyEverywhereNegativeIn(const Rectangle<T> &) const;
  bool MultiaffinePolyEverywhereNegativeIn(const Rectangle<T> &) const;
};

template <class T> class ListOfPartialTrees {
private:
  List<TreeOfPartials<T>> PartialTreeList;

public:
  explicit ListOfPartialTrees(const List<gPoly<T>> &given)
  {
    for (const auto &poly : given) {
      PartialTreeList.push_back(TreeOfPartials<T>(poly));
    }
  }
  explicit ListOfPartialTrees(const gPolyList<T> &given)
  {
    for (int i = 1; i <= given.Length(); i++) {
      PartialTreeList.push_back(TreeOfPartials<T>(given[i]));
    }
  }
  ListOfPartialTrees(const ListOfPartialTrees<T> &) = default;
  ~ListOfPartialTrees() = default;
  ListOfPartialTrees<T> &operator=(const ListOfPartialTrees<T> &) = delete;

  bool operator==(const ListOfPartialTrees &other) const
  {
    return PartialTreeList == other.PartialTreeList;
  }
  bool operator!=(const ListOfPartialTrees &other) const
  {
    return PartialTreeList != other.PartialTreeList;
  }

  const TreeOfPartials<T> &operator[](int i) const { return PartialTreeList[i]; }

  int Length() const { return PartialTreeList.size(); }
  int Dmnsn() const { return PartialTreeList.front().Dmnsn(); }
  Matrix<T> DerivativeMatrix(const Vector<T> &, int) const;
  SquareMatrix<T> SquareDerivativeMatrix(const Vector<T> &) const;
  Vector<T> ValuesOfRootPolys(const Vector<T> &, int) const;
};

#endif // GPARTLTR_H
