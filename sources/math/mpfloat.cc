//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of multiple-precision floating point class
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

#include "mpfloat.h"

#if GBT_WITH_MP_FLOAT


gbtMPFloat &gbtMPFloat::operator=(const gbtMPFloat &y)
{
  if (this != &y) {
    mpf_set(m_value, y.m_value);
  }
  return *this;
}


//------------------------------------------------------------------------
//                  gbtMPFloat: Arithmetic operators
//------------------------------------------------------------------------

gbtMPFloat gbtMPFloat::operator+(const gbtMPFloat &y) const
{
  gbtMPFloat ret(*this);
  mpf_add(ret.m_value, m_value, y.m_value);
  return ret;
}

gbtMPFloat gbtMPFloat::operator-(const gbtMPFloat &y) const
{
  gbtMPFloat ret(*this);
  mpf_sub(ret.m_value, m_value, y.m_value);
  return ret;
}

gbtMPFloat gbtMPFloat::operator-(void) const
{
  gbtMPFloat ret(*this);
  mpf_neg(ret.m_value, ret.m_value);
  return ret;
}

gbtMPFloat gbtMPFloat::operator*(const gbtMPFloat &y) const
{
  gbtMPFloat ret(*this);
  mpf_mul(ret.m_value, m_value, y.m_value);
  return ret;
}

gbtMPFloat gbtMPFloat::operator/(const gbtMPFloat &y) const
{
  gbtMPFloat ret(*this);
  mpf_div(ret.m_value, m_value, y.m_value);
  return ret;
}

gbtMPFloat &gbtMPFloat::operator+=(const gbtMPFloat &y)
{
  mpf_add(m_value, m_value, y.m_value);
  return *this;
}

gbtMPFloat &gbtMPFloat::operator-=(const gbtMPFloat &y)
{
  mpf_sub(m_value, m_value, y.m_value);
  return *this;
}

gbtMPFloat &gbtMPFloat::operator*=(const gbtMPFloat &y)
{
  mpf_mul(m_value, m_value, y.m_value);
  return *this;
}

gbtMPFloat &gbtMPFloat::operator/=(const gbtMPFloat &y)
{
  mpf_div(m_value, m_value, y.m_value);
  return *this;
}

gbtOutput &operator<<(gbtOutput &p_stream, const gbtMPFloat &p_float)
{
  // Temporary kludge: convert to double and print that
  p_stream << (double) p_float;
  return p_stream;
}


#endif   // GBT_WITH_MP_FLOAT
