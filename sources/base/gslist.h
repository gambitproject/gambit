//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Generic list container for sorting and filtering
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#ifndef GSLIST_H
#define GSLIST_H

#include "glist.h"

template <class T> class gListSorter;
template <class T> class gListFilter;

template <class T> class gSortList : public gList<T>  {
friend class gListSorter<T>;
friend class gListFilter<T>;
private:
  // the number of elements that satisfy the filter requirements
  int vis_length;	

public:
  // CONSTRUCTORS, DESTRUCTOR, AND CONSTRUCTIVE OPERATORS
  gSortList(void);
  gSortList(const gList<T> &);
  virtual ~gSortList();

  // DATA MANIPULATION
  void Swap(int a, int b);
  virtual T Remove(int);
  virtual void Flush(void);

  // DATA ACCESS
  int VisibleLength(void) const;
};

template <class T> class gListSorter  {
protected:
  virtual bool LessThan(const T &a, const T &b) const = 0;

public:
  gListSorter(void);
  virtual ~gListSorter();

  void Sort(gSortList<T> &);
};

template <class T> class gListFilter  {
protected:
  virtual bool Passes(const T &a) const = 0;

public:
  gListFilter(void);
  virtual ~gListFilter();

  void Filter(gSortList<T> &);
};

#endif
