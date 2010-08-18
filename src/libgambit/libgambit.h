//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/libgambit.h
// Top-level include file for libgambit
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

#ifndef LIBGAMBIT_H
#define LIBGAMBIT_H

#include <string>
#include <sstream>
#include <iomanip>
#include <math.h>

namespace Gambit {

inline void Epsilon(double &v, int i = 8)
  { v = ::pow(10.0, (double) -i); }

template <class T> T min(const T &a, const T &b)
{ if (a < b) return a; else return b; }

template <class T> T max(const T &a, const T &b)
{ if (a > b) return a; else return b; }

template <class T> std::string ToText(const T &p_value)
{ std::ostringstream s; s << p_value; return s.str(); }

inline std::string ToText(double p_value, int p_prec)
{ 
  std::ostringstream s; 
  s.setf(std::ios::fixed);
  s << std::setprecision(p_prec) << p_value;
  return s.str();
}

inline double abs(double a)
{ return (a >= 0.0) ? a : -a; }

//========================================================================
//                        Exception classes
//========================================================================

/// A base class for all exceptions
class Exception {
public:
  virtual ~Exception() { }
  virtual std::string GetDescription(void) const = 0;
};

/// Exception thrown on out-of-range index
class IndexException : public Exception {
public:
  virtual ~IndexException() { }
  std::string GetDescription(void) const { return "Index out of range"; }
};

/// Exception thrown on invalid index ranges
class RangeException : public Exception {
public:
  virtual ~RangeException() { }
  std::string GetDescription(void) const { return "Invalid index range"; }
};

/// Exception thrown on dimension mismatches
class DimensionException : public Exception {
public:
  virtual ~DimensionException() { }
  std::string GetDescription(void) const { return "Mismatched dimensions"; }
};

/// Exception thrown on invalid value
class ValueException : public Exception {
public:
  virtual ~ValueException() { }
  std::string GetDescription(void) const { return "Invalid value"; }
};

/// Exception thrown on attempted division by zero
class ZeroDivideException : public Exception {
public:
  virtual ~ZeroDivideException() { }
  std::string GetDescription(void) const 
    { return "Attmpted division by zero"; }
};

} // end namespace Gambit


inline int sign(const double &a)
{
  if (a > 0.0)   return 1;
  if (a < 0.0)   return -1;
  return 0;
}

inline double pow(double x, long y)
{ return pow(x, (double) y); }


#include "array.h"
#include "list.h"
#include "recarray.h"
#include "vector.h"
#include "matrix.h"
#include "map.h"

#include "rational.h"


#include "game.h"

#include "behavspt.h"
#include "behav.h"
#include "behavitr.h"

#include "stratspt.h"
#include "mixed.h"
#include "stratitr.h"

#endif // LIBGAMBIT_H
