//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// A vector class
//

#ifndef GVECTOR_H
#define GVECTOR_H

#include "base/base.h"

template <class T> class gMatrix;

/** 
 * General purpose vector representation and calculation class.
 *
 * This is a general vector class.  In order to instantiate this for a
 * type, the operators binary +, binary -, binary *, /, ==, and = must
 * be defined for the type.
 */
template <class T> class gVector : public gArray<T>   {
  friend class gMatrix<T>;
public:
  class BadDim : public gException  {
  public:
    virtual ~BadDim()   { }
    gText Description(void) const;
  };

  /** Create a vector of length len, starting at 1 */
  gVector(unsigned int len = 0);
  /** Create a vector indexed from low to high */
  gVector(int low, int high);
  /** Copy constructor */
  gVector(const gVector<T>& V);
  /** Destructor */
  virtual ~gVector();
  
  /** Assignment operator: requires vectors to be of same length */
  gVector<T>& operator=(const gVector<T>& V);
  /** Assigns the value c to all components of the vector */
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
  
  bool operator==(const gVector<T>& V) const;
  bool operator!=(const gVector<T>& V) const;
  
  /** Tests if all components of the vector are equal to a constant c */
  bool operator==(T c) const;
  bool operator!=(T c) const;
  
  // square of length
  T NormSquared() const;
  
  // check vector for identical boundaries
  bool Check(const gVector<T> &v) const;
};

#ifndef __BORLANDC__
template <class T> gOutput &operator<<(gOutput &, const gVector<T> &);
#endif

#include "math/double.h"

template <class T> gVector<gDouble> TogDouble(const gVector<T>&);

#endif   //# GVECTOR_H






