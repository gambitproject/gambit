//
// FILE: double.cc -- Implementation of gDouble: double with generous ==
//
// $Id$
// 

#ifdef __GNUG__
#pragma implementation
#endif

#include "double.h"
#include "gnulib.h"
#include "gvector.imp"
#include "garray.imp"
#include "grarray.imp"
#include "gmatrix.imp"
#include "gsmatrix.imp"
#include "gblock.imp"
#include "monomial.imp"
#include "glist.imp"
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
//                               Conversion
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//                               operators
//--------------------------------------------------------------------------

double gDouble::ToDouble() const
{
  return dbl;
}

gDouble& gDouble::operator =  (const gDouble& y)
{
  if (this == &y) return *this;

  dbl = y.dbl;   
  return *this;
}

bool gDouble::operator == (const gDouble& y) const
{
  const double epsilon = 0.000000001;  // This, and entire routine, are malleable

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

gDouble gDouble::operator - () const
{
  return gDouble(-dbl);
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

gDouble sqrt(const gDouble& x)
{
  return gDouble(sqrt(x.dbl));
}

gDouble pow(const gDouble& x, const long y)
{
  return gDouble(pow(x.dbl, y));
}

gDouble pow(const gDouble& x, const gDouble& y)
{
  return gDouble(pow(x.dbl, y.dbl));
}

#include <stdio.h>
#include "gstring.h"


#define MYGCONVERT_BUFFER_LENGTH     64
char mygconvert_buffer[MYGCONVERT_BUFFER_LENGTH];
int myprecision = 2;

gString ToString(gDouble d)
{
  sprintf(mygconvert_buffer, "%.*f", myprecision, d.ToDouble());
  return gString(mygconvert_buffer);
}

// conversions from strings to numbers

gDouble TOgDouble(const gString &s)
{ 
  double d = strtod(s, NULL);
  gDouble answer(d);
  return answer; 
}

template class gVector<gDouble>;
template class gArray<gDouble>;
template class gBlock<gDouble>;
template class gRectArray<gDouble>;
template class gMatrix<gDouble>;
template class gSquareMatrix<gDouble>;
template gOutput & operator<< (gOutput&, const gVector<gDouble>&);

#ifndef GDOUBLE

template class gMono<gDouble>;
template gOutput & operator<< (gOutput&, const gMono<gDouble>&);

template class gList<gMono<gDouble> >;
template class gNode<gMono<gDouble> >;
#endif  // ! GDouble

/*
#include "gmisc.cc"
template gDouble gmin(const gDouble &a, const gDouble &b);
template gDouble gmax(const gDouble &a, const gDouble &b);
*/

/*
#include "objcount.imp"

class gDouble;
template class Counted<gDouble>;
int Counted<gDouble>::numObjects = 0;
*/
