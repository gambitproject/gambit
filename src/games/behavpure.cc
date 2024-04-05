//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org
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

PureBehaviorProfile::PureBehaviorProfile(Game p_efg) : m_efg(p_efg), m_profile(m_efg->NumPlayers())
{
  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++) {
    GamePlayerRep *player = m_efg->GetPlayer(pl);
    m_profile[pl] = Array<GameAction>(player->NumInfosets());
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      m_profile[pl][iset] = player->GetInfoset(iset)->GetAction(1);
    }
  }
}

//------------------------------------------------------------------------
//              PureBehaviorProfile: Data access and manipulation
//------------------------------------------------------------------------

GameAction PureBehaviorProfile::GetAction(const GameInfoset &infoset) const
{
  return m_profile[infoset->GetPlayer()->GetNumber()][infoset->GetNumber()];
}

void PureBehaviorProfile::SetAction(const GameAction &action)
{
  m_profile[action->GetInfoset()->GetPlayer()->GetNumber()][action->GetInfoset()->GetNumber()] =
      action;
}

template <class T> T PureBehaviorProfile::GetPayoff(const GameNode &p_node, int pl) const
{
  T payoff(0);

  if (p_node->GetOutcome()) {
    payoff += static_cast<T>(p_node->GetOutcome()->GetPayoff(pl));
  }

  if (!p_node->IsTerminal()) {
    if (p_node->GetInfoset()->IsChanceInfoset()) {
      for (int i = 1; i <= p_node->NumChildren(); i++) {
        payoff += (static_cast<T>(p_node->GetInfoset()->GetActionProb(i)) *
                   GetPayoff<T>(p_node->GetChild(i), pl));
      }
    }
    else {
      int player = p_node->GetPlayer()->GetNumber();
      int iset = p_node->GetInfoset()->GetNumber();
      payoff += GetPayoff<T>(p_node->GetChild(m_profile[player][iset]->GetNumber()), pl);
    }
  }

  return payoff;
}

// Explicit instantiations
template double PureBehaviorProfile::GetPayoff(const GameNode &, int pl) const;
template Rational PureBehaviorProfile::GetPayoff(const GameNode &, int pl) const;

template <class T> T PureBehaviorProfile::GetPayoff(const GameAction &p_action) const
{
  PureBehaviorProfile copy(*this);
  copy.SetAction(p_action);
  return copy.GetPayoff<T>(p_action->GetInfoset()->GetPlayer()->GetNumber());
}

// Explicit instantiations
template double PureBehaviorProfile::GetPayoff(const GameAction &) const;
template Rational PureBehaviorProfile::GetPayoff(const GameAction &) const;

bool PureBehaviorProfile::IsAgentNash() const
{
  for (auto player : m_efg->GetPlayers()) {
    auto current = GetPayoff<Rational>(player);
    for (auto infoset : player->GetInfosets()) {
      for (auto action : infoset->GetActions()) {
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
  for (auto player : m_efg->GetPlayers()) {
    for (auto infoset : player->GetInfosets()) {
      temp[GetAction(infoset)] = Rational(1);
    }
  }
  return temp;
}

//========================================================================
//                    class BehaviorProfileIterator
//========================================================================

BehaviorProfileIterator::BehaviorProfileIterator(const Game &p_game)
  : m_atEnd(false), m_support(p_game), m_currentBehav(p_game->NumInfosets()), m_profile(p_game),
    m_frozenPlayer(0), m_frozenInfoset(0), m_numActiveInfosets(p_game->NumPlayers())
{
  for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
    GamePlayer player = p_game->GetPlayer(pl);
    m_numActiveInfosets[pl] = player->NumInfosets();
    Array<bool> activeForPl(player->NumInfosets());
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      activeForPl[iset] = true;
    }
    m_isActive.push_back(activeForPl);
  }
  First();
}

BehaviorProfileIterator::BehaviorProfileIterator(const BehaviorSupportProfile &p_support,
                                                 const GameAction &p_action)
  : m_atEnd(false), m_support(p_support), m_currentBehav(p_support.GetGame()->NumInfosets()),
    m_profile(p_support.GetGame()),
    m_frozenPlayer(p_action->GetInfoset()->GetPlayer()->GetNumber()),
    m_frozenInfoset(p_action->GetInfoset()->GetNumber()),
    m_numActiveInfosets(m_support.GetGame()->NumPlayers())
{
  for (int pl = 1; pl <= m_support.GetGame()->NumPlayers(); pl++) {
    GamePlayer player = m_support.GetGame()->GetPlayer(pl);
    m_numActiveInfosets[pl] = 0;
    Array<bool> activeForPl(player->NumInfosets());
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      activeForPl[iset] = p_support.IsReachable(player->GetInfoset(iset));
      m_numActiveInfosets[pl]++;
    }
    m_isActive.push_back(activeForPl);
  }

  m_currentBehav(m_frozenPlayer, m_frozenInfoset) = p_support.GetIndex(p_action);
  m_profile.SetAction(p_action);
  First();
}

void BehaviorProfileIterator::First()
{
  for (int pl = 1; pl <= m_support.GetGame()->NumPlayers(); pl++) {
    for (int iset = 1; iset <= m_support.GetGame()->GetPlayer(pl)->NumInfosets(); iset++) {
      if (pl != m_frozenPlayer && iset != m_frozenInfoset) {
        m_currentBehav(pl, iset) = 1;
        if (m_isActive[pl][iset]) {
          m_profile.SetAction(m_support.GetAction(pl, iset, 1));
        }
      }
    }
  }
}

void BehaviorProfileIterator::operator++()
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
    if (m_isActive[pl][iset] && (pl != m_frozenPlayer || iset != m_frozenInfoset)) {
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

} // end namespace Gambit
