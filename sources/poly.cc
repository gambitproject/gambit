//#
//# FILE: poly.cc -- Instantiation of polynomial classes
//# @(#) poly.cc	1.0 11/8/95
//#

#include "poly.imp"
#include "double.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class polynomial<gRational>;
TEMPLATE gOutput& operator << (gOutput& output, 
			       const polynomial<gRational>& x);

TEMPLATE class polynomial<int>;
TEMPLATE gOutput& operator << (gOutput& output, 
			       const polynomial<int>& x);

TEMPLATE class polynomial<double>;
TEMPLATE gOutput& operator << (gOutput& output, 
			       const polynomial<double>& x);

TEMPLATE class polynomial<gDouble>;
TEMPLATE gOutput& operator << (gOutput& output, 
			       const polynomial<gDouble>& x);

#include "glist.imp"
TEMPLATE class gNode< polynomial<gRational> >;
TEMPLATE class gList< polynomial<gRational> >;
TEMPLATE gOutput& operator << (gOutput& output, 
			       const gList<polynomial<gRational> >&);

TEMPLATE class gNode< polynomial<int> >;
TEMPLATE class gList< polynomial<int> >;

TEMPLATE class gNode< polynomial<double> >;
TEMPLATE class gList< polynomial<double> >;
TEMPLATE gOutput& operator << (gOutput& output, 
			       const gList<polynomial<double> >&);

TEMPLATE class gNode< polynomial<gDouble> >;
TEMPLATE class gList< polynomial<gDouble> >;

