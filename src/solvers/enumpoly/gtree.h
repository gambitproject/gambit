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
  std::list<gTreeNode<T> *> children;

public:
  gTreeNode(const T &_data) : data(_data) {}
  ~gTreeNode() = default;

  const T &GetData() const { return data; }
  const std::list<gTreeNode<T> *> &GetChildren() const { return children; }
};

template <class T> class gTree {
protected:
  gTreeNode<T> *root;

  void RecursiveCopy(gTreeNode<T> *copyn, const gTreeNode<T> *orign)
  {
    const auto &oldchildren = orign->GetChildren();
    for (const auto &child : oldchildren) {
      copyn->children.push_back(new gTreeNode<T>(child->data));
      RecursiveCopy(copyn->children.back(), child);
    }
  }
  void RecursiveFlush(const gTreeNode<T> *n)
  {
    for (const auto &child : n->GetChildren()) {
      RecursiveFlush(child);
    }
    delete n;
  }

public:
  explicit gTree(const T &root_value) : root(new gTreeNode<T>(root_value)) {}
  gTree(const gTree<T> &b) : root(nullptr)
  {
    if (b.root != nullptr) {
      root = new gTreeNode<T>(b.root->data);
      RecursiveCopy(root, b.root);
    }
  }
  ~gTree() { RecursiveFlush(root); }

  gTree<T> &operator=(const gTree<T> &) = delete;

  void InsertAt(const T &t, gTreeNode<T> *n) { n->children.push_back(new gTreeNode<T>(t)); }

  gTreeNode<T> *RootNode() const { return root; }
};

#endif // GTREE_H
