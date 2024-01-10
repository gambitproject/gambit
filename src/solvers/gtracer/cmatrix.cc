//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: library/src/gtracer/cmatrix.cc
// Implementation of matrix classes for Gametracer
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

#include <cfloat>

#include "core/matrix.h"
#include "cmatrix.h"


namespace Gambit {
namespace gametracer {

int cmatrix::LUdecomp(cmatrix &LU, std::vector<int> &ix) const
{
  if (m != n || LU.m != LU.n || LU.n != n) {
    throw std::out_of_range("invalid cmatrix in LUdecomp");
  }
  int d = 1, i, j, k;
  LU = *this;
  auto *vv = new double[n];
  double dum;

  k = 0;
  for (i = 0; i < n; i++) {
    vv[i] = fabs(x[k]);
    k++;
    for (j = 1; j < n; j++, k++) { if (vv[i] < (dum = fabs(x[k]))) { vv[i] = dum; }}
    if (vv[i] == (double) 0.0) {
      delete[]vv;
      return 0;
    }
    vv[i] = 1 / vv[i];
  }
  double sum, big;
  int imax;
  for (j = 0; j < n; j++) {
    for (i = 0; i < j; i++) {
      sum = LU.x[i * n + j];
      for (k = 0; k < i; k++) { sum -= LU.x[i * n + k] * LU.x[k * n + j]; }
      LU.x[i * n + j] = sum;
    }
    big = 0;
    for (i = j; i < n; i++) {
      sum = LU.x[i * n + j];
      for (k = 0; k < j; k++) { sum -= LU.x[i * n + k] * LU.x[k * n + j]; }
      LU.x[i * n + j] = sum;
      if ((dum = vv[i] * fabs(sum)) >= big) {
        big = dum;
        imax = i;
      }
    }
    if (j != imax) {
      for (k = 0; k < n; k++) {
        dum = LU.x[imax * n + k];
        LU.x[imax * n + k] = LU.x[j * n + k];
        LU.x[j * n + k] = dum;
      }
      d = -d;
      vv[imax] = vv[j];
    }
    ix[j] = imax;
    if (LU.x[j * n + j] == 0) {
      LU.x[j * n + j] = (double) 1.0e-20;
    }
    if (j != n - 1) {
      dum = 1 / LU.x[j * n + j];
      for (i = j + 1; i < n; i++) { LU.x[i * n + j] *= dum; }
    }
  }
  delete [] vv;
  return d;
}

void cmatrix::LUbacksub(std::vector<int> &ix, cvector &b) const
{
  if (n != m) {
    throw std::out_of_range("invalid cmatrix in LUbacksub");
  }
  int ip, ii = -1, i;
  double sum;

  for (i = 0; i < n; i++) {
    ip = ix[i];
    sum = b[ip];
    b[ip] = b[i];
    if (ii != -1) {
      for (int j = ii; j <= i - 1; j++) { sum -= x[i * n + j] * b[j]; }
    }
    else if (sum != 0) { ii = i; }
    b[i] = sum;
  }
  for (i = n - 1; i >= 0; i--) {
    sum = b[i];
    for (int j = i + 1; j <= n - 1; j++) { sum -= x[i * n + j] * b[j]; }
    b[i] = sum / x[i * n + i];
  }
}

bool cmatrix::solve(cvector &b, cvector &ret) const
{
  if (m != n) {
    throw std::out_of_range("invalid cmatrix in solve");
  }
  for (int i = 0; i < n; i++) { ret[i] = b[i]; }
  std::vector<int> ix(n);
  cmatrix a(n, n);

  if (!LUdecomp(a, ix)) {
    return false;
  }
  a.LUbacksub(ix, ret);
  return true;
}

double cmatrix::adjoint()
{
  int i, j, i0, j0, maxi, lastj = -1;
  double max, pivot;
  std::vector<int> r(m);
  std::vector<int> r2(m);
  std::vector<int> c(m);
  double D = 1.0;
  Gambit::Matrix<double> retval(0, m - 1, 0, m - 1);
  for (i = 0; i < m; i++) {
    for (j = 0; j < m; j++) {
      retval(i, j) = x[i * n + j];
    }
  }

  for (i = 0; i < m; i++) {
    r[i] = -1;
  }
  for (j = 0; j < m; j++) {
    if (D == 0.0) {
      return DBL_MAX;
    }
    max = -1.0;
    maxi = -1;
    for (i = 0; i < m; i++) {
      if (r[i] < 0 && fabs(retval(i, j)) > max) {
        max = fabs(retval(i, j));
        maxi = i;
      }
    }
    if (j != lastj && max == 0.0) {
      if (lastj >= 0) {
        return DBL_MAX;
      }
      lastj = j;
      if (j != m - 1) {
        continue;
      }
    }
    if (maxi == -1) {
      throw std::runtime_error("unable to compute cmatrix::adjoint()");
    }

    i = maxi;
    pivot = retval(i, j);
    for (i0 = 0; i0 < m; i0++) {
      if (i0 != i) {
        for (j0 = 0; j0 < m; j0++) {
          if (j0 != j) {
            retval(i0, j0) *= pivot;
            retval(i0, j0) -= retval(i0, j) * retval(i, j0);
            retval(i0, j0) /= D;
          }
        }
      }
    }
    for (i0 = 0; i0 < m; i0++) {
      retval(i0, j) = -retval(i0, j);
    }
    retval(i, j) = D;
    D = pivot;
    r[i] = j;
    c[j] = i;
    if (j == lastj) {
      break;
    }
    if (j == m - 1 && lastj >= 0) {
      j = lastj - 1;
    }
  }
  int s = 0;
  i = 0;
  r2 = r;
  while (i < m - 1) {
    j = r2[i];
    if (i != j) {
      s++;
      r2[i] = r2[j];
      r2[j] = j;
    }
    else {
      i++;
    }
  }
  for (i = 0; i < m; i++) {
    for (j = 0; j < m; j++) {
      x[i * n + j] = retval(c[i], r[j]);
    }
  }
  if (s % 2 == 1) {
    negate();
    D = -D;
  }
  return D;
}


double cmatrix::trace() const
{
  if (n != m) {
    throw std::out_of_range("non-square matrix in call to trace()");
  }
  double sum = 0.0;
  for (int i = 0; i < n; i++) {
    sum += x[i * n + i];
  }
  return sum;
}

} // end namespace Gambit::gametracer
} // end namespace Gambit
