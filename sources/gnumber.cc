//
// FILE: gnumber.cc -- Number class for Gambit project. 
//
// @(#)gnumber.cc	2.7 01/18/98
//

#ifdef __GNUG__
#pragma implementation
#endif
#include "gambitio.h"
#include "gnumber.h"
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

gNumber::gNumber(void) 
  : rep(precDOUBLE), rval(0), dval(0)
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

gNumber::gNumber(const gInteger& y)
  : rep(precRATIONAL), rval(new gRational(y))
{ }

gNumber::gNumber(const gRational& y)
  : rep(precRATIONAL), rval(new gRational(y))
{ }

gNumber::gNumber(const gNumber& y) 
  : rep(y.rep), rval((y.rval) ? new gRational(*y.rval) : 0), dval(y.dval)
{ }

gNumber::~gNumber() 
{
  if (rep == precRATIONAL)  delete rval;
}

gNumber &gNumber::operator=(const gNumber& y)
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


gNumber operator - (const gNumber& x) 
{
  gNumber r(x);
  if (x.rep == precDOUBLE)
    r.dval = -x.dval;
  else
    *(r.rval) = -(*(x.rval));
  return r;
}

gOutput& operator << (gOutput& s, const gNumber& x)
{
  if (x.GetPrecision() == precDOUBLE)
    s << x.ToDouble();
  else
    s <<  x.TogRational();
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
  /*geez.normalize();*/

  if (isDouble == true)
  {
    y = (double) geez;
    y.rep = precDOUBLE;
  }
  else
  {
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

gRational gNumber::TogRational(void) const
{
  if (rep == precDOUBLE)
    return gRational(dval);
  else
    return *rval;
}

double gNumber::ToDouble(void) const
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
bool operator == (const gNumber& x, const gNumber& y)
{
  if (x.rep == precDOUBLE && y.rep == precDOUBLE)
    return x.dval == y.dval;
  else if (x.rep == precRATIONAL && y.rep == precRATIONAL)
    return *(x.rval) == *(y.rval);
  else if (x.rep == precRATIONAL && y.rep == precDOUBLE)
  {
    return false;
//    double eps;
//    gEpsilon(eps, 8);
//    return (   (y.dval <= double(*(x.rval)) + eps)
//            && (y.dval >= double(*(x.rval)) - eps) );
  }
  else // (x.rep == precDOUBLE && y.rep == precRATIONAL)
  {
    return false;
//    double eps;
//    gEpsilon(eps, 8);
//    return (   (x.dval <= double(*(y.rval)) + eps)
//            && (x.dval >= double(*(y.rval)) - eps) );
  }
}

bool operator != (const gNumber& x, const gNumber& y)
{
  if (x.rep == precDOUBLE && y.rep == precDOUBLE)
    return x.dval != y.dval;
  else if (x.rep == precRATIONAL && y.rep == precRATIONAL)
    return *(x.rval) != *(y.rval);
  else if (x.rep == precRATIONAL && y.rep == precDOUBLE)
  {
    return true;
//    double eps;
//    gEpsilon(eps, 8);
//    return (   (y.dval >= double(*(x.rval)) + eps)
//            || (y.dval <= double(*(x.rval)) - eps) );
  }
  else // (x.rep == precDOUBLE && y.rep == precRATIONAL)
  {
    return true;
//    double eps;
//    gEpsilon(eps, 8);
//    return (   (x.dval >= double(*(y.rval)) + eps)
//            || (x.dval <= double(*(y.rval)) - eps) );
  }
}

bool operator <  (const gNumber& x, const gNumber& y)
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

bool operator <= (const gNumber& x, const gNumber& y)
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
  if (rep == precDOUBLE)
  {
    if (y.rep == precDOUBLE)
      dval += y.dval;
    else // if (y.rep == precRATIONAL;
      dval += double(*(y.rval));
  }
  else  // this is a rational
  {
    if (y.rep == precDOUBLE)
    {
      double temp = y.dval;
      temp += double(*rval);
      rep = precDOUBLE;
      dval = temp;
      rval = 0;
    }
    else // if (y.rep == precRATIONAL;
      *rval += *(y.rval);
  }
  return *this;
}

gNumber &gNumber::operator -= (const gNumber& y) 
{
  if (rep == precDOUBLE)
  {
    if (y.rep == precDOUBLE)
      dval -= y.dval;
    else // if (y.rep == precRATIONAL;
      dval -= double(*(y.rval));
  }
  else  // this is a rational
  {
    if (y.rep == precDOUBLE)
    {
      double temp = y.dval;
      temp -= double(*rval);
      rep = precDOUBLE;
      dval = temp;
      rval = 0;
    }
    else // if (y.rep == precRATIONAL;
      *rval -= *(y.rval);
  }
  return *this;
}

gNumber &gNumber::operator *= (const gNumber& y) 
{
  if (rep == precDOUBLE)
  {
    if (y.rep == precDOUBLE)
      dval *= y.dval;
    else // if (y.rep == precRATIONAL;
      dval *= double(*(y.rval));
  }
  else  // this is a rational
  {
    if (y.rep == precDOUBLE)
    {
      double temp = y.dval;
      temp *= double(*rval);
      rep = precDOUBLE;
      dval = temp;
      rval = 0;
    }
    else // if (y.rep == precRATIONAL;
      *rval *= *(y.rval);
  }
  return *this;
}

gNumber &gNumber::operator /= (const gNumber& y) 
{
  if (rep == precDOUBLE)
  {
    if (y.rep == precDOUBLE)
      dval /= y.dval;
    else // if (y.rep == precRATIONAL;
      dval /= double(*(y.rval));
  }
  else  // this is a rational
  {
    if (y.rep == precDOUBLE)
    {
      double temp = y.dval;
      temp /= double(*rval);
      rep = precDOUBLE;
      dval = temp;
      rval = 0;
    }
    else // if (y.rep == precRATIONAL;
      *rval /= *(y.rval);
  }
  return *this;
}

gNumber operator + (const gNumber& x, const gNumber& y) 
{
  if (x.rep == precDOUBLE)
  {
    if (y.rep == precDOUBLE)
    {
      gNumber r(x.dval + y.dval);
      return r;
    }
    else // if (y.rep == precRATIONAL;
    {
      double temp = x.dval;
      temp += double(*(y.rval));
      gNumber r(temp);
      return r;
    }
  }
  else
  {
    if (y.rep == precDOUBLE)
    {
      double temp = y.dval;
      temp += double(*(x.rval));
      gNumber r(temp);
      return r;
    }
    else // if (y.rep == precRATIONAL;
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
  if (x.rep == precDOUBLE)
  {
    if (y.rep == precDOUBLE)
    {
      gNumber r(x.dval - y.dval);
      return r;
    }
    else // if (y.rep == precRATIONAL;
    {
      double temp = x.dval;
      temp -= double(*(y.rval));
      gNumber r(temp);
      return r;
    }
  }
  else
  {
    if (y.rep == precDOUBLE)
    {
      double temp = -y.dval;
      temp += double(*(x.rval));
      gNumber r(temp);
      return r;
    }
    else // if (y.rep == precRATIONAL;
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
  if (x.rep == precDOUBLE)
  {
    if (y.rep == precDOUBLE)
    {
      gNumber r(x.dval * y.dval);
      return r;
    }
    else // if (y.rep == precRATIONAL;
    {
      double temp = x.dval;
      temp *= double(*(y.rval));
      gNumber r(temp);
      return r;
    }
  }
  else
  {
    if (y.rep == precDOUBLE)
    {
      double temp = y.dval;
      temp *= double(*(x.rval));
      gNumber r(temp);
      return r;
    }
    else // if (y.rep == precRATIONAL;
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
  if (x.rep == precDOUBLE)
  {
    if (y.rep == precDOUBLE)
    {
      gNumber r(x.dval / y.dval);
      return r;
    }
    else // if (y.rep == precRATIONAL;
    {
      double temp = x.dval;
      temp /= double(*(y.rval));
      gNumber r(temp);
      return r;
    }
  }
  else
  {
    if (y.rep == precDOUBLE)
    {
//      double temp = y.dval;
//      temp /= double(*(x.rval));
      double temp = double(*(x.rval));
      temp /= y.dval;
      gNumber r(temp);
      return r;
    }
    else // if (y.rep == precRATIONAL;
    {
      gRational temp(*(x.rval));
      temp /= *(y.rval);
      gNumber r(temp);
      return r;
    }
  }
}

