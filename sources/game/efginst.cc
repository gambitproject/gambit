//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of templates used in extensive form games
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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

template class gArray<gbt_efg_outcome_rep *>;
template class gBlock<gbt_efg_outcome_rep *>;

template class gArray<gBlock<bool> >;
template class gBlock<gBlock<bool> >;

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

template class gList<gbt_efg_outcome_rep *>;


