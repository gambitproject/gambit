//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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
//                  GameExplicitRep: General data access
//------------------------------------------------------------------------

Rational GameExplicitRep::GetMinPayoff() const
{
  return std::accumulate(std::next(m_players.begin()), m_players.end(),
                         GetPlayerMinPayoff(m_players.front()),
                         [this](const Rational &r, const GamePlayer &p) {
                           return std::min(r, GetPlayerMinPayoff(p));
                         });
}

Rational GameExplicitRep::GetMaxPayoff() const
{
  return std::accumulate(std::next(m_players.begin()), m_players.end(),
                         GetPlayerMaxPayoff(m_players.front()),
                         [this](const Rational &r, const GamePlayer &p) {
                           return std::max(r, GetPlayerMaxPayoff(p));
                         });
}

//------------------------------------------------------------------------
//                      GameExplicitRep: Outcomes
//------------------------------------------------------------------------

GameOutcome GameExplicitRep::NewOutcome()
{
  m_outcomes.push_back(std::make_shared<GameOutcomeRep>(this, m_outcomes.size() + 1));
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
