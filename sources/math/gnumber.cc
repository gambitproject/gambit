//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// General number class
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

#ifdef __GNUG__
#pragma implementation
#endif
#include <math.h>
#include <float.h>
#include <ctype.h>
#include "base/base.h"
#include "gmath.h"
#include "gnumber.h"

gbtText gbtNumber::DivideByZero::Description(void) const
{ return "Divide by zero in gbtNumber"; }

//-------------------------------------------------------------------------
//     gbtNumber: Constructors, Destructor, and Constructive Operators
//-------------------------------------------------------------------------

gbtNumber::gbtNumber(void) 
  : rep(GBT_PREC_RATIONAL), rval(new gbtRational)
{ }

gbtNumber::gbtNumber(float x)
  : rep(GBT_PREC_DOUBLE), dval(x)
{ }

gbtNumber::gbtNumber(double x)
  : rep(GBT_PREC_DOUBLE), dval(x)
{ }

gbtNumber::gbtNumber(int n) 
  : rep(GBT_PREC_RATIONAL), rval(new gbtRational(n))
{ }

gbtNumber::gbtNumber(long n) 
  : rep(GBT_PREC_RATIONAL), rval(new gbtRational(n))
{ }

gbtNumber::gbtNumber(const gbtInteger &y)
  : rep(GBT_PREC_RATIONAL), rval(new gbtRational(y))
{ }

gbtNumber::gbtNumber(const gbtRational &y)
  : rep(GBT_PREC_RATIONAL), rval(new gbtRational(y))
{ }

#if GBT_WITH_MP_FLOAT
gbtNumber::gbtNumber(const gbtMPFloat &y)
  : rep(GBT_PREC_DOUBLE), dval((double) y)
{ }
#endif // GBT_WITH_MP_FLOAT

gbtNumber::gbtNumber(const gbtNumber &y) 
  : rep(y.rep)
{
  if (rep == GBT_PREC_DOUBLE)
    dval = y.dval;
  else
    rval = new gbtRational(*y.rval);
}

gbtNumber::~gbtNumber() 
{
  if (rep == GBT_PREC_RATIONAL)  delete rval;
}

gbtNumber &gbtNumber::operator=(const gbtNumber &y)
{
  if (this != &y)  {
    if (rep == GBT_PREC_RATIONAL)  {
      delete rval;
      rval = 0;
    }
    if (y.rep == GBT_PREC_DOUBLE)
      dval = y.dval;  
    else
      rval = new gbtRational(*y.rval);
    rep = y.rep;
  }
  return *this;
}

//-------------------------------------------------------------------------
//                     gbtNumber: Operator overloading
//-------------------------------------------------------------------------

bool operator==(const gbtNumber &x, const gbtNumber &y)
{
  if (x.rep != y.rep)   return false;
  if (x.rep == GBT_PREC_DOUBLE)
    return x.dval == y.dval;
  else 
    return *x.rval == *y.rval;
}

bool operator!=(const gbtNumber &x, const gbtNumber &y)
{
  if (x.rep != y.rep)   return true;
  if (x.rep == GBT_PREC_DOUBLE)
    return x.dval != y.dval;
  else
    return *x.rval != *y.rval;
}

bool operator<(const gbtNumber &x, const gbtNumber &y)
{
  if (x.rep == GBT_PREC_DOUBLE && y.rep == GBT_PREC_DOUBLE)
    return x.dval < y.dval;
  else if (x.rep == GBT_PREC_RATIONAL && y.rep == GBT_PREC_RATIONAL)
    return *(x.rval) < *(y.rval);
  else if (x.rep == GBT_PREC_RATIONAL && y.rep == GBT_PREC_DOUBLE)
    return y.dval > double(*(x.rval));
  else // (x.rep == GBT_PREC_DOUBLE && y.rep == GBT_PREC_RATIONAL)
    return x.dval < double(*(y.rval));
}

