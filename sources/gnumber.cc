//
// FILE: gnumber.cc -- Number class for Gambit project. 
//
// $Id$
//

#ifdef __GNUG__
#pragma implementation
#endif
#include "gambitio.h"
#include "gnumber.h"
#include "gnulib.h"
#include "gmisc.h"
#include <math.h>
#include <values.h>
#include <float.h>
#include <assert.h>
#include <ctype.h>

void gNumber::error(const char* msg) const
{
  gerr << "gNumber class error: " << msg << '\n';
  assert(0);
}

gNumber::gNumber() 
{
  rep = DOUBLE;
  dval = 0;
  rval = 0;
}

gNumber::gNumber(double x)
{
  rep = DOUBLE;
  dval = x;
}

gNumber::gNumber(int n) 
{
  rep = RATIONAL;
  rval = new gRational(n);
}

gNumber::gNumber(long n) 
{
  rep = RATIONAL;
  rval = new gRational(n);
}

gNumber::gNumber(const gInteger& y)
{
  rep = RATIONAL;
  rval = new gRational(y);
}

gNumber::gNumber(const gRational& y)
{
  rep = RATIONAL;
  rval = new gRational(y);
}

gNumber::gNumber(const gNumber& y) 
{
  if (y.rep == RATIONAL)
  {
    rep = RATIONAL;
    rval = new gRational(*y.rval);
  }
  else if (y.rep == DOUBLE)
  {
    rep = DOUBLE;
    dval = y.dval;
    rval = 0;
  }
}

gNumber::~gNumber() 
{
  if (rep == RATIONAL)
    delete rval;
}

gNumber &gNumber::operator =  (const gNumber& y)
{
  if (rep == RATIONAL)
  {
    delete rval;
    rval = 0;
  }
  if (y.rep == DOUBLE)
    dval = y.dval;  
  else
    rval = new gRational(*y.rval);
  rep = y.rep;
  return *this;
}


gNumber operator - (const gNumber& x) 
{
  gNumber *r = new gNumber(x);
  if (x.rep == DOUBLE)
    r->dval = -x.dval;
  else
    *(r->rval) = -(*(x.rval));
  return *r;
}

gOutput& operator << (gOutput& s, const gNumber& x)
{
  if (x.GetRep() == DOUBLE)
    s << x.GetDub();
  else
    s << x.GetRat();
  return s;
}

double gNumber::GetDub() const
{
  if (rep == DOUBLE)
    return dval;
  else
    return (double) *rval;
}

gRational gNumber::GetRat() const
{
  if (rep == DOUBLE)
    return gRational(dval);
  else
    return *rval;
}

gPrecision& gNumber::GetRep() const
{
  gPrecision *r = new gPrecision(rep);
  return *r;
}

bool operator == (const gNumber& x, const gNumber& y)
{
  if (x.rep == DOUBLE && y.rep == DOUBLE)
    return x.dval == y.dval;
  else if (x.rep == RATIONAL && y.rep == RATIONAL)
    return *(x.rval) == *(y.rval);
  else if (x.rep == RATIONAL && y.rep == DOUBLE)
  {
    double eps;
    gEpsilon(eps, 8);
    return (   (y.dval <= double(*(x.rval)) + eps)
            && (y.dval >= double(*(x.rval)) - eps) );
  }
  else // (x.rep == DOUBLE && y.rep == RATIONAL)
  {
    double eps;
    gEpsilon(eps, 8);
    return (   (x.dval <= double(*(y.rval)) + eps)
            && (x.dval >= double(*(y.rval)) - eps) );
  }
}

bool operator != (const gNumber& x, const gNumber& y)
{
  if (x.rep == DOUBLE && y.rep == DOUBLE)
    return x.dval != y.dval;
  else if (x.rep == RATIONAL && y.rep == RATIONAL)
    return *(x.rval) != *(y.rval);
  else if (x.rep == RATIONAL && y.rep == DOUBLE)
  {
    double eps;
    gEpsilon(eps, 8);
    return (   (y.dval >= double(*(x.rval)) + eps)
            || (y.dval <= double(*(x.rval)) - eps) );
  }
  else // (x.rep == DOUBLE && y.rep == RATIONAL)
  {
    double eps;
    gEpsilon(eps, 8);
    return (   (x.dval >= double(*(y.rval)) + eps)
            || (x.dval <= double(*(y.rval)) - eps) );
  }
}

