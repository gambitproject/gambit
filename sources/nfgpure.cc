//
// FILE: nfgpure.cc -- Find all pure strategy Nash equilibria
//
// $Id$
//

#include "nfgpure.imp"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE int FindPureNash(const Nfg<double> &, const NFSupport &,
			  gList<MixedSolution<double> > &);
TEMPLATE int FindPureNash(const Nfg<gRational> &, const NFSupport &,
			  gList<MixedSolution<gRational> > &);

