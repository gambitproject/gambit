//#
//# FILE: gvector.h -- Implementation of vector classes
//#
//# $Id$
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

template <class T> class gMatrix;

template <class T> class gVector {
friend class gMatrix<T>;
private:
protected:
  int min, max;
  T *data;

  // check whether index is within vector boundaries
  int Check(int index) const;
  // check vector for identical boundaries
  int Check(const gVector<T> &v) const;

  T* Allocate(void);
  void Delete(T* p);
  void AllocateData(void);
  void DeleteData(void);
  void CopyData( const gVector<T> &V );

public:
	//# CONSTRUCTORS
// Create a zero-length vector
  gVector(void);
// Create a vector of length len, starting at 1
  gVector(int len) ;
// Create a vector indexed from low to high
  gVector(int low, int high);
// Copy constructor
  gVector(const gVector<T>& V);
// Destructor
  virtual ~gVector();

	//# OPERATORS

// access a vector element
  T& operator[] (int n);
// was:  const T& operator[] (int n) const
  T operator[] (int n) const;

// = operators  
  gVector<T>& operator=(const gVector<T>& V);

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
  int operator!=(const gVector<T>& V) const;

// Tests if all components of the vector are equal to a constant c
  int operator==(T c) const;
  int operator!=(T c) const;

// Manipulation functions
// NOTE: these have been deleted, as nothing really uses them
//       and they're really lame anyway.
//       (several things use gTuple::Find , but they can use gtuple.h)

// parameter access functions
  int First(void) const;
  int Last(void) const;
  int Length(void) const;

  void Dump(gOutput &) const;
};


// method implementations follow:

template <class T> inline int gVector<T>::Check(int index) const {
    return( min<=index && index<=max );
  }

template <class T> inline T* gVector<T>::Allocate(void) {
    T* p = new T[max-min+1];
    assert( p != NULL );
    return p-min;
  }

template <class T> inline void gVector<T>::Delete(T* p) {
    delete[] (p+min);
  }

template <class T> inline void gVector<T>::AllocateData(void) {
    data = Allocate();
  }

template <class T> inline void gVector<T>::DeleteData(void) {
    Delete(data);
  }

template <class T> inline void gVector<T>::CopyData( const gVector<T> &V ) {
    min=V.min; max=V.max;
    AllocateData();
    for(int i=min; i<=max; i++)
      (*this)[i]=V[i];
  }

template <class T> inline gVector<T>::gVector(void) {
    min=1; max=0;
    data= NULL;
  }

template <class T> inline gVector<T>::gVector(int len)
{
    assert( len >= 0 );
    min=1; max=len;
    AllocateData();
  }

template <class T> inline gVector<T>::gVector(int low, int high) {
    assert( high >= low-1 );
    min=low; max=high;
    AllocateData();
  }

template <class T> inline gVector<T>::gVector(const gVector<T>& V) {
    CopyData(V);
  }

template <class T> inline T& gVector<T>::operator[] (int n) {
    assert(Check(n));
    return data[n];
  }

template <class T> inline T gVector<T>::operator[] (int n) const {
    assert(Check(n));
    return data[n];
 }

template <class T> inline gVector<T>& gVector<T>::operator=(const gVector<T>& V) {
    assert(Check(V));
    DeleteData();
    CopyData(V);
  }

template <class T> inline int gVector<T>::operator!=(const gVector<T> &V) const
{ return !(*this == V); }

template <class T> inline int gVector<T>::operator!=(T c) const
{ return !(*this == c); }

template <class T> inline int gVector<T>::First(void) const { return min; }
template <class T> inline int gVector<T>::Last(void) const { return max; }
template <class T> inline int gVector<T>::Length(void) const { return max-min+1; }

// stream output

template <class T> inline gOutput &
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
  to<<"}";
}


// virtual functions
template <class T> int
gVector<T>::Check(const gVector<T> &v) const
{
  return( v.min==min && v.max==max );
}

template <class T>
gVector<T>::~gVector()
{
  DeleteData();
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


