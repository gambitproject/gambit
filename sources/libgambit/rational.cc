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
// The original copyright and license are included below.

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

#include <iostream>
#include <sstream>

#if defined(__GNUG__) && !defined(__APPLE_CC__)
#pragma implementation
#endif
#include "rational.h"
#include <math.h>
#include <float.h>
#include <assert.h>
#include <ctype.h>

void gbtRational::error(const char* msg) const
{
  //  gerr << "gbtRational class error: " << msg << '\n';
  assert(0);
}

static const gbtInteger _Int_One(1);

void gbtRational::normalize()
{
  int s = sign(den);
  if (s == 0)
    error("Zero denominator.");
  else if (s < 0)
  {
    den.negate();
    num.negate();
  }

  gbtInteger g = gcd(num, den);
  if (ucompare(g, _Int_One) != 0)
  {
    num /= g;
    den /= g;
  }
}

void      add(const gbtRational& x, const gbtRational& y, gbtRational& r)
{
  if (&r != &x && &r != &y)
  {
    mul(x.num, y.den, r.num);
    mul(x.den, y.num, r.den);
    add(r.num, r.den, r.num);
    mul(x.den, y.den, r.den);
  }
  else
  {
    gbtInteger tmp;
    mul(x.den, y.num, tmp);
    mul(x.num, y.den, r.num);
    add(r.num, tmp, r.num);
    mul(x.den, y.den, r.den);
  }
  r.normalize();
}

void      sub(const gbtRational& x, const gbtRational& y, gbtRational& r)
{
  if (&r != &x && &r != &y)
  {
    mul(x.num, y.den, r.num);
    mul(x.den, y.num, r.den);
    sub(r.num, r.den, r.num);
    mul(x.den, y.den, r.den);
  }
  else
  {
    gbtInteger tmp;
    mul(x.den, y.num, tmp);
    mul(x.num, y.den, r.num);
    sub(r.num, tmp, r.num);
    mul(x.den, y.den, r.den);
  }
  r.normalize();
}

void      mul(const gbtRational& x, const gbtRational& y, gbtRational& r)
{
  mul(x.num, y.num, r.num);
  mul(x.den, y.den, r.den);
  r.normalize();
}

void      div(const gbtRational& x, const gbtRational& y, gbtRational& r)
{
  if (&r != &x && &r != &y)
  {
    mul(x.num, y.den, r.num);
    mul(x.den, y.num, r.den);
  }
  else
  {
    gbtInteger tmp;
    mul(x.num, y.den, tmp);
    mul(y.num, x.den, r.den);
    r.num = tmp;
  }
  r.normalize();
}




void gbtRational::invert()
{
  gbtInteger tmp = num;  
  num = den;  
  den = tmp;  
  int s = sign(den);
  if (s == 0)
    error("Zero denominator.");
  else if (s < 0)
  {
    den.negate();
    num.negate();
  }
}

int compare(const gbtRational& x, const gbtRational& y)
{
  int xsgn = sign(x.num);
  int ysgn = sign(y.num);
  int d = xsgn - ysgn;
  if (d == 0 && xsgn != 0) d = compare(x.num * y.den, x.den * y.num);
  return d;
}

gbtRational::gbtRational(double x)
{
  num = 0;
  den = 1;
  if (x != 0.0)
  {
    int neg = x < 0;
    if (neg)
      x = -x;

    const long shift = 15;         // a safe shift per step
    const double width = 32768.0;  // = 2^shift
    const int maxiter = 20;        // ought not be necessary, but just in case,
                                   // max 300 bits of precision
    int expt;
    double mantissa = frexp(x, &expt);
    long exponent = expt;
    double intpart;
    int k = 0;
    while (mantissa != 0.0 && k++ < maxiter)
    {
      mantissa *= width;
      mantissa = modf(mantissa, &intpart);
      num <<= shift;
      num += (long)intpart;
      exponent -= shift;
    }
    if (exponent > 0)
      num <<= exponent;
    else if (exponent < 0)
      den <<= -exponent;
    if (neg)
      num.negate();
  }
  normalize();
}


