//
// FILE: gvector.h -- Declaration of Vector data type
//
// $Id$
//

#ifndef GVECTOR_H
#define GVECTOR_H

#include "basic.h"
#include <assert.h>
template <class T> class gVector  {
  private:
    T *storage;
    int mindex, maxdex;

		int DimCheck(const gVector<T>& v) const
			{ return (mindex == v.mindex && maxdex == v.maxdex); }

  public:
	// CONSTRUCTORS
    gVector(void)    { storage = 0;  mindex = 1;  maxdex = 0; }
    gVector(int from, int to);
		gVector(const gVector<T>& V);

	// OPERATOR OVERLOADING
    T& operator[](int index);
    T operator[](int index) const;

		gVector<T> operator+(const gVector<T>& V) const;
    gVector<T> operator+(T c) const;
		gVector<T> operator-(const gVector<T>& V) const;
    gVector<T> operator*(T c) const;
		T operator*(const gVector<T>& V) const;
    gVector<T> operator/(T c) const;
		gVector<T> operator/(const gVector<T>& V) const;

		int operator==(const gVector<T>& V) const;
    int operator==(T c) const;
		int operator!=(const gVector<T>& V) const;
    int operator!=(T c) const;

		gVector<T>& operator=(const gVector<T>& V);
		gVector<T>& operator=(T c);

        // DATA ACCESS
		uint First(void) const  { return mindex;}
		uint Last(void) const   { return maxdex;}
		uint Length(void) const { return maxdex - mindex + 1; }
		gVector<T> Stretch(int i) const;
    gVector<T> Sqwzz(int i) const;

        // DESTRUCTOR
    ~gVector()    { if (storage)  delete [] storage; }
};

#ifdef __GNUC__
#define INLINE inline
#elif defined(__BORLANDC__)
#define INLINE
#else
#error Unsupported compiler type.
#endif   // __GNUC__, __BORLANDC__

template <class T> INLINE gVector<T>::gVector(int from, int to)
{
  mindex = from;
  maxdex = to;

  if (maxdex >= mindex)   {
    storage = new T[maxdex - mindex + 1];

    for (uint i = 0; i <= maxdex - mindex; i++)   storage[i] = 0;
  }
  else
    storage = 0;
}

template <class T> INLINE gVector<T>::gVector(const gVector<T>& V)
{
  mindex = V.mindex;
  maxdex = V.maxdex;

  if (maxdex >= mindex)   {
    storage = new T[maxdex - mindex + 1];

    for (uint i = 0; i <= maxdex - mindex; i++)
      storage[i] = V.storage[i];
  }
  else
    storage = 0;
}

template <class T> INLINE T gVector<T>::operator[](int index) const
{
	assert(index >= mindex && index <= maxdex);
  return storage[index - mindex];
}

template <class T> INLINE T& gVector<T>::operator[](int index)
{
	assert(index >= mindex && index <= maxdex);
  return storage[index - mindex];
}

template <class T>
INLINE gVector<T> gVector<T>::operator+(const gVector<T>& V) const
{
  DimCheck(V);
  gVector<T> result(mindex, maxdex);

  for (uint i = 0; i < Length(); i++)
    result.storage[i] = storage[i] + V.storage[i];
  return result;
}

template <class T>
INLINE gVector<T> gVector<T>::operator+(T c) const
{
  gVector<T> result(mindex, maxdex);

  for (uint i = 0; i < Length(); i++)
    result.storage[i] = storage[i] + c;
  return result;
}

template <class T>
INLINE gVector<T> gVector<T>::operator-(const gVector<T>& V) const
{
  DimCheck(V);
  gVector<T> result(mindex, maxdex);

  for (uint i = 0; i < Length(); i++)
    result.storage[i] = storage[i] - V.storage[i];
  return result;
}

template <class T> INLINE gVector<T> gVector<T>::operator*(T c) const
{
  gVector<T> result(mindex, maxdex);

  for (uint i = 0; i < Length(); i++)
    result.storage[i] = storage[i] * c;
  return result;
}

template <class T> INLINE T gVector<T>::operator*(const gVector<T>& V) const
{
  T result = V[1];
  result = 0;

  for (uint i = 1; i <= Length(); i++)
//    result += storage[i] * V.storage[i];
    result = result +  (*this)[i] * V[i];

  return result;
}

template <class T> INLINE gVector<T> gVector<T>::operator/(T c) const
{
  gVector result(mindex, maxdex);

  assert(c != 0);
  for (uint i = 0; i < Length(); i++)
    result.storage[i] = storage[i] / c;

  return result;
}

template <class T> INLINE gVector<T> gVector<T>::operator/(const gVector<T>& V) const
{
  DimCheck(V);
  gVector result(mindex, maxdex);

  for (uint i = 0; i < Length(); i++)  {
    assert(V[i+1] != 0);
    result.storage[i] = storage[i] / V[i+1];
  }

  return result;
}

template <class T> INLINE int gVector<T>::operator==(const gVector<T>& V) const
{
  int i = 0;
  DimCheck(V);

	while ((i < Length()) && (storage[i] == V.storage[i]))  i++;
  return (i == Length());
}

template <class T> INLINE int gVector<T>::operator==(T c) const
{
  int i = 0;
	while ((i < Length()) && (storage[i] == c))  i++;
  return (i == Length());
}

template <class T> INLINE int gVector<T>::operator!=(const gVector<T>& V) const
{
  return !(*this == V);
}

template <class T> INLINE int gVector<T>::operator!=(T c) const
{
  return !(*this == c);
}


template <class T>
INLINE gVector<T>& gVector<T>::operator=(const gVector<T>& V)
{
	if (this != &V)   {    // beware of v = v
    if (storage)    delete [] storage;
    maxdex = V.maxdex;
    mindex = V.mindex;
    if (maxdex >= mindex)  
      storage = new T[maxdex - mindex + 1];
   
    for (uint i = 0; i < Length(); i++)
      storage[i] = V.storage[i];
  }

  return *this;
}

template <class T>
INLINE gVector<T>& gVector<T>::operator=(T c)
{
  if (storage)    delete [] storage;
  if (maxdex >= mindex)  
    storage = new T[maxdex - mindex + 1];
   
  for (uint i = 0; i < Length(); i++)
    storage[i] = c;

  return *this;
}

template <class T> gVector<T> gVector<T>::Stretch(int i) const
{
  assert(i > 0);

  if (maxdex >= mindex)   {
    gVector result(mindex, maxdex + i);

    for (int j = 0; j < Length(); j++)  result.storage[j] = storage[j];
    return result;
  }
  else
    return gVector<T>(mindex, maxdex + i);
}

template <class T> gVector<T> gVector<T>::Sqwzz(int i) const
{
	assert(i >= mindex && i <= maxdex);

  gVector<T> result(mindex, maxdex - 1);
  for (int j = 0; j < i - mindex; j++)   result.storage[j] = storage[j];
  for (j = i - mindex; j < result.Length(); j++)
    result.storage[j] = storage[j + 1];
  return result;
}

#endif   // GVECTOR_H

