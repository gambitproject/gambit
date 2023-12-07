//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/ipa/ipa.cc
// Compute Nash equilibria via iterated polymatrix approximation
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
#include "gambit.h"
#include "solvers/ipa/ipa.h"
#include "solvers/gtracer/gtracer.h"

using namespace Gambit::gametracer;

namespace Gambit {
namespace Nash {

List<MixedStrategyProfile<double>> IPAStrategySolve(const Game &p_game,
                                                    StrategyCallbackType p_callback)
{
  MixedStrategyProfile<double> pert = p_game->NewMixedStrategyProfile(0.0);
  for (auto strategy : p_game->GetStrategies()) {
    pert[strategy] = 0.0;
  }
  for (auto player : p_game->GetPlayers()) {
    pert[player->GetStrategies().front()] = 1.0;
  }
  return IPAStrategySolve(pert, p_callback);
}

List<MixedStrategyProfile<double>> IPAStrategySolve(const MixedStrategyProfile<double> &p_pert,
                                                    StrategyCallbackType p_callback)
{
  if (!p_pert.GetGame()->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }

  std::shared_ptr<gnmgame> A = BuildGame(p_pert.GetGame(), false);
  cvector g(ToPerturbation(p_pert));
  cvector ans(A->getNumActions());
  cvector zh(A->getNumActions(), 1.0);
  while (true) {
    const double ALPHA = 0.2;
    const double EQERR = 1e-6;
    if (IPA(*A, g, zh, ALPHA, EQERR, ans)) {
      break;
    }
  }

  List<MixedStrategyProfile<double>> solutions;
  solutions.push_back(ToProfile(p_pert.GetGame(), ans));
  p_callback(solutions.back(), "NE");
  return solutions;
}

} // namespace Nash
} // namespace Gambit
