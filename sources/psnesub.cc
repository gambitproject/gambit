//
// FILE: psnesub.cc -- Instantiation of PureNashBySubgame 
//
// $Id$
//

#include "rational.h"
#include "psnesub.imp"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif 


TEMPLATE class PureNashBySubgame<double>;
TEMPLATE class PureNashBySubgame<gRational>;
