//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/gtracer/cmatrix.h
// Definition of matrix classes for Gametracer
// This file is based on GameTracer v0.2, which is
// Copyright (c) 2002, Ben Blum and Christian Shelton
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

#ifndef GAMBIT_SOLVERS_GTRACER_CMATRIX_H
#define GAMBIT_SOLVERS_GTRACER_CMATRIX_H

#include <algorithm>
#include <cmath>
#include <cstring>
#include <utility>
#include <vector>

#include "core/util.h"

namespace Gambit::gametracer {

class cvector {
  friend class cmatrix;

public:
  class iterator {
  private:
    cvector *m_vector;
    int m_index;

  public:
    using iterator_category = std::input_iterator_tag;
    using difference_type = typename std::vector<double>::iterator::difference_type;
    using value_type = double;
    using pointer = value_type *;
    using reference = value_type &;

    iterator(cvector *p_vector, int p_index) : m_vector(p_vector), m_index(p_index) {}
    reference operator*() { return (*m_vector)[m_index]; }
    iterator &operator++()
    {
      m_index++;
      return *this;
    }
    iterator operator++(int)
    {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }
    bool operator==(const iterator &it) const
    {
      return (m_vector == it.m_vector) && (m_index == it.m_index);
    }
  };

  class const_iterator {
  private:
    const cvector *m_vector;
    int m_index;

  public:
    using iterator_category = std::input_iterator_tag;
    using difference_type = typename std::vector<double>::iterator::difference_type;
    using value_type = double;
    using pointer = value_type *;
    using reference = value_type &;

    const_iterator(const cvector *p_vector, int p_index) : m_vector(p_vector), m_index(p_index) {}
    double operator*() const { return (*m_vector)[m_index]; }
    const_iterator &operator++()
    {
      m_index++;
      return *this;
    }
    const_iterator operator++(int)
    {
      const_iterator tmp = *this;
      ++(*this);
      return tmp;
    }
    bool operator==(const const_iterator &it) const
    {
      return (m_vector == it.m_vector) && (m_index == it.m_index);
    }
  };

  explicit cvector(int _m) : m(_m), x(new double[_m]) {}

  ~cvector() { delete[] x; }

  cvector(const cvector &v) : m(v.m), x(new double[m]) { memcpy(x, v.x, m * sizeof(double)); }

  cvector(cvector &&v) noexcept : m(v.m), x(v.x)
  {
    v.m = 0;
    v.x = nullptr;
  }

  cvector(int _m, double a) : m(_m), x(new double[_m])
  {
    for (int i = 0; i < m; i++) {
      x[i] = a;
    }
  }

  cvector &operator=(cvector &&v) noexcept
  {
    std::swap(m, v.m);
    std::swap(x, v.x);
    return *this;
  }

  cvector &operator=(const cvector &v)
  {
    if (&v == this) {
      return *this;
    }
    if (v.m != m) {
      delete[] x;
      m = v.m;
      x = new double[m];
    }
    memcpy(x, v.x, m * sizeof(double));
    return *this;
  }

  double operator*(const cvector &v) const
  {
    if (m != v.m) {
      throw std::out_of_range("invalid cvector dot product");
    }
    double ret = 0.0;
    for (int i = 0; i < m; i++) {
      ret += x[i] * v.x[i];
    }
    return ret;
  }

  double operator[](int i) const { return x[i]; }

  double &operator[](int i) { return x[i]; }

  /// Return a forward iterator starting at the beginning of the vector
  iterator begin() { return {this, 0}; }
  /// Return a const forward iterator starting at the beginning of the vector
  const_iterator cbegin() const { return {this, 0}; }
  /// Return a const forward iterator past the end of the vector
  const_iterator cend() const { return {this, m}; }

  cvector &operator+=(const cvector &v)
  {
    if (v.m != m) {
      throw std::out_of_range("invalid cvector addition");
    }
    for (int i = 0; i < m; i++) {
      x[i] += v.x[i];
    }
    return *this;
  }

  cvector &operator-=(const cvector &v)
  {
    if (v.m != m) {
      throw std::out_of_range("invalid cvector subtraction");
    }
    for (int i = 0; i < m; i++) {
      x[i] -= v.x[i];
    }
    return *this;
  }

  cvector &operator*=(double a)
  {
    for (int i = 0; i < m; i++) {
      x[i] *= a;
    }
    return *this;
  }

  cvector &operator/=(double a)
  {
    for (int i = 0; i < m; i++) {
      x[i] /= a;
    }
    return *this;
  }

  double max() const
  {
    double ma = x[0];
    for (int i = 1; i < m; i++) {
      ma = std::max(ma, x[i]);
    }
    return ma;
  }

  double min() const
  {
    double mi = x[0];
    for (int i = 1; i < m; i++) {
      mi = std::min(mi, x[i]);
    }
    return mi;
  }

  double norm2() const
  {
    double ret = x[0] * x[0];
    for (int i = 1; i < m; i++) {
      ret += x[i] * x[i];
    }
    return ret;
  }

  double norm() const { return std::sqrt(norm2()); }

  double *values() const { return x; }

  int getm() const { return m; }

