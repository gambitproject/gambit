//
// FILE: ideal.cc  
//        -- Instantiations of classes gBasis and gIdeal
//
// $Id$
//

#include "gpolylst.imp"
#include "double.h"
#include "gnumber.h"
#include "glist.imp"
#include "garray.imp"
#include "grarray.imp"
#include "gmatrix.imp"

//template class gPolyList<int>;
//template gOutput &operator<<(gOutput &f, const gPolyList<int> &y);

template class gPolyList<gRational>;
template gOutput &operator<<(gOutput &f, const gPolyList<gRational> &y);

template class gPolyList<double>;
template gOutput &operator<<(gOutput &f, const gPolyList<double> &y);

template class gPolyList<gDouble>;
template gOutput &operator<<(gOutput &f, const gPolyList<gDouble> &y);

template class gList<index_pair>;

/*
#include "objcount.imp"

template class Counted<gPolyList<gDouble> >;
int Counted<gPolyList<gDouble> >::numObjects = 0;

template class Counted<gPolyList<double> >;
int Counted<gPolyList<double> >::numObjects = 0;

template class Counted<gPolyList<gRational> >;
int Counted<gPolyList<gRational> >::numObjects = 0;
*/

template class gRectArray<gPoly<gDouble>*>;
template class gArray<gPoly<gDouble>*>;

template class gRectArray<gPoly<gRational>*>;
//template class gArray<gPoly<gRational>*>;

template class gRectArray<gPoly<double>*>;
//template class gArray<gPoly<double>*>;





