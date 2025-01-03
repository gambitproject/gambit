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

template <class T> class gTree {
public:
  class Node {
  private:
    T data;
    std::list<Node> children;

  public:
    Node(const T &_data) : data(_data) {}
    Node(const Node &) = default;
    ~Node() = default;

    const T &GetData() const { return data; }
    std::list<Node> &GetChildren() { return children; }
    const std::list<Node> &GetChildren() const { return children; }
  };

  explicit gTree(const T &root_value) : root(root_value) {}
  gTree(const gTree<T> &) = default;
  ~gTree() = default;

  gTree<T> &operator=(const gTree<T> &) = delete;

  const Node &RootNode() const { return root; }
  Node &RootNode() { return root; }

private:
  Node root;
};

#endif // GTREE_H
