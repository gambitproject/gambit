//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/monomial.h
// Declaration of monomial class
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

#include "prepoly.h"

// This file provides the template class
//
//              gMono
//
// whose objects are monomials in several variables
// with coefficients of class T and nonnegative exponents.
// This role of this class is to support the class gPoly.

template <class T> class gMono {
private:
  T coef;
  exp_vect exps;

public:
  // constructors
  gMono(const gSpace *p, const T &x) : coef(x), exps(p) {}
  gMono(const T &x, const exp_vect &e) : coef(x), exps(e)
  {
    if (x == static_cast<T>(0)) {
      exps.ToZero();
    }
  }
  gMono(const gMono<T> &) = default;
  ~gMono() = default;

  // operators
  gMono<T> &operator=(const gMono<T> &) = default;

  bool operator==(const gMono<T> &y) const { return (coef == y.coef && exps == y.exps); }
  bool operator!=(const gMono<T> &y) const { return (coef != y.coef || exps != y.exps); }
  gMono<T> operator*(const gMono<T> &y) const { return {coef * y.coef, exps + y.exps}; }
  gMono<T> operator/(const gMono<T> &y) const { return {coef / y.coef, exps - y.exps}; }
  gMono<T> operator+(const gMono<T> &y) const { return {coef + y.coef, exps}; }
  gMono<T> &operator+=(const gMono<T> &y)
  {
    coef += y.coef;
    return *this;
  }
  gMono<T> &operator*=(const T &v)
  {
    coef *= v;
    return *this;
  }
  gMono<T> operator-() const { return {-coef, exps}; }

  // information
  const T &Coef() const { return coef; }
  int Dmnsn() const { return exps.Dmnsn(); }
  int TotalDegree() const { return exps.TotalDegree(); }
  bool IsConstant() const { return exps.IsConstant(); }
  bool IsMultiaffine() const { return exps.IsMultiaffine(); }
  const exp_vect &ExpV() const { return exps; }
  T Evaluate(const Gambit::Vector<T> &vals) const
  {
    T answer = Coef();
    for (int i = 1; i <= Dmnsn(); i++) {
      for (int j = 1; j <= exps[i]; j++) {
        answer *= vals[i];
      }
    }
    return answer;
  }
};
