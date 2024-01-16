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
  gMono(const gSpace *, const T &);
  gMono(const T &, const exp_vect &);
  gMono(const gMono<T> &);
  ~gMono();

  // operators
  gMono<T> &operator=(const gMono<T> &);

  bool operator==(const gMono<T> &) const;
  bool operator!=(const gMono<T> &) const;
  gMono<T> operator*(const gMono<T> &) const;
  gMono<T> operator/(const gMono<T> &) const;
  gMono<T> operator+(const gMono<T> &) const; // assert exps ==
  gMono<T> &operator+=(const gMono<T> &);     // assert exps ==
  gMono<T> &operator*=(const T &);
  gMono<T> operator-() const;

  // information
  const T &Coef() const;
  int Dmnsn() const;
  int TotalDegree() const;
  bool IsConstant() const;
  bool IsMultiaffine() const;
  const exp_vect &ExpV() const;
  T Evaluate(const Gambit::Array<T> &) const;
  T Evaluate(const Gambit::Vector<T> &) const;
};
