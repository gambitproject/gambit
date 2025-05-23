//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/core/util.h
// Core (game theory-independent) utilities for Gambit
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

#ifndef GAMBIT_CORE_UTIL_H
#define GAMBIT_CORE_UTIL_H

#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <map>

namespace Gambit {

// Naming compatible with Boost's lexical_cast concept for potential future compatibility.

template <class D, class S> D lexical_cast(const S &p_value)
{
  std::ostringstream s;
  s << p_value;
  return s.str();
}

template <class D, class S> D lexical_cast(const S &p_value, int p_prec)
{
  std::ostringstream s;
  s.setf(std::ios::fixed);
  s << std::setprecision(p_prec) << p_value;
  return s.str();
}

inline double abs(double x) { return std::fabs(x); }

inline double sqr(double x) { return x * x; }

template <class C, class T> bool contains(const C &p_container, const T &p_value)
{
  return std::find(p_container.cbegin(), p_container.cend(), p_value) != p_container.cend();
}

template <class Key, class T> bool contains(const std::map<Key, T> &map, const Key &key)
// TODO: remove when we move to C++20 which already includes a "contains" method
{
  return map.find(key) != map.end();
}
//========================================================================
//                        Exception classes
//========================================================================

/// A base class for all Gambit exceptions
class Exception : public std::runtime_error {
public:
  Exception() : std::runtime_error("") {}
  explicit Exception(const std::string &s) : std::runtime_error(s) {}
  ~Exception() noexcept override = default;
};

/// Exception thrown on out-of-range index
class IndexException : public Exception {
public:
  IndexException() : Exception("Index out of range") {}
  explicit IndexException(const std::string &s) : Exception(s) {}
  ~IndexException() noexcept override = default;
};

/// Exception thrown on invalid index ranges
class RangeException : public Exception {
public:
  RangeException() : Exception("Invalid index range") {}
  explicit RangeException(const std::string &s) : Exception(s) {}
  ~RangeException() noexcept override = default;
};

/// Exception thrown on dimension mismatches
class DimensionException : public Exception {
public:
  DimensionException() : Exception("Mismatched dimensions") {}
  explicit DimensionException(const std::string &s) : Exception(s) {}
  ~DimensionException() noexcept override = default;
};

/// Exception thrown on invalid value
class ValueException : public Exception {
public:
  ValueException() : Exception("Invalid value") {}
  explicit ValueException(const std::string &s) : Exception(s) {}
  ~ValueException() noexcept override = default;
};

/// Exception thrown on a failed assertion
class AssertionException : public Exception {
public:
  AssertionException() : Exception("Failed assertion") {}
  explicit AssertionException(const std::string &s) : Exception(s) {}
  ~AssertionException() noexcept override = default;
};

/// Exception thrown on attempted division by zero
class ZeroDivideException : public Exception {
public:
  ZeroDivideException() : Exception("Attempted division by zero") {}
  explicit ZeroDivideException(const std::string &s) : Exception(s) {}
  ~ZeroDivideException() noexcept override = default;
};

/// An exception thrown when attempting to dereference a null pointer
class NullException : public Exception {
public:
  NullException() : Exception("Dereferenced null pointer") {}
  explicit NullException(const std::string &s) : Exception(s) {}
  ~NullException() noexcept override = default;
};

} // end namespace Gambit

#endif // GAMBIT_CORE_UTIL_H
