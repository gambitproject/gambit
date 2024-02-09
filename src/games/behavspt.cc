//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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
  for (int pl = 1; pl <= p_efg->NumPlayers(); pl++) {
    m_actions.push_back(Array<Array<GameAction>>());
    GamePlayer player = p_efg->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      GameInfoset infoset = player->GetInfoset(iset);
      m_actions[pl].push_back(Array<GameAction>());
      for (int act = 1; act <= infoset->NumActions(); act++) {
        m_actions[pl][iset].push_back(infoset->GetAction(act));
      }
    }
  }

  // Initialize the list of reachable information sets and nodes
  for (int pl = 0; pl <= GetGame()->NumPlayers(); pl++) {
    GamePlayer player = (pl == 0) ? GetGame()->GetChance() : GetGame()->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      m_infosetReachable[player->GetInfoset(iset)] = false;
      for (int n = 1; n <= player->GetInfoset(iset)->NumMembers(); n++) {
        m_nonterminalReachable[player->GetInfoset(iset)->GetMember(n)] = false;
      }
    }
  }
  ActivateSubtree(GetGame()->GetRoot());
}

//========================================================================
//                 BehaviorSupportProfile: Operator overloading
//========================================================================

bool BehaviorSupportProfile::operator==(const BehaviorSupportProfile &p_support) const
{
  return (m_actions == p_support.m_actions);
}

//========================================================================
//                 BehaviorSupportProfile: General information
//========================================================================

PVector<int> BehaviorSupportProfile::NumActions() const
{
  PVector<int> answer(m_efg->NumInfosets());
  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++) {
    for (int iset = 1; iset <= m_efg->GetPlayer(pl)->NumInfosets(); iset++) {
      answer(pl, iset) = NumActions(pl, iset);
    }
  }

  return answer;
}

size_t BehaviorSupportProfile::BehaviorProfileLength() const
{
  size_t answer = 0;
  for (const auto &player : m_actions) {
    for (const auto &infoset : player) {
      answer += infoset.size();
    }
  }
  return answer;
}

int BehaviorSupportProfile::GetIndex(const GameAction &a) const
{
  if (a->GetInfoset()->GetGame() != m_efg) {
    throw MismatchException();
  }

  int pl = a->GetInfoset()->GetPlayer()->GetNumber();
  if (pl == 0) {
    // chance action; all chance actions are always in the support
    return a->GetNumber();
  }
  else {
    return m_actions[pl][a->GetInfoset()->GetNumber()].Find(a);
  }
}

bool BehaviorSupportProfile::RemoveAction(const GameAction &s)
{
  for (const auto &node : GetMembers(s->GetInfoset())) {
    DeactivateSubtree(node->GetChild(s));
  }

  GameInfoset infoset = s->GetInfoset();
  GamePlayer player = infoset->GetPlayer();
  Array<GameAction> &actions = m_actions[player->GetNumber()][infoset->GetNumber()];

  if (!actions.Contains(s)) {
    return false;
  }
  else if (actions.Length() == 1) {
    actions.Remove(actions.Find(s));
    return false;
  }
  else {
    actions.Remove(actions.Find(s));
    return true;
  }
}

bool BehaviorSupportProfile::RemoveAction(const GameAction &s, List<GameInfoset> &list)
{
  for (const auto &node : GetMembers(s->GetInfoset())) {
    DeactivateSubtree(node->GetChild(s->GetNumber()), list);
  }
  // the following returns false if s was not in the support
  return RemoveAction(s);
}

