//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/complex.h
// Declaration of a complex number class
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
#define _Complex_h 1

#include <cmath>

class gComplex
{
protected:
  double re;
  double im;

public:
// Constructors, and the destructor
  gComplex();
  gComplex(const double&, const double&);
  gComplex(const gComplex&);
  gComplex(const int& n);
  gComplex(const long& n);
  
  ~gComplex();

// Operators
  double      RealPart()                      const;
  double      ImaginaryPart()                 const;

  gComplex&   operator =  (const gComplex& y);

  bool        operator == (const gComplex& y) const;
  bool        operator != (const gComplex& y) const;

  gComplex    operator +  (const gComplex& y) const;
  gComplex    operator -  (const gComplex& y) const;
  gComplex    operator *  (const gComplex& y) const;
  gComplex    operator /  (const gComplex& y) const;

  gComplex    operator -  ()                 const;

  void       operator += (const gComplex& y);
  void       operator -= (const gComplex& y);
  void       operator *= (const gComplex& y);
  void       operator /= (const gComplex& y);

// friends outside the class
  friend double    fabs(const gComplex& x);              
  friend gComplex  sqr(const gComplex& x);              
  friend gComplex  pow(const gComplex& x, const long y);
};

// The following are facilities that are required of all arithmetic types.
// For other types they are given in gmisc.h

class gString;
gString         ToString(gComplex);
gComplex        TOgComplex(const gString &);

#endif

