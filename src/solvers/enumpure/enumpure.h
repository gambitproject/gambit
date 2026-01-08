//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: library/include/gambit/nash/enumpure.h
// Enumerate pure-strategy equilibrium profiles of games
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

#ifndef GAMBIT_NASH_ENUMPURE_H
#define GAMBIT_NASH_ENUMPURE_H

#include "games/nash.h"

namespace Gambit::Nash {

inline bool IsNash(const PureStrategyProfile &p_profile)
{
  for (const auto &player : p_profile->GetGame()->GetPlayers()) {
    const Rational current = p_profile->GetPayoff(player);
    for (auto strategy : player->GetStrategies()) {
      if (p_profile->GetStrategyValue(strategy) > current) {
        return false;
      }
    }
  }
  return true;
}

///
/// Enumerate pure-strategy Nash equilibria of a game.  By definition,
/// pure-strategy equilibrium uses the strategic representation of a game.
///
inline std::list<MixedStrategyProfile<Rational>> EnumPureStrategySolve(
    const Game &p_game,
    StrategyCallbackType<Rational> p_onEquilibrium = NullStrategyCallback<Rational>)
{
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }
  std::list<MixedStrategyProfile<Rational>> solutions;
  for (auto citer : StrategyContingencies(p_game)) {
    if (IsNash(citer)) {
      solutions.push_back(citer->ToMixedStrategyProfile());
      p_onEquilibrium(solutions.back(), "NE");
    }
  }
  return solutions;
}

inline bool IsAgentNash(const PureBehaviorProfile &p_profile)
{
  for (const auto &player : p_profile.GetGame()->GetPlayers()) {
    auto current = p_profile.GetPayoff<Rational>(player);
    for (const auto &infoset : player->GetInfosets()) {
      for (const auto &action : infoset->GetActions()) {
        if (p_profile.GetPayoff<Rational>(action) > current) {
          return false;
        }
      }
    }
  }
  return true;
}

///
/// Enumerate pure-strategy agent Nash equilibria of a game.  This uses
/// the extensive representation.  Agent Nash equilibria are not necessarily
/// Nash equilibria, as these consider deviations only at one information
/// set (rather than possible deviations by the same player at multiple
/// information sets.
///
inline std::list<MixedBehaviorProfile<Rational>>
EnumPureAgentSolve(const Game &p_game,
                   BehaviorCallbackType<Rational> p_onEquilibrium = NullBehaviorCallback<Rational>)
{
  std::list<MixedBehaviorProfile<Rational>> solutions;
  const BehaviorSupportProfile support(p_game);
  for (auto citer : BehaviorContingencies(BehaviorSupportProfile(p_game))) {
    if (IsAgentNash(citer)) {
      solutions.push_back(citer.ToMixedBehaviorProfile());
      p_onEquilibrium(solutions.back(), "NE");
    }
  }
  return solutions;
}

} // end namespace Gambit::Nash

#endif // GAMBIT_NASH_ENUMPURE_H
