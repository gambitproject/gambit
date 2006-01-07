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

namespace Gambit {

void Rational::error(const char* msg) const
{
  //  gerr << "Rational class error: " << msg << '\n';
  assert(0);
}

static const Integer _Int_One(1);

void Rational::normalize()
{
  int s = sign(den);
  if (s == 0)
    error("Zero denominator.");
  else if (s < 0)
    {
      den.negate();
      num.negate();
    }

  Integer g = gcd(num, den);
  if (ucompare(g, _Int_One) != 0)
    {
      num /= g;
      den /= g;
    }
}

void      add(const Rational& x, const Rational& y, Rational& r)
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
      Integer tmp;
      mul(x.den, y.num, tmp);
      mul(x.num, y.den, r.num);
      add(r.num, tmp, r.num);
      mul(x.den, y.den, r.den);
    }
  r.normalize();
}

void      sub(const Rational& x, const Rational& y, Rational& r)
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
      Integer tmp;
      mul(x.den, y.num, tmp);
      mul(x.num, y.den, r.num);
      sub(r.num, tmp, r.num);
      mul(x.den, y.den, r.den);
    }
  r.normalize();
}

void      mul(const Rational& x, const Rational& y, Rational& r)
{
  mul(x.num, y.num, r.num);
  mul(x.den, y.den, r.den);
  r.normalize();
}

void      div(const Rational& x, const Rational& y, Rational& r)
{
  if (&r != &x && &r != &y)
    {
      mul(x.num, y.den, r.num);
      mul(x.den, y.num, r.den);
    }
  else
    {
      Integer tmp;
      mul(x.num, y.den, tmp);
      mul(y.num, x.den, r.den);
      r.num = tmp;
    }
  r.normalize();
}




void Rational::invert()
{
  Integer tmp = num;  
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

int compare(const Rational& x, const Rational& y)
{
  int xsgn = sign(x.num);
  int ysgn = sign(y.num);
  int d = xsgn - ysgn;
  if (d == 0 && xsgn != 0) d = compare(x.num * y.den, x.den * y.num);
  return d;
}

Rational::Rational(double x)
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


Integer trunc(const Rational& x)
{
  return x.num / x.den ;
}


Rational pow(const Rational& x, const Integer& y)
{
  long yy = y.as_long();
  return pow(x, yy);
}               

Rational Rational::operator-(void) const
{
  Rational r(*this); r.negate(); return r;
}

Rational abs(const Rational& x) 
{
  Rational r(x);
  if (sign(r.num) < 0) r.negate();
  return r;
}


Rational sqr(const Rational& x)
{
  Rational r;
  mul(x.num, x.num, r.num);
  mul(x.den, x.den, r.den);
  r.normalize();
  return r;
}

Integer floor(const Rational& x)
{
  Integer q;
  Integer r;
  divide(x.num, x.den, q, r);
  if (sign(x.num) < 0 && sign(r) != 0) --q;
  return q;
}

Integer ceil(const Rational& x)
{
  Integer q;
  Integer  r;
  divide(x.num, x.den, q, r);
  if (sign(x.num) >= 0 && sign(r) != 0) ++q;
  return q;
}

Integer round(const Rational& x) 
{
  Integer q;
  Integer r;
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

Rational pow(const Rational& x, long y)
{
  Rational r;
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

std::ostream &operator << (std::ostream &s, const Rational& y)
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

std::istream &operator>>(std::istream &f, Rational &y)
{
  char ch = ' ';
  int sign = 1;
  Integer num = 0, denom = 1;

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

  y = Rational(num * sign, denom);
  y.normalize();

  return f;
}

int Rational::OK() const
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
Rational::fits_in_float() const
{
  return Rational (FLT_MIN) <= *this && *this <= Rational (FLT_MAX);
}

int
Rational::fits_in_double() const
{
  return Rational (DBL_MIN) <= *this && *this <= Rational (DBL_MAX);
}


//
// These were moved from the header file to eliminate warnings
//

Rational::Rational() : num(&_ZeroRep), den(&_OneRep) {}
Rational::~Rational() {}

Rational::Rational(const Rational& y) :num(y.num), den(y.den) {}

Rational::Rational(const Integer& n) :num(n), den(&_OneRep) {}

Rational::Rational(const Integer& n, const Integer& d) :num(n),den(d)
{
  normalize();
}

Rational::Rational(long n) :num(n), den(&_OneRep) { }

Rational::Rational(int n) :num(n), den(&_OneRep) { }

Rational::Rational(long n, long d) :num(n), den(d) { normalize(); }
Rational::Rational(int n, int d) :num(n), den(d) { normalize(); }
Rational::Rational(long n, unsigned long d) :num(n), den(d)
{
  normalize();
}
Rational::Rational(unsigned long n, long d) :num(n), den(d)
{
  normalize();
}
Rational::Rational(unsigned long n, unsigned long d) :num(n), den(d)
{
  normalize();
}

Rational &Rational::operator =  (const Rational& y)
{
  num = y.num;  den = y.den;   return *this;
}

bool Rational::operator==(const Rational &y) const
{
  return compare(num, y.num) == 0 && compare(den, y.den) == 0;
}

bool Rational::operator!=(const Rational &y) const
{
  return compare(num, y.num) != 0 || compare(den, y.den) != 0;
}

bool Rational::operator< (const Rational &y) const
{
  return compare(*this, y) <  0; 
}

bool Rational::operator<=(const Rational &y) const
{
  return compare(*this, y) <= 0; 
}

bool Rational::operator> (const Rational &y) const
{
  return compare(*this, y) >  0; 
}

bool Rational::operator>=(const Rational &y) const
{
  return compare(*this, y) >= 0; 
}

int sign(const Rational& x)
{
  return sign(x.num);
}

void Rational::negate()
{
  num.negate();
}


Rational &Rational::operator+=(const Rational& y) 
{
  add(*this, y, *this);
  return *this;
}

Rational &Rational::operator-=(const Rational& y) 
{
  sub(*this, y, *this);
  return *this;
}

Rational &Rational::operator*=(const Rational& y) 
{
  mul(*this, y, *this);
  return *this;
}

Rational &Rational::operator/=(const Rational& y) 
{
  div(*this, y, *this);
  return *this;
}

const Integer& Rational::numerator() const { return num; }
const Integer& Rational::denominator() const { return den; }
Rational::operator double() const { return ratio(num, den); }

Rational Rational::operator+(const Rational &y) const
{
  Rational r; add(*this, y, r); return r;
}

Rational Rational::operator-(const Rational &y) const
{
  Rational r; sub(*this, y, r); return r;
}

Rational Rational::operator*(const Rational &y) const
{
  Rational r; mul(*this, y, r); return r;
}

Rational Rational::operator/(const Rational &y) const
{
  Rational r; div(*this, y, r); return r;
}


std::string ToText(const Rational &r)
{
  std::string ret;
  ret += Itoa(r.numerator());
  if (r.denominator() != Integer(1)) {
    ret += "/";
    ret += Itoa(r.denominator());
  }
  
  return ret;
}

Rational ToRational(const std::string &f)
{
  char ch = ' ';
  int sign = 1;
  unsigned int index = 0, length = f.length();
  Integer num = 0, denom = 1;

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
    return Rational(num * sign, denom);
  }
  else {
    return Rational(num * sign);
  }
}

Rational ToNumber(const std::string &p_string)
{
  if (p_string.find('.') != -1 || p_string.find('e') != -1) {
    std::istringstream st(p_string);
    double d;
    st >> d;
    return Rational(d);
  }
  else {
    return ToRational(p_string);
  }
}

}  // end namespace Gambit
