//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of templates used in game trees
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

#include "base/garray.imp"
#include "base/gblock.imp"
#include "base/glist.imp"
#include "game.h"
#include "tree-game.h"

template class gbtArray<gbtTreePlayerRep *>;
template class gbtBlock<gbtTreePlayerRep *>;

template class gbtArray<gbtTreeInfosetRep *>;
template class gbtBlock<gbtTreeInfosetRep *>;

template class gbtArray<gbtTreeNodeRep *>;
template class gbtBlock<gbtTreeNodeRep *>;

template class gbtArray<gbtTreeActionRep *>;
template class gbtBlock<gbtTreeActionRep *>;

template class gbtArray<gbtTreeStrategyRep *>;
template class gbtBlock<gbtTreeStrategyRep *>;

template class gbtArray<gbtGameAction>;
template class gbtBlock<gbtGameAction>;

// Currently used in pure behavior profile.  Probably superfluous?
template class gbtArray<gbtArray<gbtGameAction> *>;

template class gbtArray<gbtTreeOutcomeRep *>;
template class gbtBlock<gbtTreeOutcomeRep *>;

template class gbtList<gbtGameInfoset>;