  void unfuzz(double fuzz)
  {
    for (int i = 0; i < m; i++) {
      if (x[i] < fuzz) {
        x[i] = 0.0;
      }
    }
  }

  void support(std::vector<int> &s)
  {
    for (int i = 0; i < m; i++) {
      if (!s[i]) {
        x[i] = 0.0;
      }
    }
  }

  void negate()
  {
    for (int i = 0; i < m; i++) {
      x[i] = -x[i];
    }
  }

private:
  int m;
  double *x;
};

inline cvector operator/(const cvector &a, const double &b) { return cvector(a) /= b; }

class cmatrix {
public:
  explicit cmatrix(int _m = 1, int _n = 1) : m(_m), n(_n), s(_m * _n), x(new double[s]) {}

  ~cmatrix() { delete[] x; }

  cmatrix(const cmatrix &ma) : m(ma.m), n(ma.n), s(ma.s), x(new double[s])
  {
    memcpy(x, ma.x, s * sizeof(double));
  }

  cmatrix(int _m, int _n, const double &a, bool diaonly = false)
    : m(_m), n(_n), s(_m * _n), x(new double[s])
  {
    if (diaonly) {
      int i;
      // for(i=0;i<s;i++) x[i] = 0;
      memset(x, 0, s * sizeof(double));
      if (n >= m) {
        for (i = 0; i < m; i++) {
          x[i * n + i] = a;
        }
      }
      else {
        for (i = 0; i < n; i++) {
          x[i * n + i] = a;
        }
      }
    }
    else {
      int i;
      if (a == 0.0) {
        memset(x, 0, s * sizeof(double));
      }
      else {
        for (i = 0; i < s; i++) {
          x[i] = a;
        }
      }
    }
  }

  cmatrix(cmatrix &&ma) noexcept : m(ma.m), n(ma.n), s(ma.s), x(ma.x)
  {
    ma.m = ma.n = ma.s = 0;
    ma.x = nullptr;
  }

  cmatrix &operator=(cmatrix &&ma) noexcept
  {
    std::swap(m, ma.m);
    std::swap(n, ma.n);
    std::swap(s, ma.s);
    std::swap(x, ma.x);
    return *this;
  }

  cmatrix &operator=(const cmatrix &ma)
  {
    if (&ma == this) {
      return *this;
    }
    if (ma.n != n || ma.m != m) {
      s = ma.s;
      m = ma.m;
      n = ma.n;
      delete[] x;
      x = new double[s];
    }
    // for(int i=0;i<s;i++) x[i] = ma.x[i];
    memcpy(x, ma.x, s * sizeof(double));
    return *this;
  }

  double operator()(int i, int j) const { return x[i * n + j]; }

  double &operator()(int i, int j) { return x[i * n + j]; }

  cmatrix &operator+=(const cmatrix &ma)
  {
    if (m != ma.m || n != ma.n) {
      throw std::out_of_range("invalid cmatrix addition");
    }
    for (int i = 0; i < s; i++) {
      x[i] += ma.x[i];
    }
    return *this;
  }

  cmatrix &operator-=(const cmatrix &ma)
  {
    if (m != ma.m || n != ma.n) {
      throw std::out_of_range("invalid cmatrix subtraction");
    }
    for (int i = 0; i < s; i++) {
      x[i] -= ma.x[i];
    }
    return *this;
  }

  cmatrix &operator*=(const cmatrix &ma)
  {
    if (n != ma.m || n != ma.n) {
      throw std::out_of_range("invalid cmatrix multiplication");
    }
    int i, j, k, c = 0;
    std::vector<double> newrow(n);
    for (i = 0; i < m; i++) {
      for (j = 0; j < n; j++) {
        newrow[j] = 0;
        for (k = 0; k < n; k++) {
          newrow[j] += x[c + k] * ma.x[k * n + j];
        }
      }
      for (j = 0; j < n; j++, c++) {
        x[c] = newrow[j];
      }
    }
    return *this;
  }

  cmatrix &operator/=(const double &a)
  {
    for (int i = 0; i < s; i++) {
      x[i] /= a;
    }
    return *this;
  }

  // LU decomposition -- ix is the row permutations
  int LUdecomp(cmatrix &LU, std::vector<int> &ix) const;

  // LU back substitution --
  //    ix from above fn call (this should be an LU combination)
  void LUbacksub(std::vector<int> &ix, cvector &col) const;

  // solves equation Ax=b (A is this, x is the returned value)
  bool solve(cvector &b, cvector &dest) const;

  void negate()
  {
    for (int i = 0; i < s; i++) {
      x[i] = -x[i];
    }
  }

  double adjoint();

  void multiply(const cvector &source, cvector &dest) const
  {
    if (n != source.m || m != dest.m) {
      throw DimensionException();
    }
    int i, j, c = 0;
    for (i = 0; i < m; i++) {
      dest[i] = 0;
      for (j = 0; j < n; j++, c++) {
        dest[i] += x[c] * source[j];
      }
    }
  }

private:
  int m, n, s;
  double *x;
};

} // end namespace Gambit::gametracer

#endif // GAMBIT_SOLVERS_GTRACER_CMATRIX_H
