//
// File: gsolver.cc  
//        -- Instantiations of class gSolver
// 
// $Id$ 
//

#include "gsolver.imp"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gSolver<gRational>;
TEMPLATE class gSolver<double>;
TEMPLATE class gSolver<gDouble>;

#include "gvector.imp"
#include "glist.imp"
