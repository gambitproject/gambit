//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/nfgensup.h
// Enumerate undominated subsupports of a support
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

#include "gambit.h"

using namespace Gambit;


// Produce all subsupports that could
// host the path of a behavioral Nash equilibrium.  These are subsupports
// that have no strategy, at an active infoset, that is weakly dominated by
// another active strategy, either in the conditional sense (for any active
// node in the infoset) or the unconditional sense.  In addition we 
// check for domination by strategys that are inactive, but whose activation
// would not activate any currently inactive infosets, so that the
// subsupport resulting from activation is consistent, in the sense
// of having active strategys at all active infosets, and not at other
// infosets.
List<StrategySupportProfile> PossibleNashSubsupports(const StrategySupportProfile &S);

