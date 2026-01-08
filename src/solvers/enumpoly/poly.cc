//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/poly.cc
// Instantiation of polynomial classes
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

#include "poly.h"
#include "polypartial.imp"

namespace Gambit {

template <class T>
std::vector<Monomial<T>> Polynomial<T>::Adder(const std::vector<Monomial<T>> &p_one,
                                              const std::vector<Monomial<T>> &p_two) const
{
  if (p_one.empty()) {
    return p_two;
  }
  if (p_two.empty()) {
    return p_one;
  }
  std::vector<Monomial<T>> answer;
  answer.reserve(p_one.size() + p_two.size());

  auto it1 = p_one.begin();
  auto it2 = p_two.begin();
  auto end1 = p_one.end();
  auto end2 = p_two.end();

  while (it1 != end1 && it2 != end2) {
    const auto &e1 = it1->ExpV();
    const auto &e2 = it2->ExpV();

    if (e1 < e2) {
      answer.push_back(*it1++);
    }
    else if (e2 < e1) {
      answer.push_back(*it2++);
    }
    else {
      T merged = it1->Coef() + it2->Coef();
      if (merged != static_cast<T>(0)) {
        answer.emplace_back(merged, e1); // more efficient than *it1 + *it2
      }
      ++it1;
      ++it2;
    }
  }

  while (it1 != end1) {
    answer.push_back(*it1++);
  }
  while (it2 != end2) {
    answer.push_back(*it2++);
  }
  return answer;
}

template <class T>
std::vector<Monomial<T>> Polynomial<T>::Mult(const std::vector<Monomial<T>> &p_one,
                                             const std::vector<Monomial<T>> &p_two) const
{
  std::vector<Monomial<T>> result;

  if (p_one.empty() || p_two.empty()) {
    return result;
  }

  result.reserve(p_one.size() * p_two.size());
  for (const auto &m1 : p_one) {
    for (const auto &m2 : p_two) {
      result.emplace_back(m1 * m2);
    }
  }
  std::sort(result.begin(), result.end(),
            [](const Monomial<T> &a, const Monomial<T> &b) { return a.ExpV() < b.ExpV(); });
  std::vector<Monomial<T>> merged;
  merged.reserve(result.size());

  auto it = result.begin();
  auto end = result.end();

  while (it != end) {
    Monomial<T> current = *it;
    ++it;
    while (it != end && it->ExpV() == current.ExpV()) {
      current += *it;
      ++it;
    }
    if (!current.IsZero()) {
      merged.emplace_back(std::move(current));
    }
  }
  return merged;
}

template <class T> Polynomial<T> Polynomial<T>::DivideByPolynomial(const Polynomial &den) const
{
  if (den.IsZero()) {
    throw ZeroDivideException();
  }

  // assumes exact divisibility!
  Polynomial result(m_space);
  if (IsZero()) {
    return result;
  }

  if (den.Degree() == 0) {
    return *this / den.NumLeadCoeff();
  }

  int last = GetDimension();
  while (last > 1 && den.DegreeOfVar(last) == 0) {
    --last;
  }

  Polynomial remainder(*this);
  while (!remainder.IsZero()) {
    const int deg_rem = remainder.DegreeOfVar(last);
    const int deg_den = den.DegreeOfVar(last);

    const Polynomial lead_rem = remainder.LeadingCoefficient(last);
    const Polynomial lead_den = den.LeadingCoefficient(last);
    const Polynomial quot = lead_rem / lead_den;

    // x_last^(deg_rem - deg_den)
    const int pow = deg_rem - deg_den;
    const Polynomial mono_pow(m_space, last, pow);

    result += quot * mono_pow;
    remainder -= quot * mono_pow * den;
  }
  return result;
}

template <class T> Polynomial<T> Polynomial<T>::PartialDerivative(int varnumber) const
{
  std::vector<Monomial<T>> terms;
  terms.reserve(m_terms.size());

  for (const auto &term : m_terms) {
    const auto &expv = term.ExpV();
    const int exponent = expv[varnumber];
    if (exponent == 0) {
      continue;
    }
    terms.emplace_back(term.Coef() * static_cast<T>(exponent), expv.DecrementExponent(varnumber));
  }

  Polynomial result(m_space);
  result.m_terms = std::move(terms);
  return result;
}

template <class T> Polynomial<T> Polynomial<T>::LeadingCoefficient(int varnumber) const
{
  Polynomial result(m_space);
  result.m_terms.reserve(m_terms.size());

  const int degree = DegreeOfVar(varnumber);
  if (degree == 0) {
    return result;
  }

  for (const auto &term : m_terms) {
    const auto &expv = term.ExpV();
    if (expv[varnumber] == degree) {
      result.m_terms.emplace_back(term.Coef(), expv.WithZeroExponent(varnumber));
    }
  }
  return result;
}

template <class T> Polynomial<T> Polynomial<T>::pow(int exponent) const
{
  if (exponent < 0) {
    throw std::domain_error("Polynomial::pow: negative exponent not supported.");
  }

  if (exponent == 0) {
    return Polynomial<T>(m_space, static_cast<T>(1));
  }
  if (exponent == 1) {
    return *this;
  }

  Polynomial base(*this);
  Polynomial result(m_space, static_cast<T>(1));
  while (exponent > 0) {
    if (exponent & 1) {
      result *= base;
    }
    exponent >>= 1;
    if (exponent > 0) {
      base *= base;
    }
  }
  return result;
}

template <class T>
Polynomial<T> Polynomial<T>::TranslateOfMono(const Monomial<T> &m,
                                             const Vector<T> &new_origin) const
{
  Polynomial result(m_space, static_cast<T>(1));

  const auto &expv = m.ExpV();
  const int dim = GetDimension();

  for (int i = 1; i <= dim; ++i) {
    const int exponent = expv[i];
    if (exponent == 0) {
      continue;
    }
    Polynomial shifted(m_space, i, 1);
    shifted += Polynomial<T>(m_space, new_origin[i]);
    result *= shifted.pow(exponent);
  }
  result *= m.Coef();
  return result;
}

template <class T> Polynomial<T> Polynomial<T>::TranslateOfPoly(const Vector<T> &new_origin) const
{
  Polynomial answer(m_space);
  answer.m_terms.reserve(m_terms.size());
  for (const auto &mono : m_terms) {
    answer += TranslateOfMono(mono, new_origin);
  }
  return answer;
}

template <class T>
Polynomial<T> Polynomial<T>::MonoInNewCoordinates(const Monomial<T> &m, const Matrix<T> &M) const
{
  Polynomial result(m_space, static_cast<T>(1));
  const auto &expv = m.ExpV();
  const int dim = GetDimension();

  int var_index = 1;

  for (auto exp_it = expv.begin(); exp_it != expv.end(); ++exp_it, ++var_index) {
    const int exponent = *exp_it;
    if (exponent == 0) {
      continue;
    }

    Polynomial linearform(m_space);
    linearform.m_terms.reserve(GetDimension());
    for (int j = 1; j <= dim; ++j) {
      const T &coeff = M(var_index, j);
      if (coeff != static_cast<T>(0)) {
        linearform.m_terms.emplace_back(coeff, ExponentVector(m_space, j, 1));
      }
    }
    result *= linearform.pow(exponent);
  }
  result *= m.Coef();
  return result;
}

template <class T> Polynomial<T> Polynomial<T>::PolyInNewCoordinates(const Matrix<T> &M) const
{
  Polynomial answer(m_space);
  answer.m_terms.reserve(m_terms.size());
  for (const auto &term : m_terms) {
    answer += MonoInNewCoordinates(term, M);
  }
  return answer;
}

template <class T> T Polynomial<T>::MaximalValueOfNonlinearPart(const T &radius) const
{
  T result = static_cast<T>(0);

  for (const auto &term : m_terms) {
    if (const int deg = term.TotalDegree(); deg > 1) {
      T rpow = static_cast<T>(1);
      for (int k = 0; k < deg; ++k) {
        rpow *= radius;
      }
      result += term.Coef() * rpow;
    }
  }
  return result;
}

template <class T> Polynomial<T> Polynomial<T>::Normalize() const
{
  if (m_terms.empty()) {
    return *this;
  }
  const T eps = std::numeric_limits<T>::epsilon();

  if (m_terms.size() == 1 && m_terms.front().TotalDegree() == 0) {
    T c = m_terms.front().Coef();
    if (std::abs(c) <= eps) {
      return *this;
    }
    return Polynomial(m_space, static_cast<T>(c > 0 ? 1 : -1));
  }

  auto it = std::max_element(m_terms.begin(), m_terms.end(),
                             [](const Monomial<T> &a, const Monomial<T> &b) {
                               return std::abs(a.Coef()) < std::abs(b.Coef());
                             });

  const T maxc = it->Coef();
  const T absmax = std::abs(maxc);
  if (absmax <= eps) {
    return *this;
  }
  return *this / maxc;
}

template class Polynomial<double>;
template class PolynomialDerivatives<double>;
template class PolynomialSystemDerivatives<double>;

} // end namespace Gambit
