//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: library/include/gtracer/cmatrix.h
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

#ifndef GAMBIT_GTRACER_CMATRIX_H
#define GAMBIT_GTRACER_CMATRIX_H

#include <iostream>
#include <cmath>
#include <cstring>
#include <vector>

namespace Gambit {
namespace gametracer {

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
    pointer operator->() { return &(*m_vector)[m_index]; }
    iterator &operator++()
    {
      m_index++;
      return *this;
    }
    bool operator==(const iterator &it) const
    {
      return (m_vector == it.m_vector) && (m_index == it.m_index);
    }
    bool operator!=(const iterator &it) const { return !(*this == it); }
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
    double operator->() const { return (*m_vector)[m_index]; }
    const_iterator &operator++()
    {
      m_index++;
      return *this;
    }
    bool operator==(const const_iterator &it) const
    {
      return (m_vector == it.m_vector) && (m_index == it.m_index);
    }
    bool operator!=(const const_iterator &it) const { return !(*this == it); }
  };

  cvector() : m(1), x(new double[1]) {}

  explicit cvector(int m) : m(m), x(new double[m]) {}

  ~cvector() { delete[] x; }

  cvector(const cvector &v) : m(v.m), x(new double[m]) { memcpy(x, v.x, m * sizeof(double)); }

  cvector(int m, double a) : m(m), x(new double[m])
  {
    for (int i = 0; i < m; i++) {
      x[i] = a;
    }
  }

  cvector operator-() const
  {
    cvector ret(m);
    for (int i = 0; i < m; i++) {
      ret.x[i] = -x[i];
    }
    return ret;
  }

