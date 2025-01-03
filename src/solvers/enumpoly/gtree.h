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
  gTreeNode<T> *parent, *prev, *next, *eldest, *youngest;

public:
  gTreeNode(const T &_data, gTreeNode<T> *_parent, gTreeNode<T> *_prev, gTreeNode<T> *_next,
            gTreeNode<T> *_eldest, gTreeNode<T> *_youngest)
    : data(_data), parent(_parent), prev(_prev), next(_next), eldest(_eldest), youngest(_youngest)
  {
  }
  ~gTreeNode() = default;

  void SetEldest(gTreeNode<T> *neweldest) { eldest = neweldest; }
  void SetYoungest(gTreeNode<T> *newyoungest) { youngest = newyoungest; }

  const T &GetData() const { return data; }
  gTreeNode<T> *GetParent() const { return parent; }
  gTreeNode<T> *GetPrev() const { return prev; }
  gTreeNode<T> *GetNext() const { return next; }
  gTreeNode<T> *GetEldest() const { return eldest; }
  gTreeNode<T> *GetYoungest() const { return youngest; }
};

template <class T> class gTree {
protected:
  gTreeNode<T> *root;

  gTreeNode<T> *RecursiveFind(const T &, gTreeNode<T> *) const;
  void RecursiveCopy(gTreeNode<T> *, const gTreeNode<T> *);
  void RecursiveFlush(const gTreeNode<T> *);
  void Flush();

public:
  gTree() : root(nullptr) {}
  explicit gTree(const T &root_value)
    : root(new gTreeNode<T>(root_value, nullptr, nullptr, nullptr, nullptr, nullptr))
  {
  }
  gTree(const gTree<T> &);
  ~gTree() { Flush(); }

  gTree<T> &operator=(const gTree<T> &);

  void InsertAt(const T &, gTreeNode<T> *);

  Gambit::List<gTreeNode<T> *> Children(const gTreeNode<T> *) const;
  gTreeNode<T> *RootNode() const { return root; }
  gTreeNode<T> *Find(const T &) const;
  bool Contains(const T &t) const;
};

#endif // GTREE_H
