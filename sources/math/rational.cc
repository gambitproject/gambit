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

#include <math.h>
#include <values.h>
#include <float.h>
#include <assert.h>
#include <ctype.h>
#include "base/gstream.h"
#include "math/rational.h"

//------------------------------------------------------------------------
//                 gRational: Private member functions
//------------------------------------------------------------------------

void gRational::error(const char* msg) const
{
  //  gerr << "gRational class error: " << msg << '\n';
  assert(0);
}

#if !USE_GNU_MP
static const gInteger _Int_One(1);
#endif  // !USE_GNU_MP

#if !USE_GNU_MP
void gRational::normalize(void)
{
  int s = sign(den);
  if (s == 0)
    error("Zero denominator.");
  else if (s < 0) {
    den.negate();
    num.negate();
  }

  gInteger g = gcd(num, den);
  if (ucompare(g, _Int_One) != 0) {
    num /= g;
    den /= g;
  }
}
#endif  // !USE_GNU_MP

//------------------------------------------------------------------------
//                      gRational: Constructors
//------------------------------------------------------------------------

#if USE_GNU_MP
gRational::gRational(void)
{
  mpq_init(m_value);
}
#else
gRational::gRational(void) 
  : num(&_ZeroRep), den(&_OneRep) 
{ }
#endif  // USE_GNU_MP

gRational::gRational(double x)
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
gRational::gRational(const gInteger &n)
{
  mpq_init(m_value);
  mpq_set_str(m_value, ToText(n), 10);
}
#else
gRational::gRational(const gInteger &n) 
  : num(n), den(&_OneRep) 
{ }
#endif  // USE_GNU_MP

#if USE_GNU_MP
gRational::gRational(const gInteger &n, const gInteger &d) 
{
  mpq_init(m_value);
  mpq_set_str(m_value, ToText(n) + "/" + ToText(d), 10);
  mpq_canonicalize(m_value);
}
#else
gRational::gRational(const gInteger &n, const gInteger &d) 
  : num(n), den(d)
{
  normalize();
}
#endif  // USE_GNU_MP

#if USE_GNU_MP
gRational::gRational(const gRational &y)
{
  mpq_init(m_value);
  mpq_set(m_value, y.m_value);
}
#else
gRational::gRational(const gRational &y) 
  : num(y.num), den(y.den)
{ }
#endif  // USE_GNU_MP

gRational::~gRational() 
{
#if USE_GNU_MP
  mpq_clear(m_value);
#endif  // USE_GNU_MP
}

gRational &gRational::operator=(const gRational &y)
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
//                         gRational: Data access 
//------------------------------------------------------------------------

int sign(const gRational &x)
{
#if USE_GNU_MP
  return mpq_sgn(x.m_value);
#else
  return sign(x.num);
#endif  // USE_GNU_MP
}

gInteger gRational::GetNumerator(void) const 
{
#if USE_GNU_MP
  gInteger r;
  mpq_get_num(r.m_value, m_value);
  return r;
#else
  return num;
#endif  // USE_GNU_MP
}

gInteger gRational::GetDenominator(void) const 
{
#if USE_GNU_MP
  gInteger r;
  mpq_get_den(r.m_value, m_value);
  return r;
#else
  return den;
#endif  // USE_GNU_MP
}

gRational::operator double() const 
{
#if USE_GNU_MP
  return mpq_get_d(m_value);
#else
  return ratio(num, den); 
#endif  // USE_GNU_MP
}

void gEpsilon(gRational &v, int /*i*/)
{ v = (gRational)0; }

//------------------------------------------------------------------------
//              gRational: Procedural versions of operators
//------------------------------------------------------------------------

void add(const gRational &x, const gRational &y, gRational &r)
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
    gInteger tmp;
    mul(x.den, y.num, tmp);
    mul(x.num, y.den, r.num);
    add(r.num, tmp, r.num);
    mul(x.den, y.den, r.den);
  }
  r.normalize();
#endif  // USE_GNU_MP
}

void sub(const gRational &x, const gRational &y, gRational &r)
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
    gInteger tmp;
    mul(x.den, y.num, tmp);
    mul(x.num, y.den, r.num);
    sub(r.num, tmp, r.num);
    mul(x.den, y.den, r.den);
  }
  r.normalize();
#endif  // USE_GNU_MP
}

void mul(const gRational &x, const gRational &y, gRational &r)
{
#if USE_GNU_MP
  mpq_mul(r.m_value, x.m_value, y.m_value);
#else
  mul(x.num, y.num, r.num);
  mul(x.den, y.den, r.den);
  r.normalize();
#endif  // USE_GNU_MP
}

void div(const gRational &x, const gRational &y, gRational &r)
{
#if USE_GNU_MP
  mpq_div(r.m_value, x.m_value, y.m_value);
#else
  if (&r != &x && &r != &y) {
    mul(x.num, y.den, r.num);
    mul(x.den, y.num, r.den);
  }
  else {
    gInteger tmp;
    mul(x.num, y.den, tmp);
    mul(y.num, x.den, r.den);
    r.num = tmp;
  }
  r.normalize();
#endif  // USE_GNU_MP
}

int compare(const gRational &x, const gRational &y)
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

void gRational::negate(void)
{
#if USE_GNU_MP
  mpq_neg(m_value, m_value);
#else
  num.negate();
#endif  // USE_GNU_MP
}

//------------------------------------------------------------------------
//                   gRational: Arithmetic operators
//------------------------------------------------------------------------