bool operator<=(const gbtNumber &x, const gbtNumber &y)
{
  if (x.rep == GBT_PREC_DOUBLE && y.rep == GBT_PREC_DOUBLE)
    return x.dval <= y.dval;
  else if (x.rep == GBT_PREC_RATIONAL && y.rep == GBT_PREC_RATIONAL)
    return *(x.rval) <= *(y.rval);
  else if (x.rep == GBT_PREC_RATIONAL && y.rep == GBT_PREC_DOUBLE)
  {
    double eps;
    gEpsilon(eps, 8);
    return y.dval >= (double(*(x.rval)) - eps);
  }
  else // (x.rep == GBT_PREC_DOUBLE && y.rep == GBT_PREC_RATIONAL)
  {
    double eps;
    gEpsilon(eps, 8);
    return x.dval <= (double(*(y.rval)) + eps);
  }
}

bool operator>(const gbtNumber &x, const gbtNumber &y)
{
  if (x.rep == GBT_PREC_DOUBLE && y.rep == GBT_PREC_DOUBLE)
    return x.dval > y.dval;
  else if (x.rep == GBT_PREC_RATIONAL && y.rep == GBT_PREC_RATIONAL)
    return *(x.rval) > *(y.rval);
  else if (x.rep == GBT_PREC_RATIONAL && y.rep == GBT_PREC_DOUBLE)
    return y.dval < double(*(x.rval));
  else // (x.rep == GBT_PREC_DOUBLE && y.rep == GBT_PREC_RATIONAL)
    return x.dval > double(*(y.rval));
}

bool operator>=(const gbtNumber &x, const gbtNumber &y)
{
  if (x.rep == GBT_PREC_DOUBLE && y.rep == GBT_PREC_DOUBLE)
    return x.dval >= y.dval;
  else if (x.rep == GBT_PREC_RATIONAL && y.rep == GBT_PREC_RATIONAL)
    return *(x.rval) >= *(y.rval);
  else if (x.rep == GBT_PREC_RATIONAL && y.rep == GBT_PREC_DOUBLE) {
    double eps;
    gEpsilon(eps, 8);
    return y.dval <= (double(*(x.rval)) + eps);
  }
  else { // (x.rep == GBT_PREC_DOUBLE && y.rep == GBT_PREC_RATIONAL)
    double eps;
    gEpsilon(eps, 8);
    return x.dval >= (double(*(y.rval)) - eps);
  }
}

gbtNumber &gbtNumber::operator+=(const gbtNumber &y) 
{
  if (rep == GBT_PREC_DOUBLE)   {
    if (y.rep == GBT_PREC_DOUBLE)
      dval += y.dval;
    else    // if (y.rep == GBT_PREC_RATIONAL)
      dval += double(*y.rval);
  }
  else   {  // this is a rational
    if (y.rep == GBT_PREC_DOUBLE)   {
      double dtmp = double(*rval) + y.dval;
      rep = GBT_PREC_DOUBLE;
      delete rval;
      dval = dtmp;
    }
    else // if (y.rep == GBT_PREC_RATIONAL;
      *rval += *y.rval;
  }
  return *this;
}

gbtNumber &gbtNumber::operator-=(const gbtNumber &y) 
{
  if (rep == GBT_PREC_DOUBLE)  {
    if (y.rep == GBT_PREC_DOUBLE)
      dval -= y.dval;
    else    // if (y.rep == GBT_PREC_RATIONAL)
      dval -= double(*y.rval);
  }
  else  {  // this is a rational
    if (y.rep == GBT_PREC_DOUBLE)  {
      double dtmp = double(*rval) - y.dval;
      rep = GBT_PREC_DOUBLE;
      delete rval;
      dval = dtmp;
    }
    else    // if (y.rep == GBT_PREC_RATIONAL)
      *rval -= *y.rval;
  }
  return *this;
}

gbtNumber &gbtNumber::operator*=(const gbtNumber &y) 
{
  if (rep == GBT_PREC_DOUBLE)   {
    if (y.rep == GBT_PREC_DOUBLE)
      dval *= y.dval;
    else    // if (y.rep == GBT_PREC_RATIONAL)
      dval *= double(*y.rval);
  }
  else   {  // this is a rational
    if (y.rep == GBT_PREC_DOUBLE)  {
      double dtmp = double(*rval) * y.dval;
      rep = GBT_PREC_DOUBLE;
      delete rval;
      dval = dtmp;
    }
    else    // if (y.rep == GBT_PREC_RATIONAL)
      *rval *= *y.rval;
  }
  return *this;
}

