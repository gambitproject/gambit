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

#include <algorithm>
#include <numeric>
#include "gambit.h"
#include "solvers/gnm/gnm.h"
#include "solvers/gtracer/gtracer.h"

using namespace Gambit;
using namespace Gambit::gametracer;

namespace Gambit {
namespace Nash {

List<MixedStrategyProfile<double>>
NashGNMStrategySolver::Solve(const Game &p_game, const std::shared_ptr<gnmgame> &p_rep,
                             const cvector &p_pert) const
{
  const int STEPS = 100;
  const double FUZZ = 1e-12;
  const int LNMFREQ = 3;
  const int LNMMAX = 10;
  const double LAMBDAMIN = -10.0;
  const bool WOBBLE = false;
  const double THRESHOLD = 1e-2;

  bool nonzero = std::accumulate(p_pert.cbegin(), p_pert.cend(), false,
                                 [](bool accum, double value) { return accum || value != 0.0; });
  if (!nonzero) {
    throw UndefinedException("Perturbation vector must have at least one nonzero component.");
  }
  List<MixedStrategyProfile<double>> eqa;
  if (m_verbose) {
    m_onEquilibrium->Render(ToProfile(p_game, p_pert), "pert");
  }
  cvector norm_pert = p_pert / p_pert.norm();
  std::list<cvector> answers;
  std::string return_message;
  GNM(*p_rep, norm_pert, answers, STEPS, FUZZ, LNMFREQ, LNMMAX, LAMBDAMIN, WOBBLE, THRESHOLD,
      m_verbose, return_message);
  for (auto answer : answers) {
    eqa.push_back(ToProfile(p_game, answer));
    m_onEquilibrium->Render(eqa.back());
  }
  return eqa;
}

List<MixedStrategyProfile<double>> NashGNMStrategySolver::Solve(const Game &p_game) const
{
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }
  std::shared_ptr<gnmgame> A = BuildGame(p_game, true);
  cvector g(A->getNumActions());
  g = 0.0;
  *g.begin() = 1.0;
  return Solve(p_game, A, g);
}

List<MixedStrategyProfile<double>>
NashGNMStrategySolver::Solve(const MixedStrategyProfile<double> &p_pert) const
{
  if (!p_pert.GetGame()->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }
  std::shared_ptr<gnmgame> A = BuildGame(p_pert.GetGame(), true);
  auto strategies = p_pert.GetGame()->GetStrategies();
  cvector g(strategies.size());
  std::transform(strategies.cbegin(), strategies.cend(), g.begin(),
                 [p_pert](const GameStrategy &s) { return p_pert[s]; });
  return Solve(p_pert.GetGame(), A, g);
}

} // namespace Nash
} // end namespace Gambit
