//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/logit/logbehav.cc
// Behavior strategy profile where action probabilities are represented using
// logarithms.
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

#include "games.h"
#include "games/gametree.h"
#include "logbehav.h"

//========================================================================
//                  LogBehavProfile<T>: Lifecycle
//========================================================================

template <class T>
LogBehavProfile<T>::LogBehavProfile(const Game &p_game)
  : m_game(p_game), m_probs(m_game->BehavProfileLength()), m_logProbs(m_game->BehavProfileLength())
{
  int index = 1;
  for (const auto &player : p_game->GetPlayers()) {
    for (const auto &infoset : player->GetInfosets()) {
      for (const auto &action : infoset->GetActions()) {
        m_profileIndex[action] = index++;
      }
    }
  }

  for (const auto &infoset : m_game->GetInfosets()) {
    T center = T(1) / T(infoset->GetActions().size());
    for (const auto &act : infoset->GetActions()) {
      SetProb(act, center);
    }
  }
}

//========================================================================
//               LogBehavProfile<T>: Operator overloading
//========================================================================

template <class T> bool LogBehavProfile<T>::operator==(const LogBehavProfile<T> &p_profile) const
{
  return (m_game == p_profile.m_game && m_probs == p_profile.m_probs);
}

//========================================================================
//              LogBehavProfile<T>: Interesting quantities
//========================================================================

template <class T> T LogBehavProfile<T>::GetActionProb(const GameAction &action) const
{
  if (action->GetInfoset()->GetPlayer()->IsChance()) {
    return static_cast<T>(action->GetInfoset()->GetActionProb(action));
  }
  return GetProb(action);
}

template <class T> T LogBehavProfile<T>::GetLogActionProb(const GameAction &action) const
{
  if (action->GetInfoset()->GetPlayer()->IsChance()) {
    return log(static_cast<T>(action->GetInfoset()->GetActionProb(action)));
  }
  return m_logProbs[m_profileIndex.at(action)];
}

template <class T> const T &LogBehavProfile<T>::GetPayoff(const GameAction &act) const
{
  ComputeSolutionData();
  return m_actionValues[act];
}

//
// Derivatives of action values with respect to log action probabilities.
// See Turocy (2001), "Computing the Quantal Response Equilibrium
// Correspondence" for details.  These assume that the profile is interior
// (totally mixed), and that the game is of perfect recall.
//
// With x the log probabilities, the value of action a at information set I of
// player i is V(a) = sum_{h in I} mu(h) v_i(h a), where mu are beliefs and v_i are
// node values.  Its derivative with respect to x_b has two parts:
//  - for b taken below h a, the change in v_i(h a): each terminal node z below h a
//    whose path contains b contributes mu(h) Pr(z | h a) u_i(z);
//  - for b taken above h, the change in beliefs: mu(h) v_i(h a) for each member h
//    whose path contains b, less V(a) times the total belief on those members.
// Both are accumulated in a single pass over the tree.
//
template <class T>
void LogBehavProfile<T>::DiffActionValuesPass(
    const GameNode &p_node, std::vector<PathAction> &p_path, Matrix<T> &p_derivs,
    std::map<GameInfoset, std::map<int, T>> &p_shares) const
{
  if (p_node->IsTerminal()) {
    const T logRealizProb = m_logRealizProbs[p_node];
    for (size_t k = 0; k < p_path.size(); k++) {
      const T weight = p_path[k].belief * exp(logRealizProb - p_path[k].logRealizProb) *
                       m_nodeValues[p_node][p_path[k].player];
      for (size_t m = k + 1; m < p_path.size(); m++) {
        p_derivs(p_path[k].index, p_path[m].index) += weight;
      }
    }
    return;
  }

  const GameInfoset infoset = p_node->GetInfoset();
  if (infoset->IsChanceInfoset()) {
    for (const auto &action : infoset->GetActions()) {
      DiffActionValuesPass(p_node->GetChild(action), p_path, p_derivs, p_shares);
    }
    return;
  }

  const GamePlayer player = infoset->GetPlayer();
  const T belief = m_beliefs[p_node];
  auto &shares = p_shares[infoset];
  for (const auto &taken : p_path) {
    shares[taken.index] += belief;
  }
  for (const auto &action : infoset->GetActions()) {
    const GameNode child = p_node->GetChild(action);
    const int index = m_profileIndex.at(action);
    const T childValue = m_nodeValues[child][player];
    for (const auto &taken : p_path) {
      p_derivs(index, taken.index) += belief * childValue;
    }
    p_path.push_back({index, player, belief, m_logRealizProbs[child]});
    DiffActionValuesPass(child, p_path, p_derivs, p_shares);
    p_path.pop_back();
  }
}