  cvector &operator=(double a)
  {
    for (int i = 0; i < m; i++) {
      x[i] = a;
    }
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

  size_t size() const { return m; }

  double operator[](int i) const { return x[i]; }

  double &operator[](int i) { return x[i]; }

  /// Return a forward iterator starting at the beginning of the vector
  iterator begin() { return iterator(this, 0); }
  /// Return a forward iterator past the end of the vector
  iterator end() { return iterator(this, m); }
  /// Return a const forward iterator starting at the beginning of the vector
  const_iterator begin() const { return const_iterator(this, 0); }
  /// Return a const forward iterator past the end of the vector
  const_iterator end() const { return const_iterator(this, m); }
  /// Return a const forward iterator starting at the beginning of the vector
  const_iterator cbegin() const { return const_iterator(this, 0); }
  /// Return a const forward iterator past the end of the vector
  const_iterator cend() const { return const_iterator(this, m); }

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

  cvector &operator+=(double a)
  {
    for (int i = 0; i < m; i++) {
      x[i] += a;
    }
    return *this;
  }

  cvector &operator-=(double a)
  {
    for (int i = 0; i < m; i++) {
      x[i] -= a;
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
    double t, ma = x[0];
    for (int i = 1; i < m; i++) {
      if ((t = x[i]) > ma) {
        ma = t;
      }
    }
    return ma;
  }

  double min() const
  {
    double t, mi = x[0];
    for (int i = 1; i < m; i++) {
      if ((t = x[i]) < mi) {
        mi = t;
      }
    }
    return mi;
  }

  double normalize()
  {
    double norm = 0.0;
    for (int i = 0; i < m; i++) {
      norm += x[i] * x[i];
    }
    norm = std::sqrt(norm);
    for (int i = 0; i < m; i++) {
      x[i] /= norm;
    }
    return norm;
  }

  bool operator==(const cvector &v) const
  {
    if (m != v.m) {
      return false;
    }
    return memcmp(x, v.x, m * sizeof(double)) == 0;
  }

  bool operator==(const double &a) const
  {
    for (int i = 0; i < m; i++) {
      if (a != x[i]) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(const cvector &v) const
  {
    if (m != v.m) {
      return true;
    }
    return memcmp(x, v.x, m * sizeof(double)) != 0;
  }

  bool operator!=(const double &a) const
  {
    for (int i = 0; i < m; i++) {
      if (a != x[i]) {
        return true;
      }
    }
    return false;
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

  friend std::ostream &operator<<(std::ostream &s, const cvector &v);

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

  double sum()
  {
    double total = 0.0;
    for (int i = 0; i < m; i++) {
      total += x[i];
    }
    return total;
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

inline cvector operator+(const cvector &a, const cvector &b) { return cvector(a) += b; }

inline cvector operator-(const cvector &a, const cvector &b) { return cvector(a) -= b; }

inline cvector operator+(const cvector &a, const double &b) { return cvector(a) += b; }

inline cvector operator-(const cvector &a, const double &b) { return cvector(a) -= b; }

inline cvector operator+(const double &a, const cvector &b) { return cvector(b) += a; }

inline cvector operator-(const double &a, const cvector &b) { return cvector(b.getm(), a) -= b; }

inline cvector operator*(const cvector &a, const double &b) { return cvector(a) *= b; }

inline cvector operator*(const double &a, const cvector &b) { return cvector(b) *= a; }

inline cvector operator/(const cvector &a, const double &b) { return cvector(a) /= b; }

inline std::ostream &operator<<(std::ostream &s, const cvector &v)
{
  for (int i = 0; i < v.m; i++) {
    s << v.x[i];
    if (i != v.m) {
      s << ' ';
    }
  }
  return s;
}

class cmatrix {
public:
  explicit cmatrix(int m = 1, int n = 1)
  {
    this->m = m;
    this->n = n;
    s = m * n;
    x = new double[s];
  }

  ~cmatrix() { delete[] x; }

  cmatrix(const cmatrix &ma, bool transpose = false)
  {
    s = ma.m * ma.n;
    x = new double[s];
    if (transpose) {
      int i, j, c;
      n = ma.m;
      m = ma.n;
      c = 0;
      for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++, c++) {
          x[c] = ma.x[i + j * m];
        }
      }
    }
    else {
      n = ma.n;
      m = ma.m;
      int i;
      for (i = 0; i < s; i++) {
        x[i] = ma.x[i];
      }
    }
  }

  cmatrix(int m, int n, const double &a, bool diaonly = false)
  {
    this->m = m;
    this->n = n;
    s = m * n;
    x = new double[s];
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

  // put v on the diagonal
  cmatrix(int m, int n, const cvector &v)
  {
    this->m = m;
    this->n = n;
    s = m * n;
    x = new double[s];
    // for(int i=0;i<s;i++) x[i] = 0;
    memset(x, 0, s * sizeof(double));
    int l = m;
    if (n < l) {
      l = n;
    }
    if (v.m < l) {
      l = v.m;
    }
    for (int i = 0, c = 0; i < l; i++, c += n + 1) {
      x[c] = v.x[i];
    }
  }

  explicit cmatrix(const cvector &v)
  {
    m = v.m;
    n = 1;
    s = m;
    x = new double[s];
    // for(int i=0;i<s;i++) x[i] = v.x[i];
    memcpy(x, v.x, s * sizeof(double));
  }

  // forms a cmatrix of the outer product (ie v1*v2') -- v2 is
  // "transposed" temporarily for this operation
  cmatrix(const cmatrix &v1, const cmatrix &v2)
  {
    if (v1.n != v2.n) {
      s = 1;
      m = 1;
      n = 1;
      x = new double[1];
      // x[0] = NaN;
      // x[0] = 0.0/0.0;
      x[0] = 0;
    }
    else {
      n = v2.m;
      m = v1.m;
      s = n * m;
      x = new double[s];
      int i, j, k, c = 0;
      for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++, c++) {
          x[c] = 0;
          for (k = 0; k < v1.n; k++) {
            x[c] += v1.x[i * v1.n + k] * v2.x[j * v1.n + k];
          }
        }
      }
    }
  }

  cmatrix(const cvector &v1, const cvector &v2)
  {
    n = v2.m;
    m = v1.m;
    s = n * m;
    x = new double[s];
    int i, j, c = 0;
    for (i = 0; i < m; i++) {
      for (j = 0; j < n; j++, c++) {
        x[c] = v1.x[i] * v2.x[j];
      }
    }
  }

  cmatrix operator-() const
  {
    cmatrix ret(m, n);
    for (int i = 0; i < s; i++) {
      ret.x[i] = -x[i];
    }
    return ret;
  }

  cmatrix &operator=(double a)
  {
    if (a == 0) {
      memset(x, 0, s * sizeof(double));
    }
    else {
      for (int i = 0; i < s; i++) {
        x[i] = a;
      }
    }
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

  cmatrix operator*(const cmatrix &ma) const
  {
    if (n != ma.m) {
      throw std::out_of_range("invalid cmatrix multiply");
    }
    cmatrix ret(m, ma.n);
    int c = 0;
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < ma.n; j++, c++) {
        ret.x[c] = 0;
        for (int k = 0; k < n; k++) {
          ret.x[c] += x[i * n + k] * ma.x[k * ma.n + j];
        }
      }
    }
    return ret;
  }

  cvector operator*(const cvector &v) const
  {
    if (n != v.m) {
      throw std::out_of_range("invalid cvector-cmatrix multiply");
    }
    cvector ret(m);
    int c = 0;
    for (int i = 0; i < m; i++, c += n) {
      ret.x[i] = 0;
      for (int j = 0; j < n; j++) {
        ret.x[i] += x[c + j] * v.x[j];
      }
    }
    return ret;
  }

  double operator()(int i, int j) const { return x[i * n + j]; }

  double &operator()(int i, int j) { return x[i * n + j]; }

  cmatrix t() const { return {*this, true}; }

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

  cmatrix &operator+=(const double &a)
  {
    for (int i = 0; i < s; i++) {
      x[i] += a;
    }
    return *this;
  }

  cmatrix &operator-=(const double &a)
  {
    for (int i = 0; i < s; i++) {
      x[i] -= a;
    }
    return *this;
  }

  cmatrix &operator*=(const double &a)
  {
    for (int i = 0; i < s; i++) {
      x[i] *= a;
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

  double max() const
  {
    double t, ma = x[0];
    for (int i = 1; i < s; i++) {
      if ((t = x[i]) > ma) {
        ma = t;
      }
    }
    return ma;
  }

  double min() const
  {
    double t, mi = x[0];
    for (int i = 1; i < s; i++) {
      if ((t = x[i]) < mi) {
        mi = t;
      }
    }
    return mi;
  }

  bool operator==(const cmatrix &ma) const
  {
    if (ma.n != n || ma.m != m) {
      return false;
    }
    return memcmp(ma.x, x, s * sizeof(double)) == 0;
  }

  bool operator==(const double &a) const
  {
    for (int i = 0; i < s; i++) {
      if (x[i] != a) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(const cmatrix &ma) const
  {
    if (ma.n != n || ma.m != m) {
      return true;
    }
    return memcmp(ma.x, x, s * sizeof(double)) != 0;
  }

  bool operator!=(const double &a) const
  {
    for (int i = 0; i < s; i++) {
      if (x[i] != a) {
        return true;
      }
    }
    return false;
  }

  // returns the square of the frobenius norm
  double norm2() const
  {
    double ret = x[0] * x[0];
    for (int i = 1; i < s; i++) {
      ret += x[i] * x[i];
    }
    return ret;
  }

  // returns the frobenius norm
  double norm() const { return std::sqrt(norm2()); }

  friend std::ostream &operator<<(std::ostream &s, const cmatrix &ma);

  friend std::istream &operator>>(std::istream &s, cmatrix &ma);

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

  double trace() const;

  void multiply(const cvector &source, cvector &dest) const
  {
    // assert(n == source.m && m == dest.m);
    int i, j, c = 0;
    for (i = 0; i < m; i++) {
      dest[i] = 0;
      for (j = 0; j < n; j++, c++) {
        dest[i] += x[c] * source[j];
      }
    }
  }

  int getm() const { return m; }

  int getn() const { return n; }

private:
  int m, n, s;
  double *x;
};

inline cmatrix operator+(const cmatrix &a, const cmatrix &b) { return cmatrix(a) += b; }

inline cmatrix operator-(const cmatrix &a, const cmatrix &b) { return cmatrix(a) -= b; }

inline cmatrix operator+(const cmatrix &a, const double &b) { return cmatrix(a) += b; }

inline cmatrix operator-(const cmatrix &a, const double &b) { return cmatrix(a) -= b; }

inline cmatrix operator+(const double &a, const cmatrix &b) { return cmatrix(b) += a; }

inline cmatrix operator-(const double &a, const cmatrix &b)
{
  return cmatrix(b.getn(), b.getm(), a) -= b;
}

inline cmatrix operator*(const cmatrix &a, const double &b) { return cmatrix(a) *= b; }

inline cmatrix operator*(const double &b, const cmatrix &a) { return cmatrix(a) *= b; }

inline cmatrix operator/(const cmatrix &a, const double &b) { return cmatrix(a) /= b; }

inline std::ostream &operator<<(std::ostream &s, const cmatrix &ma)
{
  for (int i = 0; i < ma.s; i++) {
    if (i % ma.n == 0) {
      s << std::endl;
    }
    s << ma.x[i];
    if (i != ma.s) {
      s << ' ';
    }
  }
  return s;
}

} // namespace gametracer
} // end namespace Gambit

#endif // GAMBIT_GTRACER_CMATRIX_H
