//
// FILE: bfs.cc -- Instantiation of Basic Feasible Solutions class
//
// $Id$
//

#include "rational.h"

#include "garray.imp"
#include "gblock.imp"
#include "glist.imp"
#include "gmap.imp"
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

