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

template <class T> gOutput &operator<<(gOutput &f, const BFS<T> &)
{
  return f;
}

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
// This is here to avoid a duplicate definition...
class gArray<int>;
class gArray<double>;
class gArray<gRational>;

class gBlock<int>;
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class BFS<double>;
TEMPLATE class BFS<gRational>;

TEMPLATE gOutput &operator<<(gOutput &, const BFS<double> &);
TEMPLATE gOutput &operator<<(gOutput &, const BFS<gRational> &);

TEMPLATE class gArray<BFS<double> >;
TEMPLATE class gArray<BFS<gRational> >;
TEMPLATE class gBlock<BFS<double> >;
TEMPLATE class gBlock<BFS<gRational> >;
TEMPLATE class gList<BFS<double> >;
TEMPLATE class gList<BFS<gRational> >;
TEMPLATE class gNode<BFS<double> >;
TEMPLATE class gNode<BFS<gRational> >;
TEMPLATE class gListIter<BFS<double> >;
TEMPLATE class gListIter<BFS<gRational> >;

TEMPLATE class gBaseMap<int, double>;
TEMPLATE class gOrdMap<int, double>;
TEMPLATE class gBaseMap<int, gRational>;
TEMPLATE class gOrdMap<int, gRational>;

