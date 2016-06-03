//
// This file is part of Gambit
// Copyright (c) 1994-2016, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpure/enumpure.cc
// Compute pure-strategy equilibria in extensive form games
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

#ifndef ENUMPURE_H
#define ENUMPURE_H

#include "gambit/gambit.h"
#include "gambit/nash.h"

namespace Gambit {
namespace Nash {

class EnumPureStrategySolver : public StrategySolver<Rational> {
public:
   EnumPureStrategySolver(Gambit::shared_ptr<StrategyProfileRenderer<Rational> > p_onEquilibrium = 0) 
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


class EnumPureAgentSolver : public BehavSolver<Rational> {
public:
  EnumPureAgentSolver(Gambit::shared_ptr<StrategyProfileRenderer<Rational> > p_onEquilibrium = 0)
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

}  // end namespace Gambit::Nash
}  // end namespace Gambit
 
#endif  // ENUMPURE_H
