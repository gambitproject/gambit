//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of class gDouble: double with tolerant ==
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

   The class  gDouble  is just like double except that operator == will 
return true when the two numbers are `close.'  (See the definition of 
operator == in double.cc for details.)  The guiding philosophy here is
that in floating point calculations that begin with integers or rationals,
and branch according to the signs of the computed numbers, the most common
error is to be tricked by roundoff error into thinking that some number 
is different from zero.

*/

#ifndef _Double_h
#ifdef __GNUG__
#pragma interface
#endif
#define _Double_h 1

#include <math.h>
#include "base/gstream.h"
// #include "objcount.h"

class gDouble
// : private Counted<gDouble>
{
protected:
  double dbl;

public:
// Constructors, and the destructor
  gDouble();
  gDouble(const double&);
  gDouble(const gDouble&);
  gDouble(const int& n);
  gDouble(const long& n);
  
  ~gDouble();

// Conversion
  double ToDouble() const;

// Operators
  gDouble&   operator =  (const gDouble& y);

  bool       operator == (const gDouble& y) const;
  bool       operator != (const gDouble& y) const;
  bool       operator <  (const gDouble& y) const;
  bool       operator <= (const gDouble& y) const;
  bool       operator >  (const gDouble& y) const;
  bool       operator >= (const gDouble& y) const;

  gDouble    operator +  (const gDouble& y) const;
  gDouble    operator -  (const gDouble& y) const;
  gDouble    operator *  (const gDouble& y) const;
  gDouble    operator /  (const gDouble& y) const;

  gDouble    operator -  ()                 const;

  void       operator += (const gDouble& y);
  void       operator -= (const gDouble& y);
  void       operator *= (const gDouble& y);
  void       operator /= (const gDouble& y);

// error reporting and object counting
  void    error(const char* msg) const;
//  inline  int static Count() { return Counted<gDouble>::objCount(); }

// printing
  friend gInput&    operator >> (gInput& s, gDouble& y);
  friend gOutput&   operator << (gOutput& s, const gDouble& y);

// friends outside the class
  friend int      sign(const gDouble& x);             // -1, 0, or +1
  friend gDouble  fabs(const gDouble& x);              
  friend gDouble  abs(const gDouble& x);              
  friend gDouble  sqr(const gDouble& x);              
  friend gDouble  sqrt(const gDouble& x);              
  friend gDouble  pow(const gDouble& x, const long y);
  friend gDouble  pow(const gDouble& x, const gDouble& y);
};

// The following are facilities that are required of all arithmetic types.
// For other types they are given in gmisc.h

class gText;
gText         ToText(gDouble);
gDouble         TOgDouble(const gText &);

#endif

