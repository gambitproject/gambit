//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION;
// Interface to a rational number class
//
// This file is part of Gambit
// Modifications copyright (c) 2002, The Gambit Project
//
// The original copyright and license are included below.

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

#ifndef _Rational_h
#if defined(__GNUG__) && !defined(__APPLE_CC__)
#pragma interface
#endif
#define _Rational_h 1

#include "integer.h"
#include <math.h>

class gbtRational
{
protected:
  gbtInteger          num;
  gbtInteger          den;

  void             normalize();

public:
                   gbtRational();
                   gbtRational(double);
                   gbtRational(int n);
                   gbtRational(long n);
                   gbtRational(int n, int d);
                   gbtRational(long n, long d);
                   gbtRational(long n, unsigned long d);
                   gbtRational(unsigned long n, long d);
                   gbtRational(unsigned long n, unsigned long d);
                   gbtRational(const gbtInteger& n);
                   gbtRational(const gbtInteger& n, const gbtInteger& d);
                   gbtRational(const gbtRational&);

                  ~gbtRational();

  gbtRational &operator =  (const gbtRational& y);

  bool operator==(const gbtRational &y) const;
  bool operator!=(const gbtRational &y) const;
  bool operator< (const gbtRational &y) const;
  bool operator<=(const gbtRational &y) const;
  bool operator> (const gbtRational &y) const;
  bool operator>=(const gbtRational &y) const;

  gbtRational operator+(const gbtRational &y) const;
  gbtRational operator-(const gbtRational &y) const;
  gbtRational operator*(const gbtRational &y) const;
  gbtRational operator/(const gbtRational &y) const;

  gbtRational &operator+=(const gbtRational& y);
  gbtRational &operator-=(const gbtRational& y);
  gbtRational &operator*=(const gbtRational& y);
  gbtRational &operator/=(const gbtRational& y);

  gbtRational operator-(void) const;


// builtin gbtRational functions


  void             negate();                      // x = -x
  void             invert();                      // x = 1/x

  friend int       sign(const gbtRational& x);             // -1, 0, or +1
  friend gbtRational  abs(const gbtRational& x);              // absolute value
  friend gbtRational  sqr(const gbtRational& x);              // square
  friend gbtRational  pow(const gbtRational& x, long y);
  friend gbtRational  pow(const gbtRational& x, const gbtInteger& y);
  const gbtInteger&   numerator() const;
  const gbtInteger&   denominator() const;

// coercion & conversion

                   operator double() const;
  friend gbtInteger   floor(const gbtRational& x);
  friend gbtInteger   ceil(const gbtRational& x);
  friend gbtInteger   trunc(const gbtRational& x);
  friend gbtInteger   round(const gbtRational& x);

  friend std::istream &operator>>(std::istream &s, gbtRational& y);
  friend std::ostream &operator<<(std::ostream &s, const gbtRational& y);

  int		   fits_in_float() const;
  int		   fits_in_double() const;

// procedural versions of operators

  friend int       compare(const gbtRational& x, const gbtRational& y);
  friend void      add(const gbtRational& x, const gbtRational& y, gbtRational& dest);
  friend void      sub(const gbtRational& x, const gbtRational& y, gbtRational& dest);
  friend void      mul(const gbtRational& x, const gbtRational& y, gbtRational& dest);
  friend void      div(const gbtRational& x, const gbtRational& y, gbtRational& dest);

// error detection

  void    error(const char* msg) const;
  int              OK() const;

};

std::string ToText(const gbtRational &);
gbtRational ToRational(const std::string &);
void gEpsilon(gbtRational &v, int i = 8);

//
// This is implemented so as to convert rational numbers to their
// floating point equivalent.
//
inline double ToDouble(const std::string &s)
{
  return (double) ToRational(s);
}

gbtRational ToNumber(const std::string &p_string);

#endif

