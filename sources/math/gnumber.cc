//
// FILE: gnumber.cc -- Number class for Gambit project. 
//
// $Id$
//

#ifdef __GNUG__
#pragma implementation
#endif
#include <math.h>
#include <values.h>
#include <float.h>
#include <ctype.h>
#include "base/base.h"
#include "gmath.h"
#include "gnumber.h"

gText gNumber::DivideByZero::Description(void) const
{ return "Divide by zero in gNumber"; }

//-------------------------------------------------------------------------
//     gNumber: Constructors, Destructor, and Constructive Operators
//-------------------------------------------------------------------------

gNumber::gNumber(void) 
  : rep(precRATIONAL), rval(new gRational)
{ }

gNumber::gNumber(float x)
  : rep(precDOUBLE), dval(x)
{ }

gNumber::gNumber(double x)
  : rep(precDOUBLE), dval(x)
{ }

gNumber::gNumber(int n) 
  : rep(precRATIONAL), rval(new gRational(n))
{ }

gNumber::gNumber(long n) 
  : rep(precRATIONAL), rval(new gRational(n))
{ }

gNumber::gNumber(const gInteger &y)
  : rep(precRATIONAL), rval(new gRational(y))
{ }

gNumber::gNumber(const gRational &y)
  : rep(precRATIONAL), rval(new gRational(y))
{ }

gNumber::gNumber(const gNumber &y) 
  : rep(y.rep)
{
  if (rep == precDOUBLE)
    dval = y.dval;
  else
    rval = new gRational(*y.rval);
}

gNumber::~gNumber() 
{
  if (rep == precRATIONAL)  delete rval;
}

gNumber &gNumber::operator=(const gNumber &y)
{
  if (this != &y)  {
    if (rep == precRATIONAL)  {
      delete rval;
      rval = 0;
    }
    if (y.rep == precDOUBLE)
      dval = y.dval;  
    else
      rval = new gRational(*y.rval);
    rep = y.rep;
  }
  return *this;
}

//-------------------------------------------------------------------------
//                     gNumber: Operator overloading
//-------------------------------------------------------------------------

bool operator==(const gNumber &x, const gNumber &y)
{
  if (x.rep != y.rep)   return false;
  if (x.rep == precDOUBLE)
    return x.dval == y.dval;
  else 
    return *x.rval == *y.rval;
}

bool operator!=(const gNumber &x, const gNumber &y)
{
  if (x.rep != y.rep)   return true;
  if (x.rep == precDOUBLE)
    return x.dval != y.dval;
  else
    return *x.rval != *y.rval;
}

bool operator<(const gNumber &x, const gNumber &y)
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

bool operator<=(const gNumber &x, const gNumber &y)
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

bool operator>(const gNumber &x, const gNumber &y)
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

bool operator>=(const gNumber &x, const gNumber &y)
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

gNumber &gNumber::operator+=(const gNumber &y) 
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

gNumber &gNumber::operator-=(const gNumber &y) 
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

gNumber &gNumber::operator*=(const gNumber &y) 
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

gNumber &gNumber::operator/=(const gNumber &y) 
{
  if (rep == precDOUBLE)   {
    if (y.rep == precDOUBLE)  {
      if (y.dval == 0.0)   throw DivideByZero();
      dval /= y.dval;
    }
    else   {   // if (y.rep == precRATIONAL)
      if (*y.rval == gRational(0))     throw DivideByZero();
      dval /= double(*y.rval);
    }
  }
  else  {   // this is a rational 
    if (y.rep == precDOUBLE)  {
      if (y.dval == 0.0)   throw DivideByZero();
      double dtmp = double(*rval) / y.dval;
      rep = precDOUBLE;
      delete rval;
      dval = dtmp; 
    }
    else   {  // if (y.rep == precRATIONAL)
      if (*y.rval == gRational(0))    throw DivideByZero();
      *rval /= *y.rval;
    }
  }
  return *this;
}

gNumber operator+(const gNumber &x, const gNumber &y) 
{
  if (x.rep == precDOUBLE)   {
    if (y.rep == precDOUBLE)
      return gNumber(x.dval + y.dval);
    else   // if (y.rep == precRATIONAL)
      return gNumber(x.dval + double(*y.rval));
  }
  else   {
    if (y.rep == precDOUBLE)  
      return gNumber(double(*x.rval) + y.dval);
    else   // if (y.rep == precRATIONAL)
      return gNumber(*x.rval + *y.rval);
  }
}

