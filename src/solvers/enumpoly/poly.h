//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/poly.h
// Declaration of polynomial classes
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

#include "gambit.h"
#include "interval.h"
#include "gcomplex.h"

#ifndef POLY_H
#define POLY_H

using namespace Gambit;

// @brief A univariate polynomial with coefficients of type T.
//
// Internally this is implemented as a list; there is no attempt to maintain sparseness.
template <class T> class polynomial {

private:
  List<T> coeflist;

public:
  // constructors and destructor
  explicit polynomial() = default;
  polynomial(const polynomial<T> &) = default;
  explicit polynomial(const List<T> &p_coefs) : coeflist(p_coefs) {}
  explicit polynomial(const Vector<T> &);
  polynomial(const T &, const int &);
  ~polynomial() = default;

  // unary operators
  polynomial<T> operator-() const;

  // binary operators
  polynomial<T> &operator=(const polynomial<T> &y) = default;
  bool operator==(const polynomial<T> &y) const { return coeflist == y.coeflist; }
  bool operator!=(const polynomial<T> &y) const { return !(*this == y); }
  const T &operator[](int index) const { return coeflist[index + 1]; }

  polynomial<T> operator+(const polynomial<T> &y) const;
  polynomial<T> operator-(const polynomial<T> &y) const { return polynomial<T>(*this + (-y)); }
  polynomial<T> operator*(const polynomial<T> &y) const;
  polynomial<T> operator/(const polynomial<T> &y) const;
  polynomial<T> &operator+=(const polynomial<T> &y) { return ((*this) = (*this) + y); }

  polynomial<T> &operator-=(const polynomial<T> &y) { return ((*this) = (*this) - y); }
  polynomial<T> &operator*=(const polynomial<T> &y) { return ((*this) = (*this) * y); }

  polynomial<T> &operator/=(const polynomial<T> &y) { return ((*this) = (*this) / y); }
  polynomial<T> operator%(const polynomial<T> &y) const;

  // information
  int Degree() const { return coeflist.size() - 1; }

  T LeadingCoefficient() const
  {
    return (Degree() < 0) ? static_cast<T>(0) : coeflist[Degree() + 1];
  }
};

#endif // POLY_H
