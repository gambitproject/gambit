//#
//# FILE: gvector.h -- Implementation of vector classes
//#
//# $Id$
//#

#ifndef GVECTOR_H
#define GVECTOR_H

#include "gmisc.h"
#include "gambitio.h"
#include "garray.h"

//
// <category lib=glib sect=Math>
//
// In order to use type T, the following operators must be defined:
//   +, -(binary), *(binary), /, ==, =
//

template <class T> class gMatrix;

template <class T> class gVector : public gArray<T>   {
  friend class gMatrix<T>;
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

        // arithmetic operators
    gVector<T> operator+(const gVector<T>& V) const;
    gVector<T>& operator+=(const gVector<T>& V);

    gVector<T> operator-(void);
    gVector<T> operator-(const gVector<T>& V) const;
    gVector<T>& operator-=(const gVector<T>& V);

    gVector<T> operator*(T c) const;
    gVector<T>& operator*=(T c);
    T operator*(const gVector<T>& V) const;

    gVector<T> operator/(T c) const;

        // comparison operators
    int operator==(const gVector<T>& V) const;
    int operator!=(const gVector<T>& V) const;

        // Tests if all components of the vector are equal to a constant c
    int operator==(T c) const;
    int operator!=(T c) const;

        // check vector for identical boundaries
    bool Check(const gVector<T> &v) const;
};
#ifndef __BORLANDC__
template <class T> gOutput &operator<<(gOutput &, const gVector<T> &);
#endif
#endif   //# GVECTOR_H







