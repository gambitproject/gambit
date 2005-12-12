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

#include <iostream>
#include <iomanip>

#if defined(__GNUG__) && !defined(__APPLE_CC__)
#pragma implementation
#endif
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <ctype.h>
#include "libgambit.h"

//-------------------------------------------------------------------------
//     gbtNumber: Constructors, Destructor, and Constructive Operators
//-------------------------------------------------------------------------

gbtNumber::gbtNumber(void) 
  : rep(precRATIONAL), rval(new gbtRational)
{ }

gbtNumber::gbtNumber(float x)
  : rep(precDOUBLE), dval(x)
{ }

gbtNumber::gbtNumber(double x)
  : rep(precDOUBLE), dval(x)
{ }

gbtNumber::gbtNumber(int n) 
  : rep(precRATIONAL), rval(new gbtRational(n))
{ }

gbtNumber::gbtNumber(long n) 
  : rep(precRATIONAL), rval(new gbtRational(n))
{ }

gbtNumber::gbtNumber(const gbtInteger &y)
  : rep(precRATIONAL), rval(new gbtRational(y))
{ }

gbtNumber::gbtNumber(const gbtRational &y)
  : rep(precRATIONAL), rval(new gbtRational(y))
{ }

gbtNumber::gbtNumber(const gbtNumber &y) 
  : rep(y.rep)
{
  if (rep == precDOUBLE)
    dval = y.dval;
  else
    rval = new gbtRational(*y.rval);
}

gbtNumber::~gbtNumber() 
{
  if (rep == precRATIONAL)  delete rval;
}

