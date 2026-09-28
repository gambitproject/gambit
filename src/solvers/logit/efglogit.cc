//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/logit/efglogit.cc
// Computation of agent quantal response equilibrium correspondence for
// extensive games.
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
#include <cmath>
#include <vector>

#include "games.h"
#include "games/gametree.h"
#include "logit.h"
#include "solvers/path/path.h"

namespace Gambit {

namespace {

double LogLike(const Vector<double> &p_frequencies, const Vector<double> &p_point)
{
  return std::inner_product(p_frequencies.begin(), p_frequencies.end(), p_point.begin(), 0.0,
                            std::plus<>(),
                            [](double freq, double prob) { return freq * std::log(prob); });
}

double DiffLogLike(const Vector<double> &p_frequencies, const Vector<double> &p_tangent)
{
  return std::inner_product(p_frequencies.begin(), p_frequencies.end(), p_tangent.begin(), 0.0);
}

MixedBehaviorProfile<double> PointToProfile(const Game &p_game, const Vector<double> &p_point)
{
  MixedBehaviorProfile<double> profile(p_game);
  for (size_t i = 1; i < p_point.size(); i++) {
    profile[i] = std::exp(p_point[i]);
  }
  return profile;
}

Vector<double> ProfileToPoint(const LogitQREMixedBehaviorProfile &p_profile)
{
  Vector<double> point(p_profile.size() + 1);
  for (size_t i = 1; i <= p_profile.size(); i++) {
    point[i] = std::log(p_profile[i]);
  }
  point.back() = p_profile.GetLambda();
  return point;
}

bool RegretTerminationFunction(const Game &p_game, const Vector<double> &p_point, double p_regret)
{
  return (p_point.back() < 0.0 || PointToProfile(p_game, p_point).GetAgentMaxRegret() < p_regret);
}

//
// Quantities derived from a point on the path, stored in arrays aligned with the tree layout
// of the game.  These are the quantities a MixedBehaviorProfile computes, but derived from the
// log probabilities that are the coordinates of the path.
//
// Realization probabilities are kept as logarithms, and beliefs are computed relative to the
// most likely member of each information set.  This keeps beliefs accurate at information
// sets whose realization probability is going to zero, which is needed to approximate the
// limiting sequential equilibrium well, and where the probabilities themselves may underflow.
//
// Node values here include the payoffs of the outcomes above each node, accumulated along the
// path from the root.  Off the path these differ from the values a MixedBehaviorProfile would
// compute by more than a per-node constant, because the probabilities at an information set
// need not sum to one there; this convention defines the equations the tracer has always used.
//
struct LogPathValues {
  std::vector<double> prob, logProb; // by action; entry 0 unused
  std::vector<double> logRealizProb; // by node
  std::vector<double> belief;        // by node; meaningful at personal information sets
  std::vector<double> nodeValues;    // by node, one entry per player
  std::vector<double> actionValues;  // by action; entry 0 unused
};

/// Total payoffs of the outcomes on the path from the root to each node, one entry per player
std::vector<double> ComputePathPayoffs(const TreeLayout &p_layout)
{
  const size_t numPlayers = p_layout.numPlayers;
  const auto &payoffs = p_layout.GetNumbers<double>().payoffs;
  std::vector<double> pathPayoffs(payoffs.size());
  for (size_t k = 0; k < p_layout.parent.size(); k++) {
    for (size_t pl = 0; pl < numPlayers; pl++) {
      const double inherited =
          (k == 0) ? 0.0 : pathPayoffs[static_cast<size_t>(p_layout.parent[k]) * numPlayers + pl];
      pathPayoffs[k * numPlayers + pl] = inherited + payoffs[k * numPlayers + pl];
    }
  }
  return pathPayoffs;
}

void ComputeLogPathValues(const TreeLayout &p_layout, const std::vector<double> &p_logChanceProb,
                          const std::vector<double> &p_pathPayoffs, const Vector<double> &p_point,
                          LogPathValues &p_values)
{
  const size_t numNodes = p_layout.parent.size();
  const size_t numPlayers = p_layout.numPlayers;
  const auto &numbers = p_layout.GetNumbers<double>();
  auto &v = p_values;
  v.prob.resize(p_layout.numActions + 1);
  v.logProb.resize(p_layout.numActions + 1);
  for (size_t a = 1; a <= p_layout.numActions; a++) {
    v.logProb[a] = p_point[a];
    v.prob[a] = std::exp(p_point[a]);
  }

  v.logRealizProb.resize(numNodes);
  v.logRealizProb[0] = 0.0;
  for (size_t k = 1; k < numNodes; k++) {
    const size_t a = p_layout.action[k];
    v.logRealizProb[k] =
        v.logRealizProb[p_layout.parent[k]] + ((a != 0) ? v.logProb[a] : p_logChanceProb[k]);
  }

  v.belief.assign(numNodes, 0.0);
  for (size_t i = 0; i < p_layout.numPersonalInfosets; i++) {
    const auto &members = p_layout.infosets[i].members;
    double infosetProb = 0.0;
    for (const size_t m : members) {
      infosetProb += std::exp(v.logRealizProb[m]);
    }
    if (infosetProb == 0.0) {
      // Possible when a zero-probability chance action makes the information set
      // unreachable; beliefs are then taken to be uniform
      for (const size_t m : members) {
        v.belief[m] = 1.0 / static_cast<double>(members.size());
      }
      continue;
    }
    double maxLogProb = v.logRealizProb[members.front()];
    for (const size_t m : members) {
      maxLogProb = std::max(maxLogProb, v.logRealizProb[m]);
    }
    double total = 0.0;
    for (const size_t m : members) {
      total += std::exp(v.logRealizProb[m] - maxLogProb);
    }
    const double mostLikelyBelief = 1.0 / total;
    for (const size_t m : members) {
      v.belief[m] = mostLikelyBelief * std::exp(v.logRealizProb[m] - maxLogProb);
    }
  }

  // Children follow their parents in the layout, so a reverse sweep is a postorder traversal
  v.nodeValues.resize(numNodes * numPlayers);
  for (size_t k = numNodes; k-- > 0;) {
    double *value = &v.nodeValues[k * numPlayers];
    if (p_layout.numChildren[k] == 0) {
      std::copy_n(&p_pathPayoffs[k * numPlayers], numPlayers, value);
      continue;
    }
    std::fill_n(value, numPlayers, 0.0);
    for (size_t j = 0; j < p_layout.numChildren[k]; j++) {
      const size_t child = p_layout.childList[p_layout.firstChild[k] + j];
      const size_t a = p_layout.action[child];
      const double childProb = (a != 0) ? v.prob[a] : numbers.chanceProb[child];
      for (size_t pl = 0; pl < numPlayers; pl++) {
        value[pl] += childProb * v.nodeValues[child * numPlayers + pl];
      }
    }
  }

  v.actionValues.assign(p_layout.numActions + 1, 0.0);
  for (size_t i = 0; i < p_layout.numPersonalInfosets; i++) {
    const auto &infoset = p_layout.infosets[i];
    for (const size_t member : infoset.members) {
      for (size_t j = 0; j < p_layout.numChildren[member]; j++) {
        const size_t child = p_layout.childList[p_layout.firstChild[member] + j];
        v.actionValues[p_layout.action[child]] +=
            v.belief[member] * v.nodeValues[child * numPlayers + infoset.player];
      }
    }
  }
}

class EquationSystem {
public:
  explicit EquationSystem(const Game &p_game);
  ~EquationSystem() = default;

