//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of templates used in games
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

#include "game.h"
#include "nfgsupport.h"
#include "gamebase.h"

#include "base/garray.imp"
#include "base/gblock.imp"
#include "base/glist.imp"

template class gbtArray<gbtGamePlayerBase *>;
template class gbtBlock<gbtGamePlayerBase *>;

template class gbtArray<gbtGameInfosetBase *>;
template class gbtBlock<gbtGameInfosetBase *>;

template class gbtArray<gbtGameInfoset>;
template class gbtBlock<gbtGameInfoset>;

template class gbtArray<gbtGameNodeBase *>;
template class gbtBlock<gbtGameNodeBase *>;

template class gbtArray<gbtGameNode>;
template class gbtBlock<gbtGameNode>;

template class gbtArray<gbtGameActionBase *>;
template class gbtBlock<gbtGameActionBase *>;

template class gbtArray<gbtGameAction>;
template class gbtBlock<gbtGameAction>;

template class gbtArray<gbtArray<gbtGameAction> *>;

template class gbtArray<gbtGameStrategyBase *>;
template class gbtBlock<gbtGameStrategyBase *>;

template class gbtArray<gbtGameStrategy>;
template class gbtBlock<gbtGameStrategy>;

template class gbtArray<gbtGameOutcomeBase *>;
template class gbtBlock<gbtGameOutcomeBase *>;

template class gbtArray<gbtBlock<bool> >;
template class gbtBlock<gbtBlock<bool> >;

class gbtGameActionSet;
class gbtGameActionArray;

template class gbtArray<gbtGameActionSet *>;
template class gbtArray<gbtGameActionArray *>;

template class gbtList<gbtGameNode>;
template class gbtList<gbtGameAction>;
template class gbtList<gbtGameInfoset>;
template class gbtList<gbtGameStrategy>;

template class gbtList<gbtGameOutcomeBase *>;

template class gbtList<gbtNfgSupport>;
