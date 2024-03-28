//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/liap/efgliap.cc
// Compute Nash equilibria via Lyapunov function minimization
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
//                      class AgentLyapunovFunction
//------------------------------------------------------------------------

class AgentLyapunovFunction : public FunctionOnSimplices {
public:
  explicit AgentLyapunovFunction(const MixedBehaviorProfile<double> &p_start)
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

  ~AgentLyapunovFunction() override = default;

  double GetScale() const { return m_scale; }

private:
  Game m_game;
  mutable MixedBehaviorProfile<double> m_profile;
  double m_scale, m_penalty{100.0};

  double Value(const Vector<double> &x) const override;

  bool Gradient(const Vector<double> &, Vector<double> &) const override;

  double PenalizedLiapValue(const MixedBehaviorProfile<double> &p_profile) const;
};

inline double sum_infoset_probs(const MixedBehaviorProfile<double> &p_profile,
                                const GameInfoset &p_infoset)
{
  auto actions = p_infoset->GetActions();
  return std::accumulate(actions.begin(), actions.end(), 0.0,
                         [p_profile](double t, const GameAction &a) { return t + p_profile[a]; });
}

double
AgentLyapunovFunction::PenalizedLiapValue(const MixedBehaviorProfile<double> &p_profile) const
{
  double value = 0.0;
  // Liapunov function proper - should be replaced with call to profile once
  // the penalty is removed from that implementation.
  for (auto player : p_profile.GetGame()->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      for (auto action : infoset->GetActions()) {
        value += sqr(
            std::max(m_scale * (p_profile.GetPayoff(action) - p_profile.GetPayoff(infoset)), 0.0));
      }
    }
  }
  // Penalty function for non-negativity constraint for each action
  for (auto element : static_cast<const Array<double> &>(p_profile)) {
    value += m_penalty * sqr(std::min(element, 0.0));
  }
  // Penalty function for sum-to-one constraint for each action
  for (auto player : p_profile.GetGame()->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      value += m_penalty * sqr(sum_infoset_probs(m_profile, infoset) - 1.0);
    }
  }
  return value;
}

double AgentLyapunovFunction::Value(const Vector<double> &x) const
{
  m_profile = x;
  return PenalizedLiapValue(m_profile);
}

bool AgentLyapunovFunction::Gradient(const Vector<double> &x, Vector<double> &grad) const
{
  const double DELTA = .00001;
  m_profile = x;
  for (int i = 1; i <= x.Length(); i++) {
    m_profile[i] += DELTA;
    double value = PenalizedLiapValue(m_profile);
    m_profile[i] -= 2.0 * DELTA;
    value -= PenalizedLiapValue(m_profile);
    m_profile[i] += DELTA;
    grad[i] = value / (2.0 * DELTA);
  }
  Project(grad, static_cast<const Array<int> &>(m_game->NumActions()));
  return true;
}

namespace {

MixedBehaviorProfile<double> EnforceNonnegativity(const MixedBehaviorProfile<double> &p_profile)
{
  auto profile = p_profile;
  for (auto player : p_profile.GetGame()->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      for (auto action : infoset->GetActions()) {
        profile[action] = std::max(profile[action], 0.0);
      }
    }
  }
  return profile.Normalize();
}

} // namespace

List<MixedBehaviorProfile<double>> LiapBehaviorSolve(const MixedBehaviorProfile<double> &p_start,
                                                     double p_maxregret, int p_maxitsN,
                                                     BehaviorCallbackType p_callback)
{
  if (!p_start.GetGame()->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }

  List<MixedBehaviorProfile<double>> solutions;

  MixedBehaviorProfile<double> p(p_start);
  p_callback(p, "start");

  AgentLyapunovFunction F(p);
  Matrix<double> xi(p.BehaviorProfileLength(), p.BehaviorProfileLength());
  ConjugatePRMinimizer minimizer(p.BehaviorProfileLength());
  Vector<double> gradient(p.BehaviorProfileLength()), dx(p.BehaviorProfileLength());
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

  auto p2 = EnforceNonnegativity(p);
  if (p2.GetMaxRegret() * F.GetScale() < p_maxregret) {
    p_callback(p2, "NE");
    solutions.push_back(p2);
  }
  else {
    p_callback(p2, "end");
  }
  return solutions;
}

} // namespace Nash
} // namespace Gambit
