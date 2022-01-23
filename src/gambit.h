//
// This file is part of Gambit
// Copyright (c) 1994-2022, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/gambit.h
// Top-level include file for Gambit library
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
#include <cmath>

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

inline double abs(double x) { return std::fabs(x); }

//========================================================================
//                        Exception classes
//========================================================================

/// A base class for all Gambit exceptions
class Exception : public std::runtime_error {
public:
  Exception(void) : std::runtime_error("") { }
  Exception(const std::string &s) : std::runtime_error(s) { }
  virtual ~Exception() throw() { }
};

/// Exception thrown on out-of-range index
class IndexException : public Exception {
public:
  IndexException(void) : Exception("Index out of range") { }
  IndexException(const std::string &s) : Exception(s) { }
  virtual ~IndexException() throw() { }
};

/// Exception thrown on invalid index ranges
class RangeException : public Exception {
public:
  RangeException(void) : Exception("Invalid index range") { }
  RangeException(const std::string &s) : Exception(s) { }
  virtual ~RangeException() throw() { }
};

/// Exception thrown on dimension mismatches
class DimensionException : public Exception {
public:
  DimensionException(void) : Exception("Mismatched dimensions") { }
  DimensionException(const std::string &s) : Exception(s) { }
  virtual ~DimensionException() throw() { }
};

/// Exception thrown on invalid value
class ValueException : public Exception {
public:
  ValueException(void) : Exception("Invalid value") { }
  ValueException(const std::string &s) : Exception(s) { }
  virtual ~ValueException() throw() { }
};

/// Exception thrown on a failed assertion
class AssertionException : public Exception {
public:
  AssertionException(void) : Exception("Failed assertion") { }
  AssertionException(const std::string &s) : Exception(s) { }
  virtual ~AssertionException() throw() { }
};

/// Exception thrown on attempted division by zero
class ZeroDivideException : public Exception {
public:
  ZeroDivideException(void) : Exception("Attempted division by zero") { }
  ZeroDivideException(const std::string &s) : Exception(s) { }
  virtual ~ZeroDivideException() throw() { }
};

/// An exception thrown when attempting to dereference a null pointer
class NullException : public Exception {
public:
  NullException(void) : Exception("Dereferenced null pointer") { }
  NullException(const std::string &s) : Exception(s) { }
  virtual ~NullException() throw() { }
};

} // end namespace Gambit

#include "core/shared_ptr.h"
#include "core/array.h"
#include "core/list.h"
#include "core/recarray.h"
#include "core/vector.h"
#include "core/matrix.h"

#include "core/rational.h"


#include "games/game.h"
#include "games/writer.h"

#include "games/behavspt.h"
#include "games/behav.h"
#include "games/behavitr.h"

#include "games/stratspt.h"
#include "games/mixed.h"
#include "games/stratitr.h"

#endif // LIBGAMBIT_H
