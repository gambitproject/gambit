//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// General include file for math library
//

#ifndef GMATH_H
#define GMATH_H

#include "gnumber.h"

template <class T> class gArray;

template <class T> T gmin(const T &a, const T &b);
template <class T> T gmax(const T &a, const T &b);
template <class T> T gmax(const gArray<T> &);

double abs(double a);
inline double abs(long double a) { return abs((double) a); }
gNumber abs(const gNumber &);

int sign(const double &a);

double FromText(const gText &, double &);

#endif  // GMATH_H

