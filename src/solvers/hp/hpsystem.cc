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
  : m_game(prior.GetGame()), m_prior(prior),
    m_current_sigma(prior.GetGame()->NewMixedStrategyProfile(0.0)),
    m_star(prior.MixedProfileLength())
{
  m_payoffs_against_prior.reserve(m_prior.MixedProfileLength());
  for (const auto &player : m_game->GetPlayers()) {
    for (const auto &strategy : player->GetStrategies()) {
      m_payoffs_against_prior.push_back(m_prior.GetPayoff(strategy));
    }
  }
}

void HPEquationSystem::GetValue(const Vector<double> &point, Vector<double> &lhs) const
{
  const double t = point[1];

  int temp_alpha_idx = 2;
  for (const auto &player : m_game->GetPlayers()) {
    for (const auto &strategy : player->GetStrategies()) {
      m_current_sigma[strategy] = AlphaToSigma(point[temp_alpha_idx++]);
    }
  }

  int alpha_idx = 2;
  int eq_idx = 1;
  int player_idx = 1;
  int flat_strategy_idx = 0;

  for (const auto &player : m_game->GetPlayers()) {

    const double mu = point[1 + m_star + player_idx];
    double sum_sigma = 0.0;

    // (a) Best response equations
    for (const auto &strategy : player->GetStrategies()) {
      const double alpha = point[alpha_idx++];
      const double lambda = AlphaToLambda(alpha);
      const double sigma = AlphaToSigma(alpha);
      sum_sigma += sigma;

      const double v_i = CalculateDynamicPayoff(flat_strategy_idx++, strategy, m_current_sigma, t);

      lhs[eq_idx++] = v_i + lambda - mu;
    }

    // (b) Probability sum equation
    lhs[eq_idx++] = sum_sigma - 1.0;

    player_idx++;
  }
}

void HPEquationSystem::GetJacobian(const Vector<double> &point, Matrix<double> &p_jac) const
{
  const double t = point[1];

  // Compute current sigma from alpha values
  int temp_alpha_idx = 2;
  for (const auto &player : m_game->GetPlayers()) {
    for (const auto &strategy : player->GetStrategies()) {
      m_current_sigma[strategy] = AlphaToSigma(point[temp_alpha_idx++]);
    }
  }

  // Initialize the Jacobian matrix to zero
  p_jac = 0.0;

  int eq_idx = 1;
  int player_idx = 1;
  int flat_s1_idx = 0;

  for (const auto &player1 : m_game->GetPlayers()) {

    // (a) Best response equations
    for (const auto &strat1 : player1->GetStrategies()) {

      // Column of t:
      const double payoff_vs_sigma = m_current_sigma.GetPayoff(strat1);
      const double payoff_vs_prior = m_payoffs_against_prior[flat_s1_idx];
      p_jac(1, eq_idx) = payoff_vs_sigma - payoff_vs_prior;

      // Column of mu_i: Derivative with respect to mu of this player (-1.0)
      p_jac(1 + m_star + player_idx, eq_idx) = -1.0;

      // Alpha columns:
      int alpha_col = 2;
      for (const auto &player2 : m_game->GetPlayers()) {
        for (const auto &strat2 : player2->GetStrategies()) {
          const double alpha2 = point[alpha_col];

          if (player1 == player2) {
            // Same strategy, use the derivative of lambda with respect to alpha
            if (strat1 == strat2) {
              p_jac(alpha_col, eq_idx) = AlphaToLambdaDeriv(alpha2);
            }
          }
          else {
            // Using chain rule
            const double deriv_u =
                m_current_sigma.GetPayoffDeriv(player1->GetNumber(), strat1, strat2);
            const double deriv_sigma = AlphaToSigmaDeriv(alpha2);
            p_jac(alpha_col, eq_idx) = t * deriv_u * deriv_sigma;
          }
          alpha_col++;
        }
      }
      eq_idx++;
      flat_s1_idx++;
    }

    // (b) Probability sum equation
    // Derivative with respect to 't' and 'mu' is 0
    // Only derivatives with respect to the alphas of this player
    int alpha_col = 2;
    for (const auto &player2 : m_game->GetPlayers()) {
      for (const auto &strat2 : player2->GetStrategies()) {
        if (player1 == player2) {
          p_jac(alpha_col, eq_idx) = AlphaToSigmaDeriv(point[alpha_col]);
        }
        alpha_col++;
      }
    }
    eq_idx++;
    player_idx++;
  }
}

Vector<double> HPEquationSystem::ComputeInitialPoint() const
{
  const int n_players = m_game->GetPlayers().size();
  const double tol = 1e-9; // Tolerance for floating-point comparisons

  // Dimension: 1 (t) + m_star (total strategies) + n (number of players)
  const int vector_size = 1 + m_star + n_players;
  Vector<double> start_point(vector_size);

  start_point[1] = 0.0; // t = 0

  int alpha_idx = 2;
  int player_idx = 1;
  int flat_strategy_idx = 0; // Index for accessing m_payoffs_against_prior

  for (const auto &player : m_game->GetPlayers()) {
    const int temp_idx = flat_strategy_idx;
    // Finding mu^i (the maximum payoff for player i against the prior)
    double max_payoff = -std::numeric_limits<double>::infinity();
    for (const auto &strategy : player->GetStrategies()) {
      const double payoff = m_payoffs_against_prior[flat_strategy_idx++];
      if (payoff > max_payoff) {
        max_payoff = payoff;
      }
    }

    // Store mu^i
    start_point[1 + m_star + player_idx] = max_payoff;

    // Compute alpha^i_s for each strategy s of player i

    bool found_br = false; // Flag to check if a best response has been found
    int local_s_idx = temp_idx;
    for (const auto &strategy : player->GetStrategies()) {
      const double lambda = max_payoff - m_payoffs_against_prior[local_s_idx++];
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

// v^i(t, s)
double HPEquationSystem::CalculateDynamicPayoff(int action_index, const GameStrategy &strategy,
                                                const MixedStrategyProfile<double> &current_sigma,
                                                double t) const
{
  const double payoff_against_sigma = current_sigma.GetPayoff(strategy);
  const double payoff_against_prior = m_payoffs_against_prior[action_index];
  return t * payoff_against_sigma + (1.0 - t) * payoff_against_prior;
}

} // end namespace Gambit
