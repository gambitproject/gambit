//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
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

#include "libgambit/libgambit.h"
#include "libgambit/nash.h"

using namespace Gambit;

class NashEnumPureStrategySolver : public NashStrategySolver<Rational> {
public:
   NashEnumPureStrategySolver(Gambit::shared_ptr<StrategyProfileRenderer<Rational> > p_onEquilibrium = 0) 
    : NashStrategySolver<Rational>(p_onEquilibrium) { }
  virtual ~NashEnumPureStrategySolver()  { }

  List<MixedStrategyProfile<Rational> > Solve(const Game &p_game) const;
};

inline List<MixedStrategyProfile<Rational> >
NashEnumPureStrategySolver::Solve(const Game &p_game) const
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


class NashEnumPureAgentSolver : public NashBehavSolver<Rational> {
public:
  NashEnumPureAgentSolver(Gambit::shared_ptr<StrategyProfileRenderer<Rational> > p_onEquilibrium = 0)
    : NashBehavSolver<Rational>(p_onEquilibrium) { }
  virtual ~NashEnumPureAgentSolver()  { }

  List<MixedBehaviorProfile<Rational> > Solve(const BehaviorSupportProfile &) const;
};

inline List<MixedBehaviorProfile<Rational> > 
NashEnumPureAgentSolver::Solve(const BehaviorSupportProfile &p_support) const
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

#endif  // ENUMPURE_H
