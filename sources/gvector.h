//#
//# FILE: gvector1.h -- Implementation of vector classes
//#
//# @(#)gvector.h	1.6 9/12/94
//#

#ifndef GVECTOR_H
#define GVECTOR_H

#include <assert.h>
#include "basic.h"
#include "gtuple.h"
#include "gambitio.h"

#ifdef __GNUC__
#define INLINE inline
#elif defined(__BORLANDC__)
#define INLINE
#else
#error Unsupported compiler type.
#endif   // __GNUC__, __BORLANDC__

//
// <category lib=glib sect=Math>
//
// In order to use type T, the following operators must be defined:
//   +, -(binary), *(binary), /, ==, =
//
template <class T> class gVector {
private:
protected:
  int min, max;
  T *data;

  // check whether index is within vector boundaries
  int Check(int index) const {
    return( min<=index && index<=max );
  }

  // check vector for identical boundaries
  virtual int Check(const gVector<T> &v) const {
    return( v.min==min && v.max==max );
  }

  T* Allocate(void) {
    T* p = new T[max-min+1];
    assert( p != NULL );
    return p-min;
  }

  void Delete(T* p) {
    delete[] (p+min);
  }

  void AllocateData(void) {
    data = Allocate();
  }

  void DeleteData() {
    Delete(data);
  }

  void CopyData( gVector<T> &V ) {
    min=V.min; max=V.max;
    AllocateData();
    for(int i=min; i<=max; i++)
      (*this)[i]=V[i];
  }

public:
	//# CONSTRUCTORS
// Create a zero-length vector
  gVector(void) {
    min=1; max=0;
    data= NULL;
  }
// Create a vector of length len, starting at 1
  gVector(int len) {
    assert( len >= 0 );
    min=1; max=len;
    AllocateData();
  }
// Create a vector indexed from low to high
  gVector(int low, int high) {
    assert( high >= low-1 );
    min=low; max=high;
    AllocateData();
  }
// Copy constructor
  gVector(const gVector<T>& V) {
    CopyData(V);
  }
// Destructor
  virtual ~gVector() {
    DeleteData();
  }

	//# OPERATORS

// access a vector element
  T& operator[] (int n) {
    assert(Check(n));
    return data[n];
  }
// was:  const T& operator[] (int n) const
  T operator[] (int n) const {
    assert(Check(n));
    return data[n];
  }

// = operators  
  gVector<T>& operator=(const gVector<T>& V) {
    DeleteData();
    CopyData(V);
  }
// Assigns the value c to all components of the vector,
  gVector<T>& operator=(T c);


  gVector<T> operator+(const gVector<T>& V) const;
  gVector<T>& operator+=(const gVector<T>& V);

  gVector<T> operator-(void);
  gVector<T> operator-(const gVector<T>& V) const;
  gVector<T>& operator-=(const gVector<T>& V);

  gVector<T> operator*(T c) const;
  gVector<T>& operator*=(T c);
  T operator*(const gVector<T>& V) const;

  gVector<T> operator/(T c) const;
// Term-by-term division.  Not a standard math vector function, but useful in places.
    gVector<T> operator/(const gVector<T>& V) const;
// Term-by-term division, as above
  gVector<T>& operator/=(const gVector<T>& V);

  int operator==(const gVector<T>& V) const;
  int operator!=(const gVector<T>& V) const
    { return !((*this)==V); }

// Tests if all components of the vector are equal to a constant c
  int operator==(T c) const;
  int operator!=(T c) const
    { return !((*this)==c); }

// Manipulation functions
// NOTE: these have been deleted, as nothing really uses them
//       and they're really lame anyway.
//       (several things use gTuple::Find , but they can use gtuple.h)

// parameter access functions
  int First(void) const { return min; }
  int Last(void) const { return max; }
  int Length(void) const { return max-min+1; }

  void Dump(gOutput &) const;
};


// method implementations follow:


// stream output

template <class T> gOutput &
operator<<(gOutput &to, const gVector<T> &V)
{
  V.Dump(to); return to;
}

template <class T> void
gVector<T>::Dump(gOutput &to) const
{
//to<<"vector dump: "<<Length()<<" elements: "<<First()<<".."<<Last()<<"\n";
  to<<"{ ";
  for(int i=min; i<=max; i++)
    to<<(*this)[i]<<" ";
  to<<"}\n";
}


// operator implementations

template<class T> gVector<T>&
gVector<T>::operator=(T c)
{
  for(int i=min; i<=max; i++)
    (*this)[i]= c;
  return (*this);
}

// arithmetic operators
template <class T> gVector<T>
gVector<T>::operator+(const gVector<T>& V) const
{
  assert( Check(V) );
  gVector<T> tmp(min,max);
  for(int i=min; i<=max; i++)
    tmp[i]= (*this)[i] + V[i];
  return tmp;
}

template <class T> gVector<T>
gVector<T>::operator-(const gVector<T>& V) const
{
  assert( Check(V) );
  gVector<T> tmp(min,max);
  for(int i=min; i<=max; i++)
    tmp[i]= (*this)[i] - V[i];
  return tmp;
}

template <class T> gVector<T>&
gVector<T>::operator+=(const gVector<T>& V)
{
  assert( Check(V) );
  for(int i=min; i<=max; i++)
    (*this)[i] += V[i];
  return (*this);
}

template <class T> gVector<T>&
gVector<T>::operator-=(const gVector<T>& V)
{
  assert( Check(V) );
  for(int i=min; i<=max; i++)
    (*this)[i] -= V[i];
  return (*this);
}

template <class T> gVector<T>
gVector<T>::operator-(void)
{
  gVector<T> tmp(min,max);
  for(int i=min; i<=max; i++)
    tmp[i]= -(*this)[i];
  return tmp;
}

template <class T> gVector<T>
gVector<T>::operator*(T c) const
{
  gVector<T> tmp(min,max);
  for(int i=min; i<=max; i++)
    tmp[i]= (*this)[i]*c;
  return tmp;
}

template <class T> gVector<T>&
gVector<T>::operator*=(T c)
{
  for(int i=min; i<=max; i++)
    (*this)[i] *= c;
  return (*this);
}

template <class T> T
gVector<T>::operator*(const gVector<T>& V) const
{
  assert( Check(V) );
  T sum= (T)0;
  for(int i=min; i<=max; i++)
    sum += (*this)[i] * V[i];
  return sum;
}


template <class T> gVector<T>
gVector<T>::operator/(T c) const
{
  gVector<T> tmp(min,max);
  for(int i=min; i<=max; i++)
    tmp[i]= (*this)[i]/c;
  return tmp;
}


template <class T> gVector<T>
gVector<T>::operator/(const gVector<T>& V) const
{
  assert( Check(V) );
  gVector<T> tmp(min,max);
  for(int i=min; i<=max; i++)
    tmp[i]= (*this)[i] / V[i];
  return tmp;
}

template <class T> gVector<T>&
gVector<T>::operator/=(const gVector<T>& V)
{
  assert( Check(V) );
  for(int i=min; i<=max; i++)
    (*this)[i] /= V[i];
  return (*this);
}

template <class T> int
gVector<T>::operator==(const gVector<T>& V) const
{
  assert( Check(V) );
  for(int i=min; i<=max; i++)
    if( (*this)[i] != V[i] )
      return 0;
  return 1;
}

template <class T> int
gVector<T>::operator==(T c) const
{
  for(int i=min; i<=max; i++)
    if( (*this)[i] != c )
      return 0;
  return 1;
}


#endif   //# GVECTOR_H


