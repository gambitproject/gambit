//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// A vector class
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef GVECTOR_H
#define GVECTOR_H

#include "base/base.h"

template <class T> class gbtMatrix;

/** 
 * General purpose vector representation and calculation class.
 *
 * This is a general vector class.  In order to instantiate this for a
 * type, the operators binary +, binary -, binary *, /, ==, and = must
 * be defined for the type.
 */
template <class T> class gbtVector : public gbtArray<T>   {
  friend class gbtMatrix<T>;
public:
  class BadDim : public gbtException  {
  public:
    virtual ~BadDim()   { }
    gbtText Description(void) const;
  };

  /** Create a vector of length len, starting at 1 */
  gbtVector(unsigned int len = 0);
  /** Create a vector indexed from low to high */
  gbtVector(int low, int high);
  /** Copy constructor */
  gbtVector(const gbtVector<T>& V);
  /** Destructor */
  virtual ~gbtVector();
  
  /** Assignment operator: requires vectors to be of same length */
  gbtVector<T>& operator=(const gbtVector<T>& V);
  /** Assigns the value c to all components of the vector */
  gbtVector<T>& operator=(T c);
  
  gbtVector<T> operator+(const gbtVector<T>& V) const;
  gbtVector<T>& operator+=(const gbtVector<T>& V);
  
  gbtVector<T> operator-(void);
  gbtVector<T> operator-(const gbtVector<T>& V) const;
  gbtVector<T>& operator-=(const gbtVector<T>& V);
  
  gbtVector<T> operator*(T c) const;
  gbtVector<T>& operator*=(T c);
  T operator*(const gbtVector<T>& V) const;
  
  gbtVector<T> operator/(T c) const;
  
  bool operator==(const gbtVector<T>& V) const;
  bool operator!=(const gbtVector<T>& V) const;
  
  /** Tests if all components of the vector are equal to a constant c */
  bool operator==(T c) const;
  bool operator!=(T c) const;
  
  // square of length
  T NormSquared() const;
  
  // check vector for identical boundaries
  bool Check(const gbtVector<T> &v) const;
};

#ifndef __BORLANDC__
template <class T> gbtOutput &operator<<(gbtOutput &, const gbtVector<T> &);
#endif

#include "math/double.h"

template <class T> gbtVector<gbtDouble> TogDouble(const gbtVector<T>&);

#endif   //# GVECTOR_H
