//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/hp/hpsystem.cc
// Computation of a Nash equilibria using a differentiable homotopy
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

#include <iostream>
#include "gambit.h"
#include "solvers/hp/hpsystem.h"

namespace Gambit {
HPEquationSystem::HPEquationSystem(const MixedStrategyProfile<double> &prior)
  : m_game(prior.GetGame()), m_prior(prior)
{
}

Vector<double> HPEquationSystem::ComputeInitialPoint() const
{
  const int n_players = m_game->GetPlayers().size();
  int m_star = 0;
  const double tol = 1e-9; // Tolerance for floating-point comparisons
  for (const auto &player : m_game->GetPlayers()) {
    m_star += player->GetStrategies().size();
  }

  // Dimension: 1 (t) + m_star (total strategies) + n (number of players)
  const int vector_size = 1 + m_star + n_players;
  Vector<double> start_point(vector_size);

  start_point[1] = 0.0; // t = 0

  int alpha_idx = 2;
  int player_idx = 1;

  for (const auto &player : m_game->GetPlayers()) {

    double max_payoff = -std::numeric_limits<double>::infinity();
    std::vector<double> payoffs_against_prior;
    payoffs_against_prior.reserve(player->GetStrategies().size());

    // Finding mu^i (the maximum payoff for player i against the prior)
    for (const auto &strategy : player->GetStrategies()) {
      const double payoff = m_prior.GetPayoff(strategy);
      payoffs_against_prior.push_back(payoff);

      if (payoff > max_payoff) {
        max_payoff = payoff;
      }
    }

    // Store mu^i
    start_point[1 + m_star + player_idx] = max_payoff;

    // Compute alpha^i_s for each strategy s of player i
    int local_s_idx = 0;
    bool found_br = false; // Flag to check if a best response has been found

    for (const auto &strategy : player->GetStrategies()) {
      const double lambda = max_payoff - payoffs_against_prior[local_s_idx++];
      if (std::abs(lambda) < tol && !found_br) {
        start_point[alpha_idx++] = 1.0; // Best response
        found_br = true;
      }
      else if (std::abs(lambda) < tol) {
        throw std::runtime_error("Multiple best responses found for player " +
                                 std::to_string(player_idx) +
                                 ". Only one best response is allowed.");
      }
      else {
        // Avoid sqrt of negative numbers
        start_point[alpha_idx++] = -std::sqrt(std::max(0.0, lambda));
      }
    }
    player_idx++;
  }

  return start_point;
}

MixedStrategyProfile<double>
HPEquationSystem::ExtractEquilibrium(const Vector<double> &final_point) const
{
  MixedStrategyProfile<double> ret = m_game->NewMixedStrategyProfile(0.0);
  int alpha_idx = 2; // First position is reserved to t

  for (const auto &player : m_game->GetPlayers()) {
    for (const auto &strategy : player->GetStrategies()) {
      const double alpha_val = final_point[alpha_idx++];
      const double prob = this->AlphaToSigma(alpha_val);
      ret[strategy] = prob;
    }
  }
  ret = ret.Normalize();

  return ret;
}

} // end namespace Gambit
