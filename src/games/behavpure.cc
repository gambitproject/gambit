//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org
//
// FILE: src/games/behavpure.cc
// Implementation of pure behavior profile
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
//                       class PureBehaviorProfile
//========================================================================

//------------------------------------------------------------------------
//                     PureBehaviorProfile: Lifecycle
//------------------------------------------------------------------------

PureBehaviorProfile::PureBehaviorProfile(const Game &p_efg) : m_efg(p_efg)
{
  for (const auto &infoset : m_efg->GetInfosets()) {
    m_profile[infoset] = infoset->GetActions().front();
  }
}

//------------------------------------------------------------------------
//              PureBehaviorProfile: Data access and manipulation
//------------------------------------------------------------------------

template <class T>
T PureBehaviorProfile::GetPayoff(const GameNode &p_node, const GamePlayer &p_player) const
{
  T payoff(0);

  if (p_node->GetOutcome()) {
    payoff += static_cast<T>(p_node->GetOutcome()->GetPayoff(p_player));
  }

  if (!p_node->IsTerminal()) {
    if (p_node->GetInfoset()->IsChanceInfoset()) {
      for (const auto &action : p_node->GetInfoset()->GetActions()) {
        payoff += (static_cast<T>(p_node->GetInfoset()->GetActionProb(action)) *
                   GetPayoff<T>(p_node->GetChild(action), p_player));
      }
    }
    else {
      payoff += GetPayoff<T>(p_node->GetChild(m_profile.at(p_node->GetInfoset())), p_player);
    }
  }

  return payoff;
}

// Explicit instantiations
template double PureBehaviorProfile::GetPayoff(const GameNode &, const GamePlayer &) const;
template Rational PureBehaviorProfile::GetPayoff(const GameNode &, const GamePlayer &) const;

template <class T> T PureBehaviorProfile::GetPayoff(const GameAction &p_action) const
{
  PureBehaviorProfile copy(*this);
  copy.SetAction(p_action);
  return copy.GetPayoff<T>(p_action->GetInfoset()->GetPlayer());
}

// Explicit instantiations
template double PureBehaviorProfile::GetPayoff(const GameAction &) const;
template Rational PureBehaviorProfile::GetPayoff(const GameAction &) const;

bool PureBehaviorProfile::IsAgentNash() const
{
  for (const auto &player : m_efg->GetPlayers()) {
    auto current = GetPayoff<Rational>(player);
    for (const auto &infoset : player->GetInfosets()) {
      for (const auto &action : infoset->GetActions()) {
        if (GetPayoff<Rational>(action) > current) {
          return false;
        }
      }
    }
  }
  return true;
}

MixedBehaviorProfile<Rational> PureBehaviorProfile::ToMixedBehaviorProfile() const
{
  MixedBehaviorProfile<Rational> temp(m_efg);
  temp = Rational(0);
  for (const auto &player : m_efg->GetPlayers()) {
    for (const auto &infoset : player->GetInfosets()) {
      temp[m_profile.at(infoset)] = Rational(1);
    }
  }
  return temp;
}

//========================================================================
//                    class BehaviorContingencies
//========================================================================

BehaviorContingencies::BehaviorContingencies(const BehaviorSupportProfile &p_support,
                                             const std::set<GameInfoset> &p_reachable,
                                             const std::vector<GameAction> &p_frozen)
  : m_support(p_support), m_frozen(p_frozen)
{
  if (!p_reachable.empty()) {
    for (const auto &infoset : p_reachable) {
      m_activeInfosets.push_back(infoset);
    }
  }
  else {
    for (const auto &player : m_support.GetGame()->GetPlayers()) {
      for (const auto &infoset : player->GetInfosets()) {
        if (p_support.IsReachable(infoset)) {
          m_activeInfosets.push_back(infoset);
        }
      }
    }
  }
  for (const auto &action : m_frozen) {
    m_activeInfosets.erase(std::find_if(
        m_activeInfosets.begin(), m_activeInfosets.end(),
        [action](const GameInfoset &infoset) { return infoset == action->GetInfoset(); }));
  }
}

BehaviorContingencies::iterator::iterator(BehaviorContingencies *p_cont, bool p_end)
  : m_cont(p_cont), m_atEnd(p_end), m_profile(p_cont->m_support.GetGame())
{
  if (m_atEnd) {
    return;
  }
  for (const auto &player : m_cont->m_support.GetGame()->GetPlayers()) {
    for (const auto &infoset : player->GetInfosets()) {
      m_currentBehav[infoset] = m_cont->m_support.GetActions(infoset).begin();
      m_profile.SetAction(*m_currentBehav[infoset]);
    }
  }
  for (const auto &action : m_cont->m_frozen) {
    m_profile.SetAction(action);
  }
}

BehaviorContingencies::iterator &BehaviorContingencies::iterator::operator++()
{
  for (auto infoset = m_cont->m_activeInfosets.crbegin();
       infoset != m_cont->m_activeInfosets.crend(); ++infoset) {
    ++m_currentBehav[*infoset];
    if (m_currentBehav.at(*infoset) != m_cont->m_support.GetActions(*infoset).end()) {
      m_profile.SetAction(*m_currentBehav[*infoset]);
      return *this;
    }
    m_currentBehav[*infoset] = m_cont->m_support.GetActions(*infoset).begin();
    m_profile.SetAction(*m_currentBehav[*infoset]);
  }
  m_atEnd = true;
  return *this;
}

} // end namespace Gambit
