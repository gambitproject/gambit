//#
//# FILE: gblock.h -- Implementation of a generic array container class
//#
//# $Id$
//#

#ifndef GBLOCK_H
#define GBLOCK_H

#include <stdlib.h>
#include <assert.h>
#include "gambitio.h"

//
// <category lib=glib sect=Containers>
//
// This class is designed as a convenient alternative to using a pointer
// to a block of objects.  In addition to error checking on the bounds
// of the block, it also provides members to insert, remove, and append
// new elements to/from the block.
//
// The first index into the block is defined to be 1.
//
template <class T> class gBlock    {
  private:
    int length;
    T *data;

//
// Insert the given element at the specified location.  Does not do any
// error checking as it assumes that the members which call it have already
// determined the proper location.  Returns the location at which the new
// element was placed.
//
    int InsertAt(const T &t, int where);

  public:
    gBlock(void);
//
// Constructs the a block of the given length.  All elements of the block
// are constructed according to the default constructor for type T.
//
    gBlock(int len);
//
// Constructs a block to have the same contents as another block.  This
// uses copy semantics.
//
    gBlock(const gBlock<T> &);
//
// Deallocates the block of memory, calling the destructors for any
// elements still within the block.
//
    ~gBlock();

//
// Sets the block to have the same contents as another block.
//
    gBlock<T> &operator=(const gBlock<T> &);

//
// Tests for equality of two blocks.  Blocks are equal if they have the
// same length, and the contents of all components are equal.
//+grp
    int operator==(const gBlock<T> &b) const;
    int operator!=(const gBlock<T> &b) const;
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
// Append an element to a gBlock.  Operator overloaded for ease of use.
//+grp
    gBlock<T> operator+(const T &e) const;
    gBlock<T>& operator+=(const T &e);
//-grp

//
// Concatenate two gBlocks.  + puts the result in a separate gBlock,
// while += puts it in the first argument.
//+grp
    gBlock<T> operator+(const gBlock<T>& b) const;
    gBlock<T>& operator+=(const gBlock<T>& b);
//-grp

//
// Append a new element to the block, and return the index at which the
// element can be found.  Note that this index is guaranteed to be the
// last (highest) index in the block.
//
    int Append(const T &);
//
// Insert a new element into the block at a given index.  If the index is
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

template <class T> gOutput &operator<<(gOutput &, const gBlock<T> &);


#endif    //# GBLOCK_H