void BehaviorSupportProfile::AddAction(const GameAction &s)
{
  GameInfoset infoset = s->GetInfoset();
  GamePlayer player = infoset->GetPlayer();
  Array<GameAction> &actions = m_actions[player->GetNumber()][infoset->GetNumber()];

  int act = 1;
  while (act <= actions.Length()) {
    if (actions[act] == s) {
      break;
    }
    else if (actions[act]->GetNumber() > s->GetNumber()) {
      actions.Insert(s, act);
      break;
    }
    act++;
  }

  if (act > actions.Length()) {
    actions.push_back(s);
  }

  for (const auto &node : GetMembers(s->GetInfoset())) {
    DeactivateSubtree(node);
  }
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
/// Sets p_reachable(pl,iset) to 1 if infoset (pl,iset) reachable after p_node
void ReachableInfosets(const BehaviorSupportProfile &p_support, const GameNode &p_node,
                       PVector<int> &p_reached)
{
  if (p_node->IsTerminal()) {
    return;
  }

  GameInfoset infoset = p_node->GetInfoset();
  if (!infoset->GetPlayer()->IsChance()) {
    p_reached(infoset->GetPlayer()->GetNumber(), infoset->GetNumber()) = 1;
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

// This class iterates
// over contingencies that are relevant once a particular node
// has been reached.
class BehavConditionalIterator {
private:
  bool m_atEnd{false};
  BehaviorSupportProfile m_support;
  PVector<int> m_currentBehav;
  PureBehaviorProfile m_profile;
  PVector<int> m_isActive;
  Array<int> m_numActiveInfosets;

  /// Reset the iterator to the first contingency (this is called by ctors)
  void First();

public:
  /// @name Lifecycle
  //@{
  BehavConditionalIterator(const BehaviorSupportProfile &, const PVector<int> &);
  //@}

  /// @name Iteration and data access
  //@{
  /// Advance to the next contingency (prefix version)
  void operator++();
  /// Advance to the next contingency (postfix version)
  void operator++(int) { ++(*this); }
  /// Has iterator gone past the end?
  bool AtEnd() const { return m_atEnd; }
  /// Get the current behavior profile
  const PureBehaviorProfile *operator->() const { return &m_profile; }
  //@}
};

BehavConditionalIterator::BehavConditionalIterator(const BehaviorSupportProfile &p_support,
                                                   const PVector<int> &p_active)
  : m_support(p_support), m_currentBehav(m_support.GetGame()->NumInfosets()),
    m_profile(m_support.GetGame()), m_isActive(p_active),
    m_numActiveInfosets(m_support.GetGame()->NumPlayers())
{
  for (int pl = 1; pl <= m_support.GetGame()->NumPlayers(); pl++) {
    m_numActiveInfosets[pl] = 0;
    GamePlayer player = m_support.GetGame()->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      if (m_isActive(pl, iset)) {
        m_numActiveInfosets[pl]++;
      }
    }
  }
  First();
}

void BehavConditionalIterator::First()
{
  for (int pl = 1; pl <= m_support.GetGame()->NumPlayers(); pl++) {
    for (int iset = 1; iset <= m_support.GetGame()->GetPlayer(pl)->NumInfosets(); iset++) {
      m_currentBehav(pl, iset) = 1;
      if (m_isActive(pl, iset)) {
        m_profile.SetAction(m_support.GetAction(pl, iset, 1));
      }
    }
  }
}

void BehavConditionalIterator::operator++()
{
  int pl = m_support.GetGame()->NumPlayers();
  while (pl > 0 && m_numActiveInfosets[pl] == 0) {
    --pl;
  }
  if (pl == 0) {
    m_atEnd = true;
    return;
  }

  int iset = m_support.GetGame()->GetPlayer(pl)->NumInfosets();

  while (true) {
    if (m_isActive(pl, iset)) {
      if (m_currentBehav(pl, iset) < m_support.NumActions(pl, iset)) {
        m_profile.SetAction(m_support.GetAction(pl, iset, ++m_currentBehav(pl, iset)));
        return;
      }
      else {
        m_currentBehav(pl, iset) = 1;
        m_profile.SetAction(m_support.GetAction(pl, iset, 1));
      }
    }

    iset--;
    if (iset == 0) {
      do {
        --pl;
      } while (pl > 0 && m_numActiveInfosets[pl] == 0);

      if (pl == 0) {
        m_atEnd = true;
        return;
      }
      iset = m_support.GetGame()->GetPlayer(pl)->NumInfosets();
    }
  }
}

bool BehaviorSupportProfile::Dominates(const GameAction &a, const GameAction &b, bool p_strict,
                                       bool p_conditional) const
{
  GameInfoset infoset = a->GetInfoset();
  if (infoset != b->GetInfoset()) {
    throw UndefinedException();
  }

  GamePlayer player = infoset->GetPlayer();
  int pl = player->GetNumber();
  bool equal = true;

  if (!p_conditional) {
    for (BehaviorProfileIterator iter(*this, a); !iter.AtEnd(); iter++) {
      auto ap = (*iter).GetPayoff<Rational>(a);
      auto bp = (*iter).GetPayoff<Rational>(b);

      if (p_strict) {
        if (ap <= bp) {
          return false;
        }
      }
      else {
        if (ap < bp) {
          return false;
        }
        else if (ap > bp) {
          equal = false;
        }
      }
    }
  }

  else {
    auto nodelist = GetMembers(infoset);
    if (nodelist.empty()) {
      // This may not be a good idea; I suggest checking for this
      // prior to entry
      for (const auto &member : infoset->GetMembers()) {
        nodelist.push_back(member);
      }
    }

    for (const auto &node : nodelist) {
      PVector<int> reachable(GetGame()->NumInfosets());
      reachable = 0;
      ReachableInfosets(*this, node->GetChild(a), reachable);
      ReachableInfosets(*this, node->GetChild(b), reachable);

      for (BehavConditionalIterator iter(*this, reachable); !iter.AtEnd(); iter++) {
        auto ap = iter->GetPayoff<Rational>(node->GetChild(a), pl);
        auto bp = iter->GetPayoff<Rational>(node->GetChild(b), pl);

        if (p_strict) {
          if (ap <= bp) {
            return false;
          }
        }
        else {
          if (ap < bp) {
            return false;
          }
          else if (ap > bp) {
            equal = false;
          }
        }
      }
    }
  }

  if (p_strict) {
    return true;
  }
  else {
    return !equal;
  }
}

bool SomeElementDominates(const BehaviorSupportProfile &S, const Array<GameAction> &array,
                          const GameAction &a, const bool strong, const bool conditional)
{
  for (int i = 1; i <= array.Length(); i++) {
    if (array[i] != a) {
      if (S.Dominates(array[i], a, strong, conditional)) {
        return true;
      }
    }
  }
  return false;
}

bool BehaviorSupportProfile::IsDominated(const GameAction &a, bool strong, bool conditional) const
{
  int pl = a->GetInfoset()->GetPlayer()->GetNumber();
  int iset = a->GetInfoset()->GetNumber();
  Array<GameAction> array(m_actions[pl][iset]);
  return SomeElementDominates(*this, array, a, strong, conditional);
}

bool InfosetHasDominatedElement(const BehaviorSupportProfile &S, const GameInfoset &p_infoset,
                                bool strong, bool conditional)
{
  int pl = p_infoset->GetPlayer()->GetNumber();
  int iset = p_infoset->GetNumber();
  Array<GameAction> actions;
  for (int act = 1; act <= S.NumActions(pl, iset); act++) {
    actions.push_back(S.GetAction(pl, iset, act));
  }
  for (int i = 1; i <= actions.Length(); i++) {
    if (SomeElementDominates(S, actions, actions[i], strong, conditional)) {
      return true;
    }
  }

  return false;
}

bool ElimDominatedInInfoset(const BehaviorSupportProfile &S, BehaviorSupportProfile &T, int pl,
                            int iset, bool strong, bool conditional)
{
  Array<GameAction> actions;
  for (int act = 1; act <= S.NumActions(pl, iset); act++) {
    actions.push_back(S.GetAction(pl, iset, act));
  }
  Array<bool> is_dominated(actions.Length());
  for (int k = 1; k <= actions.Length(); k++) {
    is_dominated[k] = false;
  }

  for (int i = 1; i <= actions.Length(); i++) {
    for (int j = 1; j <= actions.Length(); j++) {
      if (i != j && !is_dominated[i] && !is_dominated[j]) {
        if (S.Dominates(actions[i], actions[j], strong, conditional)) {
          is_dominated[j] = true;
        }
      }
    }
  }

  bool action_was_eliminated = false;
  int k = 1;
  while (k <= actions.Length() && !action_was_eliminated) {
    if (is_dominated[k]) {
      action_was_eliminated = true;
    }
    else {
      k++;
    }
  }
  while (k <= actions.Length()) {
    if (is_dominated[k]) {
      T.RemoveAction(actions[k]);
    }
    k++;
  }

  return action_was_eliminated;
}

bool ElimDominatedForPlayer(const BehaviorSupportProfile &S, BehaviorSupportProfile &T,
                            const int pl, int &cumiset, const bool strong, const bool conditional)
{
  bool action_was_eliminated = false;

  for (int iset = 1; iset <= S.GetGame()->GetPlayer(pl)->NumInfosets(); iset++, cumiset++) {
    if (ElimDominatedInInfoset(S, T, pl, iset, strong, conditional)) {
      action_was_eliminated = true;
    }
  }

  return action_was_eliminated;
}

BehaviorSupportProfile BehaviorSupportProfile::Undominated(bool strong, bool conditional,
                                                           const Array<int> &players,
                                                           std::ostream &) const
{
  BehaviorSupportProfile T(*this);
  int cumiset = 0;

  for (int i = 1; i <= players.Length(); i++) {
    int pl = players[i];
    ElimDominatedForPlayer(*this, T, pl, cumiset, strong, conditional);
  }

  return T;
}

// Utilities
bool BehaviorSupportProfile::HasActiveMembers(int pl, int iset) const
{
  for (auto member : m_efg->GetPlayer(pl)->GetInfoset(iset)->GetMembers()) {
    if (m_nonterminalReachable.at(member)) {
      return true;
    }
  }
  return false;
}

void BehaviorSupportProfile::ActivateSubtree(const GameNode &n)
{
  if (!n->IsTerminal()) {
    m_nonterminalReachable[n] = true;
    m_infosetReachable[n->GetInfoset()] = true;
    if (n->GetInfoset()->GetPlayer()->IsChance()) {
      for (int i = 1; i <= n->NumChildren(); i++) {
        ActivateSubtree(n->GetChild(i));
      }
    }
    else {
      const Array<GameAction> &actions(
          m_actions[n->GetInfoset()->GetPlayer()->GetNumber()][n->GetInfoset()->GetNumber()]);
      for (int i = 1; i <= actions.Length(); i++) {
        ActivateSubtree(n->GetChild(actions[i]->GetNumber()));
      }
    }
  }
}

void BehaviorSupportProfile::DeactivateSubtree(const GameNode &n)
{
  if (!n->IsTerminal()) { // THIS ALL LOOKS FISHY
    m_nonterminalReachable[n] = false;
    if (!HasActiveMembers(n->GetInfoset()->GetPlayer()->GetNumber(),
                          n->GetInfoset()->GetNumber())) {
      m_infosetReachable[n->GetInfoset()] = false;
    }
    if (!n->GetPlayer()->IsChance()) {
      Array<GameAction> actions(
          m_actions[n->GetInfoset()->GetPlayer()->GetNumber()][n->GetInfoset()->GetNumber()]);
      for (int i = 1; i <= actions.Length(); i++) {
        DeactivateSubtree(n->GetChild(actions[i]->GetNumber()));
      }
    }
    else {
      for (int i = 1; i <= n->GetInfoset()->NumActions(); i++) {
        DeactivateSubtree(n->GetChild(i));
      }
    }
  }
}

void BehaviorSupportProfile::DeactivateSubtree(const GameNode &n, List<GameInfoset> &list)
{
  if (!n->IsTerminal()) {
    m_nonterminalReachable[n] = false;
    if (!HasActiveMembers(n->GetInfoset()->GetPlayer()->GetNumber(),
                          n->GetInfoset()->GetNumber())) {
      list.push_back(n->GetInfoset());
      m_infosetReachable[n->GetInfoset()] = false;
    }
    Array<GameAction> actions(
        m_actions[n->GetInfoset()->GetPlayer()->GetNumber()][n->GetInfoset()->GetNumber()]);
    for (int i = 1; i <= actions.Length(); i++) {
      DeactivateSubtree(n->GetChild(actions[i]->GetNumber()), list);
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
