//
// FILE: efginst.cc -- Instantiation of extensive form template classes
//
// $Id$
//

#include "efg.h"

class ChanceInfoset;

#include "base/garray.imp"
#include "base/gblock.imp"
#include "base/glist.imp"

template class gArray<EFPlayer *>;
template class gBlock<EFPlayer *>;

template class gArray<Infoset *>;
template class gBlock<Infoset *>;

template class gArray<Node *>;
template class gBlock<Node *>;

template class gArray<Action const *>;
template class gBlock<Action const *>;
template class gArray<Action *>;
template class gBlock<Action *>;

template class gArray<gArray<Action const *> *>;

template class gArray<gArray<Action *> *>;

template class gArray<FullEfgNamespace::Outcome *>;
template class gBlock<FullEfgNamespace::Outcome *>;

template class gArray<gBlock<bool> >;
template class gBlock<gBlock<bool> >;

template class gList<Efg::Outcome>;

class EFActionSet;
class EFActionArray;

template bool operator==(const gArray<Action *> &, const gArray<Action *> &);
template class gArray<EFActionSet *>;
template class gArray<EFActionArray *>;

template class gList<Node *>;
template class gList<const Node *>;
template gOutput &operator<<(gOutput &, const gList<const Node *> &);

template class gList<Action *>;
template class gList<const Action *>;

template class gList<Infoset *>;
template class gList<const Infoset *>;

template class gList<EfgClient *>;