gbtInteger trunc(const gbtRational& x)
{
  return x.num / x.den ;
}


gbtRational pow(const gbtRational& x, const gbtInteger& y)
{
  long yy = y.as_long();
  return pow(x, yy);
}               

gbtRational gbtRational::operator-(void) const
{
  gbtRational r(*this); r.negate(); return r;
}

gbtRational abs(const gbtRational& x) 
{
  gbtRational r(x);
  if (sign(r.num) < 0) r.negate();
  return r;
}


gbtRational sqr(const gbtRational& x)
{
  gbtRational r;
  mul(x.num, x.num, r.num);
  mul(x.den, x.den, r.den);
  r.normalize();
  return r;
}

gbtInteger floor(const gbtRational& x)
{
  gbtInteger q;
  gbtInteger r;
  divide(x.num, x.den, q, r);
  if (sign(x.num) < 0 && sign(r) != 0) --q;
  return q;
}

gbtInteger ceil(const gbtRational& x)
{
  gbtInteger q;
  gbtInteger  r;
  divide(x.num, x.den, q, r);
  if (sign(x.num) >= 0 && sign(r) != 0) ++q;
  return q;
}

gbtInteger round(const gbtRational& x) 
{
  gbtInteger q;
  gbtInteger r;
  divide(x.num, x.den, q, r);
  r <<= 1;
  if (ucompare(r, x.den) >= 0)
  {
    if (sign(x.num) >= 0)
      ++q;
    else
      --q;
  }
  return q;
}

gbtRational pow(const gbtRational& x, long y)
{
  gbtRational r;
  if (y >= 0)
  {
    pow(x.num, y, r.num);
    pow(x.den, y, r.den);
  }
  else
  {
    y = -y;
    pow(x.num, y, r.den);
    pow(x.den, y, r.num);
    if (sign(r.den) < 0)
    {
      r.num.negate();
      r.den.negate();
    }
  }
  return r;
}

std::ostream &operator << (std::ostream &s, const gbtRational& y)
{
  if (y.denominator() == 1L)
    s << y.numerator();
  else
  {
    s << y.numerator();
    s << "/";
    s << y.denominator();
  }
  return s;
}

std::istream &operator>>(std::istream &f, gbtRational &y)
{
  char ch = ' ';
  int sign = 1;
  gbtInteger num = 0, denom = 1;

  while (isspace(ch)) {
    f.get(ch);
    if (f.eof())  return f;
  }
  
  if (ch == '-')  { 
    sign = -1;
    f.get(ch);
  }
  
  while (ch >= '0' && ch <= '9')   {
    num *= 10;
    num += (int) (ch - '0');
    f.get(ch);
  }
  
  if (ch == '/')  {
    denom = 0;
    f.get(ch);
    while (ch >= '0' && ch <= '9')  {
      denom *= 10;
      denom += (int) (ch - '0');
      f.get(ch);
    }
  }
  else if (ch == '.')  {
    denom = 1;
    f.get(ch);
    while (ch >= '0' && ch <= '9')  {
      denom *= 10;
      num *= 10;
      num += (int) (ch - '0');
      f.get(ch);
    }
  }

  f.unget();

  y = gbtRational(sign * num, denom);
  y.normalize();

  return f;
}

int gbtRational::OK() const
{
  int v = num.OK() && den.OK(); // have valid num and denom
  if (v)
    {
      v &= sign(den) > 0;           // denominator positive;
      v &=  ucompare(gcd(num, den), _Int_One) == 0; // relatively prime
    }
  if (!v) error("invariant failure");
  return v;
}

int
gbtRational::fits_in_float() const
{
    return gbtRational (FLT_MIN) <= *this && *this <= gbtRational (FLT_MAX);
}

int
gbtRational::fits_in_double() const
{
    return gbtRational (DBL_MIN) <= *this && *this <= gbtRational (DBL_MAX);
}


//
// These were moved from the header file to eliminate warnings
//

gbtRational::gbtRational() : num(&_ZeroRep), den(&_OneRep) {}
gbtRational::~gbtRational() {}

