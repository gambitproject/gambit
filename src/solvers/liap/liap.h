//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/liap/efgliap.h
// Compute Nash equilibria by minimizing Liapunov function on extensive game
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

#ifndef GAMBIT_NASH_LIAP_H
#define GAMBIT_NASH_LIAP_H

#include "games/nash.h"

namespace Gambit::Nash {

std::list<MixedBehaviorProfile<double>>
LiapAgentSolve(const MixedBehaviorProfile<double> &p_start, double p_maxregret, int p_maxitsN,
               BehaviorCallbackType<double> p_callback = NullBehaviorCallback<double>);

std::list<MixedStrategyProfile<double>>
LiapStrategySolve(const MixedStrategyProfile<double> &p_start, double p_maxregret, int p_maxitsN,
                  StrategyCallbackType<double> p_callback = NullStrategyCallback<double>);

} // namespace Gambit::Nash

#endif // GAMBIT_NASH_LIAP_H