bool operator <  (const gNumber& x, const gNumber& y)
{
  if (x.rep == DOUBLE && y.rep == DOUBLE)
    return x.dval < y.dval;
  else if (x.rep == RATIONAL && y.rep == RATIONAL)
    return *(x.rval) < *(y.rval);
  else if (x.rep == RATIONAL && y.rep == DOUBLE)
    return y.dval > double(*(x.rval));
  else // (x.rep == DOUBLE && y.rep == RATIONAL)
    return x.dval < double(*(y.rval));
}

bool operator <= (const gNumber& x, const gNumber& y)
{
  if (x.rep == DOUBLE && y.rep == DOUBLE)
    return x.dval <= y.dval;
  else if (x.rep == RATIONAL && y.rep == RATIONAL)
    return *(x.rval) <= *(y.rval);
  else if (x.rep == RATIONAL && y.rep == DOUBLE)
  {
    double eps;
    gEpsilon(eps, 8);
    return y.dval <= (double(*(x.rval)) + eps);
  }
  else // (x.rep == DOUBLE && y.rep == RATIONAL)
  {
    double eps;
    gEpsilon(eps, 8);
    return x.dval <= (double(*(y.rval)) + eps);
  }
}

bool operator >  (const gNumber& x, const gNumber& y)
{
  if (x.rep == DOUBLE && y.rep == DOUBLE)
    return x.dval > y.dval;
  else if (x.rep == RATIONAL && y.rep == RATIONAL)
    return *(x.rval) > *(y.rval);
  else if (x.rep == RATIONAL && y.rep == DOUBLE)
    return y.dval < double(*(x.rval));
  else // (x.rep == DOUBLE && y.rep == RATIONAL)
    return x.dval > double(*(y.rval));
}

bool operator >= (const gNumber& x, const gNumber& y)
{
  if (x.rep == DOUBLE && y.rep == DOUBLE)
    return x.dval >= y.dval;
  else if (x.rep == RATIONAL && y.rep == RATIONAL)
    return *(x.rval) >= *(y.rval);
  else if (x.rep == RATIONAL && y.rep == DOUBLE)
  {
    double eps;
    gEpsilon(eps, 8);
    return y.dval >= (double(*(x.rval)) - eps);
  }
  else // (x.rep == DOUBLE && y.rep == RATIONAL)
  {
    double eps;
    gEpsilon(eps, 8);
    return x.dval >= (double(*(y.rval)) - eps);
  }
}

void gNumber::operator += (const gNumber& y) 
{
  if (rep == DOUBLE)
  {
    if (y.rep == DOUBLE)
      dval += y.dval;
    else // if (y.rep == RATIONAL;
      dval += double(*(y.rval));
  }
  else  // this is a rational
  {
    if (y.rep == DOUBLE)
    {
      double temp = y.dval;
      temp += double(*rval);
      rep = DOUBLE;
      dval = temp;
      rval = 0;
    }
    else // if (y.rep == RATIONAL;
      *rval += *(y.rval);
  }
}

void gNumber::operator -= (const gNumber& y) 
{
  if (rep == DOUBLE)
  {
    if (y.rep == DOUBLE)
      dval -= y.dval;
    else // if (y.rep == RATIONAL;
      dval -= double(*(y.rval));
  }
  else  // this is a rational
  {
    if (y.rep == DOUBLE)
    {
      double temp = y.dval;
      temp -= double(*rval);
      rep = DOUBLE;
      dval = temp;
      rval = 0;
    }
    else // if (y.rep == RATIONAL;
      *rval -= *(y.rval);
  }
}

void gNumber::operator *= (const gNumber& y) 

