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

template class gbtArray<gbt_efg_player_rep *>;
template class gbtBlock<gbt_efg_player_rep *>;

template class gbtArray<gbt_efg_infoset_rep *>;
template class gbtBlock<gbt_efg_infoset_rep *>;

template class gbtArray<gbtEfgInfoset>;
template class gbtBlock<gbtEfgInfoset>;

template class gbtArray<gbt_efg_node_rep *>;
template class gbtBlock<gbt_efg_node_rep *>;

template class gbtArray<gbtEfgNode>;
template class gbtBlock<gbtEfgNode>;

template class gbtArray<gbt_efg_action_rep *>;
template class gbtBlock<gbt_efg_action_rep *>;

template class gbtArray<gbtEfgAction>;
template class gbtBlock<gbtEfgAction>;

template class gbtArray<gbtArray<gbtEfgAction> *>;

//template class gbtArray<gbtArray<Action *> *>;

template class gbtArray<gbt_efg_outcome_rep *>;
template class gbtBlock<gbt_efg_outcome_rep *>;

template class gbtArray<gbtBlock<bool> >;
template class gbtBlock<gbtBlock<bool> >;

class gbtEfgActionSet;
class EFActionArray;

//template bool operator==(const gbtArray<Action *> &, const gbtArray<Action *> &);
template class gbtArray<gbtEfgActionSet *>;
template class gbtArray<EFActionArray *>;

template class gbtList<gbtEfgNode>;
template class gbtList<gbtEfgAction>;
template class gbtList<gbtEfgInfoset>;

template class gbtList<gbt_efg_outcome_rep *>;
