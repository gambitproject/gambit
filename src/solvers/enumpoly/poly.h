//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/poly.h
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

#ifndef POLY_H
#define POLY_H

#include "gambit.h"
#include "core/sqmatrix.h"

namespace Gambit {

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
  int GetDimension() const { return m_variables.size(); }

private:
  Array<Variable> m_variables;
};

// An exponent vector is a vector of integers representing the exponents on variables in
// a space in a monomial.
//
// Exponent vectors are ordered lexicographically.
class ExponentVector {
private:
  std::shared_ptr<VariableSpace> m_space;
  Vector<int> m_components;

public:
  explicit ExponentVector(std::shared_ptr<VariableSpace> p)
    : m_space(p), m_components(p->GetDimension())
  {
    m_components = 0;
  }
  // Construct x_i^j
  ExponentVector(std::shared_ptr<VariableSpace> p, const int i, const int j)
    : m_space(p), m_components(p->GetDimension())
  {
    m_components = 0;
    m_components[i] = j;
  }
  ExponentVector(const ExponentVector &) = default;
  ~ExponentVector() = default;
  ExponentVector &operator=(const ExponentVector &) = default;

  std::shared_ptr<VariableSpace> GetSpace() const { return m_space; }
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

  ExponentVector WithZeroExponent(const int varnumber) const
  {
    ExponentVector tmp(*this);
    tmp.m_components[varnumber] = 0;
    return tmp;
  }

  int GetDimension() const { return m_space->GetDimension(); }
  bool IsMultiaffine() const
  {
    return std::all_of(m_components.begin(), m_components.end(), [](int x) { return x <= 1; });
  }
  int TotalDegree() const { return std::accumulate(m_components.begin(), m_components.end(), 0); }

  void ToZero() { m_components = 0; }

