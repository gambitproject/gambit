//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION;
// Implementation of a rational number class
//
// This file is part of Gambit
// Modifications copyright (c) 2002, The Gambit Project
//
// See rational.h for a discussion of this class.  One implementation
// provided here is based upon a rational number class included in the
// GNU C++ Library circa 1994.
// The original copyright and license are included below:
//
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
//

#include <iostream>
#include <math.h>
#include <float.h>
#include <ctype.h>
#include "math/gmath.h"
#include "math/rational.h"

//------------------------------------------------------------------------
//                 gbtRational: Private member functions
//------------------------------------------------------------------------

#if !USE_GNU_MP
static const gbtInteger _Int_One(1);
#endif  // !USE_GNU_MP

#if !USE_GNU_MP
void gbtRational::normalize(void)
{
  int s = sign(den);
  if (s == 0) {
    throw gbtDivisionByZeroException();
  }
  else if (s < 0) {
    den.negate();
    num.negate();
  }

  gbtInteger g = gcd(num, den);
  if (ucompare(g, _Int_One) != 0) {
    num /= g;
    den /= g;
  }
}
#endif  // !USE_GNU_MP

//------------------------------------------------------------------------
//                      gbtRational: Constructors
//------------------------------------------------------------------------

#if USE_GNU_MP
gbtRational::gbtRational(void)
{
  mpq_init(m_value);
}
#else
gbtRational::gbtRational(void) 
  : num(&_ZeroRep), den(&_OneRep) 
{ }
#endif  // USE_GNU_MP

gbtRational::gbtRational(double x)
{
#if USE_GNU_MP
  mpq_init(m_value);
  mpq_set_d(m_value, x);
  mpq_canonicalize(m_value);
#else
  num = 0;
  den = 1;
  if (x != 0.0) {
    int neg = x < 0;
    if (neg) {
      x = -x;
    }

    const long shift = 15;         // a safe shift per step
    const double width = 32768.0;  // = 2^shift
    const int maxiter = 20;        // ought not be necessary, but just in case,
                                   // max 300 bits of precision
    int expt;
    double mantissa = frexp(x, &expt);
    long exponent = expt;
    double intpart;
    int k = 0;
    while (mantissa != 0.0 && k++ < maxiter) {
      mantissa *= width;
      mantissa = modf(mantissa, &intpart);
      num <<= shift;
      num += (long)intpart;
      exponent -= shift;
    }
    if (exponent > 0) {
      num <<= exponent;
    }
    else if (exponent < 0) {
      den <<= -exponent;
    }
    if (neg) {
      num.negate();
    }
  }
  normalize();
#endif  // USE_GNU_MP
}

#if USE_GNU_MP
gbtRational::gbtRational(const gbtInteger &n)
{
  mpq_init(m_value);
  mpq_set_str(m_value, ToText(n), 10);
}
#else
gbtRational::gbtRational(const gbtInteger &n) 
  : num(n), den(&_OneRep) 
{ }
#endif  // USE_GNU_MP

#if USE_GNU_MP
gbtRational::gbtRational(const gbtInteger &n, const gbtInteger &d) 
{
  mpq_init(m_value);
  mpq_set_str(m_value, ToText(n) + "/" + ToText(d), 10);
  mpq_canonicalize(m_value);
}
#else
gbtRational::gbtRational(const gbtInteger &n, const gbtInteger &d) 
  : num(n), den(d)
{
  normalize();
}
#endif  // USE_GNU_MP

#if USE_GNU_MP
gbtRational::gbtRational(const gbtRational &y)
{
  mpq_init(m_value);
  mpq_set(m_value, y.m_value);
}
#else
gbtRational::gbtRational(const gbtRational &y) 
  : num(y.num), den(y.den)
{ }
#endif  // USE_GNU_MP

gbtRational::~gbtRational() 
{
#if USE_GNU_MP
  mpq_clear(m_value);
#endif  // USE_GNU_MP
}

gbtRational &gbtRational::operator=(const gbtRational &y)
{
#if USE_GNU_MP
  if (this != &y) {
    mpq_set(m_value, y.m_value);
  }
#else
  num = y.num; 
  den = y.den; 
#endif  // USE_GNU_MP
  return *this;
}

