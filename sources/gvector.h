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
#include "garray.h"

//
// <category lib=glib sect=Math>
//
// In order to use type T, the following operators must be defined:
//   +, -(binary), *(binary), /, ==, =
//

template <class T> class gMatrix;

template <class T> class gVector : public gArray<T> {
  friend class gMatrix<T>;
private:
protected:

  T* Allocate(void);
  void Delete(T* p);
  void AllocateData(void);
  void DeleteData(void);
  void CopyData( const gVector<T> &V );

public:
	//# CONSTRUCTORS
// Create a vector of length len, starting at 1
  gVector(int len = 0);
// Create a vector indexed from low to high
  gVector(int low, int high);
// Copy constructor
  gVector(const gVector<T>& V);
// Destructor
  virtual ~gVector();

	//# OPERATORS

// = operators
  gVector<T>& operator=(const gVector<T>& V);
// Assigns the value c to all components of the vector,
  gVector<T>& operator=(T c);

//arithmetic operators
  gVector<T> operator+(const gVector<T>& V) const;
  gVector<T>& operator+=(const gVector<T>& V);

  gVector<T> operator-(void);
  gVector<T> operator-(const gVector<T>& V) const;
  gVector<T>& operator-=(const gVector<T>& V);

  gVector<T> operator*(T c) const;
  gVector<T>& operator*=(T c);
  T operator*(const gVector<T>& V) const;

  gVector<T> operator/(T c) const;
// Term-by-term division.  Not a standard math vector function, but useful
    gVector<T> operator/(const gVector<T>& V) const;
// Term-by-term division, as above
  gVector<T>& operator/=(const gVector<T>& V);

// comparison operators
  int operator==(const gVector<T>& V) const;
  int operator!=(const gVector<T>& V) const;

// Tests if all components of the vector are equal to a constant c
  int operator==(T c) const;
  int operator!=(T c) const;

// check functions
    // check whether index is within vector boundaries
  int Check(int index) const;
    // check vector for identical boundaries
  int Check(const gVector<T> &v) const;

// parameter access functions
//  virtual void Dump(gOutput &) const;
};

#ifdef __GNUG__
#include "rational.h"
gOutput &operator<<(gOutput &, const gVector<double> &);
gOutput &operator<<(gOutput &, const gVector<gRational> &);
#elif defined __BORLANDC__
template <class T> gOutput &operator<<(gOutput &, const gVector<T> &);
#endif   // __GNUG__, __BORLANDC__

#endif   //# GVECTOR_H
