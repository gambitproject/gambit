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

template class gArray<gbt_efg_player_rep *>;
template class gBlock<gbt_efg_player_rep *>;

template class gArray<gbt_efg_infoset_rep *>;
template class gBlock<gbt_efg_infoset_rep *>;

template class gArray<gbtEfgInfoset>;
template class gBlock<gbtEfgInfoset>;

template class gArray<Node *>;
template class gBlock<Node *>;

template class gArray<gbt_efg_action_rep *>;
template class gBlock<gbt_efg_action_rep *>;

template class gArray<gbtEfgAction>;
template class gBlock<gbtEfgAction>;

template class gArray<gArray<gbtEfgAction> *>;

//template class gArray<gArray<Action *> *>;

template class gArray<gbt_efg_outcome_rep *>;
template class gBlock<gbt_efg_outcome_rep *>;

template class gArray<gBlock<bool> >;
template class gBlock<gBlock<bool> >;

class EFActionSet;
class EFActionArray;

//template bool operator==(const gArray<Action *> &, const gArray<Action *> &);
template class gArray<EFActionSet *>;
template class gArray<EFActionArray *>;

template class gList<Node *>;
template class gList<gbtEfgAction>;
template class gList<gbtEfgInfoset>;

template class gList<gbt_efg_outcome_rep *>;