//------------------------------------------------------------------------
//                         gbtRational: Data access 
//------------------------------------------------------------------------

int sign(const gbtRational &x)
{
#if USE_GNU_MP
  return mpq_sgn(x.m_value);
#else
  return sign(x.num);
#endif  // USE_GNU_MP
}

gbtInteger gbtRational::GetNumerator(void) const 
{
#if USE_GNU_MP
  gbtInteger r;
  mpq_get_num(r.m_value, m_value);
  return r;
#else
  return num;
#endif  // USE_GNU_MP
}

gbtInteger gbtRational::GetDenominator(void) const 
{
#if USE_GNU_MP
  gbtInteger r;
  mpq_get_den(r.m_value, m_value);
  return r;
#else
  return den;
#endif  // USE_GNU_MP
}

gbtRational::operator double() const 
{
#if USE_GNU_MP
  return mpq_get_d(m_value);
#else
  return ratio(num, den); 
#endif  // USE_GNU_MP
}

void gEpsilon(gbtRational &v, int /*i*/)
{ v = (gbtRational)0; }

//------------------------------------------------------------------------
//              gbtRational: Procedural versions of operators
//------------------------------------------------------------------------

void add(const gbtRational &x, const gbtRational &y, gbtRational &r)
{
#if USE_GNU_MP
  mpq_add(r.m_value, x.m_value, y.m_value);
#else
  if (&r != &x && &r != &y) {
    mul(x.num, y.den, r.num);
    mul(x.den, y.num, r.den);
    add(r.num, r.den, r.num);
    mul(x.den, y.den, r.den);
  }
  else {
    gbtInteger tmp;
    mul(x.den, y.num, tmp);
    mul(x.num, y.den, r.num);
    add(r.num, tmp, r.num);
    mul(x.den, y.den, r.den);
  }
  r.normalize();
#endif  // USE_GNU_MP
}

void sub(const gbtRational &x, const gbtRational &y, gbtRational &r)
{
#if USE_GNU_MP
  mpq_sub(r.m_value, x.m_value, y.m_value);
#else
  if (&r != &x && &r != &y) {
    mul(x.num, y.den, r.num);
    mul(x.den, y.num, r.den);
    sub(r.num, r.den, r.num);
    mul(x.den, y.den, r.den);
  }
  else {
    gbtInteger tmp;
    mul(x.den, y.num, tmp);
    mul(x.num, y.den, r.num);
    sub(r.num, tmp, r.num);
    mul(x.den, y.den, r.den);
  }
  r.normalize();
#endif  // USE_GNU_MP
}

void mul(const gbtRational &x, const gbtRational &y, gbtRational &r)
{
#if USE_GNU_MP
  mpq_mul(r.m_value, x.m_value, y.m_value);
#else
  mul(x.num, y.num, r.num);
  mul(x.den, y.den, r.den);
  r.normalize();
#endif  // USE_GNU_MP
}

void div(const gbtRational &x, const gbtRational &y, gbtRational &r)
{
#if USE_GNU_MP
  mpq_div(r.m_value, x.m_value, y.m_value);
#else
  if (&r != &x && &r != &y) {
    mul(x.num, y.den, r.num);
    mul(x.den, y.num, r.den);
  }
  else {
    gbtInteger tmp;
    mul(x.num, y.den, tmp);
    mul(y.num, x.den, r.den);
    r.num = tmp;
  }
  r.normalize();
#endif  // USE_GNU_MP
}

int compare(const gbtRational &x, const gbtRational &y)
{
#if USE_GNU_MP
  return mpq_cmp(x.m_value, y.m_value);
#else
  int xsgn = sign(x.num);
  int ysgn = sign(y.num);
  int d = xsgn - ysgn;
  if (d == 0 && xsgn != 0) {
    d = compare(x.num * y.den, x.den * y.num);
  }
  return d;
#endif  // USE_GNU_MP
}

