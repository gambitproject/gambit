//#
//# FILE: gblock.h -- Implementation of a generic array container class
//#
//# $Id$
//#

#ifndef GBLOCK_H
#define GBLOCK_H

#include <stdlib.h>
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
//
// Constructs the a block of the given length.  All elements of the block
// are constructed according to the default constructor for type T.
//
    gBlock(int len = 0) : length(len), data((len) ? new T[len] : 0)    { }
//
// Constructs a block to have the same contents as another block.  This
// uses copy semantics.
//
    gBlock(const gBlock<T> &);
//
// Deallocates the block of memory, calling the destructors for any
// elements still within the block.
//
    ~gBlock()    { if (data) delete [] data; }

//
// Sets the block to have the same contents as another block.
//
    gBlock<T> &operator=(const gBlock<T> &);

//
// Tests for equality of two blocks.  Blocks are equal if they have the
// same length, and the contents of all components are equal.
//+grp
    int operator==(const gBlock<T> &b) const;
    int operator!=(const gBlock<T> &b) const
      { return !(*this == b); }
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
    gBlock<T> operator+(const T &e) const
      { gBlock<T> result(*this); result.Append(e); return result; }
    gBlock<T>& operator+=(const T &e)
      { Append(e); return *this; }
//-grp

//
// Concatenate two gBlocks.  + puts the result in a separate gBlock,
// while += puts it in the first argument.
//+grp
    gBlock<T> operator+(const gBlock<T>& b) const;
    gBlock<T>& operator+=(const gBlock<T>& b)
      { *this = *this + b; return *this; }
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
    int Contains(const T &t) const    { return Find(t); }
//
// Return the number of elements currently in the block.
//
    int Length(void) const    { return length; }

//
// Print the contents of the block (for debugging purposes)
//
    void Dump(gOutput &) const;

};


#ifdef __GNUG__
#define INLINE inline
#elif defined(__BORLANDC__)
#define INLINE
#else
#error Unsupported compiler type
#endif   //# __GNUG__, __BORLANDC__

template <class T> INLINE gBlock<T>::gBlock(const gBlock<T> &b)
 : length(b.length)
{
  if (length)   {
    data = new T[length];
    for (int i = 0; i < length; i++)
      data[i] = b.data[i];
  }
  else
    data = 0;
}

template <class T> INLINE gBlock<T> &gBlock<T>::operator=(const gBlock<T> &b)
{
  if (this != &b)   {
    if (data)   delete [] data;
  
    length = b.length;
    if (length)   {
      data = new T[length];
      for (int i = 0; i < length; i++)
	data[i] = b.data[i];
    }
    else
      data = 0;
  }
  return *this;
}

template <class T> INLINE int gBlock<T>::operator==(const gBlock<T> &b) const
{
  if (length != b.length) return 0;
  for (int i = 1; i < length; i++) 
    if (data[i] != b.data[i]) return 0;
  return 1;
}

template <class T> inline const T &gBlock<T>::operator[](int n) const
{
  assert(n >= 1 && n <= length);
  return data[--n];
}

template <class T> inline T &gBlock<T>::operator[](int n)
{
  assert(n >= 1 && n <= length);
  return data[--n];
}

template <class T> INLINE 
  gBlock<T> gBlock<T>::operator+(const gBlock<T>& b) const
{
  gBlock<T> result(*this);
  for (int i = 1; i <= b.length; i++)
    result.Append(b[i]);
  return result;
}

template <class T> INLINE int gBlock<T>::InsertAt(const T &t, int n)
{
  T *new_data = new T[++length];

  for (int i = 0; i < n - 1; i++)       new_data[i] = data[i];
  new_data[i++] = t;
  for (; i < length; i++)       new_data[i] = data[i - 1];

  delete [] data;
  data = new_data;

  return n;
}

template <class T> inline int gBlock<T>::Append(const T &t)
{
  return InsertAt(t, length + 1);
}

template <class T> inline int gBlock<T>::Insert(const T &t, int n)
{
  return InsertAt(t, (n < 1) ? 1 : ((n > length + 1) ? length + 1 : n));
}

template <class T> INLINE T gBlock<T>::Remove(int n)
{
  assert(n >= 1 && n <= length);

  T ret(data[--n]);
  T *new_data = (--length) ? new T[length] : 0;

  for (int i = 0; i < n; i++)     new_data[i] = data[i];
  for (; i < length; i++)         new_data[i] = data[i + 1];

  delete [] data;
  data = new_data;

  return ret;
}

template <class T> INLINE int gBlock<T>::Find(const T &t) const
{
  for (int i = 0; i < length && data[i] != t; i++);
  return (i < length) ? ++i : 0;
}

template <class T> INLINE void gBlock<T>::Dump(gOutput &f) const
{
  f << "gBlock " << this << " contents\n";
  for (int i = 0; i < length; i++)
    f << i + 1 << ": " << data[i] << '\n';
}

//
// Uses the Dump function to output the gBlock.  Uses the << operator
// overload to use output streams, gout.
//
template <class T> inline gOutput &operator<<(gOutput &f, const gBlock<T> &b)
{
  b.Dump(f);   return f;
}


#endif    //# GBLOCK_H
