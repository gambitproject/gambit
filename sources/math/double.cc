//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of gDouble: double with generous ==
//

#ifdef __GNUG__
#pragma implementation
#endif

#include <math.h>
#include <values.h>
#include <float.h>
#include <assert.h>
#include <ctype.h>

#include "math/double.h"
#include "math/gvector.imp"
#include "base/garray.imp"
#include "base/gblock.imp"
#include "base/grarray.imp"
#include "math/gmatrix.imp"
#include "base/glist.imp"

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

gDouble abs(const gDouble& x) 
{
  return gDouble(fabs(x.dbl));
}

gDouble sqr(const gDouble& x)
{
  return gDouble(x.dbl * x.dbl);
}

gDouble sqrt(const gDouble& x)
{
  return gDouble(sqrt(x.dbl));
}

gDouble pow(const gDouble& x, const long y)
{
  return gDouble(pow(x.dbl, (double)y));
}

gDouble pow(const gDouble& x, const gDouble& y)
{
  return gDouble(pow(x.dbl, y.dbl));
}

#define MYGCONVERT_BUFFER_LENGTH     64
char mygconvert_buffer[MYGCONVERT_BUFFER_LENGTH];
int myprecision = 2;

gText ToText(gDouble d)
{
  sprintf(mygconvert_buffer, "%.*f", myprecision, d.ToDouble());
  return gText(mygconvert_buffer);
}

// conversions from strings to numbers

gDouble TOgDouble(const gText &s)
{ 
  double d = strtod(s, NULL);
  gDouble answer(d);
  return answer; 
}

template class gList<gDouble>;
template class gVector<gDouble>;
template class gArray<gDouble>;
template class gBlock<gDouble>;
template class gRectArray<gDouble>;
template class gMatrix<gDouble>;
template gOutput & operator<< (gOutput&, const gVector<gDouble>&);