void gbtRational::negate(void)
{
#if USE_GNU_MP
  mpq_neg(m_value, m_value);
#else
  num.negate();
#endif  // USE_GNU_MP
}

//------------------------------------------------------------------------
//                   gbtRational: Arithmetic operators
//------------------------------------------------------------------------

gbtRational operator+(const gbtRational &x, const gbtRational &y) 
{
  gbtRational r; add(x, y, r); return r;
}

gbtRational operator-(const gbtRational &x, const gbtRational &y)
{
  gbtRational r; sub(x, y, r); return r;
}

gbtRational operator-(const gbtRational &x) 
{
  gbtRational r(x); r.negate();  return r;
}

gbtRational operator*(const gbtRational &x, const gbtRational &y)
{
  gbtRational r; mul(x, y, r); return r;
}

gbtRational operator/(const gbtRational &x, const gbtRational &y)
{
  gbtRational r; div(x, y, r); return r;
}

void gbtRational::operator+=(const gbtRational &y) 
{
  add(*this, y, *this);
}

void gbtRational::operator-=(const gbtRational &y) 
{
  sub(*this, y, *this);
}

void gbtRational::operator*=(const gbtRational &y) 
{
  mul(*this, y, *this);
}

void gbtRational::operator/=(const gbtRational &y) 
{
  div(*this, y, *this);
}

//------------------------------------------------------------------------
//                  gbtRational: Comparison operators
//------------------------------------------------------------------------

int operator==(const gbtRational &x, const gbtRational &y)
{
#if USE_GNU_MP
  return mpq_equal(x.m_value, y.m_value);
#else
  return (compare(x.num, y.num) == 0 && compare(x.den, y.den) == 0);
#endif  // USE_GNU_MP
}

int operator!=(const gbtRational &x, const gbtRational &y)
{
#if USE_GNU_MP
  return !mpq_equal(x.m_value, y.m_value);
#else
  return (compare(x.num, y.num) != 0 || compare(x.den, y.den) != 0);
#endif  // USE_GNU_MP
}

int operator<(const gbtRational &x, const gbtRational &y)
{
  return (compare(x, y) < 0); 
}

int operator<=(const gbtRational& x, const gbtRational& y)
{
  return (compare(x, y) <= 0); 
}

int operator>(const gbtRational& x, const gbtRational& y)
{
  return (compare(x, y) > 0); 
}

int operator>=(const gbtRational& x, const gbtRational& y)
{
  return (compare(x, y) >= 0); 
}

//------------------------------------------------------------------------
//                    gbtRational: Other operations
//------------------------------------------------------------------------

void gbtRational::invert(void)
{
#if USE_GNU_MP
  mpq_inv(m_value, m_value);
#else
  gbtInteger tmp = num;  
  num = den;  
  den = tmp;  
  int s = sign(den);
  if (s == 0) {
    throw gbtDivisionByZeroException();
  }
  else if (s < 0) {
    den.negate();
    num.negate();
  }
#endif  // USE_GNU_MP
}

gbtRational pow(const gbtRational &x, const gbtInteger &y)
{
  long yy = y.as_long();
  return pow(x, yy);
}               

gbtRational fabs(const gbtRational &x) 
{
#if USE_GNU_MP
  gbtRational r;
  mpq_abs(r.m_value, x.m_value);
  return r;
#else
  gbtRational r(x);
  if (sign(r.num) < 0) r.negate();
  return r;
#endif  // USE_GNU_MP
}

gbtRational sqr(const gbtRational &x)
{
#if USE_GNU_MP
  gbtRational r;
  mpq_mul(r.m_value, x.m_value, x.m_value);
  return r;
#else
  gbtRational r;
  mul(x.num, x.num, r.num);
  mul(x.den, x.den, r.den);
  r.normalize();
  return r;
#endif   // USE_GNU_MP
}

