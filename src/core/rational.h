//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/core/rational.h
// Interface to a rational number class
//
// The original copyright and license are included below.
//

/*
Copyright (C) 1988 Free Software Foundation
    written by Doug Lea (dl@rocky.oswego.edu)

This file is part of the GNU C++ Library.  This library is free
software; you can redistribute it and/or modify it under the terms of
the GNU Library General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.  This library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU Library General Public License for more details.
You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef GAMBIT_CORE_RATIONAL_H
#define GAMBIT_CORE_RATIONAL_H

#include <cmath>
#include <compare>
#include <iosfwd>
#include <string>

#include "util.h"

namespace Gambit {

// The representation of Integer is private to integer.cc.
struct IntegerRep;

class Integer {
private:
  IntegerRep *rep;

public:
  /// @name Lifecycle
  //@{
  Integer();
  explicit Integer(int y);
  Integer(const Integer &y);
  Integer(Integer &&y) noexcept;
  ~Integer();

  Integer &operator=(const Integer &);
  Integer &operator=(Integer &&y) noexcept;
  Integer &operator=(long);
  //@}

  /// @name Unary operations on self
  //@{
  void negate(); // negate in-place
  void abs();    // absolute-value in-place
  //@}

  /// @name Comparison operators
  //@{
  bool operator==(const Integer &) const;
  bool operator==(long) const;
  std::strong_ordering operator<=>(long) const;
  //@}

  /// @name Assignment-based operations
  //@{
  Integer &operator+=(const Integer &);
  Integer &operator+=(long);
  Integer &operator-=(long);
  Integer &operator*=(long);
  Integer &operator<<=(long);
  Integer &operator*=(const Integer &);
  Integer &operator/=(const Integer &);
  //@}

  /// @name Operator overloading
  //@{
  Integer operator-() const;
  Integer operator+(const Integer &) const;
  Integer operator-(const Integer &) const;
  Integer operator*(const Integer &) const;
  Integer operator*(long) const;
  Integer operator/(const Integer &) const;
  //@}

  // builtin Integer functions that must be friends

  friend double ratio(const Integer &x, const Integer &y);

  friend Integer gcd(const Integer &, const Integer &);
  friend int sign(const Integer &); // returns -1, 0, +1

  // procedural versions of operators

  friend void abs(const Integer &x, Integer &dest);
  friend void negate(const Integer &x, Integer &dest);

  friend int compare(const Integer &, const Integer &);
  friend int ucompare(const Integer &, const Integer &);
  friend void add(const Integer &x, const Integer &y, Integer &dest);
  friend void sub(const Integer &x, const Integer &y, Integer &dest);
  friend void mul(const Integer &x, const Integer &y, Integer &dest);
  friend void div(const Integer &x, const Integer &y, Integer &dest);
  friend void divide(const Integer &x, const Integer &y, Integer &q, Integer &r);

  friend void mul(const Integer &x, long y, Integer &dest);

  // coercion & conversion

  double as_double() const;

  friend std::ostream &operator<<(std::ostream &s, const Integer &y);

  bool initialized() const { return rep != nullptr; }
  static void error(const char *msg);
};

Integer abs(const Integer &); // absolute value

extern Integer lcm(const Integer &x, const Integer &y); // least common mult

/// A representation of an arbitrary-precision rational number
class Rational {
protected:
  Integer num, den;

  void normalize();

public:
  Rational();
  explicit Rational(double);
  explicit Rational(int);
  explicit Rational(size_t);
  Rational(int n, int d);
  explicit Rational(const Integer &n);
  Rational(const Integer &n, const Integer &d);
  Rational(const Rational &);
  Rational(Rational &&) noexcept;
  ~Rational();

  Rational &operator=(const Rational &y);
  Rational &operator=(Rational &&y) noexcept;

  bool operator==(const Rational &y) const;
  bool operator<(const Rational &y) const;
  bool operator<=(const Rational &y) const;
  bool operator>(const Rational &y) const;
  bool operator>=(const Rational &y) const;

  Rational operator+(const Rational &y) const;
  Rational operator-(const Rational &y) const;
  Rational operator*(const Rational &y) const;
  Rational operator/(const Rational &y) const;

  Rational &operator+=(const Rational &y);
  Rational &operator-=(const Rational &y);
  Rational &operator*=(const Rational &y);
  Rational &operator/=(const Rational &y);

  Rational operator-() const;

  // builtin Rational functions

  void negate(); // x = -x

  friend int sign(const Rational &x);     // -1, 0, or +1
  friend Rational sqr(const Rational &x); // square
  const Integer &numerator() const;
  const Integer &denominator() const;

  // coercion & conversion

  explicit operator double() const;

  friend std::ostream &operator<<(std::ostream &s, const Rational &y);

  // procedural versions of operators

  friend int compare(const Rational &x, const Rational &y);
  friend void add(const Rational &x, const Rational &y, Rational &dest);
  friend void sub(const Rational &x, const Rational &y, Rational &dest);
  friend void mul(const Rational &x, const Rational &y, Rational &dest);
  friend void div(const Rational &x, const Rational &y, Rational &dest);
};

// Naming compatible with Boost's lexical_cast concept for potential future compatibility.
template <> Rational lexical_cast(const std::string &);

inline int sign(const Rational &x) { return sign(x.num); }

} // end namespace Gambit

#endif // GAMBIT_CORE_RATIONAL_H
