//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of a generic array container class
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

#ifndef GBLOCK_H
#define GBLOCK_H

#include <stdlib.h>
#include "garray.h"

template <class T> class gBlock : public gArray<T>   {
  private:
//
// Insert the given element at the specified location.  Does not do any
// error checking as it assumes that the members which call it have already
// determined the proper location.  Returns the location at which the new
// element was placed.
//
    int InsertAt(const T &t, int where);

  public:
//
// Constructs a block of the given length.  All elements of the block
// are constructed according to the default constructor for type T.
//
    gBlock(unsigned int len = 0);
//
// Constructs a block starting at lo, and going to hi.
//
    gBlock(int lo, int hi);
//
// Constructs a block to have the same contents as another block.  This
// uses copy semantics.
//
    gBlock(const gBlock<T> &);
//
// Deallocates the block of memory, calling the destructors for any
// elements still within the block.
//
    virtual ~gBlock();

//
// Sets the block to have the same contents as another block.
//
    gBlock<T> &operator=(const gBlock<T> &);

//
// Tests for equality of two blocks.  Blocks are equal if they have the
// same length, and the contents of all components are equal.
//+grp
    bool operator==(const gBlock<T> &b) const;
    bool operator!=(const gBlock<T> &b) const;
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
// Empty the block
//
    void Flush(void);
};

template <class T> gOutput &operator<<(gOutput &, const gBlock<T> &);

#endif    // GBLOCK_H