gbtRational pow(const gbtRational& x, long y)
{
#if USE_GNU_MP
  if (y >= 0) {
    gbtRational r = 1;
    for (long i = 1; i <= y; i++) {
      r *= r;
    }
    return r;
  }
  else {
    y = -y;
    gbtRational r = 1;
    for (long i = 1; i <= y; i++) {
      r *= r;
    }
    r.invert();
    return r;
  }
#else
  gbtRational r;
  if (y >= 0) {
    pow(x.num, y, r.num);
    pow(x.den, y, r.den);
  }
  else {
    y = -y;
    pow(x.num, y, r.den);
    pow(x.den, y, r.num);
    if (sign(r.den) < 0) {
      r.num.negate();
      r.den.negate();
    }
  }
  return r;
#endif  // USE_GNU_MP
}

//------------------------------------------------------------------------
//                    gbtRational: Input and output
//------------------------------------------------------------------------

std::ostream &operator<<(std::ostream &s, const gbtRational &y)
{
  s << ToText(y);
  return s;
}

std::istream &operator>>(std::istream &f, gbtRational &y)
{
  char ch = ' ';
  int sign = 1;
  gbtInteger num = 0, denom = 1;

  while (isspace(ch))    f >> ch;
  
  if (ch == '-')  { 
    sign = -1;
    f >> ch;
  }
  
  while (ch >= '0' && ch <= '9')   {
    num *= 10;
    num += (int) (ch - '0');
    f >> ch;
  }
  
  if (ch == '/')  {
    denom = 0;
    f >> ch;
    while (ch >= '0' && ch <= '9')  {
      denom *= 10;
      denom += (int) (ch - '0');
      f >> ch;
    }
  }
  else if (ch == '.')  {
    denom = 1;
    f >> ch;
    while (ch >= '0' && ch <= '9')  {
      denom *= 10;
      num *= 10;
      num += (int) (ch - '0');
      f >> ch;
    }
  }

  f.unget();

  y = gbtRational(sign * num, denom);
  return f;
}

std::string ToText(const gbtRational &r)
{
#if USE_GNU_MP
  // This buffer size recommended by documentation in GMP
  char buffer[mpz_sizeinbase(mpq_numref(r.m_value), 10) +
	      mpz_sizeinbase(mpq_denref(r.m_value), 10) + 3];
  mpq_get_str(buffer, 10, r.m_value);
  return std::string(buffer);
#else
  const int GCONVERT_BUFFER_LENGTH = 255;
  char gconvert_buffer[GCONVERT_BUFFER_LENGTH];

  strncpy(gconvert_buffer, 
	  Itoa(r.GetNumerator()).c_str(), GCONVERT_BUFFER_LENGTH);
  if (r.GetDenominator() != gbtInteger(1)) {
    strcat(gconvert_buffer, "/");
    strncat(gconvert_buffer, 
	    Itoa(r.GetDenominator()).c_str(), GCONVERT_BUFFER_LENGTH);
  }
  
  return std::string(gconvert_buffer);
#endif  // USE_GNU_MP
}

gbtRational FromText(const std::string &f, gbtRational &y)
{
#if USE_GNU_MP
  mpq_set_str(y.m_value, (char *) f, 10);
  mpq_canonicalize(y.m_value);
  return y;
#else
  char ch = ' ';
  int sign = 1;
  unsigned int index = 0, length = f.length();
  gbtInteger num = 0, denom = 1;

  while (isspace(ch) && index<=length) {
    ch = f[index++];
  }

  if (ch == '-' && index<=length)  {
    sign = -1;
    ch=f[index++];
  }

  while (ch >= '0' && ch <= '9' && index<=length)   {
    num *= 10;
    num += (int) (ch - '0');
    ch=f[index++];
  }

  if (ch == '/')  {
    denom = 0;
    ch=f[index++];
    while (ch >= '0' && ch <= '9' && index<=length)  {
      denom *= 10;
      denom += (int) (ch - '0');
      ch=f[index++];
    }
  }
  else if (ch == '.')  {
    denom = 1;
    ch=f[index++];
    while (ch >= '0' && ch <= '9' && index<=length)  {
      denom *= 10;
      num *= 10;
      num += (int) (ch - '0');
      ch=f[index++];
    }
  }

  if (denom != 0)
    y = gbtRational(sign * num, denom);
  else
    y = gbtRational(sign * num);
  return y;
#endif  // USE_GNU_MP
}
