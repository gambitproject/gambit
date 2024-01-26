//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: library/include/gtracer/gtracer.cc
// Top-level include file for Gametracer embedding in Gambit
// This file is based on GameTracer v0.2, which is
// Copyright (c) 2002, Ben Blum and Christian Shelton
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

#include <algorithm>
#include "gtracer.h"
#include "gambit.h"

namespace Gambit {
namespace gametracer {

std::shared_ptr<gnmgame> BuildGame(const Game &p_game, bool p_scaled)
{
  if (p_game->IsAgg()) {
    return std::shared_ptr<gnmgame>(new aggame(dynamic_cast<GameAGGRep &>(*p_game)));
  }
  Rational maxPay = p_game->GetMaxPayoff();
  Rational minPay = p_game->GetMinPayoff();
  double scale = (p_scaled && maxPay > minPay) ? 1.0 / (maxPay - minPay) : 1.0;

  auto players = p_game->GetPlayers();
  std::vector<int> actions(players.size());
  std::transform(players.cbegin(), players.cend(), actions.begin(),
                 [](const GamePlayer &p) { return p->NumStrategies(); });
  std::shared_ptr<gnmgame> A(new nfgame(actions));

  std::vector<int> profile(players.size());
  for (StrategyProfileIterator iter(p_game); !iter.AtEnd(); iter++) {
    std::transform(players.cbegin(), players.cend(), profile.begin(), [iter](const GamePlayer &p) {
      return (*iter)->GetStrategy(p)->GetNumber() - 1;
    });
    for (auto player : players) {
      A->setPurePayoff(player->GetNumber() - 1, profile,
                       scale * ((*iter)->GetPayoff(player) - minPay));
    }
  }
  return A;
}

cvector ToPerturbation(const MixedStrategyProfile<double> &p_pert)
{
  auto strategies = p_pert.GetGame()->GetStrategies();
  cvector g(strategies.size());
  std::transform(strategies.cbegin(), strategies.cend(), g.begin(),
                 [p_pert](const GameStrategy &s) { return p_pert[s]; });
  for (auto player : p_pert.GetGame()->GetPlayers()) {
    bool is_tie = false;
    auto strategies = player->GetStrategies();
    auto strategy = strategies.cbegin();
    double maxval = p_pert[*strategy];
    for (++strategy; strategy != strategies.cend(); ++strategy) {
      if (p_pert[*strategy] > maxval) {
        maxval = p_pert[*strategy];
        is_tie = false;
      }
      else if (p_pert[*strategy] == maxval) {
        is_tie = true;
      }
      if (is_tie) {
        throw std::domain_error("Perturbation vector does not have unique maximizer for player " +
                                std::to_string(player->GetNumber()));
      }
    }
  }
  g /= g.norm(); // normalized
  return g;
}

MixedStrategyProfile<double> ToProfile(const Game &p_game, const cvector &p_profile)
{
  MixedStrategyProfile<double> msp = p_game->NewMixedStrategyProfile(0.0);
  auto value = p_profile.cbegin();
  for (auto strategy : p_game->GetStrategies()) {
    msp[strategy] = *value;
    ++value;
  }
  return msp;
}

} // namespace gametracer
} // namespace Gambit
