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
#include <map>
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
// The tree of an extensive game, flattened for repeated evaluation of the agent logit
// equations along the path.  Nodes are numbered in depth-first preorder, so a node's
// parent always precedes it.  Player actions are numbered by their position in the
// behavior profile, starting at 1; 0 means that no player action leads to a node.
//
struct TreeLayout {
  struct Infoset {
    size_t player{0};      // position of the owning player, from 0
    size_t firstAction{0}; // profile position of the first action
    size_t numActions{0};
    std::vector<size_t> members; // in the information set's own order
  };

  size_t numPlayers{0};
  size_t numActions{0};
  std::vector<int> parent;           // -1 at the root
  std::vector<size_t> action;        // player action leading to the node, or 0
  std::vector<double> chanceProb;    // probability of the chance action leading to the node
  std::vector<double> logChanceProb; // its logarithm
  std::vector<int> infoset;          // player information set at the node, or -1
  std::vector<size_t> firstChild;    // children are childList[firstChild, firstChild+numChildren)
  std::vector<size_t> numChildren;
  std::vector<size_t> childList;
  std::vector<double> pathPayoffs; // numPlayers per node: sum of outcomes from the root
  std::vector<size_t> terminals;
  std::vector<Infoset> infosets;
};

void AddNode(TreeLayout &p_layout, const GameNode &p_node, int p_parent, size_t p_action,
             double p_chanceProb, const std::map<GameInfoset, int> &p_infosets,
             const std::vector<GamePlayer> &p_players, std::map<GameNode, size_t> &p_index)
{
  const size_t index = p_layout.parent.size();
  p_index[p_node] = index;
  p_layout.parent.push_back(p_parent);
  p_layout.action.push_back(p_action);
  p_layout.chanceProb.push_back(p_chanceProb);
  p_layout.logChanceProb.push_back(std::log(p_chanceProb));
  const GameOutcome outcome = p_node->GetOutcome();
  for (size_t pl = 0; pl < p_layout.numPlayers; pl++) {
    const double inherited =
        (p_parent < 0)
            ? 0.0
            : p_layout.pathPayoffs[static_cast<size_t>(p_parent) * p_layout.numPlayers + pl];
    p_layout.pathPayoffs.push_back(inherited + outcome->GetPayoff<double>(p_players[pl]));
  }
  p_layout.firstChild.push_back(p_layout.childList.size());

  const GameInfoset infoset = p_node->GetInfoset();
  if (!infoset) {
    p_layout.infoset.push_back(-1);
    p_layout.numChildren.push_back(0);
    p_layout.terminals.push_back(index);
    return;
  }
  const auto actions = infoset->GetActions();
  const int info = (infoset->IsChanceInfoset()) ? -1 : p_infosets.at(infoset);
  p_layout.infoset.push_back(info);
  p_layout.numChildren.push_back(actions.size());
  p_layout.childList.resize(p_layout.childList.size() + actions.size());

  size_t slot = p_layout.firstChild[index];
  size_t playerAction = (info >= 0) ? p_layout.infosets[info].firstAction : 0;
  for (const auto &action : actions) {
    p_layout.childList[slot++] = p_layout.parent.size();
    if (info >= 0) {
      AddNode(p_layout, p_node->GetChild(action), static_cast<int>(index), playerAction++, 1.0,
              p_infosets, p_players, p_index);
    }
    else {
      AddNode(p_layout, p_node->GetChild(action), static_cast<int>(index), 0,
              static_cast<double>(infoset->GetActionProb(action)), p_infosets, p_players, p_index);
    }
  }
}

TreeLayout BuildTreeLayout(const Game &p_game)
{
  TreeLayout layout;
  std::vector<GamePlayer> players;
  std::map<GameInfoset, int> infosetIndex;
  size_t first = 1;
  for (const auto &player : p_game->GetPlayers()) {
    players.push_back(player);
    for (const auto &infoset : player->GetInfosets()) {
      infosetIndex[infoset] = static_cast<int>(layout.infosets.size());
      layout.infosets.push_back({players.size() - 1, first, infoset->GetActions().size(), {}});
      first += infoset->GetActions().size();
    }
  }
  layout.numPlayers = players.size();
  layout.numActions = first - 1;

  std::map<GameNode, size_t> nodeIndex;
  AddNode(layout, p_game->GetRoot(), -1, 0, 1.0, infosetIndex, players, nodeIndex);
  for (const auto &[infoset, index] : infosetIndex) {
    for (const auto &member : infoset->GetMembers()) {
      layout.infosets[index].members.push_back(nodeIndex.at(member));
    }
  }
  return layout;
}

