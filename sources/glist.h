//#
//# FILE: glist.h -- A generic array container class
//#
//# $Id$
//#

#ifndef GLIST_H
#define GLIST_H

#include "gambitio.h"

template <class T> class gNode;
template <class T> class gListIter;

//
// <category lib=glib sect=Containers>
//
// This class is designed as a convenient alternative to using a pointer
// to a block of objects.  In addition to error checking on the bounds
// of the block, it also provides members to insert, remove, and append
// new elements to/from the block.  This class provides the same interface
// as the gBlock class (with minor exceptions).  The primary advantage of
// gList over gBlock is in situations that require frequent Append/Remove
// operations.  A gBlock is HIGHLY inefficient in this case.
//
//
template <class T> class gList  {
  friend class gListIter<T>;
  private:
    int length;
    gNode<T> *head, *tail;

//
// Insert the given element at the specified location.  Does not do any
// error checking as it assumes that the members which call it have already
// determined the proper location.  Returns the location at which the new
// element was placed.
//
    int InsertAt(const T &t, int where);

  public:
//
// Constructs the a block of the given length.  All elements of the block
// are constructed according to the default constructor for type T.
//
    gList(int len = 0);
//
// Constructs a block to have the same contents as another block.  This
// uses copy semantics.
//
    gList(const gList<T> &);
//
// Deallocates the block of memory, calling the destructors for any
// elements still within the block.
//
    ~gList();

//
// Sets the block to have the same contents as another block.
//
    gList<T> &operator=(const gList<T> &);

//
// Tests for equality of two blocks.  Blocks are equal if they have the
// same length, and the contents of all components are equal.
//+grp
    int operator==(const gList<T> &b) const;
    int operator!=(const gList<T> &b) const;
//-grp

//
// Return an element from the block by index into the block.
// <note> If an out-of-range index is given, the program will terminate in a
//        failed assertion.
//+grp
    const T &operator[](int) const;
    T &operator[](int);
//-grp

//
// Append an element to a gList.  Operator overloaded for ease of use.
//+grp
    gList<T> operator+(const T &e) const;
    gList<T>& operator+=(const T &e);
//-grp

//
// Concatenate two gLists.  + puts the result in a separate gList,
// while += puts it in the first argument.
//+grp
    gList<T> operator+(const gList<T>& b) const;
    gList<T>& operator+=(const gList<T>& b);
//-grp

    gList<T> &Combine(gList<T> &b);

//
// Append a new element to the list, and return the index at which the
// element can be found.  Note that this index is guaranteed to be the
// last (highest) index in the block.
//
    int Append(const T &);
//
// Insert a new element into the list at a given index.  If the index is
// less than 1, the element is inserted at index 1; if the index is greater
// than the highest index, the element is appended to the end of the block.
// Returns the index at which the element actually is placed.
//
    int Insert(const T &, int);
//
// Remove the element at a given index from the block.  Returns the value
// of the element removed.
//
    T Remove(int);

//
// Return the index at which a given element resides in the block, or zero
// if the element is not found.
//
    int Find(const T &) const;
//
// Return true (nonzero) if the element is currently residing in the block.
//
    int Contains(const T &t) const;
//
// Return the number of elements currently in the block.
//
    int Length(void) const;

//
// Empty the block
//
    void Flush(void);

//
// Print the contents of the block (for debugging purposes)
//
    void Dump(gOutput &) const;

};


//
// Uses the Dump function to output the gList.  Uses the << operator
// overload to use output streams, gout.
//
template <class T> gOutput &operator<<(gOutput &f, const gList<T> &b);

#endif    //# GLIST_H

