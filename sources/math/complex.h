//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of a complex number class
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

#ifndef _Complex_h
#if defined(__GNUG__) && !defined(__APPLE_CC__)
#pragma interface
#endif
#define _Complex_h 1

#include "base/gstream.h"
#include <math.h>

class gbtComplex
{
protected:
  double re;
  double im;

public:
// Constructors, and the destructor
  gbtComplex();
  gbtComplex(const double&, const double&);
  gbtComplex(const gbtComplex&);
  gbtComplex(const int& n);
  gbtComplex(const long& n);
  
  ~gbtComplex();

// Operators
  double      RealPart()                      const;
  double      ImaginaryPart()                 const;

  gbtComplex&   operator =  (const gbtComplex& y);

  bool        operator == (const gbtComplex& y) const;
  bool        operator != (const gbtComplex& y) const;

  gbtComplex    operator +  (const gbtComplex& y) const;
  gbtComplex    operator -  (const gbtComplex& y) const;
  gbtComplex    operator *  (const gbtComplex& y) const;
  gbtComplex    operator /  (const gbtComplex& y) const;

  gbtComplex    operator -  ()                 const;

  void       operator += (const gbtComplex& y);
  void       operator -= (const gbtComplex& y);
  void       operator *= (const gbtComplex& y);
  void       operator /= (const gbtComplex& y);

// error reporting
  void    error(const char* msg) const;

// printing
  friend gbtInput&    operator >> (gbtInput& s, gbtComplex& y);
  friend gbtOutput&   operator << (gbtOutput& s, const gbtComplex& y);

// friends outside the class
  friend double    fabs(const gbtComplex& x);              
  friend gbtComplex  sqr(const gbtComplex& x);              
  friend gbtComplex  pow(const gbtComplex& x, const long y);
};

// The following are facilities that are required of all arithmetic types.
// For other types they are given in gmisc.h

class gString;
gString         ToString(gbtComplex);
gbtComplex        TOgComplex(const gString &);

#endif
