//
// File: equisolv.cc  
//        -- Instantiations of classes 
// 
// @(#)equisolv.cc	1.1 1/7/98
//

#include "equisolv.imp"
#include "double.h"

template class EquiSolv<gRational>;
template gOutput &operator<<(gOutput &f, const EquiSolv<gRational> &y);

template class EquiSolv<double>;
template gOutput &operator<<(gOutput &f, const EquiSolv<double> &y);

template class EquiSolv<gDouble>;
template gOutput &operator<<(gOutput &f, const EquiSolv<gDouble> &y);

// #include "glist.imp"