gbtNumber &gbtNumber::operator/=(const gbtNumber &y) 
{
  if (rep == GBT_PREC_DOUBLE)   {
    if (y.rep == GBT_PREC_DOUBLE)  {
      if (y.dval == 0.0)   throw DivideByZero();
      dval /= y.dval;
    }
    else   {   // if (y.rep == GBT_PREC_RATIONAL)
      if (*y.rval == gbtRational(0))     throw DivideByZero();
      dval /= double(*y.rval);
    }
  }
  else  {   // this is a rational 
    if (y.rep == GBT_PREC_DOUBLE)  {
      if (y.dval == 0.0)   throw DivideByZero();
      double dtmp = double(*rval) / y.dval;
      rep = GBT_PREC_DOUBLE;
      delete rval;
      dval = dtmp; 
    }
    else   {  // if (y.rep == GBT_PREC_RATIONAL)
      if (*y.rval == gbtRational(0))    throw DivideByZero();
      *rval /= *y.rval;
    }
  }
  return *this;
}

gbtNumber operator+(const gbtNumber &x, const gbtNumber &y) 
{
  if (x.rep == GBT_PREC_DOUBLE)   {
    if (y.rep == GBT_PREC_DOUBLE)
      return gbtNumber(x.dval + y.dval);
    else   // if (y.rep == GBT_PREC_RATIONAL)
      return gbtNumber(x.dval + double(*y.rval));
  }
  else   {
    if (y.rep == GBT_PREC_DOUBLE)  
      return gbtNumber(double(*x.rval) + y.dval);
    else   // if (y.rep == GBT_PREC_RATIONAL)
      return gbtNumber(*x.rval + *y.rval);
  }
}

gbtNumber operator-(const gbtNumber &x, const gbtNumber &y)
{
  if (x.rep == GBT_PREC_DOUBLE)   {
    if (y.rep == GBT_PREC_DOUBLE)
      return gbtNumber(x.dval - y.dval);
    else   // if (y.rep == GBT_PREC_RATIONAL)
      return gbtNumber(x.dval - double(*y.rval));
  }
  else  {
    if (y.rep == GBT_PREC_DOUBLE)
      return gbtNumber(double(*x.rval) - y.dval);
    else   // if (y.rep == GBT_PREC_RATIONAL)
      return gbtNumber(*x.rval - *y.rval);
  }
}

gbtNumber operator*(const gbtNumber &x, const gbtNumber &y)
{
  if (x.rep == GBT_PREC_DOUBLE)  {
    if (y.rep == GBT_PREC_DOUBLE)
      return gbtNumber(x.dval * y.dval);
    else    // if (y.rep == GBT_PREC_RATIONAL)
      return gbtNumber(x.dval * double(*y.rval));
  }
  else  {
    if (y.rep == GBT_PREC_DOUBLE)  
      return gbtNumber(double(*x.rval) * y.dval);
    else    // if (y.rep == GBT_PREC_RATIONAL\)
      return gbtNumber(*x.rval * *y.rval);
  }
}

gbtNumber operator/(const gbtNumber &x, const gbtNumber &y)
{
  if (x.rep == GBT_PREC_DOUBLE)   {
    if (y.rep == GBT_PREC_DOUBLE)   {
      if (y.dval == 0.0)    throw gbtNumber::DivideByZero();
      return gbtNumber(x.dval / y.dval); 
    }
    else  {   // if (y.rep == GBT_PREC_RATIONAL)
      if (*y.rval == gbtRational(0))    throw gbtNumber::DivideByZero();
      return gbtNumber(x.dval / double(*y.rval));
    }
  }
  else   {
    if (y.rep == GBT_PREC_DOUBLE)   {
      if (y.dval == 0.0)   throw gbtNumber::DivideByZero();
      return gbtNumber(double(*x.rval) / y.dval);
    }
    else  { // if (y.rep == GBT_PREC_RATIONAL)
      if (*y.rval == gbtRational(0))   throw gbtNumber::DivideByZero();
      return gbtNumber(*x.rval / *y.rval);
    }
  }
}