gRational operator+(const gRational &x, const gRational &y) 
{
  gRational r; add(x, y, r); return r;
}

gRational operator-(const gRational &x, const gRational &y)
{
  gRational r; sub(x, y, r); return r;
}

gRational operator-(const gRational &x) 
{
  gRational r(x); r.negate();  return r;
}

gRational operator*(const gRational &x, const gRational &y)
{
  gRational r; mul(x, y, r); return r;
}

gRational operator/(const gRational &x, const gRational &y)
{
  gRational r; div(x, y, r); return r;
}

void gRational::operator+=(const gRational &y) 
{
  add(*this, y, *this);
}

void gRational::operator-=(const gRational &y) 
{
  sub(*this, y, *this);
}

void gRational::operator*=(const gRational &y) 
{
  mul(*this, y, *this);
}

void gRational::operator/=(const gRational &y) 
{
  div(*this, y, *this);
}

//------------------------------------------------------------------------
//                  gRational: Comparison operators
//------------------------------------------------------------------------

int operator==(const gRational &x, const gRational &y)
{
#if USE_GNU_MP
  return mpq_equal(x.m_value, y.m_value);
#else
  return (compare(x.num, y.num) == 0 && compare(x.den, y.den) == 0);
#endif  // USE_GNU_MP
}

int operator!=(const gRational &x, const gRational &y)
{
#if USE_GNU_MP
  return !mpq_equal(x.m_value, y.m_value);
#else
  return (compare(x.num, y.num) != 0 || compare(x.den, y.den) != 0);
#endif  // USE_GNU_MP
}

int operator<(const gRational &x, const gRational &y)
{
  return (compare(x, y) < 0); 
}

int operator<=(const gRational& x, const gRational& y)
{
  return (compare(x, y) <= 0); 
}

int operator>(const gRational& x, const gRational& y)
{
  return (compare(x, y) > 0); 
}

int operator>=(const gRational& x, const gRational& y)
{
  return (compare(x, y) >= 0); 
}

//------------------------------------------------------------------------
//                    gRational: Other operations
//------------------------------------------------------------------------

void gRational::invert(void)
{
#if USE_GNU_MP
  mpq_inv(m_value, m_value);
#else
  gInteger tmp = num;  
  num = den;  
  den = tmp;  
  int s = sign(den);
  if (s == 0) {
    error("Zero denominator.");
  }
  else if (s < 0) {
    den.negate();
    num.negate();
  }
#endif  // USE_GNU_MP
}

gRational pow(const gRational &x, const gInteger &y)
{
  long yy = y.as_long();
  return pow(x, yy);
}               

gRational abs(const gRational &x) 
{
#if USE_GNU_MP
  gRational r;
  mpq_abs(r.m_value, x.m_value);
  return r;
#else
  gRational r(x);
  if (sign(r.num) < 0) r.negate();
  return r;
#endif  // USE_GNU_MP
}

gRational sqr(const gRational &x)
{
#if USE_GNU_MP
  gRational r;
  mpq_mul(r.m_value, x.m_value, x.m_value);
  return r;
#else
  gRational r;
  mul(x.num, x.num, r.num);
  mul(x.den, x.den, r.den);
  r.normalize();
  return r;
#endif   // USE_GNU_MP
}

gRational pow(const gRational& x, long y)
{
#if USE_GNU_MP
  if (y >= 0) {
    gRational r = 1;
    for (long i = 1; i <= y; i++) {
      r *= r;
    }
    return r;
  }
  else {
    y = -y;
    gRational r = 1;
    for (long i = 1; i <= y; i++) {
      r *= r;
    }
    r.invert();
    return r;
  }
#else
  gRational r;
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
//                    gRational: Input and output
//------------------------------------------------------------------------

gOutput &operator<<(gOutput &s, const gRational &y)
{
  s << ToText(y);
  return s;
}

gInput &operator>>(gInput &f, gRational &y)
{
  char ch = ' ';
  int sign = 1;
  gInteger num = 0, denom = 1;

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

  f.unget(ch);

  y = gRational(sign * num, denom);
  return f;
}

gText ToText(const gRational &r)
{
#if USE_GNU_MP
  // This buffer size recommended by documentation in GMP
  char buffer[mpz_sizeinbase(mpq_numref(r.m_value), 10) +
	      mpz_sizeinbase(mpq_denref(r.m_value), 10) + 3];
  mpq_get_str(buffer, 10, r.m_value);
  return gText(buffer);
#else
  const int GCONVERT_BUFFER_LENGTH = 255;
  char gconvert_buffer[GCONVERT_BUFFER_LENGTH];

  strncpy(gconvert_buffer, Itoa(r.GetNumerator()), GCONVERT_BUFFER_LENGTH);
  if (r.GetDenominator() != gInteger(1)) {
    strcat(gconvert_buffer, "/");
    strncat(gconvert_buffer, Itoa(r.GetDenominator()), GCONVERT_BUFFER_LENGTH);
  }
  
  return gText(gconvert_buffer);
#endif  // USE_GNU_MP
}

gRational FromText(const gText &f, gRational &y)
{
#if USE_GNU_MP
  mpq_set_str(y.m_value, (char *) f, 10);
  mpq_canonicalize(y.m_value);
  return y;
#else
  char ch = ' ';
  int sign = 1;
  unsigned int index = 0, length = f.Length();
  gInteger num = 0, denom = 1;

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
    y = gRational(sign * num, denom);
  else
    y = gRational(sign * num);
  return y;
#endif  // USE_GNU_MP
}



