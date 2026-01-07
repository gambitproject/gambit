//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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

namespace Gambit {

class VariableSpace {
public:
  struct Variable {
    std::string name;
    int number{0};
  };

  VariableSpace() = delete;
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
  std::shared_ptr<VariableSpace> m_space;
  Vector<int> m_components;

public:
  using iterator = Vector<int>::iterator;
  using const_iterator = Vector<int>::const_iterator;

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

  auto begin() const noexcept { return m_components.begin(); }
  auto end() const noexcept { return m_components.end(); }

  auto begin() noexcept { return m_components.begin(); }
  auto end() noexcept { return m_components.end(); }

  std::shared_ptr<VariableSpace> GetSpace() const { return m_space; }
  int operator[](const int index) const { return m_components[index]; }

  bool operator==(const ExponentVector &y) const
  {
    return m_space == y.m_space && m_components == y.m_components;
  }
  bool operator!=(const ExponentVector &y) const
  {
    return m_space != y.m_space || m_components != y.m_components;
  }

  bool operator<(const ExponentVector &y) const
  {
    if (m_space != y.m_space) {
      throw std::logic_error("Cannot order exponent vectors from different spaces");
    }
    return std::lexicographical_compare(begin(), end(), y.begin(), y.end());
  }
  bool operator<=(const ExponentVector &y) const { return !(y < *this); }
  bool operator>(const ExponentVector &y) const { return y < *this; }
  bool operator>=(const ExponentVector &y) const { return !(*this < y); }

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
  ExponentVector DecrementExponent(const int varnumber) const
  {
    ExponentVector tmp(*this);
    tmp.m_components[varnumber] -= 1;
    return tmp;
  }

  int GetDimension() const { return m_space->GetDimension(); }
  bool IsMultiaffine() const
  {
    return std::all_of(m_components.begin(), m_components.end(), [](int x) { return x <= 1; });
  }
  int TotalDegree() const { return std::accumulate(m_components.begin(), m_components.end(), 0); }

