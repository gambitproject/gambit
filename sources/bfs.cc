//#
//# FILE: bfs.cc -- Instantiation of Basic Feasible Solutions class
//#
//# $Id$
//#

#include "rational.h"

#include "garray.imp"
#include "gblock.imp"
#include "gmap.imp"
#include "gtableau.imp"

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

TEMPLATE class gBaseMap<int, double>;
TEMPLATE class gOrdMap<int, double>;
TEMPLATE class gBaseMapMessage<int, double>;
TEMPLATE class gBaseMap<int, gRational>;
TEMPLATE class gOrdMap<int, gRational>;
TEMPLATE class gBaseMapMessage<int, gRational>;

