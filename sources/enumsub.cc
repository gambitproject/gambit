//
// FILE: enumsub.cc -- Instantiation of EnumBySubgame
//
// $Id$
//

#include "rational.h"
#include "enumsub.imp"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgx
#define TEMPLATE
#endif 

TEMPLATE class EnumBySubgame<double>;
TEMPLATE class EnumBySubgame<gRational>;
