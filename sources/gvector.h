//#
//# FILE: gvector.h -- Declaration of Vector data type
//#
//# $Id$
//#

#ifndef GVECTOR_H
#define GVECTOR_H

#include <assert.h>
#include "basic.h"
#include "gtuple.h"
#include "gambitio.h"

//
// <category lib=glib sect=Math>
//
// A gVector is a container similar to gTuple, with the addition
// of arithmetic operations on the class.
//
// In order to use type T, the following operators must be defined:
//   +, -(binary), *(binary), /, ==, =
//
template <class T> class gVector: public gTuple<T> {
  private:
//
// Returns true if the dimensions of the vectors are the same.
// It checks this by matching the first and last elements of the
// vector.
//  
    int DimCheck(const gVector<T>& v) const
      { return (First() == v.First() 
		&& Last() == v.Last()); }

  public:
	//# CONSTRUCTORS
//
// Create a zero-length vector
//
    gVector(void)  { }
//
// Create a vector of length len, starting at 1
//
    gVector(int len): gTuple<T>(len) { }
//
// Create a vector indexed from low to high
//
    gVector(int low, int high): gTuple<T>(low, high) { }
//
// Copy constructor
//
    gVector(const gVector<T>& V): gTuple<T>(V) { }

	//# OPERATOR OVERLOADING (see also gTuple)
//
// Addition of gVectors
//
    gVector<T> operator+(const gVector<T>& V) const;
//
// Add one gVector to another
//
    gVector<T>& operator+=(const gVector<T>& V)
      { *this = *this + V; return *this; }
//
// Unary minus; inversion.  All components have their
// sign reversed.
//
    gVector<T> operator-(void);
//
// Subtraction of gVectors
//
    gVector<T> operator-(const gVector<T>& V) const;
//
// Subtraction of one gVector from another
//
    gVector<T>& operator-=(const gVector<T>& V)
      { *this = *this - V; return *this; }
//
// Multiplication by a constant.  Multiplies all
// components by a constant c.
//
    gVector<T> operator*(T c) const;
//
// Multiplication of vectors.  Creates the dot product;
// multiplies term-by-term and sums the results.
//
    T operator*(const gVector<T>& V) const;
//
// Division by a constant.  Divides all components
// by a constant c.
//
    gVector<T> operator/(T c) const;
//
// Term-by-term division.  Not really a standard math
// vector function, but is useful in places.
//
    gVector<T> operator/(const gVector<T>& V) const;
//
// Division of one vector by another, going term-by-
// term as described above.
//
    gVector<T>& operator/=(const gVector<T>& V) 
      { *this = *this / V; return *this; }

//
// Tests if all components of the vector are equal to
// a constant c.  Checks all components in the vector.
//+grp
    int operator==(T c) const;
    int operator!=(T c) const;
//-grp

//
// Tests if vectors are equal, using the gTuple function.
// Reproduced because these operators are overloaded 
// elsewhere.
//+grp
    int operator==(const gVector<T>& V) const
      { return (gTuple<T>::operator==(V)); }
    int operator!=(const gVector<T>& V) const
      { return !(*this == V); }
//-grp

//
// Assigns the value c to all components of the vector,
// from First() to Last().
//
    gVector<T>& operator=(T c);
//
// Assigns one vector to another.  Needed because we have
// overloaded this operator elsewhere.
    gVector<T>& operator=(const gVector<T>& V)
      { gTuple<T>::operator=(V); return *this; }

        //# DESTRUCTOR
//
// Deallocate a vector.
//
    ~gVector()    { }
};

#ifdef __GNUC__
#define INLINE inline
#elif defined(__BORLANDC__)
#define INLINE
#else
#error Unsupported compiler type.
#endif   // __GNUC__, __BORLANDC__

template <class T>
INLINE gVector<T> gVector<T>::operator+(const gVector<T>& V) const
{
  assert(DimCheck(V));
  gVector<T> result(First(), Last());

  for (int i = First(); i <= Last(); i++)
    result[i] = (*this)[i] + V[i];
  return result;
}

template <class T> INLINE gVector<T> gVector<T>::operator-(void)
{
  gVector<T> result(First(), Last());
  for (int i = First(); i <= Last(); i++)
    result[i] = - (*this)[i];
  return result;
}

template <class T>
INLINE gVector<T> gVector<T>::operator-(const gVector<T>& V) const
{
  assert(DimCheck(V));
  gVector<T> result(First(), Last());

  for (int i = First(); i <= Last(); i++)
    result[i] = (*this)[i] - V[i];
  return result;
}

template <class T> INLINE gVector<T> gVector<T>::operator*(T c) const
{
  gVector<T> result(First(), Last());

  for (int i = First(); i <= Last(); i++)
    result[i] = (*this)[i] * c;
  return result;
}

template <class T> INLINE T gVector<T>::operator*(const gVector<T>& V) const
{
  T result = (T) 0;

  for (int i = First(); i <= Last(); i++)
// The explicit cast to T is necessary for when T is another gVector.
// Note that this merely gets this to compile, and does not produce
// sensible output, and certainly not the product of two matrices.
// This is a bit of a shortcoming, and we should think about the derivation
// structure of Vector to see if we can have two versions, one with and
// one without...
    result = result + ((T) ((*this)[i] * V[i]));

  return result;
}

template <class T> INLINE gVector<T> gVector<T>::operator/(T c) const
{
  gVector result(First(), Last());

  assert(c != (T) 0);
  for (int i = First(); i <= Last(); i++)
    result[i] = (*this)[i] / c;

  return result;
}

template <class T> INLINE 
  gVector<T> gVector<T>::operator/(const gVector<T>& V) const
{
  assert(DimCheck(V));
  gVector result(First(), Last());

  for (int i = First(); i <= Last(); i++)  {
    assert(V[i] != (T) 0);
    result[i] = (*this)[i] / V[i];
  }

  return result;
}

template <class T> INLINE int gVector<T>::operator==(T c) const
{
  int i = 0;
  while ((i < Length()) && ((*this)[i] == c))  i++;
  return (i == Length());
}

template <class T> INLINE int gVector<T>::operator!=(T c) const
{
  return !(*this == c);
}

template <class T>
INLINE gVector<T>& gVector<T>::operator=(T c)
{
  for (int i = First(); i <= Last(); i++)
    (*this)[i] = c;
  return *this;
}

//
// Outputs the contents of a gVector, in a standard form.
// The vector is enclosed in {}; components are separated by 
// spaces only.
//
template <class T> INLINE gOutput &operator<<(gOutput &op, const gVector<T> &v)
{
  op << '{' << ' ';
  for (int i = v.First(); i <= v.Last(); i++)  
    op << v[i] << ' ';
  op << '}';
  return op;
}

#endif   //# GVECTOR_H


