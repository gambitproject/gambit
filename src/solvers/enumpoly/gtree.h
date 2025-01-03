//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/gtree.h
// A generic tree container class
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

#ifndef GTREE_H
#define GTREE_H

#include "gambit.h"

template <class T> class gTree;

template <class T> class gTreeNode {
  friend class gTree<T>;

private:
  T data;
  std::list<gTreeNode<T>> children;

public:
  gTreeNode(const T &_data) : data(_data) {}
  gTreeNode(const gTreeNode<T> &) = default;
  ~gTreeNode() = default;

  const T &GetData() const { return data; }
  std::list<gTreeNode<T>> &GetChildren() { return children; }
  const std::list<gTreeNode<T>> &GetChildren() const { return children; }
};

template <class T> class gTree {

public:
  explicit gTree(const T &root_value) : root(gTreeNode<T>(root_value)) {}
  gTree(const gTree<T> &) = default;
  ~gTree() = default;

  gTree<T> &operator=(const gTree<T> &) = delete;

  const gTreeNode<T> &RootNode() const { return root; }
  gTreeNode<T> &RootNode() { return root; }

private:
  gTreeNode<T> root;
};

#endif // GTREE_H
