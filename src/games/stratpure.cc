//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/stratpure.cc
// Implementation of pure strategy profile
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
#include "stratpure.h"

namespace Gambit {

//========================================================================
//                    class PureStrategyProfileRep
//========================================================================

PureStrategyProfileRep::PureStrategyProfileRep(const Game &p_game)
  : m_nfg(p_game), m_profile(p_game->NumPlayers())
{
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) {
    m_profile[pl] = m_nfg->GetPlayer(pl)->GetStrategy(1);
  }
}

bool PureStrategyProfileRep::IsNash() const
{
  for (auto player : m_nfg->GetPlayers()) {
    Rational current = GetPayoff(player);
    for (auto strategy : player->GetStrategies()) {
      if (GetStrategyValue(strategy) > current) {
        return false;
      }
    }
  }
  return true;
}

bool PureStrategyProfileRep::IsStrictNash() const
{
  for (auto player : m_nfg->GetPlayers()) {
    Rational current = GetPayoff(player);
    for (auto strategy : player->GetStrategies()) {
      if (GetStrategyValue(strategy) >= current) {
        return false;
      }
    }
  }
  return true;
}

bool PureStrategyProfileRep::IsBestResponse(const GamePlayer &p_player) const
{
  Rational current = GetPayoff(p_player);
  for (auto strategy : p_player->GetStrategies()) {
    if (GetStrategyValue(strategy) > current) {
      return false;
    }
  }
  return true;
}

List<GameStrategy> PureStrategyProfileRep::GetBestResponse(const GamePlayer &p_player) const
{
  auto strategy = p_player->GetStrategy(1);
  Rational max_payoff = GetStrategyValue(strategy);
  List<GameStrategy> br;
  br.push_back(strategy);
  for (auto strategy : p_player->GetStrategies()) {
    Rational this_payoff = GetStrategyValue(strategy);
    if (this_payoff > max_payoff) {
      br.clear();
      max_payoff = this_payoff;
    }
    if (this_payoff >= max_payoff) {
      br.push_back(strategy);
    }
  }
  return br;
}

MixedStrategyProfile<Rational> PureStrategyProfileRep::ToMixedStrategyProfile() const
{
  MixedStrategyProfile<Rational> temp(m_nfg->NewMixedStrategyProfile(Rational(0)));
  temp = Rational(0);
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) {
    temp[GetStrategy(m_nfg->GetPlayer(pl))] = Rational(1);
  }
  return temp;
}

//===========================================================================
//                        class StrategyContingencies
//===========================================================================

StrategyContingencies::StrategyContingencies(const StrategySupportProfile &p_support,
                                             const std::vector<GameStrategy> &p_frozen)
  : m_support(p_support), m_frozen(p_frozen)
{
  for (auto player : m_support.GetGame()->GetPlayers()) {
    auto frozen = std::find_if(m_frozen.begin(), m_frozen.end(), [player](const GameStrategy &s) {
      return s->GetPlayer() == player;
    });
    if (frozen == m_frozen.end()) {
      m_unfrozen.push_back(player);
    }
  }
}

StrategyContingencies::iterator::iterator(StrategyContingencies *p_cont, bool p_end)
  : m_cont(p_cont), m_atEnd(p_end),
    m_profile(p_cont->m_support.GetGame()->NewPureStrategyProfile())
{
  for (auto strategy : m_cont->m_frozen) {
    m_profile->SetStrategy(strategy);
  }
  for (auto player : m_cont->m_unfrozen) {
    m_currentStrat[player] = m_cont->m_support.GetStrategies(player).begin();
    m_profile->SetStrategy(*m_currentStrat[player]);
  }
}

StrategyContingencies::iterator &StrategyContingencies::iterator::operator++()
{
  for (auto player : m_cont->m_unfrozen) {
    ++m_currentStrat[player];
    if (m_currentStrat[player] != m_cont->m_support.GetStrategies(player).end()) {
      m_profile->SetStrategy(*m_currentStrat[player]);
      return *this;
    }
    m_currentStrat[player] = m_cont->m_support.GetStrategies(player).begin();
    m_profile->SetStrategy(*m_currentStrat[player]);
  }
  m_atEnd = true;
  return *this;
}

} // namespace Gambit
