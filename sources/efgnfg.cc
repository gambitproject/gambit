//
// FILE: efgnfg.cc -- efg<->nfg conversion routines
//
// $Id$
//

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

template class gList<Correspondence *>;
template class gNode<Correspondence *>;
template gOutput &operator<<(gOutput &, const gList<Correspondence *> &);
template class gArray<gList<Correspondence *> >;






