//
// FILE: gfunc.cc -- Instantiation of base function types
//
// $Id$
//

#include "gfunc.h"


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__


TEMPLATE class gFunction<double>;
TEMPLATE class gC2Function<double>;
