//
// FILE: include/math/math.h -- General include file for math library
//
// $Id$
//

#ifndef MATH_H
#define MATH_H

#include "math/gnumber.h"

template <class T> class gArray;

template <class T> T gmin(const T &a, const T &b);
template <class T> T gmax(const T &a, const T &b);
template <class T> T gmax(const gArray<T> &);

double abs(double a);
gNumber abs(const gNumber &);

int sign(const double &a);

double FromText(const gText &, double &);

#endif  // MATH_H

