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

#include <string>

#if USE_GNU_MP
#include <gmp.h>
#else
// internal Integer representation
class gbt_integer_rep {
public:
  unsigned short  len;          // current length
  unsigned short  sz;           // allocated space (0 means static).
  short           sgn;          // 1 means >= 0; 0 means < 0 
  unsigned short  s[1];         // represented as ushort array starting here
};

extern gbt_integer_rep _ZeroRep;
extern gbt_integer_rep _OneRep;
extern gbt_integer_rep _MinusOneRep;
long Itolong(const gbt_integer_rep*);
double Itodouble(const gbt_integer_rep*);
int Iislong(const gbt_integer_rep*);
int Iisdouble(const gbt_integer_rep*);
#endif  // !USE_GNU_MP

class gbtInteger {
#if USE_GNU_MP
  friend class gbtRational;
#endif // USE_GNU_MP
protected:
#if USE_GNU_MP
  mpz_t m_value;
#else
  gbt_integer_rep *rep;
#endif  // !USE_GNU_MP

public:
  // LIFECYCLE
  gbtInteger(void);
  gbtInteger(int);
  gbtInteger(long);
  gbtInteger(unsigned long);
#if !USE_GNU_MP
  gbtInteger(gbt_integer_rep *);
#endif // !USE_GNU_MP
  gbtInteger(const gbtInteger &);
  ~gbtInteger();

  gbtInteger &operator=(const gbtInteger &);
  gbtInteger &operator=(long);

  // COMPARISON OPERATORS
  friend bool operator==(const gbtInteger &, const gbtInteger &);
  friend bool operator==(const gbtInteger &, long);
  friend bool operator!=(const gbtInteger &, const gbtInteger &);
  friend bool operator!=(const gbtInteger &, long);
  friend bool operator<(const gbtInteger &, const gbtInteger &);
  friend bool operator<(const gbtInteger &, long);
  friend bool operator<=(const gbtInteger &, const gbtInteger &);
  friend bool operator<=(const gbtInteger &, long);
  friend bool operator>(const gbtInteger &, const gbtInteger &);
  friend bool operator>(const gbtInteger &, long);
  friend bool operator>=(const gbtInteger &, const gbtInteger &);
  friend bool operator>=(const gbtInteger &, long);

  // ARITHMETIC OPERATORS
  friend gbtInteger operator-(const gbtInteger &);
  friend gbtInteger operator+(const gbtInteger &, const gbtInteger &);
  friend gbtInteger operator+(const gbtInteger &, long);
  friend gbtInteger operator+(long, const gbtInteger &);
  friend gbtInteger operator-(const gbtInteger &, const gbtInteger &);
  friend gbtInteger operator-(const gbtInteger &, long);
  friend gbtInteger operator-(long, const gbtInteger &);
  friend gbtInteger operator*(const gbtInteger &, const gbtInteger &);
  friend gbtInteger operator*(const gbtInteger &, long);
  friend gbtInteger operator*(long, const gbtInteger &);
  friend gbtInteger operator/(const gbtInteger &, const gbtInteger &);
  friend gbtInteger operator/(const gbtInteger &, long);
  friend gbtInteger operator%(const gbtInteger &, const gbtInteger &);
  friend gbtInteger operator%(const gbtInteger &, long);

// unary operations to self
  void            operator ++ ();
  void            operator -- ();
  void            negate();          // negate in-place
  void            abs();             // absolute-value in-place

// assignment-based operations

  void operator+=(const gbtInteger &);
  void operator-=(const gbtInteger &);
  void operator*=(const gbtInteger &);
  void operator/=(const gbtInteger &);
  void operator%=(const gbtInteger &);
  void operator += (long);
  void operator -= (long);
  void operator *= (long);
  void operator /= (long);
  void operator %= (long);

#if !USE_GNU_MP
  void operator <<=(const gbtInteger &);
#endif // !USE_GNU_MP

// (constructive binary operations are inlined below)

// builtin Integer functions that must be friends
  friend double ratio(const gbtInteger &, const gbtInteger &);

  friend gbtInteger gcd(const gbtInteger &, const gbtInteger &);
  friend gbtInteger lcm(const gbtInteger &, const gbtInteger &);
  friend bool even(const gbtInteger &);
  friend bool odd(const gbtInteger &);
  friend int sign(const gbtInteger &); // returns -1, 0, +1

// procedural versions of operators

  friend void     abs(const gbtInteger& x, gbtInteger& dest);
  friend void     negate(const gbtInteger& x, gbtInteger& dest);

  friend int      compare(const gbtInteger&, const gbtInteger&);  
  friend int      ucompare(const gbtInteger&, const gbtInteger&); 
  friend void     add(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);
  friend void     sub(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);
  friend void     mul(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);
  friend void     div(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);
  friend void     mod(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);
  friend void     divide(const gbtInteger& x, const gbtInteger& y, 
                         gbtInteger& q, gbtInteger& r);
  friend void     lshift(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);
  friend void     rshift(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);
  friend void     pow(const gbtInteger& x, const gbtInteger& y, gbtInteger& dest);

  friend int      compare(const gbtInteger&, long);  
  friend int      ucompare(const gbtInteger&, long); 
  friend void     add(const gbtInteger& x, long y, gbtInteger& dest);
  friend void     sub(const gbtInteger& x, long y, gbtInteger& dest);
  friend void     mul(const gbtInteger& x, long y, gbtInteger& dest);
  friend void     div(const gbtInteger& x, long y, gbtInteger& dest);
  friend void     mod(const gbtInteger& x, long y, gbtInteger& dest);
  friend void     divide(const gbtInteger& x, long y, gbtInteger& q, long& r);
  friend void     lshift(const gbtInteger& x, long y, gbtInteger& dest);
  friend void     rshift(const gbtInteger& x, long y, gbtInteger& dest);
  friend void     pow(const gbtInteger& x, long y, gbtInteger& dest);

  friend int      compare(long, const gbtInteger&);  
  friend int      ucompare(long, const gbtInteger&); 
  friend void     add(long x, const gbtInteger& y, gbtInteger& dest);
  friend void     sub(long x, const gbtInteger& y, gbtInteger& dest);
  friend void     mul(long x, const gbtInteger& y, gbtInteger& dest);

// coercion & conversion
  bool fits_in_long(void) const;
  long as_long(void) const;

  friend std::string Itoa(const gbtInteger& x, int base = 10, int width = 0);
  friend gbtInteger atoI(const char* s, int base = 10);
  
  friend std::istream& operator>>(std::istream &s, gbtInteger& y);
  friend std::ostream &operator<<(std::ostream &s, const gbtInteger& y);

// error detection
  void error(const char* msg) const;
  bool OK(void) const;  
};


  gbtInteger  abs(const gbtInteger&); // absolute value
  gbtInteger  sqr(const gbtInteger&); // square

  gbtInteger  pow(const gbtInteger& x, const gbtInteger& y);
  gbtInteger  pow(const gbtInteger& x, long y);
  gbtInteger  Ipow(long x, long y); // x to the y as gbtInteger 


extern gbtInteger  sqrt(const gbtInteger&); // floor of square root

std::string ToText(const gbtInteger &);

#endif  // INTEGER_H
