//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: library/include/gambit/nash/gnm.h
// Compute Nash equilibria via the global Newton method
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

#ifndef GAMBIT_NASH_GNM_H
#define GAMBIT_NASH_GNM_H

#include "games/nash.h"
#include "solvers/gtracer/gtracer.h"

namespace Gambit::Nash {

const double GNM_LAMBDA_END_DEFAULT = -10.0;
const int GNM_LOCAL_NEWTON_INTERVAL_DEFAULT = 3;
const int GNM_LOCAL_NEWTON_MAXITS_DEFAULT = 10;
const int GNM_STEPS_DEFAULT = 100;

std::list<MixedStrategyProfile<double>>
GNMStrategySolve(const Game &p_game, double p_lambdaEnd, int p_steps, int p_localNewtonInterval,
                 int p_localNewtonMaxits,
                 StrategyCallbackType<double> p_callback = NullStrategyCallback<double>);

/// @brief Compute the mixed strategy equilibria accessible via the initial ray determined
///        by \p p_profile using the Global Newton method
std::list<MixedStrategyProfile<double>>
GNMStrategySolve(const MixedStrategyProfile<double> &p_profile, double p_lambdaEnd, int p_steps,
                 int p_localNewtonInterval, int p_localNewtonMaxits,
                 StrategyCallbackType<double> p_callback = NullStrategyCallback<double>);

} // end namespace Gambit::Nash

#endif // GAMBIT_NASH_GNM_H
