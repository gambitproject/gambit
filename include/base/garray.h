//
// FILE: garray.h -- A basic bounds-checked array type
//
// $Id$
//

#ifndef GARRAY_H
#define GARRAY_H

#include <stdlib.h>
#include <assert.h>
#include "base/gstream.h"

template <class T> class gArray  {
  friend bool operator==(const gArray<T> &, const gArray<T> &);
  protected:
    int mindex, maxdex;
    T *data;

  public:
    class BadIndex : public gException  {
    public:
      virtual ~BadIndex();
      gText Description(void) const;
    };

    class BadRange : public gException  {
    public:
      virtual ~BadRange();
      gText Description(void) const;
    };

//
// Constructs a gArray of length 'len', starting at '1'
//
    gArray(unsigned int len = 0);
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


#endif	//# GARRAY_H
