

#include "mixedsol.imp"


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "rational.h"
TEMPLATE class MixedSolution<double>;
TEMPLATE class MixedSolution<gRational>;

TEMPLATE gOutput &operator<<(gOutput &, const MixedSolution<double> &);
TEMPLATE gOutput &operator<<(gOutput &, const MixedSolution<gRational> &);
