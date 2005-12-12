//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// A basic bounds-checked array type
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

#ifndef GARRAY_H
#define GARRAY_H

#include <stdlib.h>
#include <assert.h>

template <class T> class gbtArray  {
protected:
  int mindex, maxdex;
  T *data;

  //
  // Private helper function that accomplishes the insertion of an object
  //
  int InsertAt(const T &t, int n)
  {
    if (this->mindex > n || n > this->maxdex + 1)  throw gbtIndexException();

    T *new_data = new T[++this->maxdex - this->mindex + 1] - this->mindex;

    int i;
    for (i = this->mindex; i <= n - 1; i++) new_data[i] = this->data[i];
    new_data[i++] = t;
    for (; i <= this->maxdex; i++) new_data[i] = this->data[i - 1];

    if (this->data)   delete [] (this->data + this->mindex);
    this->data = new_data;

    return n;
  }
public:
//
// Constructs a gbtArray of length 'len', starting at '1'
//
  gbtArray(unsigned int len = 0)
    : mindex(1), maxdex(len), data((len) ? new T[len] - 1 : 0) { } 
//
// Constructs a gbtArray starting at lo and ending at hi
//
  gbtArray(int lo, int hi)
    : mindex(lo), maxdex(hi)
  {
    if (maxdex + 1 < mindex)   throw gbtRangeException();
    data = (maxdex >= mindex) ? new T[maxdex -mindex + 1] - mindex : 0;
  }
//
// Copy the contents of another array
//
  gbtArray(const gbtArray<T> &a)
    : mindex(a.mindex), maxdex(a.maxdex),
      data((maxdex >= mindex) ? new T[maxdex - mindex + 1] - mindex : 0)
  {
    for (int i = mindex; i <= maxdex; i++)  data[i] = a.data[i];
  }
//
// Destruct and deallocates gbtArray
//
  virtual ~gbtArray()
  { if (maxdex >= mindex)  delete [] (data + mindex); }

//
// Copy the contents of another array
//
  gbtArray<T> &operator=(const gbtArray<T> &a)
  {
    if (this != &a) {
      // We only reallocate if necessary.  This should be somewhat faster
      // if many objects are of the same length.  Furthermore, it is
      // _essential_ for the correctness of the gbtPVector and gbtDPVector
      // assignment operator, since it assumes the value of data does
      // not change.
      if (!data || (data && (mindex != a.mindex || maxdex != a.maxdex)))  {
	if (data)   delete [] (data + mindex);
	mindex = a.mindex;   maxdex = a.maxdex;
	data = (maxdex >= mindex) ? new T[maxdex - mindex + 1] - mindex : 0;
      }
      
      for (int i = mindex; i <= maxdex; i++) data[i] = a.data[i];
    }

    return *this;
  }

//
// Return the length of the array
//
  int Length(void) const  { return maxdex - mindex + 1; }

//
// Return the first index
//
  int First(void) const { return mindex; } 

//
// Return the last index
//
  int Last(void) const { return maxdex; }

//
// Access the index'th entry in the array
//
  const T &operator[](int index) const 
  {
    if (index < mindex || index > maxdex)  throw gbtIndexException();
    return data[index];
  }

  T &operator[](int index)
  {
    if (index < mindex || index > maxdex)  throw gbtIndexException();
    return data[index];
  }

//
// Append a new element to the array, and return the index at which the
// element can be found.  Note that this index is guaranteed to be the
// last (highest) index in the array.
//
  int Append(const T &t)
  { return InsertAt(t, this->maxdex + 1); }

//
// Insert a new element into the array at a given index.  If the index is
// less than the lowest index, the element is inserted at the beginning;
// if the index is greater than the highest index, the element is appended.
// Returns the index at which the element actually is placed.
//
  int Insert(const T &t, int n)
  {
    return InsertAt(t, (n < this->mindex) ? this->mindex : ((n > this->maxdex + 1) ? this->maxdex + 1 : n));
  }

//
// Remove the element at a given index from the array.  Returns the value
// of the element removed.
//
  T Remove(int n)
  {
    if (n < this->mindex || n > this->maxdex) throw gbtIndexException();

    T ret(this->data[n]);
    T *new_data = (--this->maxdex>=this->mindex) ? new T[this->maxdex-this->mindex+1] - this->mindex : 0;
    
    int i;
    for (i = this->mindex; i < n; i++)      new_data[i] = this->data[i];
    for (; i <= this->maxdex; i++)         new_data[i] = this->data[i + 1];

    delete [] (this->data + this->mindex);
    this->data = new_data;

    return ret;
  }

//
// Return the index at which a given element resides in the array, or
// -1 if the element is not found.
//
  int Find(const T &t) const
  {
    int i;
    for (i = this->mindex; i <= this->maxdex && this->data[i] != t; i++);
    return (i <= this->maxdex) ? i : 0;
  } 
//
// Return true if the element is currently residing in the block.
//
  bool Contains(const T &t) const
  { return Find(t) != 0; }
};

template <class T> bool operator==(const gbtArray<T> &a, const gbtArray<T> &b)
{
  if (a.First() != b.First() || a.Last() != b.Last())   return false;
  for (int i = a.First(); i <= a.Last(); i++)
    if (a[i] != b[i])   return false;
  return true;
}

template <class T> bool operator!=(const gbtArray<T> &a, const gbtArray<T> &b)
{
  return !(a == b);
}

#endif	// GARRAY_H
