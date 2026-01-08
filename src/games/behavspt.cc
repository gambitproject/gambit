//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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
#include "gameseq.h"

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
  for (const auto &player : p_efg->GetPlayersWithChance()) {
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
  m_reachableInfosets = nullptr;
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
  m_reachableInfosets = nullptr;
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

//========================================================================
//                 BehaviorSupportProfile: Sequence form
//========================================================================

std::shared_ptr<GameSequenceForm> BehaviorSupportProfile::GetSequenceForm() const
{
  if (!m_sequenceForm) {
    m_sequenceForm = std::make_shared<GameSequenceForm>(*this);
  }
  return m_sequenceForm;
}

BehaviorSupportProfile::Sequences BehaviorSupportProfile::GetSequences() const { return {this}; }

BehaviorSupportProfile::PlayerSequences
BehaviorSupportProfile::GetSequences(GamePlayer &p_player) const
{
  return {this, p_player};
}

int BehaviorSupportProfile::GetConstraintEntry(const GameInfoset &p_infoset,
                                               const GameAction &p_action) const
{
  return GetSequenceForm()->GetConstraintEntry(p_infoset, p_action);
}

const Rational &
BehaviorSupportProfile::GetPayoff(const std::map<GamePlayer, GameSequence> &p_profile,
                                  const GamePlayer &p_player) const
{
  return GetSequenceForm()->GetPayoff(p_profile, p_player);
}

BehaviorSupportProfile::SequenceContingencies
BehaviorSupportProfile::GetSequenceContingencies() const
{
  return {this};
}

MixedBehaviorProfile<double>
BehaviorSupportProfile::ToMixedBehaviorProfile(const std::map<GameSequence, double> &x) const
{
  MixedBehaviorProfile<double> b(*this);
  for (auto sequence : GetSequences()) {
    if (sequence->action == nullptr) {
      continue;
    }
    const double parent_prob = x.at(sequence->parent.lock());
    if (parent_prob > 0) {
      b[sequence->action] = x.at(sequence) / parent_prob;
    }
    else {
      b[sequence->action] = 0;
    }
  }
  return b;
}

size_t BehaviorSupportProfile::Sequences::size() const
{
  return std::accumulate(m_support->GetSequenceForm()->m_sequences.cbegin(),
                         m_support->GetSequenceForm()->m_sequences.cend(), 0,
                         [](int acc, const std::pair<GamePlayer, std::vector<GameSequence>> &seq) {
                           return acc + seq.second.size();
                         });
}

BehaviorSupportProfile::Sequences::iterator BehaviorSupportProfile::Sequences::begin() const
{
  return {m_support->GetSequenceForm(), false};
}
BehaviorSupportProfile::Sequences::iterator BehaviorSupportProfile::Sequences::end() const
{
  return {m_support->GetSequenceForm(), true};
}

BehaviorSupportProfile::Sequences::iterator::iterator(
    const std::shared_ptr<GameSequenceForm> p_sfg, bool p_end)
  : m_sfg(p_sfg)
{
  if (p_end) {
    m_currentPlayer = m_sfg->m_sequences.cend();
  }
  else {
    m_currentPlayer = m_sfg->m_sequences.cbegin();
    m_currentSequence = m_currentPlayer->second.cbegin();
  }
}

BehaviorSupportProfile::Sequences::iterator &
BehaviorSupportProfile::Sequences::iterator::operator++()
{
  if (m_currentPlayer == m_sfg->m_sequences.cend()) {
    return *this;
  }
  m_currentSequence++;
  if (m_currentSequence != m_currentPlayer->second.cend()) {
    return *this;
  }
  m_currentPlayer++;
  if (m_currentPlayer != m_sfg->m_sequences.cend()) {
    m_currentSequence = m_currentPlayer->second.cbegin();
  }
  return *this;
}

bool BehaviorSupportProfile::Sequences::iterator::operator==(const iterator &it) const
{
  if (m_sfg != it.m_sfg || m_currentPlayer != it.m_currentPlayer) {
    return false;
  }
  if (m_currentPlayer == m_sfg->m_sequences.end()) {
    return true;
  }
  return (m_currentSequence == it.m_currentSequence);
}

std::vector<GameSequence>::const_iterator BehaviorSupportProfile::PlayerSequences::begin() const
{
  return m_support->GetSequenceForm()->m_sequences.at(m_player).begin();
}

std::vector<GameSequence>::const_iterator BehaviorSupportProfile::PlayerSequences::end() const
{
  return m_support->GetSequenceForm()->m_sequences.at(m_player).end();
}

size_t BehaviorSupportProfile::PlayerSequences::size() const
{
  return m_support->GetSequenceForm()->m_sequences.at(m_player).size();
}

BehaviorSupportProfile::SequenceContingencies::iterator::iterator(
    const std::shared_ptr<GameSequenceForm> p_sfg, bool p_end)
  : m_sfg(p_sfg), m_end(p_end)
{
  for (auto [player, sequences] : m_sfg->m_sequences) {
    m_indices[player] = 0;
  }
}

std::map<GamePlayer, GameSequence>
BehaviorSupportProfile::SequenceContingencies::iterator::operator*() const
{
  std::map<GamePlayer, GameSequence> ret;
  for (auto [player, index] : m_indices) {
    ret[player] = m_sfg->m_sequences.at(player)[index];
  }
  return ret;
}

std::map<GamePlayer, GameSequence>
BehaviorSupportProfile::SequenceContingencies::iterator::operator->() const
{
  std::map<GamePlayer, GameSequence> ret;
  for (auto [player, index] : m_indices) {
    ret[player] = m_sfg->m_sequences.at(player)[index];
  }
  return ret;
}

BehaviorSupportProfile::SequenceContingencies::iterator &
BehaviorSupportProfile::SequenceContingencies::iterator::operator++()
{
  for (auto [player, index] : m_indices) {
    if (index < m_sfg->m_sequences.at(player).size() - 1) {
      m_indices[player]++;
      return *this;
    }
    m_indices[player] = 0;
  }
  m_end = true;
  return *this;
}

//========================================================================
//                 BehaviorSupportProfile: Reachable Information Sets
//========================================================================

void BehaviorSupportProfile::FindReachableInfosets(GameNode p_node) const
{
  if (!p_node->IsTerminal()) {
    auto infoset = p_node->GetInfoset();
    (*m_reachableInfosets)[infoset] = true;
    if (p_node->GetPlayer()->IsChance()) {
      for (auto action : infoset->GetActions()) {
        FindReachableInfosets(p_node->GetChild(action));
      }
    }
    else {
      for (auto action : GetActions(infoset)) {
        FindReachableInfosets(p_node->GetChild(action));
      }
    }
  }
}

std::shared_ptr<std::map<GameInfoset, bool>> BehaviorSupportProfile::GetReachableInfosets() const
{
  if (!m_reachableInfosets) {
    m_reachableInfosets = std::make_shared<std::map<GameInfoset, bool>>();
    for (size_t pl = 0; pl <= GetGame()->NumPlayers(); pl++) {
      const GamePlayer player = (pl == 0) ? GetGame()->GetChance() : GetGame()->GetPlayer(pl);
      for (const auto &infoset : player->GetInfosets()) {
        (*m_reachableInfosets)[infoset] = false;
      }
    }
    FindReachableInfosets(GetGame()->GetRoot());
  }
  return m_reachableInfosets;
}

} // end namespace Gambit
