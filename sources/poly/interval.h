//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to interval type
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

#ifndef INTERVAL_H
#define INTERVAL_H

#include "math/rational.h"


//!
//! gbtInterval models the concept of a nonempty compact interval.
//! Since boundary points can be identified, open and half 
//! open (bounded) intervals can be effected, but less directly.
//!
template <class T> class gbtInterval {
private:
  T lower_bd, upper_bd;

  // Defined but not implemented to preserve constness
  gbtInterval<T> &operator=(const gbtInterval<T> &);

public:
  // constructors
  gbtInterval(const gbtInterval<T>& given) 
    : lower_bd(given.lower_bd), upper_bd(given.upper_bd)
  { }

  gbtInterval(const T low, const T high) 
    : lower_bd(low), upper_bd(high)
  { 
    if (low > high)  throw gbtRangeException();
  }

  ~gbtInterval() { }

  // operators
  bool operator==(const gbtInterval<T> &y) const
    { return (lower_bd == y.lower_bd && upper_bd == y.upper_bd); }
  bool operator!=(const gbtInterval<T>& y) const
    { return (lower_bd != y.lower_bd || upper_bd != y.upper_bd); }

  // information
  const T &LowerBound(void) const { return lower_bd; }
  const T &UpperBound(void) const { return upper_bd; }
  bool Contains(const T &number) const
    { return (lower_bd <= number && number <= upper_bd); }

  bool Contains(const gbtInterval<T> &I) const
    { return (lower_bd <= I.lower_bd && I.upper_bd <= upper_bd); }

  bool LiesBelow(const T &number) const { return (upper_bd <= number); }
  bool LiesAbove(const T &number) const { return (number <= lower_bd); }
  T Length(void) const { return (upper_bd - lower_bd); } 
  T Midpoint(void) const { return (upper_bd + lower_bd) / (T) 2; } 

  gbtInterval<T> LeftHalf(void) const 
    { return gbtInterval<T>(lower_bd, Midpoint()); }
  gbtInterval<T> RightHalf(void) const
    { return gbtInterval<T>(Midpoint(), upper_bd); }
  gbtInterval<T> SameCenterTwiceLength(void) const
    { return gbtInterval<T>((T) 2 * LowerBound() - Midpoint(),
			    (T) 2 * UpperBound() - Midpoint()); 
    }
  gbtInterval<T> SameCenterWithNewLength(const T &L) const 
    {
      return gbtInterval<T>(Midpoint() - L/(T)2, Midpoint() + L/(T)2); 
    }
};

#endif // INTERVAL_H
