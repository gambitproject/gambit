//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// General number class
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

#ifndef GNUMBER_H
#define GNUMBER_H

#include "base/base.h"
#include "rational.h"

class gOutput;

typedef enum { precDOUBLE, precRATIONAL } gPrecision;

class gNumber  {
protected:
  gPrecision rep;
  union {
    gRational *rval;
    double dval;
  };

public:
  class DivideByZero : public gException  {
  public:
    virtual ~DivideByZero()  { }
    gText Description(void) const;
  };

  // CONSTRUCTORS, DESTRUCTOR, CONSTRUCTIVE OPERATORS
  gNumber(void);
  gNumber(float);
  gNumber(double);
  gNumber(int n);
  gNumber(long n);
  gNumber(const gInteger &y);
  gNumber(const gRational &y);
  gNumber(const gNumber &y);
  ~gNumber();

  gNumber &operator=(const gNumber &y);

  // OPERATOR OVERLOADING
  friend bool operator==(const gNumber &x, const gNumber &y);
  friend bool operator!=(const gNumber &x, const gNumber &y);
  friend bool operator< (const gNumber &x, const gNumber &y);
  friend bool operator<=(const gNumber &x, const gNumber &y);
  friend bool operator> (const gNumber &x, const gNumber &y);
  friend bool operator>=(const gNumber &x, const gNumber &y);

  friend gNumber operator+(const gNumber &x, const gNumber &y);
  friend gNumber operator-(const gNumber &x, const gNumber &y);
  friend gNumber operator*(const gNumber &x, const gNumber &y);
  friend gNumber operator/(const gNumber &x, const gNumber &y);
  friend gNumber operator-(const gNumber &x);

  gNumber &operator+=(const gNumber &y);
  gNumber &operator-=(const gNumber &y);
  gNumber &operator*=(const gNumber &y);
  gNumber &operator/=(const gNumber &y);

  friend gOutput &operator<<(gOutput &s, const gNumber &y);
  friend gInput &operator>>(gInput &s, gNumber &y);

  // MISCELLANEOUS MATHEMATICAL FUNCTIONS
  friend gNumber pow(const gNumber&,long);

  // PRECISION-RELATED FUNCTIONS AND CASTS
  operator double() const;
  operator gRational() const;
  gPrecision Precision(void) const { return rep; }
  bool IsInteger(void) const;
};

gOutput &operator<<(gOutput &, const gNumber &);

gText ToText(const gNumber &);
gText ToText(const gNumber &p_number, int p_precision);
gNumber	FromText(const gText &, gNumber &);
gNumber ToNumber(const gText &);    

void gEpsilon(gNumber &v, int i=8);

#endif  // GNUMBER_H

