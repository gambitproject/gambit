//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of multiple-precision floating point class
//
// This file is part of Gambit
// Copyright (c) 2003, The Gambit Project
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

//
// The gbtMPFloat class implements a multiple-precision floating point
// number. This is currently only available when Gambit is built with
// the GNU mp library, and this class is just a wrapper around the
// multiple-precision floating point type provided there.
// 
// The preprocessor symbol GBT_WITH_MP_FLOAT is defined to be 1 if
// this class is available, and 0 if not, to make it possible to 
// conditionally compile code using it independent of the use of
// the USE_GNU_MP macro, in case other implementations become available
// in the future.
//

#ifndef MPFLOAT_H
#define MPFLOAT_H

#ifdef USE_GNU_MP
#define GBT_WITH_MP_FLOAT 1

#include <gmp.h>
#include "base/base.h"

class gbtMPFloat {
private:
  mpf_t m_value;

public:
  // LIFECYCLE
  gbtMPFloat(void) { mpf_init(m_value); }
  gbtMPFloat(const gbtMPFloat &x) { mpf_init_set(m_value, x.m_value); }
  gbtMPFloat(int x) { mpf_init_set_si(m_value, x); }
  gbtMPFloat(double x) { mpf_init_set_d(m_value, x); }
  ~gbtMPFloat() { mpf_clear(m_value); }

  gbtMPFloat &operator=(const gbtMPFloat &);

  // COMPARISON OPERATORS
  bool operator==(const gbtMPFloat &y) const
    { return (mpf_cmp(m_value, y.m_value) == 0); }
  bool operator!=(const gbtMPFloat &y) const
    { return (mpf_cmp(m_value, y.m_value) != 0); }
  bool operator>=(const gbtMPFloat &y) const
    { return (mpf_cmp(m_value, y.m_value) >= 0); }
  bool operator>(const gbtMPFloat &y) const
    { return (mpf_cmp(m_value, y.m_value) > 0); }
  bool operator<=(const gbtMPFloat &y) const
    { return (mpf_cmp(m_value, y.m_value) <= 0); }
  bool operator<(const gbtMPFloat &y) const
    { return (mpf_cmp(m_value, y.m_value) < 0); }

  // ARITHMETIC OPERATORS
  gbtMPFloat operator+(const gbtMPFloat &) const;
  gbtMPFloat operator-(const gbtMPFloat &) const;
  gbtMPFloat operator-(void) const;
  gbtMPFloat operator*(const gbtMPFloat &) const;
  gbtMPFloat operator/(const gbtMPFloat &) const;

  gbtMPFloat &operator+=(const gbtMPFloat &);
  gbtMPFloat &operator-=(const gbtMPFloat &);
  gbtMPFloat &operator*=(const gbtMPFloat &);
  gbtMPFloat &operator/=(const gbtMPFloat &);

  // CONVERSION OPERATORS
  operator double(void) const  { return mpf_get_d(m_value); }

  // GLOBAL-SCOPE FRIEND OPERATIONS
  friend gbtMPFloat abs(const gbtMPFloat &);
  friend gbtMPFloat sqrt(const gbtMPFloat &);
};

//
// Global-scope functions to match <math.h>-type functions
//
inline gbtMPFloat abs(const gbtMPFloat &y)
{
  gbtMPFloat ret;
  mpf_abs(ret.m_value, y.m_value);
  return ret;
}

inline gbtMPFloat sqrt(const gbtMPFloat &y)
{ 
  gbtMPFloat ret;
  mpf_sqrt(ret.m_value, y.m_value);
  return ret;
}

inline std::string ToText(const gbtMPFloat &y)
{ return ToText((double) y); }

gbtOutput &operator<<(gbtOutput &, const gbtMPFloat &);


#else  // we don't have GNU mp
#define GBT_WITH_MP_FLOAT 0
#endif  // USE_GNU_MP


#endif  // MPFLOAT_H
