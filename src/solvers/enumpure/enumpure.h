//
// This file is part of Gambit
// Copyright (c) 1994-2022, The Gambit Project (http://www.gambit-project.org)
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
   EnumPureStrategySolver(shared_ptr<StrategyProfileRenderer<Rational> > p_onEquilibrium = 0) 
    : StrategySolver<Rational>(p_onEquilibrium) { }
  virtual ~EnumPureStrategySolver()  { }

  List<MixedStrategyProfile<Rational> > Solve(const Game &p_game) const;
};

inline List<MixedStrategyProfile<Rational> >
EnumPureStrategySolver::Solve(const Game &p_game) const
{
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException("Computing equilibria of games with imperfect recall is not supported.");
  }
  List<MixedStrategyProfile<Rational> > solutions;
  for (StrategyProfileIterator citer(p_game); !citer.AtEnd(); citer++) {
    if ((*citer)->IsNash()) {
      MixedStrategyProfile<Rational> profile = (*citer)->ToMixedStrategyProfile();
      m_onEquilibrium->Render(profile);
      solutions.Append(profile);
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
class EnumPureAgentSolver : public BehavSolver<Rational> {
public:
  EnumPureAgentSolver(shared_ptr<StrategyProfileRenderer<Rational> > p_onEquilibrium = 0)
    : BehavSolver<Rational>(p_onEquilibrium) { }
  virtual ~EnumPureAgentSolver()  { }

  List<MixedBehaviorProfile<Rational> > Solve(const BehaviorSupportProfile &) const;
};

inline List<MixedBehaviorProfile<Rational> > 
EnumPureAgentSolver::Solve(const BehaviorSupportProfile &p_support) const
{
  List<MixedBehaviorProfile<Rational> > solutions;
  for (BehaviorProfileIterator citer(p_support); !citer.AtEnd(); citer++) {
    if (citer->IsAgentNash()) {
      MixedBehaviorProfile<Rational> profile = citer->ToMixedBehaviorProfile();
      m_onEquilibrium->Render(profile);
      solutions.Append(profile);
    }
  }
  return solutions;
}

}  // end namespace Nash
}  // end namespace Gambit
 
#endif  // GAMBIT_NASH_ENUMPURE_H