gbtRational::gbtRational(const gbtRational& y) :num(y.num), den(y.den) {}

gbtRational::gbtRational(const gbtInteger& n) :num(n), den(&_OneRep) {}

gbtRational::gbtRational(const gbtInteger& n, const gbtInteger& d) :num(n),den(d)
{
  normalize();
}

gbtRational::gbtRational(long n) :num(n), den(&_OneRep) { }

gbtRational::gbtRational(int n) :num(n), den(&_OneRep) { }

gbtRational::gbtRational(long n, long d) :num(n), den(d) { normalize(); }
gbtRational::gbtRational(int n, int d) :num(n), den(d) { normalize(); }
gbtRational::gbtRational(long n, unsigned long d) :num(n), den(d)
{
  normalize();
}
gbtRational::gbtRational(unsigned long n, long d) :num(n), den(d)
{
  normalize();
}
gbtRational::gbtRational(unsigned long n, unsigned long d) :num(n), den(d)
{
  normalize();
}

gbtRational &gbtRational::operator =  (const gbtRational& y)
{
  num = y.num;  den = y.den;   return *this;
}

bool gbtRational::operator==(const gbtRational &y) const
{
  return compare(num, y.num) == 0 && compare(den, y.den) == 0;
}

bool gbtRational::operator!=(const gbtRational &y) const
{
  return compare(num, y.num) != 0 || compare(den, y.den) != 0;
}

bool gbtRational::operator< (const gbtRational &y) const
{
  return compare(*this, y) <  0; 
}

bool gbtRational::operator<=(const gbtRational &y) const
{
  return compare(*this, y) <= 0; 
}

bool gbtRational::operator> (const gbtRational &y) const
{
  return compare(*this, y) >  0; 
}

bool gbtRational::operator>=(const gbtRational &y) const
{
  return compare(*this, y) >= 0; 
}

int sign(const gbtRational& x)
{
  return sign(x.num);
}

void gbtRational::negate()
{
  num.negate();
}


gbtRational &gbtRational::operator+=(const gbtRational& y) 
{
  add(*this, y, *this);
  return *this;
}

gbtRational &gbtRational::operator-=(const gbtRational& y) 
{
  sub(*this, y, *this);
  return *this;
}

gbtRational &gbtRational::operator*=(const gbtRational& y) 
{
  mul(*this, y, *this);
  return *this;
}

gbtRational &gbtRational::operator/=(const gbtRational& y) 
{
  div(*this, y, *this);
  return *this;
}

const gbtInteger& gbtRational::numerator() const { return num; }
const gbtInteger& gbtRational::denominator() const { return den; }
gbtRational::operator double() const { return ratio(num, den); }

gbtRational gbtRational::operator+(const gbtRational &y) const
{
  gbtRational r; add(*this, y, r); return r;
}

gbtRational gbtRational::operator-(const gbtRational &y) const
{
  gbtRational r; sub(*this, y, r); return r;
}

gbtRational gbtRational::operator*(const gbtRational &y) const
{
  gbtRational r; mul(*this, y, r); return r;
}

gbtRational gbtRational::operator/(const gbtRational &y) const
{
  gbtRational r; div(*this, y, r); return r;
}


std::string ToText(const gbtRational &r)
{
  std::string ret;
  ret += Itoa(r.numerator());
  if (r.denominator() != gbtInteger(1)) {
    ret += "/";
    ret += Itoa(r.denominator());
  }
  
  return ret;
}

gbtRational ToRational(const std::string &f)
{
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

  if (denom != 0) {
    return gbtRational(sign * num, denom);
  }
  else {
    return gbtRational(sign * num);
  }
}

void gEpsilon(gbtRational &v, int /* i */)
{ v = (gbtRational)0;}


gbtRational ToNumber(const std::string &p_string)
{
  if (p_string.find('.') != -1 || p_string.find('e') != -1) {
    std::istringstream st(p_string);
    double d;
    st >> d;
    return gbtRational(d);
  }
  else {
    return ToRational(p_string);
  }
}

