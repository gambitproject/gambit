//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of class gbtDouble: double with tolerant ==
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

/*

   The class  gbtDouble  is just like double except that operator == will 
return true when the two numbers are `close.'  (See the definition of 
operator == in double.cc for details.)  The guiding philosophy here is
that in floating point calculations that begin with integers or rationals,
and branch according to the signs of the computed numbers, the most common
error is to be tricked by roundoff error into thinking that some number 
is different from zero.

*/

#ifndef _Double_h
#if defined(__GNUG__) && !defined(__APPLE_CC__)
#pragma interface
#endif
#define _Double_h 1

#include <math.h>
#include "base/gstream.h"
// #include "objcount.h"

class gbtDouble
// : private Counted<gbtDouble>
{
protected:
  double dbl;

public:
// Constructors, and the destructor
  gbtDouble();
  gbtDouble(const double&);
  gbtDouble(const gbtDouble&);
  gbtDouble(const int& n);
  gbtDouble(const long& n);
  
  ~gbtDouble();

// Conversion
  double ToDouble() const;

// Operators
  gbtDouble&   operator =  (const gbtDouble& y);

  bool       operator == (const gbtDouble& y) const;
  bool       operator != (const gbtDouble& y) const;
  bool       operator <  (const gbtDouble& y) const;
  bool       operator <= (const gbtDouble& y) const;
  bool       operator >  (const gbtDouble& y) const;
  bool       operator >= (const gbtDouble& y) const;

  gbtDouble    operator +  (const gbtDouble& y) const;
  gbtDouble    operator -  (const gbtDouble& y) const;
  gbtDouble    operator *  (const gbtDouble& y) const;
  gbtDouble    operator /  (const gbtDouble& y) const;

  gbtDouble    operator -  ()                 const;

  void       operator += (const gbtDouble& y);
  void       operator -= (const gbtDouble& y);
  void       operator *= (const gbtDouble& y);
  void       operator /= (const gbtDouble& y);

// error reporting and object counting
  void    error(const char* msg) const;
//  inline  int static Count() { return Counted<gbtDouble>::objCount(); }

// printing
  friend gbtInput&    operator >> (gbtInput& s, gbtDouble& y);
  friend gbtOutput&   operator << (gbtOutput& s, const gbtDouble& y);

// friends outside the class
  friend int      sign(const gbtDouble& x);             // -1, 0, or +1
  friend gbtDouble  fabs(const gbtDouble& x);              
  friend gbtDouble  abs(const gbtDouble& x);              
  friend gbtDouble  sqr(const gbtDouble& x);              
  friend gbtDouble  sqrt(const gbtDouble& x);              
  friend gbtDouble  pow(const gbtDouble& x, const long y);
  friend gbtDouble  pow(const gbtDouble& x, const gbtDouble& y);
};

// The following are facilities that are required of all arithmetic types.
// For other types they are given in gmisc.h

class gbtText;
gbtText         ToText(gbtDouble);
gbtDouble         TOgDouble(const gbtText &);

#endif
