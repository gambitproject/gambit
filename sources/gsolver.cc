//
// File: gsolver.cc  
//        -- Instantiations of class gSolver
// 
// @(#)gsolver.cc	1.2 01/07/98 
//

#include "gsolver.imp"

template class gSolver<gRational>;
template class gSolver<double>;
template class gSolver<gDouble>;

#include "gvector.imp"
#include "glist.imp"
