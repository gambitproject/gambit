//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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

namespace Gambit {
namespace Nash {

///
/// Enumerate pure-strategy Nash equilibria of a game.  By definition,
/// pure-strategy equilibrium uses the strategic representation of a game.
///
inline List<MixedStrategyProfile<Rational>> EnumPureStrategySolve(
    const Game &p_game,
    StrategyCallbackType<Rational> p_onEquilibrium = NullStrategyCallback<Rational>)
{
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }
  List<MixedStrategyProfile<Rational>> solutions;
  for (auto citer : StrategyContingencies(p_game)) {
    if (citer->IsNash()) {
      solutions.push_back(citer->ToMixedStrategyProfile());
      p_onEquilibrium(solutions.back(), "NE");
    }
  }
  return solutions;
}

///
/// Enumerate pure-strategy agent Nash equilibria of a game.  This uses
/// the extensive representation.  Agent Nash equilibria are not necessarily
/// Nash equilibria, as these consider deviations only at one information
/// set (rather than possible deviations by the same player at multiple
/// information sets.
///
inline List<MixedBehaviorProfile<Rational>>
EnumPureAgentSolve(const Game &p_game,
                   BehaviorCallbackType<Rational> p_onEquilibrium = NullBehaviorCallback<Rational>)
{
  List<MixedBehaviorProfile<Rational>> solutions;
  BehaviorSupportProfile support(p_game);
  for (auto citer : BehaviorContingencies(BehaviorSupportProfile(p_game))) {
    if (citer.IsAgentNash()) {
      solutions.push_back(citer.ToMixedBehaviorProfile());
      p_onEquilibrium(solutions.back(), "NE");
    }
  }
  return solutions;
}

} // end namespace Nash
} // end namespace Gambit

#endif // GAMBIT_NASH_ENUMPURE_H
