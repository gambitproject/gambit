//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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
  for (size_t pl = 1; pl <= m_nfg->NumPlayers(); pl++) {
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
//                        class StrategyProfileIterator
//===========================================================================

//---------------------------------------------------------------------------
//                               Lifecycle
//---------------------------------------------------------------------------

StrategyProfileIterator::StrategyProfileIterator(const StrategySupportProfile &p_support)
  : m_atEnd(false), m_support(p_support), m_currentStrat(m_support.GetGame()->NumPlayers()),
    m_profile(m_support.GetGame()->NewPureStrategyProfile()), m_frozen1(0), m_frozen2(0)
{
  First();
}

StrategyProfileIterator::StrategyProfileIterator(const StrategySupportProfile &p_support, int pl,
                                                 int st)
  : m_atEnd(false), m_support(p_support), m_currentStrat(m_support.GetGame()->NumPlayers()),
    m_profile(m_support.GetGame()->NewPureStrategyProfile()), m_frozen1(pl), m_frozen2(0)
{
  m_currentStrat[pl] = st;
  m_profile->SetStrategy(m_support.GetStrategy(pl, st));
  First();
}

StrategyProfileIterator::StrategyProfileIterator(const StrategySupportProfile &p_support,
                                                 const GameStrategy &p_strategy)
  : m_atEnd(false), m_support(p_support), m_currentStrat(p_support.GetGame()->NumPlayers()),
    m_profile(p_support.GetGame()->NewPureStrategyProfile()),
    m_frozen1(p_strategy->GetPlayer()->GetNumber()), m_frozen2(0)
{
  m_currentStrat[m_frozen1] = p_strategy->GetNumber();
  m_profile->SetStrategy(p_strategy);
  First();
}

StrategyProfileIterator::StrategyProfileIterator(const StrategySupportProfile &p_support, int pl1,
                                                 int st1, int pl2, int st2)
  : m_atEnd(false), m_support(p_support), m_currentStrat(m_support.GetGame()->NumPlayers()),
    m_profile(m_support.GetGame()->NewPureStrategyProfile()), m_frozen1(pl1), m_frozen2(pl2)
{
  m_currentStrat[pl1] = st1;
  m_profile->SetStrategy(m_support.GetStrategy(pl1, st1));
  m_currentStrat[pl2] = st2;
  m_profile->SetStrategy(m_support.GetStrategy(pl2, st2));
  First();
}

//---------------------------------------------------------------------------
//                                Iteration
//---------------------------------------------------------------------------

void StrategyProfileIterator::First()
{
  for (int pl = 1; pl <= m_support.GetGame()->NumPlayers(); pl++) {
    if (pl == m_frozen1 || pl == m_frozen2) {
      continue;
    }
    m_profile->SetStrategy(m_support.GetStrategy(pl, 1));
    m_currentStrat[pl] = 1;
  }
}

void StrategyProfileIterator::operator++()
{
  int pl = 1;

  while (true) {
    if (pl == m_frozen1 || pl == m_frozen2) {
      pl++;
      if (pl > m_support.GetGame()->NumPlayers()) {
        m_atEnd = true;
        return;
      }
      continue;
    }

    if (m_currentStrat[pl] < m_support.NumStrategies(pl)) {
      m_profile->SetStrategy(m_support.GetStrategy(pl, ++(m_currentStrat[pl])));
      return;
    }
    m_profile->SetStrategy(m_support.GetStrategy(pl, 1));
    m_currentStrat[pl] = 1;
    pl++;
    if (pl > m_support.GetGame()->NumPlayers()) {
      m_atEnd = true;
      return;
    }
  }
}

} // namespace Gambit