//
// Quantities derived from a point on the path, stored in arrays aligned with a TreeLayout.
//
// Realization probabilities are kept as logarithms, and beliefs are computed relative to
// the most likely member of each information set.  This keeps beliefs accurate at
// information sets whose realization probability is going to zero, which is needed to
// approximate the limiting sequential equilibrium well.
//
struct PathQuantities {
  std::vector<double> prob, logProb;           // by profile position; entry 0 unused
  std::vector<double> logRealizProb;           // by node
  std::vector<double> belief;                  // by node; meaningful at player nodes
  std::vector<double> nodeValues;              // numPlayers per node, including outcomes above
  std::vector<double> actionValues;            // by profile position; entry 0 unused
  std::vector<std::pair<size_t, size_t>> path; // scratch: (child node, action) pairs
};

void ComputePathQuantities(const TreeLayout &p_layout, const Vector<double> &p_point,
                           PathQuantities &p_quantities)
{
  const size_t numNodes = p_layout.parent.size();
  const size_t numPlayers = p_layout.numPlayers;
  auto &q = p_quantities;
  q.prob.resize(p_layout.numActions + 1);
  q.logProb.resize(p_layout.numActions + 1);
  for (size_t a = 1; a <= p_layout.numActions; a++) {
    q.logProb[a] = p_point[a];
    q.prob[a] = std::exp(p_point[a]);
  }

  q.logRealizProb.resize(numNodes);
  q.logRealizProb[0] = 0.0;
  for (size_t k = 1; k < numNodes; k++) {
    const size_t a = p_layout.action[k];
    q.logRealizProb[k] = q.logRealizProb[p_layout.parent[k]] +
                         ((a != 0) ? q.logProb[a] : p_layout.logChanceProb[k]);
  }

  q.belief.assign(numNodes, 0.0);
  for (const auto &infoset : p_layout.infosets) {
    double infosetProb = 0.0;
    for (const size_t m : infoset.members) {
      infosetProb += std::exp(q.logRealizProb[m]);
    }
    if (infosetProb == 0.0) {
      // Possible when a zero-probability chance action makes the information set
      // unreachable; beliefs are then taken to be uniform
      for (const size_t m : infoset.members) {
        q.belief[m] = 1.0 / static_cast<double>(infoset.members.size());
      }
      continue;
    }
    double maxLogProb = q.logRealizProb[infoset.members.front()];
    for (const size_t m : infoset.members) {
      maxLogProb = std::max(maxLogProb, q.logRealizProb[m]);
    }
    double total = 0.0;
    for (const size_t m : infoset.members) {
      total += std::exp(q.logRealizProb[m] - maxLogProb);
    }
    const double mostLikelyBelief = 1.0 / total;
    for (const size_t m : infoset.members) {
      q.belief[m] = mostLikelyBelief * std::exp(q.logRealizProb[m] - maxLogProb);
    }
  }

  q.nodeValues.resize(numNodes * numPlayers);
  q.actionValues.assign(p_layout.numActions + 1, 0.0);
  for (size_t k = numNodes; k-- > 0;) {
    double *value = &q.nodeValues[k * numPlayers];
    if (p_layout.numChildren[k] == 0) {
      std::copy_n(&p_layout.pathPayoffs[k * numPlayers], numPlayers, value);
      continue;
    }
    std::fill_n(value, numPlayers, 0.0);
    for (size_t j = 0; j < p_layout.numChildren[k]; j++) {
      const size_t child = p_layout.childList[p_layout.firstChild[k] + j];
      const size_t a = p_layout.action[child];
      const double childProb = (a != 0) ? q.prob[a] : p_layout.chanceProb[child];
      for (size_t pl = 0; pl < numPlayers; pl++) {
        value[pl] += childProb * q.nodeValues[child * numPlayers + pl];
      }
    }
  }
  // Accumulate action values in preorder, visiting members of each information set in the
  // same sequence as a depth-first traversal
  for (size_t k = 0; k < numNodes; k++) {
    const int info = p_layout.infoset[k];
    if (info < 0) {
      continue;
    }
    const size_t player = p_layout.infosets[info].player;
    for (size_t j = 0; j < p_layout.numChildren[k]; j++) {
      const size_t child = p_layout.childList[p_layout.firstChild[k] + j];
      q.actionValues[p_layout.action[child]] +=
          q.belief[k] * q.nodeValues[child * numPlayers + player];
    }
  }
}

