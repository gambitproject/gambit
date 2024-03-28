//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/liap/nfgliap.cc
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

#include "gambit.h"
#include "core/function.h"
#include "liap.h"

namespace Gambit {
namespace Nash {

//------------------------------------------------------------------------
//                    class StrategicLyapunovFunction
//------------------------------------------------------------------------

class StrategicLyapunovFunction : public FunctionOnSimplices {
public:
  explicit StrategicLyapunovFunction(const MixedStrategyProfile<double> &p_start)
    : m_game(p_start.GetGame()), m_profile(p_start)
  {
    m_scale = m_game->GetMaxPayoff() - m_game->GetMinPayoff();
    if (m_scale == 0.0) {
      m_scale = 1.0;
    }
    else {
      m_scale = 1.0 / m_scale;
    }
  }
  ~StrategicLyapunovFunction() override = default;

  double GetScale() const { return m_scale; }

private:
  Game m_game;
  mutable MixedStrategyProfile<double> m_profile;
  double m_scale, m_penalty{100.0};

  double Value(const Vector<double> &) const override;
  bool Gradient(const Vector<double> &, Vector<double> &) const override;

  double LiapDerivValue(const MixedStrategyProfile<double> &, const GameStrategy &) const;
};

inline double sum_player_probs(const MixedStrategyProfile<double> &p_profile,
                               const GamePlayer &p_player)
{
  auto strategies = p_player->GetStrategies();
  return std::accumulate(
      strategies.cbegin(), strategies.cend(), 0.0,
      [p_profile](double t, const GameStrategy &s) { return t + p_profile[s]; });
}

double StrategicLyapunovFunction::Value(const Vector<double> &v) const
{
  m_profile = v;
  double value = 0;
  // Liapunov function proper - should be replaced with call to profile once
  // the penalty is removed from that implementation.
  for (auto player : m_profile.GetGame()->GetPlayers()) {
    for (auto strategy : player->GetStrategies()) {
      value += sqr(
          std::max(m_scale * (m_profile.GetPayoff(strategy) - m_profile.GetPayoff(player)), 0.0));
    }
  }
  // Penalty function for non-negativity constraint for each strategy
  for (auto player : m_profile.GetGame()->GetPlayers()) {
    for (auto strategy : player->GetStrategies()) {
      value += m_penalty * sqr(std::min(m_profile[strategy], 0.0));
    }
  }
  // Penalty function for sum-to-one constraint for each player
  for (auto player : m_profile.GetGame()->GetPlayers()) {
    value += m_penalty * sqr(sum_player_probs(m_profile, player) - 1.0);
  }
  return value;
}

double StrategicLyapunovFunction::LiapDerivValue(const MixedStrategyProfile<double> &p_profile,
                                                 const GameStrategy &p_wrt_strategy) const
{
  double deriv = 0.0;
  for (auto player : m_game->GetPlayers()) {
    for (auto strategy : player->GetStrategies()) {
      double loss = sqr(m_scale) * (p_profile.GetPayoff(strategy) - p_profile.GetPayoff(player));
      if (loss <= 0.0) {
        continue;
      }
      deriv -= loss * p_profile.GetPayoffDeriv(player->GetNumber(), p_wrt_strategy);
      if (player != p_wrt_strategy->GetPlayer()) {
        deriv += loss * p_profile.GetPayoffDeriv(player->GetNumber(), strategy, p_wrt_strategy);
      }
    }
  }
  // Penalty function for non-negativity constraint for each strategy
  deriv += m_penalty * std::min(p_profile[p_wrt_strategy], 0.0);
  // Penalty function for sum-to-one constraint for each player
  deriv += m_penalty * (sum_player_probs(p_profile, p_wrt_strategy->GetPlayer()) - 1.0);
  return 2.0 * deriv;
}

bool StrategicLyapunovFunction::Gradient(const Vector<double> &v, Vector<double> &d) const
{
  m_profile = v;
  auto element = d.begin();
  for (auto player : m_game->GetPlayers()) {
    for (auto strategy : player->GetStrategies()) {
      *element = LiapDerivValue(m_profile, strategy);
      ++element;
    }
  }
  Project(d, m_game->NumStrategies());
  return true;
}

namespace {

MixedStrategyProfile<double> EnforceNonnegativity(const MixedStrategyProfile<double> &p_profile)
{
  auto profile = p_profile;
  for (auto player : p_profile.GetGame()->GetPlayers()) {
    for (auto strategy : player->GetStrategies()) {
      profile[strategy] = std::max(profile[strategy], 0.0);
    }
  }
  return profile.Normalize();
}

} // namespace

List<MixedStrategyProfile<double>> LiapStrategySolve(const MixedStrategyProfile<double> &p_start,
                                                     double p_maxregret, int p_maxitsN,
                                                     StrategyCallbackType p_callback)
{
  if (!p_start.GetGame()->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }

  List<MixedStrategyProfile<double>> solutions;

  MixedStrategyProfile<double> p(p_start);
  p_callback(p, "start");

  StrategicLyapunovFunction F(p);
  ConjugatePRMinimizer minimizer(p.MixedProfileLength());
  Vector<double> gradient(p.MixedProfileLength()), dx(p.MixedProfileLength());
  double fval;
  minimizer.Set(F, static_cast<const Vector<double> &>(p), fval, gradient, .001, .00001);

  for (int iter = 1; iter <= p_maxitsN; iter++) {
    Vector<double> point(p);
    if (!minimizer.Iterate(F, point, fval, gradient, dx)) {
      break;
    }
    p = point;
    if (std::sqrt(gradient.NormSquared()) < 1.0e-8) {
      break;
    }
  }

  p = EnforceNonnegativity(p);
  if (p.GetMaxRegret() * F.GetScale() < p_maxregret) {
    p_callback(p, "NE");
    solutions.push_back(p);
  }
  else {
    p_callback(p, "end");
  }

  return solutions;
}

} // namespace Nash
} // namespace Gambit
