//
// File: quiksolv.cc -- Instantiations of quick-solver classes
// 
// $Id$
//

#include "quiksolv.imp"
#include "math/double.h"
#include "base/grarray.imp"

template class gRectArray<bool>;

template class QuikSolv<gRational>;
template gOutput &operator<<(gOutput &f, const QuikSolv<gRational> &y);

//template class QuikSolv<double>;
//template gOutput &operator<<(gOutput &f, const QuikSolv<double> &y);

template class QuikSolv<gDouble>;
template gOutput &operator<<(gOutput &f, const QuikSolv<gDouble> &y);




