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
//                     class BFS<T>: member functions
//--------------------------------------------------------------------------

template <class T> BFS<T>::BFS(void) : gOrdMap<int, T>((T) 0)  { }

template <class T> BFS<T>::BFS(const T &d) : gOrdMap<int, T>((T) d)  { }

template <class T> BFS<T>::BFS(const BFS<T> &m) : gOrdMap<int, T>(m)  { }

template <class T> int BFS<T>::operator==(const BFS<T> &M) const
{
  if (length != M.length)  return 0;

  for (int i = 0; i < length; i++)
    if (keys[i] != M.keys[i])  return 0;

  return 1;
}

template <class T> int BFS<T>::operator!=(const BFS<T> &M) const
{
  return !(*this == M);
}

template <class T> gOutput &operator<<(gOutput &f, const BFS<T> &b)
{
  b.Dump(f);
  return f;
}


template class BFS<double>;
template class BFS<gRational>;

template gOutput &operator<<(gOutput &, const BFS<double> &);
template gOutput &operator<<(gOutput &, const BFS<gRational> &);

template class gArray<BFS<double> >;
template class gArray<BFS<gRational> >;
template class gBlock<BFS<double> >;
template class gBlock<BFS<gRational> >;
template class gList<BFS<double> >;
template class gList<BFS<gRational> >;

template class gBaseMap<int, double>;
template class gOrdMap<int, double>;
template class gBaseMap<int, gRational>;
template class gOrdMap<int, gRational>;

