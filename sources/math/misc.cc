//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Miscellaneous mathematical functions
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

#include "gmath.h"
#include "double.h"

//--------------------------------------------------------------------------
//                      Simple mathematical functions
//--------------------------------------------------------------------------

template <class T> T gmin(const T &a, const T &b)
{
  if (a < b)   return a;   else return b;
}

template <class T> T gmax(const T &a, const T &b)
{
  if (a > b)   return a;   else return b;
}

template <class T> T gmax(const gArray<T> &p_array)
{
  T max = p_array[1];

  for (int i = 2; i <= p_array.Length(); i++) {
    if (p_array[i] > max)
      max = p_array[i];
  }

  return max;
}

//--------------------------------------------------------------------------
//                     Template function instantiations
//--------------------------------------------------------------------------

template int gmin(const int &a, const int &b);
template float gmin(const float &a, const float &b);
template double gmin(const double &a, const double &b);
template gRational gmin(const gRational &a, const gRational &b);
template gDouble gmin(const gDouble &a, const gDouble &b);

template int gmax(const int &a, const int &b);
template float gmax(const float &a, const float &b);
template double gmax(const double &a, const double &b);
template gRational gmax(const gRational &a, const gRational &b);
template gDouble gmax(const gDouble &a, const gDouble &b);

template int gmax(const gArray<int> &);

#ifndef hpux
double abs(double a)
{
  if (a >= 0.0)   return a;   else return -a;
}
#endif   //# hpux

gNumber abs(const gNumber &a)
{
  if (a > gNumber(0))  return a;  else return -a;
}

//
// Nasty little hack to make Borland C happy
#ifdef __BORLANDC__
inline
#endif   // __BORLANDC__
int sign(const double &a)
{
  if (a > 0.0)   return 1;
  if (a < 0.0)   return -1;
  return 0;
}

// this two-step process allows us to read in a double using either the
// standard form xxx.xxxx or a/b form.
double FromText(const gText &f,double &d)
{
  gRational R;
  FromText(f, R);
  d = (double) R;
  return d;
}

