//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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

#ifndef GAMBIT_SOLVERS_NASHSUPPORT_NASHSUPPORT_H
#define GAMBIT_SOLVERS_NASHSUPPORT_NASHSUPPORT_H

#include "gambit.h"

using namespace Gambit;

class PossibleNashStrategySupportsResult {
public:
  std::list<StrategySupportProfile> m_supports;
};
// Compute the set of strategy support profiles which can be the support of
// a totally-mixed Nash equilibrium, using the heuristic search method of
// Porter, Nudelman & Shoham (2004).
std::shared_ptr<PossibleNashStrategySupportsResult> PossibleNashStrategySupports(const Game &);

class PossibleNashBehaviorSupportsResult {
public:
  std::list<BehaviorSupportProfile> m_supports;
};

// Compute the set of behavior support profiles which can be the support of
// a totally-mixed Nash equilibrium.
std::shared_ptr<PossibleNashBehaviorSupportsResult> PossibleNashBehaviorSupports(const Game &);

#endif // GAMBIT_SOLVERS_NASHSUPPORT_NASHSUPPORT_H