  void ToZero() { m_components = 0; }
};

/// A monomial of multiple variables with non-negative exponents
template <class T> class Monomial {
  T coef;
  ExponentVector exps;

  static T int_power(T base, int exp)
  {
    T result = static_cast<T>(1);
    while (exp > 0) {
      if (exp & 1) {
        result *= base;
      }
      base *= base;
      exp >>= 1;
    }
    return result;
  }

public:
  Monomial(std::shared_ptr<VariableSpace> p, const T &x) : coef(x), exps(p) {}
  Monomial(const T &x, const ExponentVector &e) : coef(x), exps(e)
  {
    if (!exps.GetSpace()) {
      throw std::logic_error("Monomial must be defined with respect to a space");
    }
    if (x == static_cast<T>(0)) {
      exps.ToZero();
    }
  }
  Monomial(const Monomial &) = default;
  Monomial(Monomial &&) noexcept = default;
  Monomial &operator=(Monomial &&) noexcept = default;
  ~Monomial() = default;

  // operators
  Monomial &operator=(const Monomial &) = default;

  auto begin() noexcept { return exps.begin(); }
  auto end() noexcept { return exps.end(); }

  auto begin() const noexcept { return exps.begin(); }
  auto end() const noexcept { return exps.end(); }

  bool IsZero() const { return coef == static_cast<T>(0); }
  bool operator==(const Monomial &y) const { return (coef == y.coef && exps == y.exps); }
  bool operator!=(const Monomial &y) const { return (coef != y.coef || exps != y.exps); }
  bool operator<(const Monomial &y) const { return exps < y.exps; }
  Monomial operator*(const Monomial &y) const
  {
    if (GetSpace() != y.GetSpace()) {
      throw std::logic_error("Cannot multiply monomials of different spaces");
    }
    return {coef * y.coef, exps + y.exps};
  }
  Monomial operator+(const Monomial &y) const
  {
    if (exps != y.exps) {
      throw std::logic_error("Attempted to add monomials with different exponents.");
    }
    return {coef + y.coef, exps};
  }
  Monomial &operator+=(const Monomial &y)
  {
    if (exps != y.exps) {
      throw std::logic_error("Attempted to add monomials with different exponents.");
    }
    coef += y.coef;
    return *this;
  }
  Monomial &operator*=(const T &v)
  {
    coef *= v;
    return *this;
  }
  Monomial operator-() const { return {-coef, exps}; }

  std::shared_ptr<VariableSpace> GetSpace() const { return exps.GetSpace(); }
  const T &Coef() const { return coef; }
  int TotalDegree() const { return exps.TotalDegree(); }
  bool IsMultiaffine() const { return exps.IsMultiaffine(); }
  const ExponentVector &ExpV() const { return exps; }

  T Evaluate(const Vector<T> &p_values) const
  {
    T answer = coef;

    auto exp_it = exps.begin();
    auto exp_end = exps.end();
    auto val_it = p_values.begin();
    for (; exp_it != exp_end; ++exp_it, ++val_it) {
      if (const int e = *exp_it; e > 0) {
        answer *= int_power(*val_it, e);
      }
    }

    return answer;
  }
};

// A multivariate polynomial
template <class T> class Polynomial {
  std::shared_ptr<VariableSpace> m_space;
  std::vector<Monomial<T>> m_terms;

  // Arithmetic
  std::vector<Monomial<T>> Adder(const std::vector<Monomial<T>> &,
                                 const std::vector<Monomial<T>> &) const;
  std::vector<Monomial<T>> Mult(const std::vector<Monomial<T>> &,
                                const std::vector<Monomial<T>> &) const;
  Polynomial DivideByPolynomial(const Polynomial &den) const;

  Polynomial TranslateOfMono(const Monomial<T> &, const Vector<T> &) const;
  Polynomial MonoInNewCoordinates(const Monomial<T> &, const Matrix<T> &) const;

public:
  explicit Polynomial(std::shared_ptr<VariableSpace> p) : m_space(p) {}
  // A constant polynomial
  explicit Polynomial(std::shared_ptr<VariableSpace> p, const T &constant) : m_space(p)
  {
    if (constant != static_cast<T>(0)) {
      m_terms.push_back(Monomial<T>(p, constant));
    }
  }
  Polynomial(const Polynomial &) = default;
  Polynomial(Polynomial &&) = default;
  Polynomial &operator=(Polynomial &&) = default;
  // Constructs a polynomial x_{var_no}^exp
  explicit Polynomial(std::shared_ptr<VariableSpace> p, const int var_no, const int exp)
    : m_space(p)
  {
    m_terms.push_back(Monomial<T>(static_cast<T>(1), ExponentVector(p, var_no, exp)));
  }
  // Constructs a polynomial with single monomial
  explicit Polynomial(const Monomial<T> &mono) : m_space(mono.GetSpace())
  {
    if (!m_space) {
      throw std::logic_error("Polynomials must be defined with respect to a space");
    }
    m_terms.push_back(mono);
  }
  ~Polynomial() = default;

  Polynomial &operator=(const Polynomial &) = default;
  Polynomial operator-() const
  {
    Polynomial neg(*this);
    for (auto &term : neg.m_terms) {
      term = -term;
    }
    return neg;
  }
  Polynomial operator-(const Polynomial &p) const
  {
    Polynomial dif(*this);
    dif -= p;
    return dif;
  }
  void operator-=(const Polynomial &p)
  {
    auto neg_terms = p.m_terms;
    for (auto &term : neg_terms) {
      term = -term;
    }
    m_terms = Adder(m_terms, neg_terms);
  }
  Polynomial operator+(const Polynomial<T> &p) const
  {
    Polynomial sum(*this);
    sum += p;
    return sum;
  }
  Polynomial operator+(const T &v) const
  {
    Polynomial result(*this);
    result += v;
    return result;
  }
  void operator+=(const Polynomial &p) { m_terms = Adder(m_terms, p.m_terms); }
  void operator+=(const T &val) { *this += Polynomial<T>(m_space, val); }
  Polynomial operator*(const Polynomial &p) const
  {
    Polynomial prod(*this);
    prod *= p;
    return prod;
  }
  Polynomial operator*(const T &v) const
  {
    Polynomial result(*this);
    result *= v;
    return result;
  }
  void operator*=(const Polynomial &p) { m_terms = Mult(m_terms, p.m_terms); }
  void operator*=(const T &val)
  {
    for (auto &term : m_terms) {
      term *= val;
    }
  }
  Polynomial operator/(const T &val) const
  {
    if (val == static_cast<T>(0)) {
      throw ZeroDivideException();
    }
    return (*this) * (static_cast<T>(1) / val);
  }
  Polynomial operator/(const Polynomial &den) const { return DivideByPolynomial(den); }

  bool operator==(const Polynomial &p) const
  {
    return m_space == p.m_space && m_terms == p.m_terms;
  }
  bool operator!=(const Polynomial &p) const
  {
    return m_space != p.m_space || m_terms != p.m_terms;
  }

  Polynomial pow(int exponent) const;

  //-------------
  // Information
  //-------------

  [[nodiscard]] std::shared_ptr<VariableSpace> GetSpace() const noexcept { return m_space; }
  [[nodiscard]] int GetDimension() const noexcept { return m_space->GetDimension(); }
  [[nodiscard]] int DegreeOfVar(int var_no) const
  {
    return std::accumulate(
        m_terms.begin(), m_terms.end(), 0,
        [&var_no](int v, const Monomial<T> &m) { return std::max(v, m.ExpV()[var_no]); });
  }
  [[nodiscard]] int Degree() const
  {
    return std::accumulate(m_terms.begin(), m_terms.end(), 0, [](int v, const Monomial<T> &m) {
      return std::max(v, m.TotalDegree());
    });
  }
  [[nodiscard]] const std::vector<Monomial<T>> &GetTerms() const noexcept { return m_terms; }
  bool IsZero() const noexcept { return m_terms.empty(); }
  bool IsConstant() const noexcept
  {
    return m_terms.size() == 1 && m_terms.front().TotalDegree() == 0;
  }

  Polynomial LeadingCoefficient(int varnumber) const;
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
  Polynomial PartialDerivative(int varnumber) const;

  Polynomial TranslateOfPoly(const Vector<T> &) const;
  Polynomial PolyInNewCoordinates(const Matrix<T> &) const;
  T MaximalValueOfNonlinearPart(const T &) const;
  Polynomial Normalize() const;
};

} // end namespace Gambit

#endif // POLY_H
