//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/gpoly.h
// Declaration of multivariate polynomial type
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

#ifndef GPOLY_H
#define GPOLY_H

#include "gambit.h"
#include "core/sqmatrix.h"
#include "prepoly.h"

/// A monomial of multiple variables with non-negative exponents
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
  T Evaluate(const Vector<T> &vals) const
  {
    T answer = coef;
    for (int i = 1; i <= exps.Dmnsn(); i++) {
      for (int j = 1; j <= exps[i]; j++) {
        answer *= vals[i];
      }
    }
    return answer;
  }
};

// These classes are used to store and mathematically manipulate polynomials.
template <class T> class gPoly {

private:
  const gSpace *Space; // pointer to variable Space of space
  const term_order *Order;
  Gambit::List<gMono<T>> Terms; // alternative implementation

  //----------------------
  // some private members
  //----------------------

  // Arithmetic
  Gambit::List<gMono<T>> Adder(const Gambit::List<gMono<T>> &,
                               const Gambit::List<gMono<T>> &) const;
  Gambit::List<gMono<T>> Mult(const Gambit::List<gMono<T>> &,
                              const Gambit::List<gMono<T>> &) const;
  gPoly<T> DivideByPolynomial(const gPoly<T> &den) const;

  // The following is used to construct the translate of *this.
  gPoly<T> TranslateOfMono(const gMono<T> &, const Gambit::Vector<T> &) const;
  gPoly<T> MonoInNewCoordinates(const gMono<T> &, const Gambit::SquareMatrix<T> &) const;

public:
  //---------------------------
  // Construction, destruction:
  //---------------------------

  // Null gPoly constructor
  gPoly(const gSpace *, const term_order *);
  // Constructs a constant gPoly
  gPoly(const gSpace *, const T &, const term_order *);
  // Constructs a gPoly equal to another;
  gPoly(const gPoly<T> &);
  // Constructs a gPoly that is x_{var_no}^exp;
  gPoly(const gSpace *p, int var_no, int exp, const term_order *);
  // Constructs a gPoly that is the monomial coeff*vars^exps;
  gPoly(const gSpace *p, exp_vect exps, T coeff, const term_order *);
  // Constructs a gPoly with single monomial
  gPoly(const gSpace *p, const gMono<T> &, const term_order *);

  ~gPoly() = default;

  //----------
  // Operators:
  //----------

  gPoly<T> &operator=(const gPoly<T> &);
  // Set polynomial equal to the SOP form in the string
  gPoly<T> operator-() const;
  gPoly<T> operator-(const gPoly<T> &) const;
  void operator-=(const gPoly<T> &);
  gPoly<T> operator+(const gPoly<T> &) const;
  void operator+=(const gPoly<T> &);
  void operator+=(const T &);
  gPoly<T> operator*(const gPoly<T> &) const;
  void operator*=(const gPoly<T> &);
  void operator*=(const T &);
  gPoly<T> operator/(const T &val) const;     // division by a constant
  gPoly<T> operator/(const gPoly<T> &) const; // division by a polynomial

  bool operator==(const gPoly<T> &p) const;
  bool operator!=(const gPoly<T> &p) const;

  //-------------
  // Information:
  //-------------

  const gSpace *GetSpace() const;
  const term_order *GetOrder() const;
  int Dmnsn() const;
  int DegreeOfVar(int var_no) const;
  int Degree() const;
  T GetCoef(const Gambit::Array<int> &Powers) const;
  T GetCoef(const exp_vect &Powers) const;
  gPoly<T> LeadingCoefficient(int varnumber) const;
  T NumLeadCoeff() const; // deg == 0
  bool IsConstant() const;
  bool IsMultiaffine() const;
  // assumes UniqueActiveVariable() is true
  T Evaluate(const Gambit::Vector<T> &values) const;
  gPoly<T> PartialDerivative(int varnumber) const;
  Gambit::List<gMono<T>> MonomialList() const;

  gPoly<T> TranslateOfPoly(const Gambit::Vector<T> &) const;
  gPoly<T> PolyInNewCoordinates(const Gambit::SquareMatrix<T> &) const;
  T MaximalValueOfNonlinearPart(const T &) const;
};

//-------------
// Conversion:
//-------------

template <class T> gPoly<double> ToDouble(const gPoly<T> &);
template <class T> gPoly<double> NormalizationOfPoly(const gPoly<T> &);

// global multiply by scalar operators
template <class T> gPoly<T> operator*(const T &val, const gPoly<T> &poly);
template <class T> gPoly<T> operator*(const gPoly<T> &poly, const T &val);

// global add to scalar operators
template <class T> gPoly<T> operator+(const T &val, const gPoly<T> &poly);
template <class T> gPoly<T> operator+(const gPoly<T> &poly, const T &val);

#endif // # GPOLY_H
