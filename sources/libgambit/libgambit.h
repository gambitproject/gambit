//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Top-level include file for libgambit
//
// This file is part of Gambit
// Copyright (c) 2005, The Gambit Project
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

class gbtException   {
public:
  virtual ~gbtException() { }
  virtual std::string GetDescription(void) const = 0;
};

class gbtIndexException : public gbtException {
public:
  virtual ~gbtIndexException() { }
  std::string GetDescription(void) const { return "Index out of range"; }
};

class gbtRangeException : public gbtException  {
public:
  virtual ~gbtRangeException() { }
  std::string GetDescription(void) const { return "Invalid index range"; }
};

class gbtDimensionException : public gbtException  {
public:
  virtual ~gbtDimensionException() { }
  std::string GetDescription(void) const { return "Mismatched dimensions"; }
};

class gbtZeroDivideException : public gbtException {
public:
  virtual ~gbtZeroDivideException() { }
  std::string GetDescription(void) const 
    { return "Attmpted division by zero"; }
};

inline void gEpsilon(double &v, int i = 8)
{ v = pow(10.0, (double) -i); }

template <class T> T gmin(const T &a, const T &b)
{ if (a < b) return a; else return b; }

template <class T> T gmax(const T &a, const T &b)
{ if (a > b) return a; else return b; }

namespace Gambit {

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

} // end namespace Gambit

inline int sign(const double &a)
{
  if (a > 0.0)   return 1;
  if (a < 0.0)   return -1;
  return 0;
}

inline double pow(double x, long y)
{ return pow(x, (double) y); }


#include "garray.h"
#include "glist.h"
#include "recarray.h"
#include "vector.h"
#include "matrix.h"
#include "gmap.h"

#include "rational.h"


#include "game.h"

#include "behavspt.h"
#include "behav.h"
#include "behavitr.h"

#include "stratspt.h"
#include "mixed.h"
#include "stratitr.h"

#endif // LIBGAMBIT_H
