//
// FILE: csumsub.cc -- Instantiation of ZSumBySubgame
//
// $Id$
//

#include "rational.h"
#include "csumsub.imp"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif 

TEMPLATE class ZSumBySubgame<double>;
TEMPLATE class ZSumBySubgame<gRational>;

