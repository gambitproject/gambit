//#
//# FILE: garray.h -- A basic bounds-checked array type
//#
//# $Id$
//#

#ifndef GARRAY_H
#define GARRAY_H

#include <stdlib.h>
#include <assert.h>
#include "gambitio.h"

template <class T> class gArray  {
  friend bool operator==(const gArray<T> &, const gArray<T> &);
//#ifdef __BORLANDC__  // need this for BC++ compile, but not for g++
//  friend bool operator!=(const gArray<T> &, const gArray<T> &);
//#endif
  protected:
    int mindex, maxdex;
    T *data;

  public:
//
// Constructs a gArray of length 'len', starting at '1'
//
    gArray(int len = 0);
//
// Constructs a gArray starting at lo and ending at hi
//
    gArray(int lo, int hi);
//
// duplicate the input gArray<T> constant referrence
//
    gArray(const gArray<T> &);
//
// Destruct and deallocates gArray
//
    virtual ~gArray();
//
// Copies data from input gArray
//
    gArray<T> &operator=(const gArray<T> &);

//
// return length in the invoking gArray<T>
//
    int Length(void) const;

//
// return first index
//
    int First(void) const;

//
// return last index
//
    int Last(void) const;

//
// Check bounds of index. Exit program if out of bounds
//
    const T &operator[](int index) const;
    T &operator[](int index);
//
// Output data of the array
//
    virtual void Dump(gOutput &) const;
};
template <class T> bool operator==(const gArray<T> &, const gArray<T> &);
template <class T> bool operator!=(const gArray<T> &, const gArray<T> &);

template <class T> gOutput &operator<<(gOutput &, const gArray<T> &);

// ***********************
// Commented out for now
// ***********************
/*
template <class T> class gArrayPtr {
  
protected:
  typedef T* P;                                                                
  gArray<void *> *point;
public:
  
  // Construct a gArray of T Pointers of lenght 'len', starting at '1'
  gArrayPtr(int len = 0) { point = new gArray<void *>(len); }


  // Construct a gArray of T Pointers starting at lo and ending at hi
  gArrayPtr(int lo, int hi) { point = new gArray<void *>(lo, hi); }


  // duplicate the input gArray of T pointers constant referrence
  gArrayPtr(const gArrayPtr<T> &a) { point = new gArray<void *> (*(a.point)); }


  // Destruct and deallocates the gArray of T pointers
  virtual ~gArrayPtr() { delete point; }


  // Copies data from input gArray of T Pointers
  gArrayPtr<T> &operator=(const gArrayPtr<T> &p2) 
    { *point = *(p2.point); return (*this); }


  // return length in the invoking gArray of T pointers
  int Length(void) const { return (*point).Length(); }


  // return first index of array of T pointers
  int First(void) const { return (*point).First(); }


  // return last index
  int Last(void) const { return (*point).Last(); }


  // return a T pointer at location 'index' in the array.
  const P &operator[](int index) const { return ((P &) (*point)[index]); }
  P &operator[](int index) { return ((P &) (*point)[index]); }


  // output data of the array (debugging)
  virtual void Dump(gOutput &g) const { (*point).Dump(g); }

};
*/


#endif	//# GARRAY_H
