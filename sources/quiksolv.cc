//
// File: quiksolv.cc  
//        -- Instantiations of classes 
// 
// $Id$
//

#include "quiksolv.imp"
#include "double.h"
#include "grarray.imp"

template class gRectArray<bool>;

template class QuikSolv<gDouble>;
template gOutput &operator<<(gOutput &f, const QuikSolv<gDouble> &y);