  // Compute the value of the system of equations at the specified point.
  void GetValue(const Vector<double> &p_point, Vector<double> &p_lhs) const;

  // Compute the Jacobian matrix at the specified point.
  void GetJacobian(const Vector<double> &p_point, Matrix<double> &p_matrix) const;

private:
  std::shared_ptr<const TreeLayout> m_layout;
  std::vector<double> m_logChanceProb; // by node
  std::vector<double> m_pathPayoffs;   // by node, one entry per player
  mutable LogPathValues m_values;
  mutable Matrix<double> m_derivs;
};

EquationSystem::EquationSystem(const Game &p_game)
  : m_layout(p_game->GetTreeLayout()), m_pathPayoffs(ComputePathPayoffs(*m_layout)),
    m_derivs(m_layout->numActions, m_layout->numActions)
{
  const auto &chanceProb = m_layout->GetNumbers<double>().chanceProb;
  m_logChanceProb.resize(chanceProb.size());
  std::ranges::transform(chanceProb, m_logChanceProb.begin(),
                         [](double p) { return std::log(p); });
}

// Equations are, for each information set in turn, that its action probabilities sum to
// one, followed by one equation for each action but the first relating its log probability
// to that of the first.
void EquationSystem::GetValue(const Vector<double> &p_point, Vector<double> &p_lhs) const
{
  ComputeLogPathValues(*m_layout, m_logChanceProb, m_pathPayoffs, p_point, m_values);
  const auto &v = m_values;
  const double lambda = p_point.back();
  size_t row = 1;
  for (size_t i = 0; i < m_layout->numPersonalInfosets; i++) {
    const auto &infoset = m_layout->infosets[i];
    const size_t ref = infoset.firstAction;
    double sum = -1.0;
    for (size_t a = ref; a < ref + infoset.numActions; a++) {
      sum += v.prob[a];
    }
    p_lhs[row++] = sum;
    for (size_t a = ref + 1; a < ref + infoset.numActions; a++) {
      p_lhs[row++] =
          v.logProb[a] - v.logProb[ref] - lambda * (v.actionValues[a] - v.actionValues[ref]);
    }
  }
}

void EquationSystem::GetJacobian(const Vector<double> &p_point, Matrix<double> &p_matrix) const
{
  // Rows of p_matrix are variables (log probabilities, then lambda); columns are equations,
  // in the same order as in GetValue().
  ComputeLogPathValues(*m_layout, m_logChanceProb, m_pathPayoffs, p_point, m_values);
  const auto &v = m_values;
  DiffActionValues(
      *m_layout, v.belief, v.nodeValues, v.actionValues,
      [&v](size_t p_from, size_t p_to) {
        return std::exp(v.logRealizProb[p_to] - v.logRealizProb[p_from]);
      },
      m_derivs);
  const size_t numActions = m_layout->numActions;
  const double lambda = p_point.back();

  p_matrix = 0.0;
  size_t column = 1;
  for (size_t i = 0; i < m_layout->numPersonalInfosets; i++) {
    const auto &infoset = m_layout->infosets[i];
    const size_t first = infoset.firstAction;
    const size_t last = first + infoset.numActions - 1;
    for (size_t a = first; a <= last; a++) {
      p_matrix(a, column) = v.prob[a];
    }
    column++;
    for (size_t a = first + 1; a <= last; a++, column++) {
      for (size_t var = 1; var < first; var++) {
        p_matrix(var, column) = -lambda * (m_derivs(a, var) - m_derivs(first, var));
      }
      for (size_t var = last + 1; var <= numActions; var++) {
        p_matrix(var, column) = -lambda * (m_derivs(a, var) - m_derivs(first, var));
      }
      p_matrix(a, column) = 1.0;
      p_matrix(first, column) = -1.0;
      p_matrix(numActions + 1, column) = v.actionValues[first] - v.actionValues[a];
    }
  }
}

class TracingCallbackFunction {
public:
  TracingCallbackFunction(const Game &p_game,
                          LogitEventCallbackType<LogitQREMixedBehaviorProfile> p_onEvent)
    : m_game(p_game), m_onEvent(p_onEvent)
  {
  }
  ~TracingCallbackFunction() = default;

