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

class HPEquation {
public:
  virtual ~HPEquation() = default;

  virtual double Value(const Vector<double> &point,
                       const MixedStrategyProfile<double> &current_sigma,
                       const std::vector<double> &payoffs_against_prior) const = 0;
};

namespace {

// Transforms alpha to sigma and lambda
inline double AlphaToSigma(double alpha) { return (alpha > 0.0) ? (alpha * alpha) : 0.0; }
inline double AlphaToLambda(double alpha) { return (alpha < 0.0) ? (alpha * alpha) : 0.0; }

// d(sigma)/d(alpha)
inline double AlphaToSigmaDeriv(double alpha) { return (alpha > 0.0) ? 2.0 * alpha : 0.0; }
// d(lambda)/d(alpha)
inline double AlphaToLambdaDeriv(double alpha) { return (alpha < 0.0) ? 2.0 * alpha : 0.0; }

// Eq (a): Best Response Equation
class BestResponseEquation final : public HPEquation {
  GameStrategy m_strategy;
  int m_alpha_idx;
  int m_mu_idx;
  int m_flat_s_idx;

public:
  BestResponseEquation(const GameStrategy &strat, int alpha_idx, int mu_idx, int flat_s_idx)
    : m_strategy(strat), m_alpha_idx(alpha_idx), m_mu_idx(mu_idx), m_flat_s_idx(flat_s_idx)
  {
  }

  ~BestResponseEquation() override = default;

  double Value(const Vector<double> &point, const MixedStrategyProfile<double> &current_sigma,
               const std::vector<double> &payoffs_against_prior) const override
  {
    const double t = point[1];
    const double alpha = point[m_alpha_idx];
    const double lambda = AlphaToLambda(alpha);
    const double mu = point[m_mu_idx];

    // Calculate Dynamic Payoff
    const double payoff_vs_sigma = current_sigma.GetPayoff(m_strategy);
    const double payoff_vs_prior = payoffs_against_prior[m_flat_s_idx];

    // v^i(t, s)
    const double v_i = t * payoff_vs_sigma + (1.0 - t) * payoff_vs_prior;

    return v_i + lambda - mu;
  }
};

// Eq (b): Probability Sum Equation
class ProbabilitySumEquation final : public HPEquation {
  int m_first_alpha_idx;
  int m_last_alpha_idx;

public:
  ProbabilitySumEquation(int first_idx, int last_idx)
    : m_first_alpha_idx(first_idx), m_last_alpha_idx(last_idx)
  {
  }

  ~ProbabilitySumEquation() override = default;

  double Value(const Vector<double> &point, const MixedStrategyProfile<double> &current_sigma,
               const std::vector<double> &payoffs_against_prior) const override
  {
    double sum_sigma = 0.0;
    for (int i = m_first_alpha_idx; i < m_last_alpha_idx; ++i) {
      sum_sigma += AlphaToSigma(point[i]);
    }
    return sum_sigma - 1.0;
  }
};

} // end namespace

HPEquationSystem::HPEquationSystem(const MixedStrategyProfile<double> &prior)
  : m_game(prior.GetGame()), m_prior(prior), m_star(prior.MixedProfileLength()),
    m_current_sigma(prior.GetGame()->NewMixedStrategyProfile(0.0))
{
  m_payoffs_against_prior.reserve(m_prior.MixedProfileLength());
  for (const auto &player : m_game->GetPlayers()) {
    for (const auto &strategy : player->GetStrategies()) {
      m_payoffs_against_prior.push_back(m_prior.GetPayoff(strategy));
    }
  }
  // Pre-allocate space for all equations: m_star (Best Responses) + n (Prob Sums)
  m_equations.reserve(m_star + m_game->GetPlayers().size());

  int alpha_idx = 2;
  int player_idx = 1;
  int flat_strategy_idx = 0;

  for (const auto &player : m_game->GetPlayers()) {
    const int first_alpha = alpha_idx;
    const int mu_idx = 1 + m_star + player_idx;

    // Instantiate Best Response Equations
    for (const auto &strategy : player->GetStrategies()) {
      m_equations.push_back(
          std::make_shared<BestResponseEquation>(strategy, alpha_idx, mu_idx, flat_strategy_idx));
      alpha_idx++;
      flat_strategy_idx++;
    }

    // Instantiate Probability Sum Equations
    m_equations.push_back(std::make_shared<ProbabilitySumEquation>(first_alpha, alpha_idx));

    player_idx++;
  }
}

void HPEquationSystem::GetValue(const Vector<double> &point, Vector<double> &lhs) const
{
  // Update internal mutable state
  UpdateSigma(point);

  // Evaluate all equations
  for (size_t i = 1; i <= m_equations.size(); ++i) {
    lhs[i] = m_equations[i - 1]->Value(point, m_current_sigma, m_payoffs_against_prior);
  }
}

void HPEquationSystem::GetJacobian(const Vector<double> &point, Matrix<double> &p_jac) const
{
  // Update internal mutable state
  UpdateSigma(point);

  // Rows of p_jac are variables (t, then alpha for each strategy, then mu for each player);
  // columns are equations: for each player, a best response equation for each strategy, then
  // that the player's probabilities sum to one.  Variable 1 + k is the alpha of the strategy at
  // position k of the profile.
  const double t = point[1];
  p_jac = 0.0;
  size_t column = 1;
  size_t first = 1; // position in the profile of the current player's first strategy
  int mu_row = 2 + m_star;
  for (const auto &player : m_game->GetPlayers()) {
    m_current_sigma.GetPayoffDerivBlock(player, m_blockValues, m_blockDerivs);
    const size_t last = first + m_blockValues.size() - 1;
    for (size_t r = 1; r <= m_blockValues.size(); r++, column++) {
      p_jac(1, column) = m_blockValues[r] - m_payoffs_against_prior[first + r - 2];
      for (size_t k = 1; k <= m_blockDerivs.NumColumns(); k++) {
        if (k < first || k > last) {
          p_jac(1 + k, column) = t * m_blockDerivs(r, k) * AlphaToSigmaDeriv(point[1 + k]);
        }
      }
      p_jac(first + r, column) = AlphaToLambdaDeriv(point[first + r]);
      p_jac(mu_row, column) = -1.0;
    }
    for (size_t k = first; k <= last; k++) {
      p_jac(1 + k, column) = AlphaToSigmaDeriv(point[1 + k]);
    }
    column++;
    mu_row++;
    first = last + 1;
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
      const double prob = AlphaToSigma(alpha_val);
      ret[strategy] = prob;
    }
  }
  ret = ret.Normalize();

  return ret;
}

void HPEquationSystem::UpdateSigma(const Vector<double> &point) const
{
  int temp_alpha_idx = 2;
  for (const auto &player : m_game->GetPlayers()) {
    for (const auto &strategy : player->GetStrategies()) {
      m_current_sigma[strategy] = AlphaToSigma(point[temp_alpha_idx++]);
    }
  }
}

} // end namespace Gambit