template <class T> void LogBehavProfile<T>::DiffActionValues(Matrix<T> &p_derivs) const
{
  ComputeSolutionData();
  p_derivs = T(0);
  std::vector<PathAction> path;
  std::map<GameInfoset, std::map<int, T>> shares;
  DiffActionValuesPass(m_game->GetRoot(), path, p_derivs, shares);
  for (const auto &[infoset, infosetShares] : shares) {
    for (const auto &action : infoset->GetActions()) {
      const int index = m_profileIndex.at(action);
      const T value = m_actionValues[action];
      for (const auto &[taken, share] : infosetShares) {
        p_derivs(index, taken) -= share * value;
      }
    }
  }
}

//========================================================================
//             LogBehavProfile<T>: Cached profile information
//========================================================================

template <class T> void LogBehavProfile<T>::ComputeSolutionDataPass2(const GameNode &node) const
{
  const GameOutcome outcome = node->GetOutcome();
  for (auto player : m_game->GetPlayers()) {
    m_nodeValues[node][player] += outcome->GetPayoff<T>(player);
  }

  const GameInfoset infoset = node->GetInfoset();
  if (!infoset) {
    return;
  }

  // push down payoffs from outcomes attached to non-terminal nodes
  for (auto child : node->GetChildren()) {
    m_nodeValues[child] = m_nodeValues[node];
  }

  for (auto player : m_game->GetPlayers()) {
    m_nodeValues[node][player] = T(0);
  }

  for (auto child : node->GetChildren()) {
    ComputeSolutionDataPass2(child);
    const GameAction action = child->GetPriorAction();
    for (auto player : m_game->GetPlayers()) {
      m_nodeValues[node][player] += GetActionProb(action) * m_nodeValues[child][player];
    }
    if (!infoset->IsChanceInfoset()) {
      m_actionValues[action] += m_beliefs[node] * m_nodeValues[child][infoset->GetPlayer()];
    }
  }
}

template <class T> void LogBehavProfile<T>::ComputeSolutionDataPass1(const GameNode &node) const
{
  m_logRealizProbs[node] = (node->GetParent()) ? m_logRealizProbs[node->GetParent()] +
                                                     GetLogActionProb(node->GetPriorAction())
                                               : T(0);
  for (auto child : node->GetChildren()) {
    ComputeSolutionDataPass1(child);
  }
}

template <class T> void LogBehavProfile<T>::ComputeSolutionData() const
{
  if (m_cacheValid) {
    return;
  }
  m_actionValues.clear();
  m_beliefs.clear();
  m_nodeValues.clear();
  ComputeSolutionDataPass1(m_game->GetRoot());

  for (auto player : m_game->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      // The log-profile assumes that the mixed behavior profile has full support.
      // However, if a game has zero-probability chance actions, then it is possible
      // for an information set not to be reached.  In this event, we set the beliefs
      // at those information sets to be uniform across the nodes.
      T infosetProb = T(0);
      for (auto member : infoset->GetMembers()) {
        infosetProb += exp(m_logRealizProbs[member]);
      }
      if (infosetProb == T(0)) {
        for (auto member : infoset->GetMembers()) {
          m_beliefs[member] = 1.0 / T(infoset->GetMembers().size());
        }
        continue;
      }

      T maxLogProb = m_logRealizProbs[infoset->GetMember(1)];
      for (auto member : infoset->GetMembers()) {
        if (m_logRealizProbs[member] > maxLogProb) {
          maxLogProb = m_logRealizProbs[member];
        }
      }

      T total = 0.0;
      for (auto member : infoset->GetMembers()) {
        total += exp(m_logRealizProbs[member] - maxLogProb);
      }

      // The belief for the most likely node
      T mostLikelyBelief = 1.0 / total;
      for (auto member : infoset->GetMembers()) {
        m_beliefs[member] = mostLikelyBelief * exp(m_logRealizProbs[member] - maxLogProb);
      }
    }
  }

  ComputeSolutionDataPass2(m_game->GetRoot());
  m_cacheValid = true;
}

template class LogBehavProfile<double>;
