//
// File: quiksolv.cc  
//        -- Instantiations of classes 
// 
// @(#)quiksolv.cc	1.1 1/7/98
//

#include "quiksolv.imp"
#include "double.h"
#include "grarray.imp"

template class gRectArray<bool>;

template class QuikSolv<gRational>;
template gOutput &operator<<(gOutput &f, const QuikSolv<gRational> &y);

template class QuikSolv<double>;
template gOutput &operator<<(gOutput &f, const QuikSolv<double> &y);

template class QuikSolv<gDouble>;
template gOutput &operator<<(gOutput &f, const QuikSolv<gDouble> &y);

