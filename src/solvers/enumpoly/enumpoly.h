//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/enumpoly.h
// Finds Nash equilibria of a game by solving systems of polynomial equations
// by enumerating supports.
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

#ifndef GAMBIT_SOLVERS_ENUMPOLY_ENUMPOLY_H
#define GAMBIT_SOLVERS_ENUMPOLY_ENUMPOLY_H

#include "games/nash.h"
#include "solvers/nashsupport/nashsupport.h"

namespace Gambit {
namespace Nash {

using EnumPolyMixedStrategyObserverFunctionType =
    std::function<void(const MixedStrategyProfile<double> &)>;

inline void EnumPolyNullMixedStrategyObserver(const MixedStrategyProfile<double> &) {}

using EnumPolyStrategySupportObserverFunctionType =
    std::function<void(const std::string &, const StrategySupportProfile &)>;

inline void EnumPolyNullStrategySupportObserver(const std::string &,
                                                const StrategySupportProfile &)
{
}

List<MixedStrategyProfile<double>> EnumPolyStrategySolve(
    const Game &p_game, int p_stopAfter, double p_maxregret,
    EnumPolyMixedStrategyObserverFunctionType p_onEquilibrium = EnumPolyNullMixedStrategyObserver,
    EnumPolyStrategySupportObserverFunctionType p_onSupport = EnumPolyNullStrategySupportObserver);

using EnumPolyMixedBehaviorObserverFunctionType =
    std::function<void(const MixedBehaviorProfile<double> &)>;

inline void EnumPolyNullMixedBehaviorObserver(const MixedBehaviorProfile<double> &) {}

using EnumPolyBehaviorSupportObserverFunctionType =
    std::function<void(const std::string &, const BehaviorSupportProfile &)>;

inline void EnumPolyNullBehaviorSupportObserver(const std::string &,
                                                const BehaviorSupportProfile &)
{
}

List<MixedBehaviorProfile<double>> EnumPolyBehaviorSolve(
    const Game &, int p_stopAfter, double p_maxregret,
    EnumPolyMixedBehaviorObserverFunctionType p_onEquilibrium = EnumPolyNullMixedBehaviorObserver,
    EnumPolyBehaviorSupportObserverFunctionType p_onSupport = EnumPolyNullBehaviorSupportObserver);

} // namespace Nash
} // namespace Gambit

#endif // GAMBIT_SOLVERS_ENUMPOLY_ENUMPOLY_H
