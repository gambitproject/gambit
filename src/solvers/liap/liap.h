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

#include <variant>
#include "games/nash.h"

namespace Gambit::Nash {

template <class Profile> struct LiapStartEvent {
  const Profile &profile;
};

template <class Profile> struct LiapEndEvent {
  const Profile &profile;
  double regret;
};

template <class Profile>
using LiapEvent = std::variant<LiapStartEvent<Profile>, LiapEndEvent<Profile>>;

template <class Profile>
using LiapEventCallbackType = std::function<void(const LiapEvent<Profile> &)>;

template <class Profile> void NullLiapEventCallback(const LiapEvent<Profile> &) {}

std::list<MixedBehaviorProfile<double>>
LiapAgentSolve(const MixedBehaviorProfile<double> &p_start, double p_maxregret, int p_maxitsN,
               BehaviorCallbackType<double> p_onEquilibrium = NullBehaviorCallback<double>,
               LiapEventCallbackType<MixedBehaviorProfile<double>> p_onEvent =
                   NullLiapEventCallback<MixedBehaviorProfile<double>>);

std::list<MixedStrategyProfile<double>>
LiapStrategySolve(const MixedStrategyProfile<double> &p_start, double p_maxregret, int p_maxitsN,
                  StrategyCallbackType<double> p_onEquilibrium = NullStrategyCallback<double>,
                  LiapEventCallbackType<MixedStrategyProfile<double>> p_onEvent =
                      NullLiapEventCallback<MixedStrategyProfile<double>>);

} // namespace Gambit::Nash

#endif // GAMBIT_NASH_LIAP_H