gbtNumber &gbtNumber::operator=(const gbtNumber &y)
{
  if (this != &y)  {
    if (rep == precRATIONAL)  {
      delete rval;
      rval = 0;
    }
    if (y.rep == precDOUBLE)
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
  if (x.rep == precDOUBLE)
    return x.dval == y.dval;
  else 
    return *x.rval == *y.rval;
}

bool operator!=(const gbtNumber &x, const gbtNumber &y)
{
  if (x.rep != y.rep)   return true;
  if (x.rep == precDOUBLE)
    return x.dval != y.dval;
  else
    return *x.rval != *y.rval;
}

bool operator<(const gbtNumber &x, const gbtNumber &y)
{
  if (x.rep == precDOUBLE && y.rep == precDOUBLE)
    return x.dval < y.dval;
  else if (x.rep == precRATIONAL && y.rep == precRATIONAL)
    return *(x.rval) < *(y.rval);
  else if (x.rep == precRATIONAL && y.rep == precDOUBLE)
    return y.dval > double(*(x.rval));
  else // (x.rep == precDOUBLE && y.rep == precRATIONAL)
    return x.dval < double(*(y.rval));
}

bool operator<=(const gbtNumber &x, const gbtNumber &y)
{
  if (x.rep == precDOUBLE && y.rep == precDOUBLE)
    return x.dval <= y.dval;
  else if (x.rep == precRATIONAL && y.rep == precRATIONAL)
    return *(x.rval) <= *(y.rval);
  else if (x.rep == precRATIONAL && y.rep == precDOUBLE)
  {
    double eps;
    gEpsilon(eps, 8);
    return y.dval >= (double(*(x.rval)) - eps);
  }
  else // (x.rep == precDOUBLE && y.rep == precRATIONAL)
  {
    double eps;
    gEpsilon(eps, 8);
    return x.dval <= (double(*(y.rval)) + eps);
  }
}

bool operator>(const gbtNumber &x, const gbtNumber &y)
{
  if (x.rep == precDOUBLE && y.rep == precDOUBLE)
    return x.dval > y.dval;
  else if (x.rep == precRATIONAL && y.rep == precRATIONAL)
    return *(x.rval) > *(y.rval);
  else if (x.rep == precRATIONAL && y.rep == precDOUBLE)
    return y.dval < double(*(x.rval));
  else // (x.rep == precDOUBLE && y.rep == precRATIONAL)
    return x.dval > double(*(y.rval));
}

bool operator>=(const gbtNumber &x, const gbtNumber &y)
{
  if (x.rep == precDOUBLE && y.rep == precDOUBLE)
    return x.dval >= y.dval;
  else if (x.rep == precRATIONAL && y.rep == precRATIONAL)
    return *(x.rval) >= *(y.rval);
  else if (x.rep == precRATIONAL && y.rep == precDOUBLE) {
    double eps;
    gEpsilon(eps, 8);
    return y.dval <= (double(*(x.rval)) + eps);
  }
  else { // (x.rep == precDOUBLE && y.rep == precRATIONAL)
    double eps;
    gEpsilon(eps, 8);
    return x.dval >= (double(*(y.rval)) - eps);
  }
}

gbtNumber &gbtNumber::operator+=(const gbtNumber &y) 
{
  if (rep == precDOUBLE)   {
    if (y.rep == precDOUBLE)
      dval += y.dval;
    else    // if (y.rep == precRATIONAL)
      dval += double(*y.rval);
  }
  else   {  // this is a rational
    if (y.rep == precDOUBLE)   {
      double dtmp = double(*rval) + y.dval;
      rep = precDOUBLE;
      delete rval;
      dval = dtmp;
    }
    else // if (y.rep == precRATIONAL;
      *rval += *y.rval;
  }
  return *this;
}

gbtNumber &gbtNumber::operator-=(const gbtNumber &y) 
{
  if (rep == precDOUBLE)  {
    if (y.rep == precDOUBLE)
      dval -= y.dval;
    else    // if (y.rep == precRATIONAL)
      dval -= double(*y.rval);
  }
  else  {  // this is a rational
    if (y.rep == precDOUBLE)  {
      double dtmp = double(*rval) - y.dval;
      rep = precDOUBLE;
      delete rval;
      dval = dtmp;
    }
    else    // if (y.rep == precRATIONAL)
      *rval -= *y.rval;
  }
  return *this;
}

gbtNumber &gbtNumber::operator*=(const gbtNumber &y) 
{
  if (rep == precDOUBLE)   {
    if (y.rep == precDOUBLE)
      dval *= y.dval;
    else    // if (y.rep == precRATIONAL)
      dval *= double(*y.rval);
  }
  else   {  // this is a rational
    if (y.rep == precDOUBLE)  {
      double dtmp = double(*rval) * y.dval;
      rep = precDOUBLE;
      delete rval;
      dval = dtmp;
    }
    else    // if (y.rep == precRATIONAL)
      *rval *= *y.rval;
  }
  return *this;
}

gbtNumber &gbtNumber::operator/=(const gbtNumber &y) 
{
  if (rep == precDOUBLE)   {
    if (y.rep == precDOUBLE)  {
      if (y.dval == 0.0)   throw gbtZeroDivideException();
      dval /= y.dval;
    }
    else   {   // if (y.rep == precRATIONAL)
      if (*y.rval == gbtRational(0))     throw gbtZeroDivideException();
      dval /= double(*y.rval);
    }
  }
  else  {   // this is a rational 
    if (y.rep == precDOUBLE)  {
      if (y.dval == 0.0)   throw gbtZeroDivideException();
      double dtmp = double(*rval) / y.dval;
      rep = precDOUBLE;
      delete rval;
      dval = dtmp; 
    }
    else   {  // if (y.rep == precRATIONAL)
      if (*y.rval == gbtRational(0))    throw gbtZeroDivideException();
      *rval /= *y.rval;
    }
  }
  return *this;
}

gbtNumber operator+(const gbtNumber &x, const gbtNumber &y) 
{
  if (x.rep == precDOUBLE)   {
    if (y.rep == precDOUBLE)
      return gbtNumber(x.dval + y.dval);
    else   // if (y.rep == precRATIONAL)
      return gbtNumber(x.dval + double(*y.rval));
  }
  else   {
    if (y.rep == precDOUBLE)  
      return gbtNumber(double(*x.rval) + y.dval);
    else   // if (y.rep == precRATIONAL)
      return gbtNumber(*x.rval + *y.rval);
  }
}

gbtNumber operator-(const gbtNumber &x, const gbtNumber &y)
{
  if (x.rep == precDOUBLE)   {
    if (y.rep == precDOUBLE)
      return gbtNumber(x.dval - y.dval);
    else   // if (y.rep == precRATIONAL)
      return gbtNumber(x.dval - double(*y.rval));
  }
  else  {
    if (y.rep == precDOUBLE)
      return gbtNumber(double(*x.rval) - y.dval);
    else   // if (y.rep == precRATIONAL)
      return gbtNumber(*x.rval - *y.rval);
  }
}

gbtNumber operator*(const gbtNumber &x, const gbtNumber &y)
{
  if (x.rep == precDOUBLE)  {
    if (y.rep == precDOUBLE)
      return gbtNumber(x.dval * y.dval);
    else    // if (y.rep == precRATIONAL)
      return gbtNumber(x.dval * double(*y.rval));
  }
  else  {
    if (y.rep == precDOUBLE)  
      return gbtNumber(double(*x.rval) * y.dval);
    else    // if (y.rep == precRATIONAL\)
      return gbtNumber(*x.rval * *y.rval);
  }
}

gbtNumber operator/(const gbtNumber &x, const gbtNumber &y)
{
  if (x.rep == precDOUBLE)   {
    if (y.rep == precDOUBLE)   {
      if (y.dval == 0.0)    throw gbtZeroDivideException();
      return gbtNumber(x.dval / y.dval); 
    }
    else  {   // if (y.rep == precRATIONAL)
      if (*y.rval == gbtRational(0))    throw gbtZeroDivideException();
      return gbtNumber(x.dval / double(*y.rval));
    }
  }
  else   {
    if (y.rep == precDOUBLE)   {
      if (y.dval == 0.0)   throw gbtZeroDivideException();
      return gbtNumber(double(*x.rval) / y.dval);
    }
    else  { // if (y.rep == precRATIONAL)
      if (*y.rval == gbtRational(0))   throw gbtZeroDivideException();
      return gbtNumber(*x.rval / *y.rval);
    }
  }
}

gbtNumber operator-(const gbtNumber &x)
{
  gbtNumber r(x);
  if (x.rep == precDOUBLE)
    r.dval = -x.dval;
  else
    *r.rval = -*x.rval;
  return r;
}

std::ostream &operator<<(std::ostream &s, const gbtNumber &x)
{
  if (x.Precision() == precDOUBLE)
    s << x.dval;
  else
    s << *x.rval;
  return s;
}

  //  Basically identical to the rational >> operator, but sets y to be 
  // either a double or rational depending on input.
std::istream &operator>>(std::istream &f, gbtNumber &y)
{
  char ch = ' ';
  int sign = 1;
  bool isDouble = false;
  gbtInteger num = 0, denom = 1;
  long old_pos = f.tellg();

  while (isspace(ch))    f >> ch;
  
  if (ch != '-' && !isdigit(ch) && ch != '.') {
    f.seekg(old_pos);
    return f;
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

    f.seekg(old_pos);
    f >> dval;
    y = dval;
    return f;
  }

  f.unget();
  gbtRational geez;
  geez = gbtRational(sign * num, denom);
  
  if (isDouble == true)   {
    y = (double) geez;
    y.rep = precDOUBLE;
  }
  else  {
    y = geez;
    y.rep = precRATIONAL;
  }

  return f;
}

//-------------------------------------------------------------------------
//           gbtNumber: Miscellaneous mathematical functions 
//-------------------------------------------------------------------------

gbtNumber pow(const gbtNumber &x, long n)
{
  if (x.rep == precDOUBLE)
    return pow(x.dval, (double) n);
  else
    return pow(*x.rval, n);
}

//-------------------------------------------------------------------------
//           gbtNumber: Precision-related functions and casts
//-------------------------------------------------------------------------

gbtNumber::operator double(void) const
{
  if (rep == precDOUBLE)
    return dval;
  else
    return (double) *rval;
}

gbtNumber::operator gbtRational(void) const
{
  if (rep == precDOUBLE)
    return gbtRational(dval);
  else
    return *rval;
}

bool gbtNumber::IsInteger(void) const
{
  return ((rep == precDOUBLE && fmod(dval, 1.0) == 0.0) ||
	  (rep == precRATIONAL && rval->denominator() == 1));
}

std::string ToText(const gbtNumber &n)
{
  if (n.Precision() == precDOUBLE)
    return ToText((double) n);
  else
    return ToText(n.operator gbtRational());
}

std::string ToText(const gbtNumber &p_number, int p_precision)
{
  if (p_number.Precision() == precDOUBLE) {
    std::ostringstream s;
    s.setf(std::ios::fixed);
    s << std::setprecision(p_precision) << (double) p_number;
    return s.str();
  }
  else {
    return ToText(p_number.operator gbtRational());
  }
}

gbtNumber ToNumber(const std::string &p_string)
{
  if (p_string.find('.') != -1 || p_string.find('e') != -1) {
    std::istringstream st(p_string);
    double d;
    st >> d;
    return gbtNumber(d);
  }
  else {
    return ToRational(p_string);
  }
}

void gEpsilon(gbtNumber &n, int i)
{
  if (n.Precision() == precRATIONAL)
    n = (gbtRational)0;
  else
    n = pow(10.0,(double)-i);
}

