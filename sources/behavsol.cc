

#include "behavsol.imp"


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "rational.h"
TEMPLATE class BehavSolution<double>;
TEMPLATE class BehavSolution<gRational>;

TEMPLATE gOutput &operator<<(gOutput &, const BehavSolution<double> &);
TEMPLATE gOutput &operator<<(gOutput &, const BehavSolution<gRational> &);
