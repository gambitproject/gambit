//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/gnm/gnm.cc
// Compute Nash equilibria via the global Newton method
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

#include <numeric>
#include "gambit.h"
#include "solvers/gnm/gnm.h"
#include "solvers/gtracer/gtracer.h"

using namespace Gambit;
using namespace Gambit::gametracer;

namespace {

List<MixedStrategyProfile<double>>
Solve(const Game &p_game, const std::shared_ptr<gnmgame> &p_rep, const cvector &p_pert,
      double p_lambdaEnd, int p_steps, int p_localNewtonInterval, int p_localNewtonMaxits,
      std::function<void(const MixedStrategyProfile<double> &, const std::string &)> &p_callback)
{
  const double FUZZ = 1e-12;
  const bool WOBBLE = false;
  const double THRESHOLD = 1e-2;

  if (p_lambdaEnd >= 0.0) {
    throw std::out_of_range("Value of lambdaEnd must be negative");
  }
  bool nonzero = std::accumulate(p_pert.cbegin(), p_pert.cend(), false,
                                 [](bool accum, double value) { return accum || value != 0.0; });
  if (!nonzero) {
    throw UndefinedException("Perturbation vector must have at least one nonzero component.");
  }
  List<MixedStrategyProfile<double>> eqa;
  p_callback(ToProfile(p_game, p_pert), "pert");
  cvector norm_pert = p_pert / p_pert.norm();
  std::list<cvector> answers;
  std::string return_message;
  auto callback = [p_game, p_callback](const std::string &label, const cvector &sigma) {
    p_callback(ToProfile(p_game, sigma), label);
  };
  GNM(*p_rep, norm_pert, answers, p_steps, FUZZ, p_localNewtonInterval, p_localNewtonMaxits,
      p_lambdaEnd, WOBBLE, THRESHOLD, callback, return_message);
  for (auto answer : answers) {
    eqa.push_back(ToProfile(p_game, answer));
    p_callback(eqa.back(), "NE");
  }
  return eqa;
}

} // namespace

namespace Gambit {
namespace Nash {

List<MixedStrategyProfile<double>> GNMStrategySolve(const Game &p_game, double p_lambdaEnd,
                                                    int p_steps, int p_localNewtonInterval,
                                                    int p_localNewtonMaxits,
                                                    StrategyCallbackType p_callback)
{
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }
  std::shared_ptr<gnmgame> A = BuildGame(p_game, true);
  MixedStrategyProfile<double> pert = p_game->NewMixedStrategyProfile(0.0);
  for (auto strategy : p_game->GetStrategies()) {
    pert[strategy] = 0.0;
  }
  for (auto player : p_game->GetPlayers()) {
    pert[player->GetStrategies().front()] = 1.0;
  }
  return Solve(p_game, A, ToPerturbation(pert), p_lambdaEnd, p_steps, p_localNewtonInterval,
               p_localNewtonMaxits, p_callback);
}

List<MixedStrategyProfile<double>> GNMStrategySolve(const MixedStrategyProfile<double> &p_pert,
                                                    double p_lambdaEnd, int p_steps,
                                                    int p_localNewtonInterval,
                                                    int p_localNewtonMaxits,
                                                    StrategyCallbackType p_callback)
{
  if (!p_pert.GetGame()->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }
  std::shared_ptr<gnmgame> A = BuildGame(p_pert.GetGame(), true);
  return Solve(p_pert.GetGame(), A, ToPerturbation(p_pert), p_lambdaEnd, p_steps,
               p_localNewtonInterval, p_localNewtonMaxits, p_callback);
}

} // namespace Nash
} // end namespace Gambit
