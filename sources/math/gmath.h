//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// General include file for math library
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

