//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/nfgensup.cc
// Enumerate undominated subsupports of a support
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

#include "nashsupport.h"

using namespace Gambit;

namespace {

// Given a candidate support profile p_candidate, check whether this
// can be the support of a totally mixed Nash equilibrium, by
// checking for weak dominations, and dominations by strategies
// that are in the game but outside the support.
bool AcceptCandidate(const StrategySupportProfile &p_support,
                     const StrategySupportProfile &p_candidate)
{
  StrategySupportProfile current(p_candidate);
  StrategySupportProfile::const_iterator cursor(p_support);
  do {
    GameStrategy strat = cursor.GetStrategy();
    if (!current.Contains(strat)) {
      continue;
    }
    for (int j = 1; j <= strat->GetPlayer()->NumStrategies(); j++) {
      GameStrategy other_strat = strat->GetPlayer()->GetStrategy(j);
      if (other_strat != strat) {
        if (current.Contains(other_strat) &&
            current.Dominates(other_strat, strat, false)) {
          return false;
        }
        current.AddStrategy(other_strat);
        if (current.Dominates(other_strat, strat, false)) {
          return false;
        }
        current.RemoveStrategy(other_strat);
      }
    }
  } while (cursor.GoToNext());
  return true;
}

void PossibleNashSupports(const StrategySupportProfile &p_support,
                          const StrategySupportProfile &p_current,
                          const StrategySupportProfile::const_iterator &cursor,
                          std::list<StrategySupportProfile> &p_result)
{
  std::list<GameStrategy> deletion_list;
  for (auto strategy : p_current) {
    if (p_current.IsDominated(strategy, true)) {
      if (cursor.IsSubsequentTo(strategy)) {
        return;
      }
      deletion_list.push_back(strategy);
    }
  }

  if (!deletion_list.empty()) {
    // There are strategies strictly dominated within the p_current support profile.
    // Drop them, and try again on the support restricted to undominated strategies.
    StrategySupportProfile restricted(p_current);
    for (auto strategy : deletion_list) {
      restricted.RemoveStrategy(strategy);
    }
    PossibleNashSupports(p_support, restricted, cursor, p_result);
  }
  else {
    // There are no strictly dominated strategies among the p_current support profile.
    // This therefore could be a potential support profile.
    if (AcceptCandidate(p_support, p_current)) {
      p_result.push_back(p_current);
    }
    StrategySupportProfile::const_iterator c_copy(cursor);
    do {
      GameStrategy strategy = c_copy.GetStrategy();
      if (p_current.Contains(strategy) &&
          p_current.NumStrategies(strategy->GetPlayer()->GetNumber()) > 1) {
        StrategySupportProfile restricted(p_current);
        restricted.RemoveStrategy(strategy);
        PossibleNashSupports(p_support, restricted, c_copy, p_result);
      }
    } while (c_copy.GoToNext());
  }
}

}  // end anonymous namespace


std::shared_ptr<PossibleNashStrategySupportsResult> PossibleNashStrategySupports(const Game &p_game)
{
  auto result = std::make_shared<PossibleNashStrategySupportsResult>();
  StrategySupportProfile support(p_game);
  StrategySupportProfile sact(p_game);
  StrategySupportProfile::const_iterator cursor(support);
  PossibleNashSupports(support, sact, cursor, result->m_supports);
  return result;
}


