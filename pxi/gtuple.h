//#
//# FILE: gtuple.h -- Implementation of a tuple type
//#
//# @(#)gtuple.h	1.4 9/14/94
//#

#ifndef GTUPLE_H
#define GTUPLE_H

#include "gblock.h"

//
// <category lib=glib sect=Containers>
//
// This class represents a tuple of objects.  It differs from a gBlock in two
// ways: it has an arbitrary starting index (whereas a gBlock always
// starts at 1), and it is designed to be of a fixed (or at least
// infrequently changing) size.
//
// Most of the implementation details of this class are handled by the
// gBlock<T> member.
//
template <class T> class gTuple     {
  protected:
    int mindex, maxdex;
    gBlock<T> data;
    
  public:
//
// Construct a tuple of zero length
//
    gTuple(void) : mindex(1), maxdex(0)   { }
//
// Construct a tuple of a given size, consisting of the default value of
// the contained type, with the first index defined to be 1.
//
    gTuple(int size) : mindex(1), maxdex(size), data(size)   { }
//
// Construct a tuple with given first and last indices, consisting of the
// default value of the contained type.
//
    gTuple(int low, int high) 
      : mindex(low), maxdex(high), data(high - low + 1)   { }
//
// Construct a tuple to have the same contents and boundary indices as another
// tuple.
//
    gTuple(const gTuple<T> &t)
      : mindex(t.mindex), maxdex(t.maxdex), data(t.data)   { }
//
// Destruct a tuple, calling the destructor for each contained element.
//
    ~gTuple()   { }

//
// Set a tuple to be identical to another tuple, using copy semantics.
//
    gTuple<T> &operator=(const gTuple<T> &t)
      { mindex = t.mindex;  maxdex = t.maxdex;  data = t.data;  return *this; }

//
// Obtain an element of the tuple by its index.
// <note> Attempting to access an element outside the tuple's boundaries
//        will result in program termination with a failed assertion.
//+grp
    const T &operator[](int n) const   { return data[n - mindex + 1]; }
    T &operator[](int n)               { return data[n - mindex + 1]; }
//-grp

//
// Checks if two gTuples are equal.  It checks if the limits are
// the same, and if the contents of data are the same.
//+grp
    int operator==(const gTuple<T>& V) const
      { return (mindex == V.mindex && maxdex == V.maxdex && data == V.data); }
    int operator!=(const gTuple<T>& V) const
      { return !(*this == V); }
//-grp

//
// Expand the tuple by adding on a number of elements at the end of the
// tuple.
//
    gTuple<T> &Expand(int qty);
//
// Expand the tuple by inserting a number of elements inside of the tuple.
// If the insertion point is lower than the first index of the tuple, the
// elements will be inserted at the beginning of the tuple.  If the insertion
// point is higher than the last index of the tuple, the elements will be
// appended to the end of the tuple.
//
    gTuple<T> &Insert(int where, int qty = 1);
//
// Shrink the tuple by removing a number of elements from the tuple.  If
// the deletion point is before the beginning or beyond the end of the tuple,
// the operation will have no effect.  If the length of the deletion goes
// beyond the end of the tuple, the remainder of the tuple will be deleted.
//
    gTuple<T> &Contract(int where, int qty = 1);

//
// Returns the number of elements in the tuple
//
    int Length(void) const    { return maxdex - mindex + 1; }
//
// Returns the value of the first index in the tuple
//
    int First(void) const     { return mindex; }
//
// Returns the value of the last index in the tuple
//
    int Last(void) const      { return maxdex; }
//
// Sets the value of the first index in the tuple
//
    void SetFirst(int low);
//
// Sets the value of the last index in the tuple
//
    void SetLast(int high);
//
// Return the index at which a given element resides in the tuple, or a number
// out of the range of the tuple if the element is not present
//
    int Find(const T &t) const   { return data.Find(t) + mindex - 1; }
//
// Return true (nonzero) if the element is currently present in the tuple
//
    int Contains(const T &t) const  { return data.Contains(t); }

//
// Output the contents of the tuple (for debugging purposes)
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

template <class T> INLINE gTuple<T> &gTuple<T>::Expand(int qty)
{
  T foo;
  maxdex += qty;
  for (int i = 1; i <= qty; i++)
    data.Append(foo);
  return *this;
}

template <class T> INLINE gTuple<T> &gTuple<T>::Insert(int where, int qty)
{
  T foo;
  if (where <= mindex) {
    for (int i = 1; i <= qty; i++)
      data.Insert(foo, 1);
  }
  else if (where >= maxdex) {
    for (int i = 1; i <= qty; i++)
      data.Insert(foo, Length());
  }
  else {
    for (int i = 1; i <= qty; i++)
      data.Insert(foo, where - mindex + 1);
  }
  maxdex += qty;
  return *this;
}

template <class T> INLINE gTuple<T> &gTuple<T>::Contract(int where, int qty)
{
  if (where < mindex || where > maxdex) return *this;
  if (where + qty - 1 > maxdex) qty = maxdex - where + 1;
  for (int i = 1; i <= qty; i++)
    data.Remove(where - mindex + 1);
  maxdex -= qty;
  return *this;
}

template <class T> INLINE void gTuple<T>::SetFirst(int low)    
{
  int len = Length();
  len += (mindex-low);
  if (low == mindex) return;
  if (low < mindex) {
    Insert(mindex, mindex-low);
  }
  else {
    Contract(mindex, low-mindex);
  }
  mindex = low;
  maxdex = mindex + len - 1;
}

template <class T> INLINE void gTuple<T>::SetLast(int high)
{
  int len = Length();
  len += (high-maxdex);
  if (high == maxdex) return;
  if (high > maxdex) {
    Expand(high-maxdex);
  }
  else {
    Contract(high + 1, maxdex - high);
  }
  maxdex = high;
  mindex = maxdex - len + 1;
}


template <class T> INLINE void gTuple<T>::Dump(gOutput &f) const
{
  f << "{ ";
	for (int i = 1; i <= data.Length(); f << (T)data[i++] << ' ');
  f << '}';
}

//
// Output for gTuple, makes use of the << operator to stream output
//
template <class T> inline gOutput &operator<<(gOutput &f, const gTuple<T> &t)
{
  t.Dump(f);   return f;
}

#endif   //# GTUPLE_H

