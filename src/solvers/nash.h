//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/nash.h
// Framework for computing (sub)sets of Nash equilibria.
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

#ifndef GAMBIT_SOLVERS_NASH_H
#define GAMBIT_SOLVERS_NASH_H

#include <functional>
#include <list>

#include "core/cancel.h"
#include "games/stratmixed.h"
#include "games/behavmixed.h"

namespace Gambit::Nash {

template <class T>
using StrategyCallbackType = std::function<void(const MixedStrategyProfile<T> &)>;

/// @brief A fallback callback function for mixed strategy profiles that does nothing
template <class T> void NullStrategyCallback(const MixedStrategyProfile<T> &) {}

template <class T>
using BehaviorCallbackType = std::function<void(const MixedBehaviorProfile<T> &)>;

/// @brief A fallback callback function for mixed behavior profiles that does nothing
template <class T> void NullBehaviorCallback(const MixedBehaviorProfile<T> &) {}

/// @brief Throws if the game does not have perfect recall
inline void RequirePerfectRecall(const Game &p_game)
{
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }
}

/// @brief The width of the range of expected payoffs, used as the unit for regret criteria
inline Rational ExpectedPayoffScale(const Game &p_game)
{
  const auto [minPay, maxPay] = p_game->GetExpectedPayoffRange();
  return maxPay - minPay;
}

/// @brief The width of the range of expected payoffs of the agent form of the game
inline Rational AgentExpectedPayoffScale(const Game &p_game)
{
  const auto [minPay, maxPay] = p_game->GetAgentExpectedPayoffRange();
  return maxPay - minPay;
}

/// @brief Converts a maximum regret, expressed as a fraction of the range of expected payoffs,
///        into the game's own payoff units
/// @details If all expected payoffs are equal the regret is returned unchanged.
template <class T> T ScaleRegret(const Game &p_game, const T &p_maxRegret)
{
  const Rational scale = ExpectedPayoffScale(p_game);
  return (scale == Rational(0)) ? p_maxRegret : p_maxRegret * static_cast<T>(scale);
}

template <class T>
std::list<MixedBehaviorProfile<T>>
ToMixedBehaviorProfile(const std::list<MixedStrategyProfile<T>> &p_list)
{
  std::list<MixedBehaviorProfile<T>> ret;
  for (const auto &profile : p_list) {
    ret.push_back(MixedBehaviorProfile<T>(profile));
  }
  return ret;
}

} // namespace Gambit::Nash

#endif // GAMBIT_SOLVERS_NASH_H