gbtNumber operator-(const gbtNumber &x)
{
  gbtNumber r(x);
  if (x.rep == GBT_PREC_DOUBLE)
    r.dval = -x.dval;
  else
    *r.rval = -*x.rval;
  return r;
}

gbtOutput& operator<<(gbtOutput& s, const gbtNumber &x)
{
  if (x.Precision() == GBT_PREC_DOUBLE)
    s << x.dval;
  else
    s << *x.rval;
  return s;
}

  //  Basically identical to the rational >> operator, but sets y to be 
  // either a double or rational depending on input.
gbtInput& operator>>(gbtInput& f, gbtNumber &y)
{
  char ch = ' ';
  int sign = 1;
  bool isDouble = false;
  gbtInteger num = 0, denom = 1;
  long old_pos = f.getpos();

  while (isspace(ch))    f >> ch;
  
  if (ch != '-' && !isdigit(ch) && ch != '.') {
    f.setpos(old_pos);
    throw gbtFileInput::ReadFailed();
  }

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
    double dval;

    f.setpos(old_pos);
    f >> dval;
    y = dval;
    return f;
  }

  f.unget(ch);
  gbtRational geez;
  geez = gbtRational(sign * num, denom);
  
  if (isDouble == true)   {
    y = (double) geez;
    y.rep = GBT_PREC_DOUBLE;
  }
  else  {
    y = geez;
    y.rep = GBT_PREC_RATIONAL;
  }

  return f;
}

//-------------------------------------------------------------------------
//           gbtNumber: Miscellaneous mathematical functions 
//-------------------------------------------------------------------------

gbtNumber pow(const gbtNumber &x, long n)
{
  if (x.rep == GBT_PREC_DOUBLE)
    return pow(x.dval, n);
  else
    return pow(*x.rval, n);
}

//-------------------------------------------------------------------------
//           gbtNumber: Precision-related functions and casts
//-------------------------------------------------------------------------

gbtNumber::operator double(void) const
{
  if (rep == GBT_PREC_DOUBLE)
    return dval;
  else
    return (double) *rval;
}

gbtNumber::operator gbtRational(void) const
{
  if (rep == GBT_PREC_DOUBLE)
    return gbtRational(dval);
  else
    return *rval;
}

#if GBT_WITH_MP_FLOAT
gbtNumber::operator gbtMPFloat(void) const
{
  if (rep == GBT_PREC_DOUBLE) {
    return gbtMPFloat(dval);
  }
  else {
    return gbtMPFloat((double) *rval);
  }
}
#endif // GBT_WITH_MP_FLOAT

bool gbtNumber::IsInteger(void) const
{
  return ((rep == GBT_PREC_DOUBLE && fmod(dval, 1.0) == 0.0) ||
	  (rep == GBT_PREC_RATIONAL && rval->GetDenominator() == 1));
}

gbtText ToText(const gbtNumber &n)
{
  if (n.Precision() == GBT_PREC_DOUBLE)
    return ToText((double) n);
  else
    return ToText(n.operator gbtRational());
}

gbtText ToText(const gbtNumber &p_number, int p_precision)
{
  if (p_number.Precision() == GBT_PREC_DOUBLE)
    return ToText((double) p_number, p_precision);
  else
    return ToText(p_number.operator gbtRational());
}

// Rational if there is no decimal point
gbtNumber FromText(const gbtText &s,gbtNumber &n)
{
  gbtRational r;
  double d;
  gbtText tmp=s;
  if (tmp.LastOccur('.'))
    n=FromText(s,d);
  else
    n=FromText(s,r);
  return n;
}

gbtNumber ToNumber(const gbtText &p_string)
{
  gbtNumber tmp;
  return FromText(p_string, tmp);
}

void gEpsilon(gbtNumber &n, int i)
{
  if (n.Precision() == GBT_PREC_RATIONAL)
    n = (gbtRational)0;
  else
    n = pow(10.0,(double)-i);
}
