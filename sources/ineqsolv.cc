//
// File: ineqsolv.cc  
//        -- Instantiations of classes 
// 
// $Id$
//

#include "ineqsolv.imp"
#include "double.h"
#include "grarray.imp"

template class IneqSolv<gRational>;
template gOutput &operator<<(gOutput &f, const IneqSolv<gRational> &y);

//template class IneqSolv<double>;
//template gOutput &operator<<(gOutput &f, const IneqSolv<double> &y);

template class IneqSolv<gDouble>;
template gOutput &operator<<(gOutput &f, const IneqSolv<gDouble> &y);

