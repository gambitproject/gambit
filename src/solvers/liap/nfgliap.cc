//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/liap/nfgliap.cc
// Compute Nash equilibria by minimizing Liapunov function
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

#include "games.h"
#include "core/function.h"
#include "liap.h"

namespace Gambit::Nash {

//------------------------------------------------------------------------
//                    class StrategicLyapunovFunction
//------------------------------------------------------------------------

class StrategicLyapunovFunction : public FunctionOnSimplices {
public:
  explicit StrategicLyapunovFunction(const MixedStrategyProfile<double> &p_start)
    : m_game(p_start.GetGame()), m_profile(p_start)
  {
    m_scale = static_cast<double>(m_game->GetMaxPayoff() - m_game->GetMinPayoff());
    m_scale = (m_scale == 0.0) ? 1.0 : 1.0 / m_scale;
  }
  ~StrategicLyapunovFunction() override = default;

  double GetScale() const { return m_scale; }

private:
  Game m_game;
  mutable MixedStrategyProfile<double> m_profile;
  double m_scale, m_penalty{100.0};

  double Value(const Vector<double> &) const override;
  bool Gradient(const Vector<double> &, Vector<double> &) const override;
};

inline double sum_player_probs(const MixedStrategyProfile<double> &p_profile,
                               const GamePlayer &p_player)
{
  return sum_function(p_player->GetStrategies(),
                      [&](const auto &strategy) -> double { return p_profile[strategy]; });
}

double StrategicLyapunovFunction::Value(const Vector<double> &v) const
{
  m_profile = v;
  // Liapunov function proper
  double value = sqr(m_scale) * m_profile.GetLiapValue();
  // Penalty for non-negativity constraint for each strategy
  value += m_penalty *
           sum_function(m_profile.GetGame()->GetStrategies(), [&](const auto &strategy) -> double {
             return sqr(std::min(m_profile[strategy], 0.0));
           });
  // Penalty for sum-to-one constraint for each player
  value += m_penalty *
           sum_function(m_profile.GetGame()->GetPlayers(), [&](const auto &player) -> double {
             return sqr(sum_player_probs(m_profile, player) - 1.0);
           });
  return value;
}

bool StrategicLyapunovFunction::Gradient(const Vector<double> &v, Vector<double> &d) const
{
  m_profile = v;
  d = 0.0;
  Vector<double> values;
  Matrix<double> derivs;
  size_t offset = 0;
  for (const auto &player : m_game->GetPlayers()) {
    const auto &strategies = player->GetStrategies();
    m_profile.GetPayoffDerivBlock(player, values, derivs);
    const double payoff = m_profile.GetPayoff(player);
    Vector<double> weights(strategies.size());
    double totalLoss = 0.0;
    for (size_t row = 1; row <= strategies.size(); row++) {
      weights[row] = sqr(m_scale) * std::max(values[row] - payoff, 0.0);
      totalLoss += weights[row];
    }
    if (totalLoss > 0.0) {
      size_t row = 1;
      for (const auto &strategy : strategies) {
        weights[row++] -= totalLoss * m_profile[strategy];
      }
      d += weights * derivs;
      for (row = 1; row <= strategies.size(); row++) {
        d[offset + row] -= totalLoss * values[row];
      }
    }
    offset += strategies.size();
  }
  size_t index = 1;
  for (const auto &strategy : m_game->GetStrategies()) {
    // Penalty function for non-negativity constraint for each strategy
    d[index] += m_penalty * std::min(m_profile[strategy], 0.0);
    // Penalty function for sum-to-one constraint for each player
    d[index] += m_penalty * (sum_player_probs(m_profile, strategy->GetPlayer()) - 1.0);
    index++;
  }
  d *= 2.0;
  Project(d, m_game->GetStrategies().shape_array());
  return true;
}

namespace {

MixedStrategyProfile<double> EnforceNonnegativity(const MixedStrategyProfile<double> &p_profile)
{
  auto profile = p_profile;
  for (const auto &strategy : p_profile.GetGame()->GetStrategies()) {
    profile[strategy] = std::max(profile[strategy], 0.0);
  }
  return profile.Normalize();
}

} // namespace

LiapStrategyResult
LiapStrategySolve(const MixedStrategyProfile<double> &p_start, double p_maxregret, int p_maxitsN,
                  const StrategyCallbackType<double> &p_onEquilibrium,
                  const LiapEventCallbackType<MixedStrategyProfile<double>> &p_onEvent,
                  const CancelToken &p_cancel)
{
  RequirePerfectRecall(p_start.GetGame());

  MixedStrategyProfile<double> p(p_start);
  p_onEvent(LiapStartEvent<MixedStrategyProfile<double>>{p});

  const StrategicLyapunovFunction F(p);
  ConjugatePRMinimizer minimizer(p.MixedProfileLength());
  Vector<double> gradient(p.MixedProfileLength()), dx(p.MixedProfileLength());
  double fval;
  minimizer.Set(F, p.GetProbVector(), fval, gradient, .001, .00001);

  bool minimizerFailed = false;
  for (int iter = 1; iter <= p_maxitsN; iter++) {
    p_cancel.Check();
    Vector<double> point(p.GetProbVector());
    try {
      if (!minimizer.Iterate(F, point, fval, gradient, dx)) {
        break;
      }
    }
    catch (const FunctionMinimizerError &) {
      minimizerFailed = true;
      break;
    }
    p = point;
    if (std::sqrt(gradient.NormSquared()) < 1.0e-8) {
      break;
    }
  }

  p = EnforceNonnegativity(p);
  const double regret = p.GetMaxRegret() * F.GetScale();
  if (regret < p_maxregret) {
    p_onEquilibrium(p);
    return {.equilibrium = p, .success = true, .reason = LiapTerminationReason::Converged};
  }
  p_onEvent(LiapEndEvent<MixedStrategyProfile<double>>{.profile = p, .regret = regret});
  return {.equilibrium = std::nullopt,
          .success = false,
          .reason = minimizerFailed ? LiapTerminationReason::MinimizerFailed
                                    : LiapTerminationReason::RegretTargetNotReached};
}

} // namespace Gambit::Nash