  void AppendPoint(const Vector<double> &p_point);
  void OnBifurcation(const Vector<double> &p_before, const Vector<double> &p_after);
  void OnPerturbation(bool p_active, const Vector<double> &p_point);
  const std::list<LogitQREMixedBehaviorProfile> &GetProfiles() const { return m_profiles; }
  const std::vector<LogitBifurcation<LogitQREMixedBehaviorProfile>> &GetBifurcations() const
  {
    return m_bifurcations;
  }

private:
  LogitQREMixedBehaviorProfile PointToQRE(const Vector<double> &p_point) const;

  Game m_game;
  LogitEventCallbackType<LogitQREMixedBehaviorProfile> m_onEvent;
  std::list<LogitQREMixedBehaviorProfile> m_profiles;
  std::vector<LogitBifurcation<LogitQREMixedBehaviorProfile>> m_bifurcations;
};

LogitQREMixedBehaviorProfile
TracingCallbackFunction::PointToQRE(const Vector<double> &p_point) const
{
  return {PointToProfile(m_game, p_point), p_point.back(), 1.0};
}

void TracingCallbackFunction::AppendPoint(const Vector<double> &p_point)
{
  m_profiles.push_back(PointToQRE(p_point));
  m_onEvent(LogitPathEvent<LogitQREMixedBehaviorProfile>{m_profiles.back()});
}

void TracingCallbackFunction::OnBifurcation(const Vector<double> &p_before,
                                            const Vector<double> &p_after)
{
  m_bifurcations.push_back({PointToQRE(p_before), PointToQRE(p_after)});
  const auto &bifurcation = m_bifurcations.back();
  m_onEvent(
      LogitBifurcationEvent<LogitQREMixedBehaviorProfile>{bifurcation.before, bifurcation.after});
}

void TracingCallbackFunction::OnPerturbation(bool p_active, const Vector<double> &p_point)
{
  const auto qre = PointToQRE(p_point);
  m_onEvent(LogitPerturbationEvent<LogitQREMixedBehaviorProfile>{p_active, qre});
}

class EstimatorCallbackFunction {
public:
  EstimatorCallbackFunction(const Game &p_game, const Vector<double> &p_frequencies,
                            LogitEventCallbackType<LogitQREMixedBehaviorProfile> p_onEvent);
  ~EstimatorCallbackFunction() = default;

