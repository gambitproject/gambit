// 
// FILE: gslist.h -- Generic list container for sorting and filtering
//
// $Id$
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
  gSortList<T> &list;
  virtual bool LessThan(const T &a, const T &b) const = 0;

public:
  gListSorter(gSortList<T> &list);
  virtual ~gListSorter();
  void Sort(void);
};

template <class T> class gListFilter  {
protected:
  gSortList<T> &list;
  virtual bool Passes(const T &a) const = 0;

public:
  gListFilter(gSortList<T> &list);
  virtual ~gListFilter();
  void Filter(void);
};

#endif