{
  if (rep == DOUBLE)
  {
    if (y.rep == DOUBLE)
      dval *= y.dval;
    else // if (y.rep == RATIONAL;
      dval *= double(*(y.rval));
  }
  else  // this is a rational
  {
    if (y.rep == DOUBLE)
    {
      double temp = y.dval;
      temp *= double(*rval);
      rep = DOUBLE;
      dval = temp;
      rval = 0;
    }
    else // if (y.rep == RATIONAL;
      *rval *= *(y.rval);
  }
}

void gNumber::operator /= (const gNumber& y) 
{
  if (rep == DOUBLE)
  {
    if (y.rep == DOUBLE)
      dval /= y.dval;
    else // if (y.rep == RATIONAL;
      dval /= double(*(y.rval));
  }
  else  // this is a rational
  {
    if (y.rep == DOUBLE)
    {
      double temp = y.dval;
      temp /= double(*rval);
      rep = DOUBLE;
      dval = temp;
      rval = 0;
    }
    else // if (y.rep == RATIONAL;
      *rval /= *(y.rval);
  }
}

gNumber operator + (const gNumber& x, const gNumber& y) 
{
  if (x.rep == DOUBLE)
  {
    if (y.rep == DOUBLE)
    {
      gNumber r(x.dval + y.dval);
      return r;
    }
    else // if (y.rep == RATIONAL;
    {
      double temp = x.dval;
      temp += double(*(y.rval));
      gNumber r(temp);
      return r;
    }
  }
  else
  {
    if (y.rep == DOUBLE)
    {
      double temp = y.dval;
      temp += double(*(x.rval));
      gNumber r(temp);
      return r;
    }
    else // if (y.rep == RATIONAL;
    {
      gRational temp(*(x.rval));
      temp += *(y.rval);
      gNumber r(temp);
      return r;
    }
  }
}

gNumber operator - (const gNumber& x, const gNumber& y)
{
  if (x.rep == DOUBLE)
  {
    if (y.rep == DOUBLE)
    {
      gNumber r(x.dval - y.dval);
      return r;
    }
    else // if (y.rep == RATIONAL;
    {
      double temp = x.dval;
      temp -= double(*(y.rval));
      gNumber r(temp);
      return r;
    }
  }
  else
  {
    if (y.rep == DOUBLE)
    {
      double temp = y.dval;
      temp -= double(*(x.rval));
      gNumber r(temp);
      return r;
    }
    else // if (y.rep == RATIONAL;
    {
      gRational temp(*(x.rval));
      temp -= *(y.rval);
      gNumber r(temp);
      return r;
    }
  }
}

gNumber operator * (const gNumber& x, const gNumber& y)
{
  if (x.rep == DOUBLE)
  {
    if (y.rep == DOUBLE)
    {
      gNumber r(x.dval * y.dval);
      return r;
    }
    else // if (y.rep == RATIONAL;
    {
      double temp = x.dval;
      temp *= double(*(y.rval));
      gNumber r(temp);
      return r;
    }
  }
  else
  {
    if (y.rep == DOUBLE)
    {
      double temp = y.dval;
      temp *= double(*(x.rval));
      gNumber r(temp);
      return r;
    }
    else // if (y.rep == RATIONAL;
    {
      gRational temp(*(x.rval));
      temp *= *(y.rval);
      gNumber r(temp);
      return r;
    }
  }
}

gNumber operator / (const gNumber& x, const gNumber& y)
{
  if (x.rep == DOUBLE)
  {
    if (y.rep == DOUBLE)
    {
      gNumber r(x.dval / y.dval);
      return r;
    }
    else // if (y.rep == RATIONAL;
    {
      double temp = x.dval;
      temp /= double(*(y.rval));
      gNumber r(temp);
      return r;
    }
  }
  else
  {
    if (y.rep == DOUBLE)
    {
      double temp = y.dval;
      temp /= double(*(x.rval));
      gNumber r(temp);
      return r;
    }
    else // if (y.rep == RATIONAL;
    {
      gRational temp(*(x.rval));
      temp /= *(y.rval);
      gNumber r(temp);
      return r;
    }
  }
}

