//
// FILE: efgnfg.cc -- efg<->nfg conversion routines
//
// @(#)efgnfg.cc	2.2 3/27/97
//

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "efg.h"
#include "nfg.h"
#include "nfplayer.h"
#include "nfstrat.h"
#include "mixed.h"
#include "glist.h"
#include "nfgiter.h"
#include "nfgciter.h"
#include "lexicon.h"


#include "glist.imp"
#include "garray.imp"

TEMPLATE class gList<Correspondence *>;
TEMPLATE class gNode<Correspondence *>;
TEMPLATE gOutput &operator<<(gOutput &, const gList<Correspondence *> &);
TEMPLATE class gArray<gList<Correspondence *> >;






