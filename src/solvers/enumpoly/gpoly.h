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

using namespace Gambit;

class VariableSpace {
public:
  struct Variable {
    std::string name;
    int number;
  };

  explicit VariableSpace(size_t nvars)
  {
    for (size_t i = 1; i <= nvars; i++) {
      m_variables.push_back({"n" + std::to_string(i), static_cast<int>(i)});
    }
  }
  VariableSpace(const VariableSpace &) = delete;
  ~VariableSpace() = default;
  VariableSpace &operator=(const VariableSpace &) = delete;
  const Variable &operator[](const int index) const { return m_variables[index]; }
  int Dmnsn() const { return m_variables.size(); }

private:
  Array<Variable> m_variables;
};

// An exponent vector is a vector of integers representing the exponents on variables in
// a space in a monomial.
//
// Exponent vectors are ordered lexicographically.
class ExponentVector {
private:
  const VariableSpace *m_space;
  Vector<int> m_components;

public:
  explicit ExponentVector(const VariableSpace *p) : m_space(p), m_components(p->Dmnsn())
  {
    m_components = 0;
  }
  // Construct x_i^j
  ExponentVector(const VariableSpace *p, const int i, const int j)
    : m_space(p), m_components(p->Dmnsn())
  {
    m_components = 0;
    m_components[i] = j;
  }
  ExponentVector(const VariableSpace *space, const Vector<int> &exps)
    : m_space(space), m_components(exps)
  {
  }
  ExponentVector(const ExponentVector &) = default;
  ~ExponentVector() = default;

  // Operators
  ExponentVector &operator=(const ExponentVector &) = default;

  int operator[](int index) const { return m_components[index]; }

  bool operator==(const ExponentVector &y) const
  {
    return m_space == y.m_space && m_components == y.m_components;
  }
  bool operator!=(const ExponentVector &y) const
  {
    return m_space != y.m_space || m_components != y.m_components;
  }
  ExponentVector operator+(const ExponentVector &v) const
  {
    ExponentVector tmp(*this);
    tmp.m_components += v.m_components;
    return tmp;
  }

  // Other operations
  ExponentVector WithZeroExponent(const int varnumber) const
  {
    ExponentVector tmp(*this);
    tmp.m_components[varnumber] = 0;
    return tmp;
  }
  ExponentVector WithDecrementedExponent(const int varnumber) const
  {
    ExponentVector tmp(*this);
    tmp.m_components[varnumber]--;
    return tmp;
  }

  // Information
  int Dmnsn() const { return m_space->Dmnsn(); }
  bool IsConstant() const
  {
    for (int i = 1; i <= Dmnsn(); i++) {
      if ((*this)[i] > 0) {
        return false;
      }
    }
    return true;
  }
  bool IsMultiaffine() const
  {
    for (int i = 1; i <= Dmnsn(); i++) {
      if ((*this)[i] > 1) {
        return false;
      }
    }
    return true;
  }
  int TotalDegree() const
  {
    int exp_sum = 0;
    for (int i = 1; i <= Dmnsn(); i++) {
      exp_sum += (*this)[i];
    }
    return exp_sum;
  }

  // Manipulation
  void ToZero()
  {
    for (int i = 1; i <= Dmnsn(); i++) {
      m_components[i] = 0;
    }
  }

  bool operator<(const ExponentVector &y) const
  {
    for (int i = 1; i <= Dmnsn(); i++) {
      if (m_components[i] < y.m_components[i]) {
        return true;
      }
      if (m_components[i] > y.m_components[i]) {
        return false;
      }
    }
    return false;
  }
  bool operator<=(const ExponentVector &y) const { return *this < y || *this == y; }
  bool operator>(const ExponentVector &y) const { return !(*this <= y); }
  bool operator>=(const ExponentVector &y) const { return !(*this < y); }
};

/// A monomial of multiple variables with non-negative exponents
template <class T> class gMono {
private:
  T coef;
  ExponentVector exps;

public:
  // constructors
  gMono(const VariableSpace *p, const T &x) : coef(x), exps(p) {}
  gMono(const T &x, const ExponentVector &e) : coef(x), exps(e)
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
  const ExponentVector &ExpV() const { return exps; }
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
  const VariableSpace *Space;   // pointer to variable Space of space
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
  gPoly(const VariableSpace *);
  // Constructs a constant gPoly
  gPoly(const VariableSpace *, const T &);
  // Constructs a gPoly equal to another;
  gPoly(const gPoly<T> &);
  // Constructs a gPoly that is x_{var_no}^exp;
  gPoly(const VariableSpace *p, int var_no, int exp);
  // Constructs a gPoly that is the monomial coeff*vars^exps;
  gPoly(const VariableSpace *p, ExponentVector exps, T coeff);
  // Constructs a gPoly with single monomial
  gPoly(const VariableSpace *p, const gMono<T> &);

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

  const VariableSpace *GetSpace() const;
  int Dmnsn() const;
  int DegreeOfVar(int var_no) const;
  int Degree() const;
  T GetCoef(const Gambit::Vector<int> &Powers) const;
  T GetCoef(const ExponentVector &Powers) const;
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
