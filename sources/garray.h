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
  protected:
    int length;
    T *data;

  public:
//
// Constructs a gArray of length 'len'
//
    gArray(int len = 0);
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
// Check bounds of index. Exit program if out of bounds
//
    const T &operator[](int index) const;
    T &operator[](int index);
//
// Output data of the array
//
    virtual void Dump(gOutput &) const;
};

template <class T> gOutput &operator<<(gOutput &, const gArray<T> &);

#endif	//# GARRAY_H
