//#
//# FILE: poly.cc -- Instantiation of polynomial classes
//# @(#) poly.cc	1.0 11/8/95
//#

#include "poly.imp"
#include "double.h"
#include "gnumber.h"

template class polynomial<gRational>;
template gOutput& operator << (gOutput& output, 
			       const polynomial<gRational>& x);

template class polynomial<int>;
template gOutput& operator << (gOutput& output,
			       const polynomial<int>& x);

template class polynomial<double>;
template gOutput& operator << (gOutput& output, 
			       const polynomial<double>& x);

template class polynomial<gDouble>;
template gOutput& operator << (gOutput& output, 
			       const polynomial<gDouble>& x);

template class polynomial<gNumber>;
template gOutput& operator << (gOutput& output, 
			       const polynomial<gNumber>& x);

#include "glist.imp"
template class gNode< polynomial<gRational> >;
template class gList< polynomial<gRational> >;
template gOutput& operator << (gOutput& output, 
			       const gList<polynomial<gRational> >&);

template class gNode< polynomial<int> >;
template class gList< polynomial<int> >;

template class gNode< polynomial<double> >;
template class gList< polynomial<double> >;
template gOutput& operator << (gOutput& output, 
			       const gList<polynomial<double> >&);

template class gNode< polynomial<gDouble> >;
template class gList< polynomial<gDouble> >;

template class gNode< polynomial<gNumber> >;
template class gList< polynomial<gNumber> >;
template gOutput& operator << (gOutput& output, 
			       const gList<polynomial<gNumber> >&);

