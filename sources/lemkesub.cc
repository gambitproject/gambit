//
// FILE: lemkesub.cc -- Instantiation of LemkeBySubgame
//
// $Id$
//

#include "rational.h"
#include "lemkesub.imp"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgx
#define TEMPLATE
#endif 

TEMPLATE class LemkeBySubgame<double>;
TEMPLATE class LemkeBySubgame<gRational>;
