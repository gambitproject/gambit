//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Top-level include file for libgambit
//
// This file is part of Gambit
// Copyright (c) 2005, The Gambit Project
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

#ifndef LIBGAMBIT_H
#define LIBGAMBIT_H

#include "base.h"
#include "garray.h"
#include "glist.h"
#include "grarray.h"

template <class T> T gmin(const T &a, const T &b)
{ if (a < b) return a; else return b; }

template <class T> T gmax(const T &a, const T &b)
{ if (a > b) return a; else return b; }

inline double abs(double a)
{ return (a >= 0.0) ? a : -a; }

inline int sign(const double &a)
{
  if (a > 0.0)   return 1;
  if (a < 0.0)   return -1;
  return 0;
}

inline double pow(double x, long y)
{ return pow(x, (double) y); }

#include "rational.h"
#include "gnumber.h"

#include "gvector.h"
#include "matrix.h"

#include "game.h"

#include "efstrat.h"
#include "behav.h"
#include "efgiter.h"

#include "nfstrat.h"
#include "mixed.h"
#include "nfgiter.h"

#endif // LIBGAMBIT_H
