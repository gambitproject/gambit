//
// FILE: simpsub.cc.cc -- Instantiation of SimpdivBySubgame
//
// $Id$
//

#include "rational.h"
#include "simpsub.imp"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif 

TEMPLATE class SimpdivBySubgame<double>;
TEMPLATE class SimpdivBySubgame<gRational>;
