//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of gbtDouble: double with generous ==
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
//                              class: gbtDouble
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//                       constructors and a destructor
//--------------------------------------------------------------------------

gbtDouble::gbtDouble() 
: dbl(0)
{}

gbtDouble::gbtDouble(const double& x)
: dbl(x)
{}

gbtDouble::gbtDouble(const gbtDouble& y) 
: dbl(y.dbl)
{}

gbtDouble::gbtDouble(const int& n)
: dbl(n)
{}

gbtDouble::gbtDouble(const long& n) 
: dbl(n)
{}

gbtDouble::~gbtDouble() 
{}

//--------------------------------------------------------------------------
//                               Conversion
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
//                               operators
//--------------------------------------------------------------------------

double gbtDouble::ToDouble() const
{
  return dbl;
}

gbtDouble& gbtDouble::operator =  (const gbtDouble& y)
{
  if (this == &y) return *this;

  dbl = y.dbl;   
  return *this;
}

bool gbtDouble::operator == (const gbtDouble& y) const
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

bool gbtDouble::operator != (const gbtDouble& y) const
{
  return !(*this == y);
}

bool gbtDouble::operator <  (const gbtDouble& y) const
{
  if (*this == y || dbl > y.dbl) return false; else return true;
}

bool gbtDouble::operator <= (const gbtDouble& y) const
{
  if (*this == y || dbl < y.dbl) return true; else return false;
}

bool gbtDouble::operator >  (const gbtDouble& y) const
{
  if (*this == y || dbl < y.dbl) return false; else return true;
}

bool gbtDouble::operator >= (const gbtDouble& y) const
{
  if (*this == y || dbl > y.dbl) return true; else return false;
}

void gbtDouble::operator += (const gbtDouble& y)
{
  *this = gbtDouble(dbl + y.dbl);
}

void gbtDouble::operator -= (const gbtDouble& y) 
{
  *this = gbtDouble(dbl - y.dbl);
}

void gbtDouble::operator *= (const gbtDouble& y) 
{
  *this = gbtDouble(dbl*y.dbl);
}

void gbtDouble::operator /= (const gbtDouble& y) 
{
  if (y == (gbtDouble)0) error("Attempt to divide by 0.");
  *this = gbtDouble(dbl/y.dbl);
}

gbtDouble gbtDouble::operator + (const gbtDouble& y) const
{
  return gbtDouble(dbl + y.dbl);
}

gbtDouble gbtDouble::operator - (const gbtDouble& y) const
{
  return gbtDouble(dbl - y.dbl);
}

gbtDouble gbtDouble::operator * (const gbtDouble& y) const
{
  return gbtDouble(dbl*y.dbl);
}

gbtDouble gbtDouble::operator / (const gbtDouble& y) const
{
  return gbtDouble(dbl/y.dbl);
}

gbtDouble gbtDouble::operator - () const
{
  return gbtDouble(-dbl);
}

//--------------------------------------------------------------------------
//                                  errors
//--------------------------------------------------------------------------

void gbtDouble::error(const char* msg) const
{
  //  gerr << "gbtDouble class error: " << msg << '\n';
  assert(0);
}

//--------------------------------------------------------------------------
//                            input/output
//--------------------------------------------------------------------------

gbtOutput& operator << (gbtOutput& s, const gbtDouble& y)
{
  s << y.dbl;
  return s;
}

gbtInput &operator>>(gbtInput &f, gbtDouble &y)
{
  f >> y.dbl;
  return f;
}


// FUNCTIONS OUTSIDE THE CLASS

int sign(const gbtDouble& x)
{
 if      (x == (gbtDouble)0) return 0;
 else if (x >  (gbtDouble)0) return 1;
 else                     return -1;
}

gbtDouble fabs(const gbtDouble& x) 
{
  return gbtDouble(fabs(x.dbl));
}

gbtDouble abs(const gbtDouble& x) 
{
  return gbtDouble(fabs(x.dbl));
}

gbtDouble sqr(const gbtDouble& x)
{
  return gbtDouble(x.dbl * x.dbl);
}

gbtDouble sqrt(const gbtDouble& x)
{
  return gbtDouble(sqrt(x.dbl));
}

gbtDouble pow(const gbtDouble& x, const long y)
{
  return gbtDouble(pow(x.dbl, (double)y));
}

gbtDouble pow(const gbtDouble& x, const gbtDouble& y)
{
  return gbtDouble(pow(x.dbl, y.dbl));
}

#define MYGCONVERT_BUFFER_LENGTH     64
char mygconvert_buffer[MYGCONVERT_BUFFER_LENGTH];
int myprecision = 2;

gbtText ToText(gbtDouble d)
{
  sprintf(mygconvert_buffer, "%.*f", myprecision, d.ToDouble());
  return gbtText(mygconvert_buffer);
}

// conversions from strings to numbers

gbtDouble TOgDouble(const gbtText &s)
{ 
  double d = strtod(s, NULL);
  gbtDouble answer(d);
  return answer; 
}

template class gbtList<gbtDouble>;
template class gbtVector<gbtDouble>;
template class gbtArray<gbtDouble>;
template class gbtBlock<gbtDouble>;
template class gbtRectArray<gbtDouble>;
template class gbtMatrix<gbtDouble>;
template gbtOutput & operator<< (gbtOutput&, const gbtVector<gbtDouble>&);
