//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria via Mangasarian's algorithm
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

#ifndef NFGMIXED_H
#define NFGMIXED_H

#include "game/game.h"

//!
//! This implements the procedure described in 
//! Mangasarian, O. L., "Equilibrium points of bimatrix games", 
//! SIAM 12 (1964): 778-780 for finding all extreme points of 
//! the Nash equilibrium components of a two person game. 
//! 
template <class T> gbtList<gbtMixedProfile<T> > 
gbtNashEnumMixedNfg(const gbtGame &p_game, const T &p_precision,
		    int p_stopAfter = 0);

#endif  // NFGMIXED_H
