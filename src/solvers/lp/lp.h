//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/lcp/efglp.h
// Compute Nash equilibria via linear programming
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

#ifndef LP_LP_H
#define LP_LP_H

#include "games/nash.h"

namespace Gambit::Nash {

template <class T>
std::list<MixedStrategyProfile<T>>
LpStrategySolve(const Game &p_game,
                StrategyCallbackType<T> p_onEquilibrium = NullStrategyCallback<T>);

template <class T>
std::list<MixedBehaviorProfile<T>>
LpBehaviorSolve(const Game &p_game,
                BehaviorCallbackType<T> p_onEquilibrium = NullBehaviorCallback<T>);

}; // namespace Gambit::Nash

#endif // LP_LP_H
