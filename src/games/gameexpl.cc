//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/games/gameexpl.cc
// Implementation of support for explicit internal representations of games
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

#include <iostream>
#include <numeric>

#include "gambit.h"
#include "gameexpl.h"

namespace Gambit {

//========================================================================
//                       class GameExplicitRep
//========================================================================

//------------------------------------------------------------------------
//                     GameExplicitRep: Lifecycle
//------------------------------------------------------------------------

GameExplicitRep::~GameExplicitRep()
{
  for (auto player : m_players) {
    player->Invalidate();
  }
  for (auto outcome : m_outcomes) {
    outcome->Invalidate();
  }
}

//------------------------------------------------------------------------
//                  GameExplicitRep: General data access
//------------------------------------------------------------------------

Rational GameExplicitRep::GetMinPayoff(int player) const
{
  int p1, p2;

  if (m_outcomes.empty()) {
    return Rational(0);
  }

  if (player) {
    p1 = p2 = player;
  }
  else {
    p1 = 1;
    p2 = NumPlayers();
  }

  Rational minpay = static_cast<Rational>(m_outcomes.front()->GetPayoff(p1));
  for (auto outcome : m_outcomes) {
    for (int p = p1; p <= p2; p++) {
      minpay = std::min(minpay, static_cast<Rational>(outcome->GetPayoff(p)));
    }
  }
  return minpay;
}

Rational GameExplicitRep::GetMaxPayoff(int player) const
{
  int p1, p2;

  if (m_outcomes.empty()) {
    return Rational(0);
  }

  if (player) {
    p1 = p2 = player;
  }
  else {
    p1 = 1;
    p2 = NumPlayers();
  }

  Rational maxpay = static_cast<Rational>(m_outcomes.front()->GetPayoff(p1));
  for (auto outcome : m_outcomes) {
    for (int p = p1; p <= p2; p++) {
      maxpay = std::max(maxpay, static_cast<Rational>(outcome->GetPayoff(p)));
    }
  }
  return maxpay;
}

//------------------------------------------------------------------------
//                GameExplicitRep: Dimensions of the game
//------------------------------------------------------------------------

Array<int> GameExplicitRep::NumStrategies() const
{
  const_cast<GameExplicitRep *>(this)->BuildComputedValues();
  Array<int> dim(m_players.size());
  for (int pl = 1; pl <= m_players.size(); pl++) {
    dim[pl] = m_players[pl]->m_strategies.size();
  }
  return dim;
}

GameStrategy GameExplicitRep::GetStrategy(int p_index) const
{
  const_cast<GameExplicitRep *>(this)->BuildComputedValues();
  for (int pl = 1, i = 1; pl <= m_players.Length(); pl++) {
    for (int st = 1; st <= m_players[pl]->m_strategies.Length(); st++, i++) {
      if (p_index == i) {
        return m_players[pl]->m_strategies[st];
      }
    }
  }
  throw IndexException();
}

int GameExplicitRep::NumStrategyContingencies() const
{
  const_cast<GameExplicitRep *>(this)->BuildComputedValues();
  return std::accumulate(m_players.begin(), m_players.end(), 1, [](int ncont, GamePlayerRep *p) {
    return ncont * p->m_strategies.size();
  });
}

int GameExplicitRep::MixedProfileLength() const
{
  const_cast<GameExplicitRep *>(this)->BuildComputedValues();
  return std::accumulate(m_players.begin(), m_players.end(), 0,
                         [](int size, GamePlayerRep *p) { return size + p->m_strategies.size(); });
}

//------------------------------------------------------------------------
//                      GameExplicitRep: Outcomes
//------------------------------------------------------------------------

GameOutcome GameExplicitRep::NewOutcome()
{
  m_outcomes.push_back(new GameOutcomeRep(this, m_outcomes.size() + 1));
  return m_outcomes.back();
}

//------------------------------------------------------------------------
//                GameExplicitRep: Writing data files
//------------------------------------------------------------------------

void GameExplicitRep::Write(std::ostream &p_stream,
                            const std::string &p_format /*="native"*/) const
{
  if (p_format == "efg" || (p_format == "native" && IsTree())) {
    WriteEfgFile(p_stream);
  }
  else if (p_format == "nfg" || (p_format == "native" && !IsTree())) {
    WriteNfgFile(p_stream);
  }
  else {
    throw UndefinedException();
  }
}

} // end namespace Gambit
