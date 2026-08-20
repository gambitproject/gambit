//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/ipa/ipa.cc
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
#include "games.h"
#include "solvers/ipa/ipa.h"
#include "solvers/gtracer/gtracer.h"

using namespace Gambit::gametracer;

namespace {

std::string IPATerminationMessage(const IPAResult &p_result)
{
  switch (p_result.reason) {
  case IPATerminationReason::Converged:
    return "converged after " + std::to_string(p_result.numIterations) + " iterations to error " +
           std::to_string(p_result.finalError);
  case IPATerminationReason::NonfiniteStrategy:
    return "encountered a non-finite strategy profile after " +
           std::to_string(p_result.numIterations) + " iterations";
  case IPATerminationReason::MaxIterationsReached:
  default:
    return "iteration limit reached after " + std::to_string(p_result.numIterations) +
           " iterations; final error is " + std::to_string(p_result.finalError);
  }
}

} // namespace

namespace Gambit::Nash {

std::list<MixedStrategyProfile<double>>
IPAStrategySolve(const Game &p_game, StrategyCallbackType<double> p_onEquilibrium,
                 IPAEventCallbackType p_onEvent, const CancelToken &p_cancel)
{
  MixedStrategyProfile<double> pert = p_game->NewMixedStrategyProfile(0.0);
  for (const auto &player : p_game->GetPlayers()) {
    for (const auto &strategy : player->GetStrategies()) {
      pert[strategy] = 0.0;
    }
  }
  for (auto player : p_game->GetPlayers()) {
    pert[player->GetStrategies().front()] = 1.0;
  }
  return IPAStrategySolve(pert, p_onEquilibrium, p_onEvent, p_cancel);
}

std::list<MixedStrategyProfile<double>>
IPAStrategySolve(const MixedStrategyProfile<double> &p_pert,
                 StrategyCallbackType<double> p_onEquilibrium, IPAEventCallbackType p_onEvent,
                 const CancelToken &p_cancel)
{
  if (!p_pert.GetGame()->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }

  const Game game = p_pert.GetGame();
  const std::shared_ptr<gnmgame> A = BuildGame(game, false);
  const cvector g(ToPerturbation(p_pert));
  cvector zh(A->getNumActions(), 1.0);

  const double ALPHA = 0.2;
  const double EQERR = 1e-6;
  const int MAX_RESTARTS = 100;

  auto onStep = [game, p_onEvent](int p_iteration, const cvector &p_strategy, double p_zDiff,
                                  double p_sDiff) {
    const auto profile = ToProfile(game, p_strategy);
    p_onEvent(IPAStepEvent{
        .profile = profile, .iteration = p_iteration, .zDiff = p_zDiff, .sDiff = p_sDiff});
  };

  IPAResult result{cvector(A->getNumActions()), IPATerminationReason::MaxIterationsReached, 0};
  for (int restart = 0; restart < MAX_RESTARTS; restart++) {
    p_cancel.Check();
    result = IPA(*A, g, zh, ALPHA, EQERR, 100, onStep, p_cancel);
    p_onEvent(
        IPATerminationEvent{.reason = result.reason, .message = IPATerminationMessage(result)});
    if (result.reason == IPATerminationReason::Converged) {
      break;
    }
    if (result.reason == IPATerminationReason::NonfiniteStrategy) {
      throw std::runtime_error(
          "IPA encountered a non-finite strategy profile; the perturbation vector "
          "may not be suitable for this game");
    }
  }
  if (result.reason != IPATerminationReason::Converged) {
    throw std::runtime_error("IPA failed to converge after " + std::to_string(MAX_RESTARTS) +
                             " restarts");
  }

  std::list<MixedStrategyProfile<double>> solutions;
  solutions.push_back(ToProfile(game, result.strategy));
  p_onEquilibrium(solutions.back());
  return solutions;
}

} // namespace Gambit::Nash
