//
// FILE: efgpure.cc -- Find all pure strategy Nash equilibria
//
// $Id$
//

#include "efgpure.imp"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE int FindPureNash(const Efg<double> &, gList<BehavSolution<double> > &);
TEMPLATE int FindPureNash(const Efg<gRational> &,gList<BehavSolution<gRational> > &);


#ifdef __GNUG__
template class EfgPSNEBySubgame<double>;
template class EfgPSNEBySubgame<gRational>;
#elif defined __BORLANDC__
#pragma option -Jgd
class EfgPSNEBySubgame<double>;
class EfgPSNEBySubgame<gRational>;
#endif   // __GNUG__, __BORLANDC__