//
// Derivatives of action values with respect to log action probabilities, written into
// p_derivs, a square matrix indexed by profile position.  See Turocy (2001), "Computing the
// Quantal Response Equilibrium Correspondence".  These assume that the profile is interior
// (totally mixed), and that the game is of perfect recall.
//
// The value of action a at information set I of player i is
// V(a) = sum_{h in I} mu(h) v_i(h a), where mu are beliefs and v_i are node values.  Its
// derivative with respect to the log probability of action b has two parts:
//  - for b taken above h, the change in beliefs: mu(h) (v_i(h a) - V(a)) for each member h
//    whose path contains b;
//  - for b taken below h a, the change in v_i(h a): each terminal node z below h a whose
//    path contains b contributes mu(h) Pr(z | h a) u_i(z).
// Entries where a and b are at the same information set are zero.
//
void DiffActionValues(const TreeLayout &p_layout, PathQuantities &p_quantities,
                      Matrix<double> &p_derivs)
{
  const size_t numPlayers = p_layout.numPlayers;
  auto &q = p_quantities;
  auto &path = q.path;
  p_derivs = 0.0;

  for (size_t h = 0; h < p_layout.parent.size(); h++) {
    const int info = p_layout.infoset[h];
    if (info < 0) {
      continue;
    }
    path.clear();
    for (int c = static_cast<int>(h); p_layout.parent[c] >= 0; c = p_layout.parent[c]) {
      if (p_layout.action[c] != 0) {
        path.emplace_back(c, p_layout.action[c]);
      }
    }
    if (path.empty()) {
      continue;
    }
    const size_t player = p_layout.infosets[info].player;
    for (size_t j = 0; j < p_layout.numChildren[h]; j++) {
      const size_t child = p_layout.childList[p_layout.firstChild[h] + j];
      const size_t a = p_layout.action[child];
      const double weight =
          q.belief[h] * (q.nodeValues[child * numPlayers + player] - q.actionValues[a]);
      for (const auto &[node, b] : path) {
        p_derivs(a, b) += weight;
      }
    }
  }

  for (const size_t z : p_layout.terminals) {
    path.clear();
    for (int c = static_cast<int>(z); p_layout.parent[c] >= 0; c = p_layout.parent[c]) {
      if (p_layout.action[c] != 0) {
        path.emplace_back(c, p_layout.action[c]);
      }
    }
    // path runs from the terminal node towards the root; each action is paired with
    // every action taken after it, i.e. earlier in the list
    for (size_t k = 1; k < path.size(); k++) {
      const auto &[child, a] = path[k];
      const size_t h = p_layout.parent[child];
      const size_t player = p_layout.infosets[p_layout.infoset[h]].player;
      const double weight = q.belief[h] * std::exp(q.logRealizProb[z] - q.logRealizProb[child]) *
                            p_layout.pathPayoffs[z * numPlayers + player];
      for (size_t m = 0; m < k; m++) {
        p_derivs(a, path[m].second) += weight;
      }
    }
  }
}

class EquationSystem {
public:
  explicit EquationSystem(const Game &p_game)
    : m_layout(BuildTreeLayout(p_game)), m_derivs(m_layout.numActions, m_layout.numActions)
  {
  }
  ~EquationSystem() = default;

  // Compute the value of the system of equations at the specified point.
  void GetValue(const Vector<double> &p_point, Vector<double> &p_lhs) const;

  // Compute the Jacobian matrix at the specified point.
  void GetJacobian(const Vector<double> &p_point, Matrix<double> &p_matrix) const;

private:
  TreeLayout m_layout;
  mutable PathQuantities m_quantities;
  mutable Matrix<double> m_derivs;
};

// Equations are, for each information set in turn, that its action probabilities sum to
// one, followed by one equation for each action but the first relating its log probability
// to that of the first.
void EquationSystem::GetValue(const Vector<double> &p_point, Vector<double> &p_lhs) const
{
  ComputePathQuantities(m_layout, p_point, m_quantities);
  const auto &q = m_quantities;
  const double lambda = p_point.back();
  size_t row = 1;
  for (const auto &infoset : m_layout.infosets) {
    const size_t ref = infoset.firstAction;
    double sum = -1.0;
    for (size_t a = ref; a < ref + infoset.numActions; a++) {
      sum += q.prob[a];
    }
    p_lhs[row++] = sum;
    for (size_t a = ref + 1; a < ref + infoset.numActions; a++) {
      p_lhs[row++] =
          q.logProb[a] - q.logProb[ref] - lambda * (q.actionValues[a] - q.actionValues[ref]);
    }
  }
}

void EquationSystem::GetJacobian(const Vector<double> &p_point, Matrix<double> &p_matrix) const
{
  // Rows of p_matrix are variables (log probabilities, then lambda); columns are equations,
  // in the same order as in GetValue().
  ComputePathQuantities(m_layout, p_point, m_quantities);
  const size_t numActions = m_layout.numActions;
  DiffActionValues(m_layout, m_quantities, m_derivs);
  const auto &q = m_quantities;
  const double lambda = p_point.back();

  p_matrix = 0.0;
  size_t column = 1;
  for (const auto &infoset : m_layout.infosets) {
    const size_t first = infoset.firstAction;
    const size_t last = first + infoset.numActions - 1;
    for (size_t a = first; a <= last; a++) {
      p_matrix(a, column) = q.prob[a];
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
      p_matrix(numActions + 1, column) = q.actionValues[first] - q.actionValues[a];
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
