//
// FILE: gnarray.h -- Implementation of an N-dimensional array
//
// $Id$
//

#ifndef GNARRAY_H
#define GNARRAY_H

#include <assert.h>
#include "gstream.h"
#include "garray.h"

//
// Basic n-dimensional array
//
template <class T> class gNArray   {
  protected:
    long storage_size;
    T *storage;
    gArray<int> dim;

    void DumpFrom(gOutput &, int, gArray<int> &) const;

  public:
#ifdef USE_EXCEPTIONS
    class BadDim : public gException   {
    public:
      virtual ~BadDim()  { }
      gText Description(void) const;
    };

    class BadIndex : public gException   {
    public:
      virtual ~BadIndex()   { }
      gText Description(void) const;
    };
#endif   // USE_EXCEPTIONS

    gNArray(const gArray<int> &d);
    gNArray(const gNArray<T>& a);
    ~gNArray();

    gNArray<T> &operator=(const gNArray<T> &);

    T operator[](const gArray<int> &) const;
    T &operator[](const gArray<int> &);
/*
    const T &operator[](long l) const;
    T &operator[](long l);*/

    const gArray<int> &Dimensions(void) const;

    void Output(gOutput &) const;
};

#endif   // GNARRAY_H
