//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION;
// Interface to a rational number class
//
// This file is part of Gambit
// Original portions (c) 2002, The Gambit Project

//
// This defines an interface to a rational number class.  The class is
// currently implemented in two different ways:
//
// o A "native" Gambit implementation, which is essentially a minor
//   modification of a rational number class which was part of the old
//   GNU C++ Library circa 1994.
//   The original copyright and license statement from that code follows:
//----begin license----
// Copyright (C) 1988 Free Software Foundation
// written by Doug Lea (dl@rocky.oswego.edu)
//
// This file is part of the GNU C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the terms of
// the GNU Library General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your
// option) any later version.  This library is distributed in the hope
// that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the GNU Library General Public License for more details.
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//-----end license---
//   This version is being kept so as to have a version of the class 
//   available on all systems, particularly Windows ones, for which another
//   external library is not available, as well as a baseline for testing
//   correctness and efficiency.  Experience indicates that, while this
//   version may not fast, it is almost certainly correct.
//
// o An implementation which uses the GNU mp library (gmp).  The hope is
//   that this version should be more efficient for calculation, if
//   available.  Currently, this implementation requires that
//   the preprocessor symbol USE_GNU_MP is defined.  This implementation
//   is done using the C API for GNU mp, both for historical reasons
//   (i.e., compatibility with existing Gambit code), and because the
//   C++ API for GNU mp is not as guaranteed stable.  In a perfect world,
//   we would probably just use GNU mp and be done with it; however,
//   since there are Windows machines out there, the world is clearly
//   imperfect.
//
// Other implementations of this class using other libraries are also
// possible, and perhaps a good idea
//

#ifndef RATIONAL_H
#define RATIONAL_H

#include <math.h>
#if USE_GNU_MP
#include <gmp.h>
#endif // USE_GNU_MP
#include "math/integer.h"

class gInput;
class gOutput;

class gRational {
protected:
#if USE_GNU_MP
  mpq_t m_value;
#else
  gInteger num, den;
#endif  // USE_GNU_MP

#if !USE_GNU_MP
  void normalize();
#endif  // USE_GNU_MP

  // error reporting
  void error(const char *msg) const;

public:
  // LIFECYCLE
  gRational(void);
  gRational(double);
  gRational(const gInteger &);
  gRational(const gInteger &n, const gInteger &d);
  gRational(const gRational &);
  ~gRational();

  gRational &operator=(const gRational& y);
  
  // COMPARISON OPERATORS
  friend int operator==(const gRational &x, const gRational &y);
  friend int operator!=(const gRational &x, const gRational &y);
  friend int operator<(const gRational &x, const gRational &y);
  friend int operator<=(const gRational &x, const gRational &y);
  friend int operator>(const gRational &x, const gRational &y);
  friend int operator>=(const gRational &x, const gRational &y);

  // ARITHMETIC OPERATORS
  friend gRational operator+(const gRational& x, const gRational& y);
  friend gRational operator-(const gRational& x, const gRational& y);
  friend gRational operator*(const gRational& x, const gRational& y);
  friend gRational operator/(const gRational& x, const gRational& y);

  void operator+=(const gRational& y);
  void operator-=(const gRational& y);
  void operator*=(const gRational& y);
  void operator/=(const gRational& y);

  friend gRational operator-(const gRational& x);

  // PROCEDURAL VERSIONS OF OPERATORS
  friend int compare(const gRational &x, const gRational &y);
  friend void add(const gRational &x, const gRational &y, gRational &dest);
  friend void sub(const gRational &x, const gRational &y, gRational &dest);
  friend void mul(const gRational &x, const gRational &y, gRational &dest);
  friend void div(const gRational &x, const gRational &y, gRational &dest);

  // OTHER MATHEMATICAL OPERATIONS
  void negate(void);   // x = -x
  void invert(void);   // x = 1/x

  friend int sign(const gRational& x);             // -1, 0, or +1
  friend gRational abs(const gRational& x);              // absolute value
  friend gRational sqr(const gRational& x);              // square
  friend gRational pow(const gRational& x, long y);
  friend gRational pow(const gRational& x, const gInteger& y);

  // DATA ACCESS
  gInteger GetNumerator(void) const;
  gInteger GetDenominator(void) const;

  // CONVERSIONS
  operator double() const;
  friend gText ToText(const gRational &);
  friend gRational FromText(const gText &, gRational &);

  // INPUT AND OUTPUT
  friend gInput &operator>>(gInput& s, gRational& y);
  friend gOutput &operator<<(gOutput& s, const gRational& y);
};

void gEpsilon(gRational &v, int i = 8);

#endif  // RATIONAL_H

