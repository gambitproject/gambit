//#
//# FILE: gnarray.h -- Implementation of an N-dimensional array
//#
//# $Id$
//#

#ifndef GNARRAY_H
#define GNARRAY_H

#ifdef __GNUG__
#define INLINE inline
#elif defined __BORLANDC__
#define INLINE
#else
#error Unsupported compiler type.
#endif   // __GNUG__, __BORLANDC__

#include <assert.h>
#include "gambitio.h"
#include "gvector.h"
#include "gtuple.h"

//
// Basic n-dimensional array
//
template <class T> class gNArray   {
  protected:
    long storage_size;
    T *storage;
    gVector<int> dim;

    void DumpFrom(gOutput &, int, gVector<int> &) const;
    void ReadFrom(gInput &, const gVector<int> &, gVector<int> &, int);

  public:
    gNArray(void) : storage_size(0), storage(0)    { }
    gNArray(const gVector<int> &d);
    gNArray(const gNArray<T>& a);
    ~gNArray();

    gNArray<T> &operator=(const gNArray<T> &);

    T operator[](const gVector<int> &) const;
    T &operator[](const gVector<int> &);

    T operator[](long l) const
      { assert(l >= 0 && l < storage_size);
	return storage[l];
      }

    T &operator[](long l)
      { assert(l >= 0 && l < storage_size);
	return storage[l];
      }

    const gVector<int> &Dimensionality(void) const   { return dim; }

    void Input(gInput &, const gVector<int> &, int);
    void Output(gOutput &) const;
};

template <class T> INLINE gNArray<T>::gNArray(const gVector<int> &d)
  : dim(d)
{
  if (dim.Length() <= 0)   {
    storage = 0;
    storage_size = 0;
  }
  else   {
    assert(dim.First() == 1);
    storage_size = 1;
    
    for (int i = 1; i <= dim.Length(); i++)   {
      assert(dim[i] >= 1);
      storage_size *= dim[i];
    }

    storage = new T[storage_size];
    for (i = 0; i < storage_size; storage[i++] = 0);
  }
}

template <class T> INLINE gNArray<T>::gNArray(const gNArray<T> &a)
  : dim(a.dim), storage_size(a.storage_size)
{
  storage = (storage_size > 0) ? new T[storage_size] : 0;
  for (int i = 0; i < storage_size; i++)
    storage[i] = a.storage[i];
}

template <class T> inline gNArray<T>::~gNArray()
{
  if (storage)    delete [] storage;
}

template <class T> INLINE
gNArray<T> &gNArray<T>::operator=(const gNArray<T> &a)
{
  if (this != &a)   {
    if (storage)    delete [] storage;
    dim = a.dim;
    storage_size = a.storage_size;
    storage = (storage_size > 0) ? new T[storage_size] : 0;
    for (int i = 0; i < storage_size; i++)
      storage[i] = a.storage[i];
  }
  return *this;
}

template <class T> INLINE T gNArray<T>::operator[](const gVector<int> &v) const
{
  assert(dim.Length() > 0 && dim.Length() == v.Length());
  
  for (int i = 1, location = 0, offset = 1; i <= dim.Length(); i++)   {
    assert(v[i] > 0 && v[i] <= dim[i]);
    location += (v[i] - 1) * offset;
    offset *= dim[i];
  }

  return storage[location];
}

template <class T> INLINE T &gNArray<T>::operator[](const gVector<int> &v)
{
  assert(dim.Length() > 0 && dim.Length() == v.Length());
  
  for (int i = 1, location = 0, offset = 1; i <= dim.Length(); i++)   {
    assert(v[i] > 0 && v[i] <= dim[i]);
    location += (v[i] - 1) * offset;
    offset *= dim[i];
  }

  return storage[location];
}

template <class T> INLINE void gNArray<T>::Output(gOutput &f) const
{
  if (dim.Length() > 0)   {
    for (int i = dim.Length(); i > 0; f << i-- << ' ');
    f << '\n';

    gVector<int> v(1, dim.Length());
    DumpFrom(f, 1, v);
    f << '\n';
  }
}

template <class T> INLINE
void gNArray<T>::DumpFrom(gOutput &f, int offset, gVector<int> &v) const
{
  for (int i = 1; i <= dim[offset]; i++)   {
    v[offset] = i;
    if (offset == dim.Length())
      f << (*this)[v] << ' ';
    else
      DumpFrom(f, offset + 1, v);
  }
}

template <class T> INLINE
void gNArray<T>::Input(gInput &f, const gVector<int> &norder, int i)
{
  gVector<int> strat(1, i);
  ReadFrom(f, norder, strat, i);
}

template <class T> INLINE
void gNArray<T>::ReadFrom(gInput &f, const gVector<int> &norder,
			  gVector<int> &strat, int i)
{
  for (int j = 1; j <= dim[norder[i]]; j++)   {
    strat[norder[i]] = j;
    if (i > 1)
      ReadFrom(f, norder, strat, i - 1);
    else 
      f >> (*this)[strat];
  }
}



template <class T> class gIndexedNArray : private gNArray<T>   {
  private:
    gTuple<long> *index;
    
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

template <class T> inline gIndexedNArray<T>::gIndexedNArray(void)
{
  index = 0;
}

template <class T> INLINE
gIndexedNArray<T>::gIndexedNArray(const gVector<int> &d)
  : gNArray<T>(d)
{
  long offset = 1;

  index = new gTuple<long>[dim.Length()];
  for (int i = 1; i <= dim.Length(); i++)  {
    for (int j = 0; j < dim[i]; j++)
      index[i - 1] += j * offset;
    offset *= dim[i];
  }
}

template <class T>
INLINE gIndexedNArray<T>::gIndexedNArray(const gIndexedNArray<T> &a)
  : gNArray<T>(a)
{
  index = new gTuple<long>[dim.Length()];
  for (int i = 0; i < dim.Length(); i++)
    index[i] = a.index[i];
}

template <class T> inline gIndexedNArray<T>::~gIndexedNArray()
{
  if (index)   delete [] index;
}

template <class T> INLINE
gIndexedNArray<T> &gIndexedNArray<T>::operator=(const gIndexedNArray<T> &a)
{
  if (this != &a)   {
    gNArray<T>::operator=(a);

    if (index)   delete [] index;
    index = new gTuple<long>[dim.Length()];
    for (int i = 0; i < dim.Length(); i++)
      index[i] = a.index[i];
  }
  return *this;
}

template <class T> INLINE
T gIndexedNArray<T>::operator=(const gVector<int> &v) const
{
  long location = 0;

  assert(dim.Length() > 0 && dim.Length() == v.Length());
  
  for (int i = 1; i <= dim.Length(); i++)   {
    assert(v[i] > 0 && v[i] <= dim[i]);
    location += index[i - 1][v[i]];
  }

  return storage[location];
}

template <class T> INLINE
T &gIndexedNArray<T>::operator=(const gVector<int> &v)
{
  long location = 0;

  assert(dim.Length() > 0 && dim.Length() == v.Length());
  
  for (int i = 1; i <= dim.Length(); i++)   {
    assert(v[i] > 0 && v[i] <= dim[i]);
    location += index[i - 1][v[i]];
  }

  return storage[location];
}

#endif    // GNARRAY_H

