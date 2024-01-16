//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/complex.h
// Declaration of a complex number class
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

#ifndef GCOMPLEX_H
#define GCOMPLEX_H

#include <cmath>

class gComplex {
protected:
  double re;
  double im;

public:
  // Constructors, and the destructor
  gComplex() : re(0.0), im(0.0) {}
  gComplex(double p_re, double p_im) : re(p_re), im(p_im) {}
  gComplex(const gComplex &) = default;
  explicit gComplex(int n) : re(n), im(0.0) {}

  ~gComplex() = default;

  // Operators
  double RealPart() const { return re; }
  double ImaginaryPart() const { return im; }

  gComplex &operator=(const gComplex &y) = default;

  bool operator==(const gComplex &y) const { return (re == y.re && im == y.im); }
  bool operator!=(const gComplex &y) const { return (re != y.re || im != y.im); }

  gComplex operator+(const gComplex &y) const { return {re + y.re, im + y.im}; }
  gComplex operator-(const gComplex &y) const { return {re - y.re, im - y.im}; }
  gComplex operator*(const gComplex &y) const
  {
    return {re * y.re - im * y.im, re * y.im + im * y.re};
  }
  gComplex operator/(const gComplex &y) const;

  gComplex operator-() const { return {-re, -im}; }

  void operator+=(const gComplex &y)
  {
    re += y.re;
    im += y.im;
  }
  void operator-=(const gComplex &y)
  {
    re -= y.re;
    im -= y.im;
  }
  void operator*=(const gComplex &y)
  {
    re = re * y.re - im * y.im;
    im = re * y.im + im * y.re;
  }
  void operator/=(const gComplex &y);

  // friends outside the class
  friend double fabs(const gComplex &x);
  friend gComplex pow(const gComplex &x, long y);
};

inline double fabs(const gComplex &x) { return sqrt(x.re * x.re + x.im * x.im); }

#endif // GCOMPLEX_H