gNumber operator-(const gNumber &x, const gNumber &y)
{
  if (x.rep == precDOUBLE)   {
    if (y.rep == precDOUBLE)
      return gNumber(x.dval - y.dval);
    else   // if (y.rep == precRATIONAL)
      return gNumber(x.dval - double(*y.rval));
  }
  else  {
    if (y.rep == precDOUBLE)
      return gNumber(double(*x.rval) - y.dval);
    else   // if (y.rep == precRATIONAL)
      return gNumber(*x.rval - *y.rval);
  }
}

gNumber operator*(const gNumber &x, const gNumber &y)
{
  if (x.rep == precDOUBLE)  {
    if (y.rep == precDOUBLE)
      return gNumber(x.dval * y.dval);
    else    // if (y.rep == precRATIONAL)
      return gNumber(x.dval * double(*y.rval));
  }
  else  {
    if (y.rep == precDOUBLE)  
      return gNumber(double(*x.rval) * y.dval);
    else    // if (y.rep == precRATIONAL\)
      return gNumber(*x.rval * *y.rval);
  }
}

gNumber operator/(const gNumber &x, const gNumber &y)
{
  if (x.rep == precDOUBLE)   {
    if (y.rep == precDOUBLE)   {
      if (y.dval == 0.0)    throw gNumber::DivideByZero();
      return gNumber(x.dval / y.dval); 
    }
    else  {   // if (y.rep == precRATIONAL)
      if (*y.rval == gRational(0))    throw gNumber::DivideByZero();
      return gNumber(x.dval / double(*y.rval));
    }
  }
  else   {
    if (y.rep == precDOUBLE)   {
      if (y.dval == 0.0)   throw gNumber::DivideByZero();
      return gNumber(double(*x.rval) / y.dval);
    }
    else  { // if (y.rep == precRATIONAL)
      if (*y.rval == gRational(0))   throw gNumber::DivideByZero();
      return gNumber(*x.rval / *y.rval);
    }
  }
}

gNumber operator-(const gNumber &x)
{
  gNumber r(x);
  if (x.rep == precDOUBLE)
    r.dval = -x.dval;
  else
    *r.rval = -*x.rval;
  return r;
}

gOutput& operator<<(gOutput& s, const gNumber &x)
{
  if (x.Precision() == precDOUBLE)
    s << x.dval;
  else
    s << *x.rval;
  return s;
}

  //  Basically identical to the rational >> operator, but sets y to be 
  // either a double or rational depending on input.
gInput& operator>>(gInput& f, gNumber &y)
{
  char ch = ' ';
  int sign = 1;
  bool isDouble = false;
  gInteger num = 0, denom = 1;
  long old_pos = f.getpos();

  while (isspace(ch))    f >> ch;
  
  if (ch != '-' && !isdigit(ch) && ch != '.') {
    f.setpos(old_pos);
    throw gFileInput::ReadFailed();
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
  gRational geez;
  geez = gRational(sign * num, denom);
  
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
//           gNumber: Miscellaneous mathematical functions 
//-------------------------------------------------------------------------

gNumber pow(const gNumber &x, long n)
{
  if (x.rep == precDOUBLE)
    return pow(x.dval, n);
  else
    return pow(*x.rval, n);
}

//-------------------------------------------------------------------------
//           gNumber: Precision-related functions and casts
//-------------------------------------------------------------------------

gNumber::operator double(void) const
{
  if (rep == precDOUBLE)
    return dval;
  else
    return (double) *rval;
}

gNumber::operator gRational(void) const
{
  if (rep == precDOUBLE)
    return gRational(dval);
  else
    return *rval;
}

bool gNumber::IsInteger(void) const
{
  return ((rep == precDOUBLE && fmod(dval, 1.0) == 0.0) ||
	  (rep == precRATIONAL && rval->denominator() == 1));
}

gText ToText(const gNumber &n)
{
  if (n.Precision() == precDOUBLE)
    return ToText((double) n);
  else
    return ToText(n.operator gRational());
}

gText ToText(const gNumber &p_number, int p_precision)
{
  if (p_number.Precision() == precDOUBLE)
    return ToText((double) p_number, p_precision);
  else
    return ToText(p_number.operator gRational());
}

// Rational if there is no decimal point
gNumber FromText(const gText &s,gNumber &n)
{
  gRational r;
  double d;
  gText tmp=s;
  if (tmp.LastOccur('.'))
    n=FromText(s,d);
  else
    n=FromText(s,r);
  return n;
}

gNumber ToNumber(const gText &p_string)
{
  gNumber tmp;
  return FromText(p_string, tmp);
}

void gEpsilon(gNumber &n, int i)
{
  if (n.Precision() == precRATIONAL)
    n = (gRational)0;
  else
    n = pow(10.0,(double)-i);
}

