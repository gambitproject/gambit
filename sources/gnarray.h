//#
//# FILE: gnarray.h -- Implementation of an N-dimensional array
//#
//# @(#)gnarray.h	2.2 01/28/98
//#

#ifndef GNARRAY_H
#define GNARRAY_H

#include <assert.h>
#include "gambitio.h"
#include "gvector.h"

//
// Basic n-dimensional array
//
template <class T> class gNArray   {
  protected:
    long storage_size;
    T *storage;
    gArray<int> dim;

    void DumpFrom(gOutput &, int, gVector<int> &) const;
//    void ReadFrom(gInput &, const gVector<int> &, gVector<int> &, int);

  public:
    gNArray(void);
    gNArray(const gArray<int> &d);
    gNArray(const gNArray<T>& a);
    ~gNArray();

    gNArray<T> &operator=(const gNArray<T> &);

    /* not used for now
    T operator[](const gVector<int> &) const;
    T &operator[](const gVector<int> &);
    */

    T operator[](const gArray<int> &) const;
    T &operator[](const gArray<int> &);

    const T &operator[](long l) const;
    T &operator[](long l);

    const gArray<int> &Dimensionality(void) const;

//    void Input(gInput &, const gVector<int> &, int);
    void Output(gOutput &) const;
};

#ifdef UNUSED
template <class T> class gIndexedNArray : private gNArray<T>   {
  private:
    gArray<long> *index;
    
  public:
    gIndexedNArray(void);
    gIndexedNArray(const gVector<int> &d);
    gIndexedNArray(const gIndexedNArray<T> &);
	 ~gIndexedNArray();

	 gIndexedNArray<T> &operator=(const gIndexedNArray<T> &);

    T operator[](const gVector<int> &) const;
	 T &operator[](const gVector<int> &);
    
    const gVector<int> &Dimensionality(void) const   { return dim; }

    void Input(gInput &f, const gVector<int> &v, int i)
      { gNArray<T>::Input(f, v, i); }
    void Output(gOutput &f) const
      { gNArray<T>::Output(f); }
};
#endif   // UNUSED

#endif    // GNARRAY_H

