//
// File: equisolv.cc  
//        -- Instantiations of classes 
// 
// @(#)equisolv.cc	1.1 1/7/98
//

#include "equisolv.imp"
#include "double.h"
#include "glist.imp"
#include "gpolylst.imp"
#include "quiksolv.imp"

template gList<gPolyList<double> >;
template gList<gPolyList<gRational> >;
template gList<gPolyList<gDouble> >;

template gList<ListOfPartialTrees<double> >;
template gList<ListOfPartialTrees<gRational> >;
template gList<ListOfPartialTrees<gDouble> >;

template class EquiSolv<gRational>;
template gOutput &operator<<(gOutput &f, const EquiSolv<gRational> &y);

template class EquiSolv<double>;
template gOutput &operator<<(gOutput &f, const EquiSolv<double> &y);

template class EquiSolv<gDouble>;
template gOutput &operator<<(gOutput &f, const EquiSolv<gDouble> &y);



