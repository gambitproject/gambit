//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to an arbitrary-length integer class
//
// This file is part of Gambit
// Modifications copyright (c) 2002, The Gambit Project
//
// The original copyright and license are included below.
//

// This may look like C code, but it is really -*- C++ -*-

/* 
Copyright (C) 1988 Free Software Foundation
    written by Doug Lea (dl@rocky.oswego.edu)

This file is part of the GNU C++ Library.  This library is free
software; you can redistribute it and/or modify it under the terms of
the GNU Library General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.  This library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU Library General Public License for more details.
You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef INTEGER_H
#define INTEGER_H

#include "base/gtext.h"

class gInput;
class gOutput;

#if USE_GNU_MP
#include <gmp.h>
#else
class IntRep;
long Itolong(const IntRep*);
double Itodouble(const IntRep*);
int Iislong(const IntRep*);
int Iisdouble(const IntRep*);
#endif  // !USE_GNU_MP

class gInteger {
#if USE_GNU_MP
  friend class gRational;
#endif // USE_GNU_MP
protected:
#if USE_GNU_MP
  mpz_t m_value;
#else
  IntRep *rep;
#endif  // !USE_GNU_MP

public:
  // LIFECYCLE
  gInteger(void);
  gInteger(int);
  gInteger(long);
  gInteger(unsigned long);
#if !USE_GNU_MP
  gInteger(IntRep *);
#endif // !USE_GNU_MP
  gInteger(const gInteger &);
  ~gInteger();

  gInteger &operator=(const gInteger &);
  gInteger &operator=(long);

  // COMPARISON OPERATORS
  friend bool operator==(const gInteger &, const gInteger &);
  friend bool operator==(const gInteger &, long);
  friend bool operator!=(const gInteger &, const gInteger &);
  friend bool operator!=(const gInteger &, long);
  friend bool operator<(const gInteger &, const gInteger &);
  friend bool operator<(const gInteger &, long);
  friend bool operator<=(const gInteger &, const gInteger &);
  friend bool operator<=(const gInteger &, long);
  friend bool operator>(const gInteger &, const gInteger &);
  friend bool operator>(const gInteger &, long);
  friend bool operator>=(const gInteger &, const gInteger &);
  friend bool operator>=(const gInteger &, long);

  // ARITHMETIC OPERATORS
  friend gInteger operator-(const gInteger &);
  friend gInteger operator+(const gInteger &, const gInteger &);
  friend gInteger operator+(const gInteger &, long);
  friend gInteger operator+(long, const gInteger &);
  friend gInteger operator-(const gInteger &, const gInteger &);
  friend gInteger operator-(const gInteger &, long);
  friend gInteger operator-(long, const gInteger &);
  friend gInteger operator*(const gInteger &, const gInteger &);
  friend gInteger operator*(const gInteger &, long);
  friend gInteger operator*(long, const gInteger &);
  friend gInteger operator/(const gInteger &, const gInteger &);
  friend gInteger operator/(const gInteger &, long);
  friend gInteger operator%(const gInteger &, const gInteger &);
  friend gInteger operator%(const gInteger &, long);

// unary operations to self
  void            operator ++ ();
  void            operator -- ();
  void            negate();          // negate in-place
  void            abs();             // absolute-value in-place

// assignment-based operations

  void operator+=(const gInteger &);
  void operator-=(const gInteger &);
  void operator*=(const gInteger &);
  void operator/=(const gInteger &);
  void operator%=(const gInteger &);
  void operator += (long);
  void operator -= (long);
  void operator *= (long);
  void operator /= (long);
  void operator %= (long);

// (constructive binary operations are inlined below)

// builtin Integer functions that must be friends
  friend double ratio(const gInteger &, const gInteger &);

  friend gInteger gcd(const gInteger &, const gInteger &);
  friend gInteger lcm(const gInteger &, const gInteger &);
  friend bool even(const gInteger &);
  friend bool odd(const gInteger &);
  friend int sign(const gInteger &); // returns -1, 0, +1

// procedural versions of operators

  friend void     abs(const gInteger& x, gInteger& dest);
  friend void     negate(const gInteger& x, gInteger& dest);

  friend int      compare(const gInteger&, const gInteger&);  
  friend int      ucompare(const gInteger&, const gInteger&); 
  friend void     add(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     sub(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     mul(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     div(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     mod(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     divide(const gInteger& x, const gInteger& y, 
                         gInteger& q, gInteger& r);
  friend void     lshift(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     rshift(const gInteger& x, const gInteger& y, gInteger& dest);
  friend void     pow(const gInteger& x, const gInteger& y, gInteger& dest);

  friend int      compare(const gInteger&, long);  
  friend int      ucompare(const gInteger&, long); 
  friend void     add(const gInteger& x, long y, gInteger& dest);
  friend void     sub(const gInteger& x, long y, gInteger& dest);
  friend void     mul(const gInteger& x, long y, gInteger& dest);
  friend void     div(const gInteger& x, long y, gInteger& dest);
  friend void     mod(const gInteger& x, long y, gInteger& dest);
  friend void     divide(const gInteger& x, long y, gInteger& q, long& r);
  friend void     lshift(const gInteger& x, long y, gInteger& dest);
  friend void     rshift(const gInteger& x, long y, gInteger& dest);
  friend void     pow(const gInteger& x, long y, gInteger& dest);

  friend int      compare(long, const gInteger&);  
  friend int      ucompare(long, const gInteger&); 
  friend void     add(long x, const gInteger& y, gInteger& dest);
  friend void     sub(long x, const gInteger& y, gInteger& dest);
  friend void     mul(long x, const gInteger& y, gInteger& dest);

// coercion & conversion
  bool fits_in_long(void) const;
  long as_long(void) const;

  friend gText Itoa(const gInteger& x, int base = 10, int width = 0);
  friend gInteger atoI(const char* s, int base = 10);
  
  friend gInput& operator>>(gInput &s, gInteger& y);
  friend gOutput& operator<<(gOutput &s, const gInteger& y);

// error detection
  void error(const char* msg) const;
  bool OK(void) const;  
};


  gInteger  abs(const gInteger&); // absolute value
  gInteger  sqr(const gInteger&); // square

  gInteger  pow(const gInteger& x, const gInteger& y);
  gInteger  pow(const gInteger& x, long y);
  gInteger  Ipow(long x, long y); // x to the y as gInteger 


extern gInteger  sqrt(const gInteger&); // floor of square root

gText ToText(const gInteger &);

#endif  // INTEGER_H








