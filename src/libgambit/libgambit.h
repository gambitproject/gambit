//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
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

#include <stdexcept>
#include <string>
#include <sstream>
#include <iomanip>

namespace Gambit {

// Naming compatible with Boost's lexical_cast concept for potential future compatibility.

template <class D, class S> D lexical_cast(const S &p_value)
{ std::ostringstream s; s << p_value; return s.str(); }

template <class D, class S> D lexical_cast(const S &p_value, int p_prec)
{ 
  std::ostringstream s; 
  s.setf(std::ios::fixed);
  s << std::setprecision(p_prec) << p_value;
  return s.str();
}

//========================================================================
//                        Exception classes
//========================================================================

/// A base class for all Gambit exceptions
class Exception : public std::runtime_error {
public:
  Exception(void) : std::runtime_error("") { }
  Exception(const char *s) : std::runtime_error(s) { }
  virtual ~Exception() throw() { }
};

/// Exception thrown on out-of-range index
class IndexException : public Exception {
public:
  virtual ~IndexException() throw() { }
  const char *what(void) const throw() { return "Index out of range"; }
};

/// Exception thrown on invalid index ranges
class RangeException : public Exception {
public:
  virtual ~RangeException() throw() { }
  const char *what(void) const throw() { return "Invalid index range"; }
};

/// Exception thrown on dimension mismatches
class DimensionException : public Exception {
public:
  virtual ~DimensionException() throw() { }
  const char *what(void) const throw() { return "Mismatched dimensions"; }
};

/// Exception thrown on invalid value
class ValueException : public Exception {
public:
  virtual ~ValueException() throw() { }
  const char *what(void) const throw() { return "Invalid value"; }
};

/// Exception thrown on a failed assertion
class AssertionException : public Exception {
public:
  AssertionException(const char *s) : Exception(s) { }
  virtual ~AssertionException() throw() { }
};

/// Exception thrown on attempted division by zero
class ZeroDivideException : public Exception {
public:
  virtual ~ZeroDivideException() throw() { }
  const char *what(void) const throw()  { return "Attmpted division by zero"; }
};

/// An exception thrown when attempting to dereference a null pointer
class NullException : public Exception {
public:
  virtual ~NullException() throw() { }
  const char *what(void) const throw()  { return "Dereferencing null pointer"; }
};

} // end namespace Gambit

#include "array.h"
#include "list.h"
#include "recarray.h"
#include "vector.h"
#include "matrix.h"

#include "rational.h"


#include "game.h"

#include "behavspt.h"
#include "behav.h"
#include "behavitr.h"

#include "stratspt.h"
#include "mixed.h"
#include "stratitr.h"

#endif // LIBGAMBIT_H
