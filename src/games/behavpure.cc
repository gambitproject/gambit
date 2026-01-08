//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org
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
    payoff += p_node->GetOutcome()->GetPayoff<T>(p_player);
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

BehaviorContingencies::BehaviorContingencies(const BehaviorSupportProfile &p_support)
  : m_support(p_support)
{
  for (const auto &player : m_support.GetGame()->GetPlayers()) {
    for (const auto &infoset : player->GetInfosets()) {
      if (p_support.IsReachable(infoset)) {
        m_reachableInfosets.push_back(infoset);
      }
    }
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
}

BehaviorContingencies::iterator &BehaviorContingencies::iterator::operator++()
{
  for (auto infoset = m_cont->m_reachableInfosets.crbegin();
       infoset != m_cont->m_reachableInfosets.crend(); ++infoset) {
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
