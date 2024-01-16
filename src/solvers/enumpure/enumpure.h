//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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
class EnumPureStrategySolver : public StrategySolver<Rational> {
public:
  explicit EnumPureStrategySolver(
      std::shared_ptr<StrategyProfileRenderer<Rational>> p_onEquilibrium = nullptr)
    : StrategySolver<Rational>(p_onEquilibrium)
  {
  }
  ~EnumPureStrategySolver() override = default;

  List<MixedStrategyProfile<Rational>> Solve(const Game &p_game) const override;
};

inline List<MixedStrategyProfile<Rational>> EnumPureStrategySolver::Solve(const Game &p_game) const
{
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }
  List<MixedStrategyProfile<Rational>> solutions;
  for (StrategyProfileIterator citer(p_game); !citer.AtEnd(); citer++) {
    if ((*citer)->IsNash()) {
      MixedStrategyProfile<Rational> profile = (*citer)->ToMixedStrategyProfile();
      m_onEquilibrium->Render(profile);
      solutions.push_back(profile);
    }
  }
  return solutions;
}

inline List<MixedStrategyProfile<Rational>> EnumPureStrategySolve(const Game &p_game)
{
  return EnumPureStrategySolver().Solve(p_game);
}

///
/// Enumerate pure-strategy agent Nash equilibria of a game.  This uses
/// the extensive representation.  Agent Nash equilibria are not necessarily
/// Nash equilibria, as these consider deviations only at one information
/// set (rather than possible deviations by the same player at multiple
/// information sets.
///
class EnumPureAgentSolver : public BehavSolver<Rational> {
public:
  explicit EnumPureAgentSolver(
      std::shared_ptr<StrategyProfileRenderer<Rational>> p_onEquilibrium = nullptr)
    : BehavSolver<Rational>(p_onEquilibrium)
  {
  }
  ~EnumPureAgentSolver() override = default;

  List<MixedBehaviorProfile<Rational>> Solve(const Game &) const override;
};

inline List<MixedBehaviorProfile<Rational>> EnumPureAgentSolver::Solve(const Game &p_game) const
{
  List<MixedBehaviorProfile<Rational>> solutions;
  BehaviorSupportProfile support(p_game);
  for (BehaviorProfileIterator citer(p_game); !citer.AtEnd(); citer++) {
    if ((*citer).IsAgentNash()) {
      MixedBehaviorProfile<Rational> profile = (*citer).ToMixedBehaviorProfile();
      m_onEquilibrium->Render(profile);
      solutions.push_back(profile);
    }
  }
  return solutions;
}

inline List<MixedBehaviorProfile<Rational>> EnumPureAgentSolve(const Game &p_game)
{
  return EnumPureAgentSolver().Solve(p_game);
}

} // end namespace Nash
} // end namespace Gambit

#endif // GAMBIT_NASH_ENUMPURE_H
