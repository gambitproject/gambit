// File: gslist.h -- a generic list container with a swap operation to
// allow for sorting.  The vis_length member allows for filtering the
// list by making 'visible' only the first few elements
// Also defines two classes to operate on a sortable list: gListSorter and a
// gListFilter.  These abstract classes must be derived from and overloaded
// the appropriate functions to implement filtering and sorting of the list
// $Id$

#ifndef GSLIST_H
#define GSLIST_H
#include "glist.h"

template <class T> class gListSorter;
template <class T> class gListFilter;
template <class T> class gSortList : public gList<T>
{
friend class gListSorter<T>;
friend class gListFilter<T>;
private:
	int vis_length;	// the number of elements that satisfy the filter requirements
public:
//
// Constructs the empty list
//
		gSortList(void);
//
// Constructs a block to have the same contents as another block.  This
// uses copy semantics.
//
		gSortList(const gList<T> &);
//
// Swaps two elements
//
		void Swap(int a,int b);
//
// Returns the 'visible length'
//
		int VisLength(void) const;
//
// Remove the element at a given index from the block.  Returns the value
// of the element removed.
//
    virtual T Remove(int);


};

typedef enum {LessThan,Equal,GreaterThan} CompareResult;
template <class T> class gListSorter
{
private:
	gSortList<T> &list;
protected:
	virtual CompareResult Compare(const T &a,const T &b) const = 0;
public:
	gListSorter(gSortList<T> &list);
	void Sort(void);
};

template <class T> class gListFilter
{
private:
	gSortList<T> &list;
protected:
	virtual bool Passes(const T &a) const =0;
public:
	gListFilter(gSortList<T> &list);
	void Filter(void);
};

#endif
