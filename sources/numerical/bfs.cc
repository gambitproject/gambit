//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of basic feasible solution class
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

#include "math/rational.h"

#include "base/garray.imp"
#include "base/gblock.imp"
#include "base/glist.imp"
#include "base/gmap.imp"
#include "bfs.h"

//--------------------------------------------------------------------------
//                     class gbtBasicFeasibleSolution<T>: member functions
//--------------------------------------------------------------------------

template <class T> gbtBasicFeasibleSolution<T>::gbtBasicFeasibleSolution(void) : gbtOrdMap<int, T>((T) 0)  { }

template <class T> gbtBasicFeasibleSolution<T>::gbtBasicFeasibleSolution(const T &d) : gbtOrdMap<int, T>((T) d)  { }

template <class T> gbtBasicFeasibleSolution<T>::gbtBasicFeasibleSolution(const gbtBasicFeasibleSolution<T> &m) : gbtOrdMap<int, T>(m)  { }

template <class T> int gbtBasicFeasibleSolution<T>::operator==(const gbtBasicFeasibleSolution<T> &M) const
{
  if (length != M.length)  return 0;

  for (int i = 0; i < length; i++)
    if (keys[i] != M.keys[i])  return 0;

  return 1;
}

template <class T> int gbtBasicFeasibleSolution<T>::operator!=(const gbtBasicFeasibleSolution<T> &M) const
{
  return !(*this == M);
}

template <class T> gbtOutput &operator<<(gbtOutput &f, const gbtBasicFeasibleSolution<T> &b)
{
  b.Dump(f);
  return f;
}


template class gbtBasicFeasibleSolution<double>;
template class gbtBasicFeasibleSolution<gbtRational>;

template gbtOutput &operator<<(gbtOutput &, const gbtBasicFeasibleSolution<double> &);
template gbtOutput &operator<<(gbtOutput &, const gbtBasicFeasibleSolution<gbtRational> &);

template class gbtArray<gbtBasicFeasibleSolution<double> >;
template class gbtArray<gbtBasicFeasibleSolution<gbtRational> >;
template class gbtBlock<gbtBasicFeasibleSolution<double> >;
template class gbtBlock<gbtBasicFeasibleSolution<gbtRational> >;
template class gbtList<gbtBasicFeasibleSolution<double> >;
template class gbtList<gbtBasicFeasibleSolution<gbtRational> >;

template class gbtBaseMap<int, double>;
template class gbtOrdMap<int, double>;
template class gbtBaseMap<int, gbtRational>;
template class gbtOrdMap<int, gbtRational>;