  void EvaluatePoint(const Vector<double> &p_point);
  void OnBifurcation(const Vector<double> &p_before, const Vector<double> &p_after);
  void OnPerturbation(bool p_active, const Vector<double> &p_point);
  const LogitQREMixedBehaviorProfile &GetMaximizer() const { return m_bestProfile; }

private:
  LogitQREMixedBehaviorProfile PointToQRE(const Vector<double> &p_point) const;

  Game m_game;
  const Vector<double> &m_frequencies;
  LogitEventCallbackType<LogitQREMixedBehaviorProfile> m_onEvent;
  LogitQREMixedBehaviorProfile m_bestProfile;
};

EstimatorCallbackFunction::EstimatorCallbackFunction(
    const Game &p_game, const Vector<double> &p_frequencies,
    LogitEventCallbackType<LogitQREMixedBehaviorProfile> p_onEvent)
  : m_game(p_game), m_frequencies(p_frequencies), m_onEvent(p_onEvent),
    m_bestProfile(MixedBehaviorProfile<double>(p_game), 0.0,
                  LogLike(p_frequencies, static_cast<const Vector<double> &>(
                                             MixedBehaviorProfile<double>(p_game))))
{
}

LogitQREMixedBehaviorProfile
EstimatorCallbackFunction::PointToQRE(const Vector<double> &p_point) const
{
  const MixedBehaviorProfile<double> profile(PointToProfile(m_game, p_point));
  return {profile, p_point.back(),
          LogLike(m_frequencies, static_cast<const Vector<double> &>(profile))};
}

void EstimatorCallbackFunction::EvaluatePoint(const Vector<double> &p_point)
{
  auto qre = PointToQRE(p_point);
  m_onEvent(LogitPathEvent<LogitQREMixedBehaviorProfile>{qre});
  if (qre.GetLogLike() > m_bestProfile.GetLogLike()) {
    m_bestProfile = qre;
  }
}

void EstimatorCallbackFunction::OnBifurcation(const Vector<double> &p_before,
                                              const Vector<double> &p_after)
{
  const auto before = PointToQRE(p_before);
  const auto after = PointToQRE(p_after);
  m_onEvent(LogitBifurcationEvent<LogitQREMixedBehaviorProfile>{before, after});
}

void EstimatorCallbackFunction::OnPerturbation(bool p_active, const Vector<double> &p_point)
{
  const auto qre = PointToQRE(p_point);
  m_onEvent(LogitPerturbationEvent<LogitQREMixedBehaviorProfile>{p_active, qre});
}

} // namespace

LogitTrace<LogitQREMixedBehaviorProfile>
LogitBehaviorSolve(const LogitQREMixedBehaviorProfile &p_start, double p_regret,
                   PathTracer::TraceDirection p_direction, double p_firstStep, double p_maxAccel,
                   Nash::BehaviorCallbackType<double> p_onEquilibrium,
                   LogitEventCallbackType<LogitQREMixedBehaviorProfile> p_onEvent,
                   const CancelToken &p_cancel)
{
  if (p_start.size() == 0) {
    return {{p_start}, {}};
  }
  PathTracer tracer;
  tracer.SetMaxDecel(p_maxAccel);
  tracer.SetStepsize(p_firstStep);

  const double scale = p_start.GetGame()->GetMaxPayoff() - p_start.GetGame()->GetMinPayoff();
  if (scale != 0.0) {
    p_regret *= scale;
  }
  const Game game = p_start.GetGame();
  Vector<double> x(ProfileToPoint(p_start));
  TracingCallbackFunction callback(game, p_onEvent);
  EquationSystem system(game);
  const auto result = tracer.TracePath(
      [&system](const Vector<double> &p_point, Vector<double> &p_lhs) {
        system.GetValue(p_point, p_lhs);
      },
      [&system](const Vector<double> &p_point, Matrix<double> &p_jac) {
        system.GetJacobian(p_point, p_jac);
      },
      x, p_direction, x.size(),
      [game, p_regret](const Vector<double> &p_point) {
        return RegretTerminationFunction(game, p_point, p_regret);
      },
      [&callback](const Vector<double> &p_point) -> void { callback.AppendPoint(p_point); },
      NullCriterionFunction, NullCriterionBracketFunction, p_cancel,
      [&callback](const Vector<double> &p_before, const Vector<double> &p_after) {
        callback.OnBifurcation(p_before, p_after);
      },
      [&callback](bool p_active, const Vector<double> &p_point) {
        callback.OnPerturbation(p_active, p_point);
      });
  const auto &profiles = callback.GetProfiles();
  if (profiles.back().GetProfile().GetAgentMaxRegret() < p_regret) {
    p_onEquilibrium(profiles.back().GetProfile());
  }
  return {profiles, callback.GetBifurcations(), {result.stats}};
}

LogitLambdaResult<LogitQREMixedBehaviorProfile> LogitBehaviorSolveLambda(
    const LogitQREMixedBehaviorProfile &p_start, const std::list<double> &p_targetLambda,
    PathTracer::TraceDirection p_direction, double p_firstStep, double p_maxAccel,
    LogitEventCallbackType<LogitQREMixedBehaviorProfile> p_onEvent)
{
  if (p_start.size() == 0) {
    return {{p_start}, {}};
  }
  PathTracer tracer;
  tracer.SetMaxDecel(p_maxAccel);
  tracer.SetStepsize(p_firstStep);

  const Game game = p_start.GetGame();
  Vector<double> x(ProfileToPoint(p_start));
  TracingCallbackFunction callback(game, p_onEvent);
  EquationSystem system(game);
  LogitLambdaResult<LogitQREMixedBehaviorProfile> ret;
  for (auto lam : p_targetLambda) {
    const auto result = tracer.TracePath(
        [&system](const Vector<double> &p_point, Vector<double> &p_lhs) {
          system.GetValue(p_point, p_lhs);
        },
        [&system](const Vector<double> &p_point, Matrix<double> &p_jac) {
          system.GetJacobian(p_point, p_jac);
        },
        x, p_direction, x.size(), LambdaPositiveTerminationFunction,
        [&callback](const Vector<double> &p_point) -> void { callback.AppendPoint(p_point); },
        [lam](const Vector<double> &x, const Vector<double> &) -> double {
          return x.back() - lam;
        },
        NullCriterionBracketFunction, CancelToken(),
        [&callback](const Vector<double> &p_before, const Vector<double> &p_after) {
          callback.OnBifurcation(p_before, p_after);
        },
        [&callback](bool p_active, const Vector<double> &p_point) {
          callback.OnPerturbation(p_active, p_point);
        });
    ret.profiles.push_back(callback.GetProfiles().back());
    ret.stats.push_back(result.stats);
  }
  return ret;
}

LogitQREMixedBehaviorProfile
LogitBehaviorEstimate(const MixedBehaviorProfile<double> &p_frequencies, double p_maxLambda,
                      PathTracer::TraceDirection p_direction, double p_stopAtLocal,
                      double p_firstStep, double p_maxAccel,
                      LogitEventCallbackType<LogitQREMixedBehaviorProfile> p_onEvent)
{
  const LogitQREMixedBehaviorProfile start(p_frequencies.GetGame());
  if (start.size() == 0) {
    return start;
  }
  PathTracer tracer;
  tracer.SetMaxDecel(p_maxAccel);
  tracer.SetStepsize(p_firstStep);

  Vector<double> x(ProfileToPoint(start)), restart(x);
  const Vector<double> freq_vector(static_cast<const Vector<double> &>(p_frequencies));
  EstimatorCallbackFunction callback(
      start.GetGame(), static_cast<const Vector<double> &>(p_frequencies), p_onEvent);
  EquationSystem system(start.GetGame());
  while (true) {
    tracer.TracePath(
        [&system](const Vector<double> &p_point, Vector<double> &p_lhs) {
          system.GetValue(p_point, p_lhs);
        },
        [&system](const Vector<double> &p_point, Matrix<double> &p_jac) {
          system.GetJacobian(p_point, p_jac);
        },
        x, p_direction, x.size(),
        [p_maxLambda](const Vector<double> &p_point) {
          return LambdaRangeTerminationFunction(p_point, 0, p_maxLambda);
        },
        [&callback](const Vector<double> &p_point) -> void { callback.EvaluatePoint(p_point); },
        [freq_vector](const Vector<double> &, const Vector<double> &p_tangent) -> double {
          return DiffLogLike(freq_vector, p_tangent);
        },
        [&restart](const Vector<double> &, const Vector<double> &p_restart) -> void {
          restart = p_restart;
        },
        CancelToken(),
        [&callback](const Vector<double> &p_before, const Vector<double> &p_after) {
          callback.OnBifurcation(p_before, p_after);
        },
        [&callback](bool p_active, const Vector<double> &p_point) {
          callback.OnPerturbation(p_active, p_point);
        });
    if (p_stopAtLocal || x.back() >= p_maxLambda) {
      break;
    }
    x = restart;
  }
  return callback.GetMaximizer();
}

} // end namespace Gambit
