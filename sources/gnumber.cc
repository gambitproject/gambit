//
// FILE: gnumber.cc -- Number class for Gambit project. 
//
// $Id$
//

#ifdef __GNUG__
#pragma implementation
#endif
#include "gstream.h"
#include "gnumber.h"
#include "gmisc.h"
#include <math.h>
#include <values.h>
#include <float.h>
#include <assert.h>
#include <ctype.h>

#include "gtext.h"

#ifdef USE_EXCEPTIONS
gText gNumber::DivideByZero::Description(void) const
{ return "Divide by zero in gNumber"; }
#endif   // USE_EXCEPTIONS


gNumber::gNumber(void) 
  : rep(precRATIONAL), rval(new gRational)
{ }

gNumber::gNumber(double x)
  : rep(precDOUBLE), rval(0), dval(x)
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
  : rep(y.rep), rval((y.rval) ? new gRational(*y.rval) : 0), dval(y.dval)
{ }

gNumber::~gNumber() 
{
  if (rval)  delete rval;
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


gNumber operator-(const gNumber &x)
{
  gNumber r(x);
  if (x.rep == precDOUBLE)
    r.dval = -x.dval;
  else
    *r.rval = -*x.rval;
  return r;
}

gOutput& operator << (gOutput& s, const gNumber& x)
{
  if (x.GetPrecision() == precDOUBLE)
    s << x.dval;
  else
    s << *x.rval;
  return s;
}

  //  Basically identical to the rational >> operator, but sets y to be 
  // either a double or rational depending on input.
gInput& operator >> (gInput& f, gNumber& y)
{
  char ch = ' ';
  int sign = 1;
  bool isDouble = false;
  gInteger num = 0, denom = 1;
  long old_pos = f.getpos();

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

Precision gNumber::GetPrecision(void) const
{
  return rep;
}

  // Comparison operators
bool operator==(const gNumber& x, const gNumber& y)
{
  if (x.rep != y.rep)   return false;
  if (x.rep == precDOUBLE)
    return x.dval == y.dval;
  else 
    return *x.rval == *y.rval;
}

bool operator!=(const gNumber& x, const gNumber& y)
{
  if (x.rep != y.rep)   return true;
  if (x.rep == precDOUBLE)
    return x.dval != y.dval;
  else
    return *x.rval != *y.rval;
}

bool operator<(const gNumber& x, const gNumber& y)
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

bool operator<=(const gNumber& x, const gNumber& y)
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

bool operator >  (const gNumber& x, const gNumber& y)
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

bool operator >= (const gNumber& x, const gNumber& y)
{
  if (x.rep == precDOUBLE && y.rep == precDOUBLE)
    return x.dval >= y.dval;
  else if (x.rep == precRATIONAL && y.rep == precRATIONAL)
    return *(x.rval) >= *(y.rval);
  else if (x.rep == precRATIONAL && y.rep == precDOUBLE)
  {
    double eps;
    gEpsilon(eps, 8);
    return y.dval <= (double(*(x.rval)) + eps);
  }
  else // (x.rep == precDOUBLE && y.rep == precRATIONAL)
  {
    double eps;
    gEpsilon(eps, 8);
    return x.dval >= (double(*(y.rval)) - eps);
  }
}

gNumber &gNumber::operator += (const gNumber& y) 
{
  if (rep == precDOUBLE)   {
    if (y.rep == precDOUBLE)
      dval += y.dval;
    else    // if (y.rep == precRATIONAL)
      dval += double(*y.rval);
  }
  else   {  // this is a rational
    if (y.rep == precDOUBLE)   {
      dval = double(*rval) + y.dval;
      rep = precDOUBLE;
      delete rval;
      rval = 0;
    }
    else // if (y.rep == precRATIONAL;
      *rval += *y.rval;
  }
  return *this;
}

gNumber &gNumber::operator -= (const gNumber& y) 
{
  if (rep == precDOUBLE)  {
    if (y.rep == precDOUBLE)
      dval -= y.dval;
    else    // if (y.rep == precRATIONAL)
      dval -= double(*y.rval);
  }
  else  {  // this is a rational
    if (y.rep == precDOUBLE)  {
      dval = double(*rval) - y.dval;
      rep = precDOUBLE;
      delete rval;
      rval = 0;
    }
    else    // if (y.rep == precRATIONAL;
      *rval -= *y.rval;
  }
  return *this;
}

gNumber &gNumber::operator *= (const gNumber& y) 
{
  if (rep == precDOUBLE)   {
    if (y.rep == precDOUBLE)
      dval *= y.dval;
    else    // if (y.rep == precRATIONAL)
      dval *= double(*y.rval);
  }
  else   {  // this is a rational
    if (y.rep == precDOUBLE)  {
      dval = double(*rval) * y.dval;
      rep = precDOUBLE;
      delete rval;
      rval = 0;
    }
    else    // if (y.rep == precRATIONAL)
      *rval *= *y.rval;
  }
  return *this;
}

gNumber &gNumber::operator/=(const gNumber& y) 
{
  if (rep == precDOUBLE)   {
    if (y.rep == precDOUBLE)  {
#ifdef USE_EXCEPTIONS
      if (y.dval == 0.0)   throw DivideByZero();
#endif   // USE_EXCEPTIONS
      dval /= y.dval;
    }
    else   {   // if (y.rep == precRATIONAL)
#ifdef USE_EXCEPTIONS
      if (*y.rval == gRational(0))     throw DivideByZero();
#endif   // USE_EXCEPTIONS
      dval /= double(*y.rval);
    }
  }
  else  {   // this is a rational 
    if (y.rep == precDOUBLE)  {
#ifdef USE_EXCEPTIONS
      if (y.dval == 0.0)   throw DivideByZero();
#endif   // USE_EXCEPTIONS
      dval = double(*rval) / y.dval;
      rep = precDOUBLE;
      delete rval;
      rval = 0;
    }
    else   {  // if (y.rep == precRATIONAL)
#ifdef USE_EXCEPTIONS
      if (*y.rval == gRational(0))    throw DivideByZero();
#endif   // USE_EXCEPTIONS
      *rval /= *y.rval;
    }
  }
  return *this;
}

gNumber operator + (const gNumber& x, const gNumber& y) 
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

gNumber operator-(const gNumber& x, const gNumber& y)
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

gNumber operator*(const gNumber& x, const gNumber& y)
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

gNumber operator/(const gNumber& x, const gNumber& y)
{
  if (x.rep == precDOUBLE)   {
    if (y.rep == precDOUBLE)   {
#ifdef USE_EXCEPTIONS
      if (y.dval == 0.0)    throw gNumber::DivideByZero();
#endif   // USE_EXCEPTIONS
      return gNumber(x.dval / y.dval); 
    }
    else  {   // if (y.rep == precRATIONAL)
#ifdef USE_EXCEPTIONS
      if (*y.rval == gRational(0))    throw gNumber::DivideByZero();
#endif   // USE_EXCEPTIONS
      return gNumber(x.dval / double(*y.rval));
    }
  }
  else   {
    if (y.rep == precDOUBLE)   {
#ifdef USE_EXCEPTIONS
      if (y.dval == 0.0)   throw gNumber::DivideByZero();
#endif   // USE_EXCEPTIONS
      return gNumber(double(*x.rval) / y.dval);
    }
    else  { // if (y.rep == precRATIONAL)
#ifdef USE_EXCEPTIONS
      if (*y.rval == gRational(0))   throw gNumber::DivideByZero();
#endif   // USE_EXCEPTIONS
      return gNumber(*x.rval / *y.rval);
    }
  }
}

bool gNumber::IsInteger(void) const
{
  return ((rep == precDOUBLE && fmod(dval, 1.0) == 0.0) ||
	  (rep == precRATIONAL && rval->denominator() == 1));
}

gNumber pow(const gNumber &x, long n)
{
if(x.rep==precDOUBLE)
  return pow(x.dval,n);
else
  return pow(*x.rval,n);
}
