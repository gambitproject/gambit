//
// FILE: subsolve.cc -- Implementation of solve-by-subgame algorithms
//
// $Id$
//

// This file is a mess as far as Borland instantiations are concerned. The
// positions of each #pragma option and instantiations are critical.

#include "efg.h"
#include "efgutils.h"
#include "nfg.h"
#include "nfstrat.h"
#include "gwatch.h"
#include "rational.h"

#ifdef __BORLANDC__
#include "behavsol.h"
class gNode<BehavProfile<double> >;
class gNode<BehavProfile<gRational> >;
class gList<BehavProfile<double> >;
class gList<BehavProfile<gRational> >;
class gNode<BehavSolution<double> >;
class gNode<BehavSolution<gRational> >;
class gList<BehavSolution<double> >;
class gList<BehavSolution<gRational> >;
#include "mixedsol.h"
class gNode<MixedProfile<double> >;
class gNode<MixedProfile<gRational> >;
class gList<MixedProfile<double> >;
class gList<MixedProfile<gRational> >;
class gNode<MixedSolution<double> >;
class gNode<MixedSolution<gRational> >;
class gList<MixedSolution<double> >;
class gList<MixedSolution<gRational> >;

gOutput &operator<<(gOutput &, const gArray<int> &);
gOutput &operator<<(gOutput &, const gArray<double> &);
gOutput &operator<<(gOutput &, const gArray<gRational> &);
#endif

#pragma option -Jgd		// must come before subsolve.h since SubgameSolver<double> is defined in it.

#include "subsolve.imp"


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgx
class gArray<unsigned char>;

class gNode<gArray<int> >;
class gList<gArray<int> >;


bool operator==(const gArray<EFOutcome *> &a, const gArray<EFOutcome *> &b)
{
	if (a.mindex != b.mindex || a.maxdex != b.maxdex)   return false;
	for (int i = a.mindex; i <= a.maxdex; i++)
		if (a[i] != b[i])   return false;
	return true;
}

bool operator!=(const gArray<EFOutcome *> &a, const gArray<EFOutcome *> &b)
{
	return !(a == b);
}

bool operator==(const gArray<int> &a, const gArray<int> &b)
{
	if (a.mindex != b.mindex || a.maxdex != b.maxdex)   return false;
	for (int i = a.mindex; i <= a.maxdex; i++)
		if (a[i] != b[i])   return false;
	return true;
}

bool operator!=(const gArray<int> &a, const gArray<int> &b)
{return !(a == b);}

#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__


TEMPLATE class SubgameSolver<double>;
TEMPLATE class SubgameSolver<gRational>;


#include "garray.imp"

TEMPLATE class gArray<gArray<Infoset *> *>;

TEMPLATE bool operator==(const gArray<EFOutcome *> &, const gArray<EFOutcome *> &);
TEMPLATE bool operator!=(const gArray<EFOutcome *> &, const gArray<EFOutcome *> &);

TEMPLATE gOutput &operator<<(gOutput &, const gArray<EFOutcome *> &);


#include "glist.imp"

TEMPLATE class gList<gVector<double> >;
TEMPLATE class gNode<gVector<double> >;
TEMPLATE class gList<gVector<gRational> >;
TEMPLATE class gNode<gVector<gRational> >;

TEMPLATE class gList<EFOutcome *>;
TEMPLATE class gNode<EFOutcome *>;

TEMPLATE class gList<gArray<EFOutcome *> >;
TEMPLATE class gNode<gArray<EFOutcome *> >;

