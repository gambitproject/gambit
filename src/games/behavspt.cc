//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/libgambit/behavspt.cc
// Implementation of supports for extensive forms
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

#include "gambit.h"

namespace Gambit {

//========================================================================
//                      BehaviorSupportProfile: Lifecycle
//========================================================================

BehaviorSupportProfile::BehaviorSupportProfile(const Game &p_efg) : m_efg(p_efg)
{
  for (const auto &player : p_efg->GetPlayers()) {
    for (const auto &infoset : player->GetInfosets()) {
      for (const auto &action : infoset->GetActions()) {
        m_actions[infoset].push_back(action);
      }
    }
  }

  // Initialize the list of reachable information sets and nodes
  for (int pl = 0; pl <= GetGame()->NumPlayers(); pl++) {
    const GamePlayer player = (pl == 0) ? GetGame()->GetChance() : GetGame()->GetPlayer(pl);
    for (const auto &infoset : player->GetInfosets()) {
      m_infosetReachable[infoset] = true;
      for (const auto &member : infoset->GetMembers()) {
        m_nonterminalReachable[member] = true;
      }
    }
  }
}

//========================================================================
//                 BehaviorSupportProfile: General information
//========================================================================

size_t BehaviorSupportProfile::BehaviorProfileLength() const
{
  size_t answer = 0;
  for (const auto &[infoset, actions] : m_actions) {
    answer += actions.size();
  }
  return answer;
}

void BehaviorSupportProfile::AddAction(const GameAction &p_action)
{
  auto &support = m_actions.at(p_action->GetInfoset());
  auto pos = std::find_if(support.begin(), support.end(), [p_action](const GameAction &a) {
    return a->GetNumber() >= p_action->GetNumber();
  });
  if (pos == support.end() || *pos != p_action) {
    // Action is not in the support at the infoset; add at this location to keep sorted by number
    support.insert(pos, p_action);
    for (const auto &node : GetMembers(p_action->GetInfoset())) {
      ActivateSubtree(node->GetChild(p_action));
    }
  }
}

bool BehaviorSupportProfile::RemoveAction(const GameAction &p_action)
{
  auto &support = m_actions.at(p_action->GetInfoset());
  auto pos = std::find(support.begin(), support.end(), p_action);
  if (pos != support.end()) {
    support.erase(pos);
    for (const auto &node : GetMembers(p_action->GetInfoset())) {
      DeactivateSubtree(node->GetChild(p_action));
    }
    return !support.empty();
  }
  return false;
}

std::list<GameInfoset> BehaviorSupportProfile::GetInfosets(const GamePlayer &p_player) const
{
  std::list<GameInfoset> answer;
  for (const auto &infoset : p_player->GetInfosets()) {
    if (m_infosetReachable.at(infoset)) {
      answer.push_back(infoset);
    }
  }
  return answer;
}

namespace {

void ReachableInfosets(const BehaviorSupportProfile &p_support, const GameNode &p_node,
                       std::set<GameInfoset> &p_reached)
{
  if (p_node->IsTerminal()) {
    return;
  }

  GameInfoset infoset = p_node->GetInfoset();
  if (!infoset->GetPlayer()->IsChance()) {
    p_reached.insert(infoset);
    for (const auto &action : p_support.GetActions(infoset)) {
      ReachableInfosets(p_support, p_node->GetChild(action), p_reached);
    }
  }
  else {
    for (const auto &child : p_node->GetChildren()) {
      ReachableInfosets(p_support, child, p_reached);
    }
  }
}

} // end anonymous namespace

bool BehaviorSupportProfile::Dominates(const GameAction &a, const GameAction &b,
                                       bool p_strict) const
{
  GameInfoset infoset = a->GetInfoset();
  if (infoset != b->GetInfoset()) {
    throw UndefinedException();
  }

  GamePlayer player = infoset->GetPlayer();
  int thesign = 0;

  auto nodelist = GetMembers(infoset);
  for (const auto &node : GetMembers(infoset)) {
    std::set<GameInfoset> reachable;
    ReachableInfosets(*this, node->GetChild(a), reachable);
    ReachableInfosets(*this, node->GetChild(b), reachable);

    auto contingencies = BehaviorContingencies(*this, reachable);
    if (p_strict) {
      if (!std::all_of(contingencies.begin(), contingencies.end(),
                       [&](const PureBehaviorProfile &profile) {
                         return profile.GetPayoff<Rational>(node->GetChild(a), player) >
                                profile.GetPayoff<Rational>(node->GetChild(b), player);
                       })) {
        return false;
      }
    }
    else {
      for (const auto &iter : contingencies) {
        auto newsign = sign(iter.GetPayoff<Rational>(node->GetChild(a), player) -
                            iter.GetPayoff<Rational>(node->GetChild(b), player));
        if (newsign < 0) {
          return false;
        }
        thesign = std::max(thesign, newsign);
      }
    }
  }
  return p_strict || thesign > 0;
}

bool BehaviorSupportProfile::IsDominated(const GameAction &p_action, const bool p_strict) const
{
  const auto &actions = GetActions(p_action->GetInfoset());
  return std::any_of(actions.begin(), actions.end(), [&](const GameAction &a) {
    return a != p_action && Dominates(a, p_action, p_strict);
  });
}

BehaviorSupportProfile BehaviorSupportProfile::Undominated(const bool p_strict) const
{
  BehaviorSupportProfile result(*this);
  for (const auto &player : m_efg->GetPlayers()) {
    for (const auto &infoset : player->GetInfosets()) {
      const auto &actions = GetActions(infoset);
      std::set<GameAction> dominated;
      for (const auto &action1 : actions) {
        if (contains(dominated, action1)) {
          continue;
        }
        for (const auto &action2 : actions) {
          if (action1 == action2 || contains(dominated, action2)) {
            continue;
          }
          if (Dominates(action1, action2, p_strict)) {
            dominated.insert(action2);
            result.RemoveAction(action2);
          }
        }
      }
    }
  }
  return result;
}

bool BehaviorSupportProfile::HasReachableMembers(const GameInfoset &p_infoset) const
{
  const auto &members = p_infoset->GetMembers();
  return std::any_of(members.begin(), members.end(),
                     [&](const GameNode &n) { return m_nonterminalReachable.at(n); });
}

void BehaviorSupportProfile::ActivateSubtree(const GameNode &n)
{
  if (!n->IsTerminal()) {
    m_nonterminalReachable[n] = true;
    m_infosetReachable[n->GetInfoset()] = true;
    if (n->GetInfoset()->GetPlayer()->IsChance()) {
      for (const auto &child : n->GetChildren()) {
        ActivateSubtree(child);
      }
    }
    else {
      for (const auto &action : GetActions(n->GetInfoset())) {
        ActivateSubtree(n->GetChild(action));
      }
    }
  }
}

void BehaviorSupportProfile::DeactivateSubtree(const GameNode &n)
{
  if (!n->IsTerminal()) { // THIS ALL LOOKS FISHY
    m_nonterminalReachable[n] = false;
    if (!HasReachableMembers(n->GetInfoset())) {
      m_infosetReachable[n->GetInfoset()] = false;
    }
    if (!n->GetPlayer()->IsChance()) {
      for (const auto &action : GetActions(n->GetInfoset())) {
        DeactivateSubtree(n->GetChild(action));
      }
    }
    else {
      for (const auto &action : n->GetInfoset()->GetActions()) {
        DeactivateSubtree(n->GetChild(action));
      }
    }
  }
}

std::list<GameNode> BehaviorSupportProfile::GetMembers(const GameInfoset &p_infoset) const
{
  std::list<GameNode> answer;
  for (const auto &member : p_infoset->GetMembers()) {
    if (m_nonterminalReachable.at(member)) {
      answer.push_back(member);
    }
  }
  return answer;
}

} // end namespace Gambit
