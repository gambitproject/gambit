//#
//# FILE: double.cc -- Implementation of gDouble: double with generous ==
//#
//# @(#)double.cc	1.0  2/18/96
//#

#ifdef __GNUG__
#pragma implementation
#endif

#include "double.h"
#include "gnulib.h"
#include <math.h>
#include <values.h>
#include <float.h>
#include <assert.h>
#include <ctype.h>

//--------------------------------------------------------------------------
//                              class: gDouble
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//                       constructors and a destructor
//--------------------------------------------------------------------------

gDouble::gDouble() 
: dbl(0)
{}

gDouble::gDouble(const double& x)
: dbl(x)
{}

gDouble::gDouble(const gDouble& y) 
: dbl(y.dbl)
{}

gDouble::gDouble(const int& n)
: dbl(n)
{}

gDouble::gDouble(const long& n) 
: dbl(n)
{}

gDouble::~gDouble() 
{}

//--------------------------------------------------------------------------
//                               operators
//--------------------------------------------------------------------------

gDouble &gDouble::operator =  (const gDouble& y)
{
  if (this == &y) return *this;

  dbl = y.dbl;   
  return *this;
}

bool gDouble::operator == (const gDouble& y) const
{
  const double epsilon = 0.000001;  // This, and entire routine, are malleable

  if (dbl == 0) {
    if (fabs(y.dbl) < epsilon) return true;
    else                       return false;
  }
  else if (y.dbl == 0) { 
    if (fabs(dbl) < epsilon) return true;
    else                     return false;
  }
  else {
    if ( fabs(dbl - y.dbl)/(fabs(dbl) + fabs(y.dbl)) < epsilon ||
	(fabs(dbl) < epsilon && fabs(y.dbl) < epsilon) )        return true;
    else                                                        return false;
  }
}

bool gDouble::operator != (const gDouble& y) const
{
  return !(*this == y);
}

bool gDouble::operator <  (const gDouble& y) const
{
  if (*this == y || dbl > y.dbl) return false; else return true;
}

bool gDouble::operator <= (const gDouble& y) const
{
  if (*this == y || dbl < y.dbl) return true; else return false;
}

bool gDouble::operator >  (const gDouble& y) const
{
  if (*this == y || dbl < y.dbl) return false; else return true;
}

bool gDouble::operator >= (const gDouble& y) const
{
  if (*this == y || dbl > y.dbl) return true; else return false;
}

void gDouble::operator += (const gDouble& y)
{
  *this = gDouble(dbl + y.dbl);
}

void gDouble::operator -= (const gDouble& y) 
{
  *this = gDouble(dbl - y.dbl);
}

void gDouble::operator *= (const gDouble& y) 
{
  *this = gDouble(dbl*y.dbl);
}

void gDouble::operator /= (const gDouble& y) 
{
  if (y == (gDouble)0) error("Attempt to divide by 0.");
  *this = gDouble(dbl/y.dbl);
}

gDouble gDouble::operator + (const gDouble& y) const
{
  return gDouble(dbl + y.dbl);
}

gDouble gDouble::operator - (const gDouble& y) const
{
  return gDouble(dbl - y.dbl);
}

gDouble gDouble::operator * (const gDouble& y) const
{
  return gDouble(dbl*y.dbl);
}

gDouble gDouble::operator / (const gDouble& y) const
{
  return gDouble(dbl/y.dbl);
}

//--------------------------------------------------------------------------
//                                  errors
//--------------------------------------------------------------------------

void gDouble::error(const char* msg) const
{
  gerr << "gDouble class error: " << msg << '\n';
  assert(0);
}

//--------------------------------------------------------------------------
//                            input/output
//--------------------------------------------------------------------------

gOutput& operator << (gOutput& s, const gDouble& y)
{
  s << y.dbl;
  return s;
}

gInput &operator>>(gInput &f, gDouble &y)
{
  f >> y.dbl;
  return f;
}


// FUNCTIONS OUTSIDE THE CLASS

int sign(const gDouble& x)
{
 if      (x == (gDouble)0) return 0;
 else if (x >  (gDouble)0) return 1;
 else                     return -1;
}

gDouble fabs(const gDouble& x) 
{
  return gDouble(fabs(x.dbl));
}

gDouble sqr(const gDouble& x)
{
  return gDouble(sqr(x.dbl));
}

gDouble pow(const gDouble& x, long y)
{
  return gDouble(pow(x.dbl, y));
}