  bool operator<(const ExponentVector &y) const
  {
    for (int i = 1; i <= GetDimension(); i++) {
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
template <class T> class Monomial {
private:
  T coef;
  ExponentVector exps;

public:
  Monomial(std::shared_ptr<VariableSpace> p, const T &x) : coef(x), exps(p) {}
  Monomial(const T &x, const ExponentVector &e) : coef(x), exps(e)
  {
    if (x == static_cast<T>(0)) {
      exps.ToZero();
    }
  }
  Monomial(const Monomial<T> &) = default;
  ~Monomial() = default;

  // operators
  Monomial<T> &operator=(const Monomial<T> &) = default;

  bool operator==(const Monomial<T> &y) const { return (coef == y.coef && exps == y.exps); }
  bool operator!=(const Monomial<T> &y) const { return (coef != y.coef || exps != y.exps); }
  Monomial<T> operator*(const Monomial<T> &y) const { return {coef * y.coef, exps + y.exps}; }
  Monomial<T> operator+(const Monomial<T> &y) const { return {coef + y.coef, exps}; }
  Monomial<T> &operator+=(const Monomial<T> &y)
  {
    coef += y.coef;
    return *this;
  }
  Monomial<T> &operator*=(const T &v)
  {
    coef *= v;
    return *this;
  }
  Monomial<T> operator-() const { return {-coef, exps}; }

  std::shared_ptr<VariableSpace> GetSpace() const { return exps.GetSpace(); }
  const T &Coef() const { return coef; }
  int TotalDegree() const { return exps.TotalDegree(); }
  bool IsMultiaffine() const { return exps.IsMultiaffine(); }
  const ExponentVector &ExpV() const { return exps; }
  T Evaluate(const Vector<T> &vals) const
  {
    T answer = coef;
    for (int i = 1; i <= exps.GetDimension(); i++) {
      for (int j = 1; j <= exps[i]; j++) {
        answer *= vals[i];
      }
    }
    return answer;
  }
};

// A multivariate polynomial
template <class T> class Polynomial {
private:
  std::shared_ptr<VariableSpace> m_space;
  List<Monomial<T>> m_terms;

  // Arithmetic
  List<Monomial<T>> Adder(const List<Monomial<T>> &, const List<Monomial<T>> &) const;
  List<Monomial<T>> Mult(const List<Monomial<T>> &, const List<Monomial<T>> &) const;
  Polynomial<T> DivideByPolynomial(const Polynomial<T> &den) const;

  Polynomial<T> TranslateOfMono(const Monomial<T> &, const Vector<T> &) const;
  Polynomial<T> MonoInNewCoordinates(const Monomial<T> &, const SquareMatrix<T> &) const;

public:
  Polynomial(std::shared_ptr<VariableSpace> p) : m_space(p) {}
  // A constant polynomial
  Polynomial(std::shared_ptr<VariableSpace> p, const T &constant) : m_space(p)
  {
    if (constant != static_cast<T>(0)) {
      m_terms.push_back(Monomial<T>(p, constant));
    }
  }
  Polynomial(const Polynomial<T> &) = default;
  // Constructs a polynomial x_{var_no}^exp
  Polynomial(std::shared_ptr<VariableSpace> p, const int var_no, const int exp) : m_space(p)
  {
    m_terms.push_back(Monomial<T>(static_cast<T>(1), ExponentVector(p, var_no, exp)));
  }
  // Constructs a polynomial with single monomial
  explicit Polynomial(const Monomial<T> &mono) : m_space(mono.GetSpace())
  {
    m_terms.push_back(mono);
  }
  ~Polynomial() = default;

  Polynomial<T> &operator=(const Polynomial<T> &) = default;
  Polynomial<T> operator-() const
  {
    Polynomial<T> neg(*this);
    for (size_t j = 1; j <= m_terms.size(); j++) {
      neg.m_terms[j] = -m_terms[j];
    }
    return neg;
  }
  Polynomial<T> operator-(const Polynomial<T> &p) const
  {
    Polynomial<T> dif(*this);
    dif -= p;
    return dif;
  }
  void operator-=(const Polynomial<T> &p)
  {
    Polynomial<T> neg = p;
    for (size_t i = 1; i <= neg.m_terms.size(); i++) {
      neg.m_terms[i] = -neg.m_terms[i];
    }
    m_terms = Adder(m_terms, neg.m_terms);
  }
  Polynomial<T> operator+(const Polynomial<T> &p) const
  {
    Polynomial<T> sum(*this);
    sum += p;
    return sum;
  }
  Polynomial<T> operator+(const T &v) const
  {
    Polynomial<T> result(*this);
    result += v;
    return result;
  }
  void operator+=(const Polynomial<T> &p) { m_terms = Adder(m_terms, p.m_terms); }
  void operator+=(const T &val) { *this += Polynomial<T>(m_space, val); }
  Polynomial<T> operator*(const Polynomial<T> &p) const
  {
    Polynomial<T> prod(*this);
    prod *= p;
    return prod;
  }
  Polynomial<T> operator*(const T &v) const
  {
    Polynomial<T> result(*this);
    result *= v;
    return result;
  }
  void operator*=(const Polynomial<T> &p) { m_terms = Mult(m_terms, p.m_terms); }
  void operator*=(const T &val)
  {
    for (size_t j = 1; j <= m_terms.size(); j++) {
      m_terms[j] *= val;
    }
  }
  Polynomial<T> operator/(const T &val) const
  {
    if (val == static_cast<T>(0)) {
      throw ZeroDivideException();
    }
    return (*this) * (static_cast<T>(1) / val);
  }
  Polynomial<T> operator/(const Polynomial<T> &den) const { return DivideByPolynomial(den); }

  bool operator==(const Polynomial<T> &p) const
  {
    return m_space == p.m_space && m_terms == p.m_terms;
  }
  bool operator!=(const Polynomial<T> &p) const
  {
    return m_space != p.m_space || m_terms != p.m_terms;
  }

  //-------------
  // Information
  //-------------

  std::shared_ptr<VariableSpace> GetSpace() const { return m_space; }
  int GetDimension() const { return m_space->GetDimension(); }
  int DegreeOfVar(int var_no) const
  {
    return std::accumulate(
        m_terms.begin(), m_terms.end(), 0,
        [&var_no](int v, const Monomial<T> &m) { return std::max(v, m.ExpV()[var_no]); });
  }
  int Degree() const
  {
    return std::accumulate(m_terms.begin(), m_terms.end(), 0, [](int v, const Monomial<T> &m) {
      return std::max(v, m.TotalDegree());
    });
  }
  Polynomial<T> LeadingCoefficient(int varnumber) const;
  T NumLeadCoeff() const
  {
    return (m_terms.size() == 1) ? m_terms.front().Coef() : static_cast<T>(0);
  }
  bool IsMultiaffine() const
  {
    return std::all_of(m_terms.begin(), m_terms.end(),
                       [](const Monomial<T> &t) { return t.IsMultiaffine(); });
  }
  T Evaluate(const Vector<T> &values) const
  {
    return std::accumulate(
        m_terms.begin(), m_terms.end(), static_cast<T>(0),
        [&values](const T &v, const Monomial<T> &m) { return v + m.Evaluate(values); });
  }
  Polynomial<T> PartialDerivative(int varnumber) const;
  const List<Monomial<T>> &MonomialList() const { return m_terms; }

  Polynomial<T> TranslateOfPoly(const Vector<T> &) const;
  Polynomial<T> PolyInNewCoordinates(const SquareMatrix<T> &) const;
  T MaximalValueOfNonlinearPart(const T &) const;
  Polynomial<T> Normalize() const;
};

} // end namespace Gambit

#endif // POLY_H
