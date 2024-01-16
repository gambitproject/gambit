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
  // Constructor
  gTreeNode(const T &_data, gTreeNode<T> *_parent, gTreeNode<T> *_prev, gTreeNode<T> *_next,
            gTreeNode<T> *_eldest, gTreeNode<T> *_youngest);

  ~gTreeNode();

  inline void SetData(const T &newdata) { data = newdata; }
  inline void SetParent(gTreeNode<T> *newparent) { parent = newparent; }
  inline void SetPrev(gTreeNode<T> *newprev) { prev = newprev; }
  inline void SetNext(gTreeNode<T> *newnext) { next = newnext; }
  inline void SetEldest(gTreeNode<T> *neweldest) { eldest = neweldest; }
  inline void SetYoungest(gTreeNode<T> *newyoungest) { youngest = newyoungest; }

  inline T GetData() const { return data; }
  inline gTreeNode<T> *GetParent() const { return parent; }
  inline gTreeNode<T> *GetPrev() const { return prev; }
  inline gTreeNode<T> *GetNext() const { return next; }
  inline gTreeNode<T> *GetEldest() const { return eldest; }
  inline gTreeNode<T> *GetYoungest() const { return youngest; }
};

template <class T> class gTree {
protected:
  gTreeNode<T> *root;

  gTreeNode<T> *RecursiveFind(const T &, gTreeNode<T> *) const;
  void RecursiveCopy(gTreeNode<T> *, const gTreeNode<T> *);
  void RecursiveFlush(const gTreeNode<T> *);
  void Flush();

public:
  gTree();
  explicit gTree(const T &);
  gTree(const gTree<T> &);
  virtual ~gTree();

  gTree<T> &operator=(const gTree<T> &);

  bool operator==(const gTree<T> &b) const;
  bool operator!=(const gTree<T> &b) const;

  // Constructive Manipulation
  void InsertAt(const T &, gTreeNode<T> *);

  // Information
  Gambit::List<gTreeNode<T> *> Children(const gTreeNode<T> *) const;
  gTreeNode<T> *RootNode() const;
  gTreeNode<T> *Find(const T &) const;
  bool Contains(const T &t) const;
  bool SubtreesAreIsomorphic(const gTreeNode<T> *, const gTreeNode<T> *) const;
};

#endif // GTREE_H
