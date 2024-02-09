//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/gameexpl.h
// Declaration of base class for explicit game representations
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

#ifndef GAMEEXPL_H
#define GAMEEXPL_H

#include "game.h"

namespace Gambit {

class GameExplicitRep : public GameRep {
  template <class T> friend class MixedStrategyProfile;

protected:
  Array<GamePlayerRep *> m_players;
  Array<GameOutcomeRep *> m_outcomes;

public:
  /// @name Lifecycle
  //@{
  /// Destructor
  ~GameExplicitRep() override;
  //@}

  /// @name General data access
  //@{
  /// Returns the smallest payoff in any outcome of the game
  Rational GetMinPayoff(int pl = 0) const override;
  /// Returns the largest payoff in any outcome of the game
  Rational GetMaxPayoff(int pl = 0) const override;
  //@}

  /// @name Dimensions of the game
  //@{
  /// The number of strategies for each player
  Array<int> NumStrategies() const override;
  /// Gets the i'th strategy in the game, numbered globally
  GameStrategy GetStrategy(int p_index) const override;
  /// Returns the number of strategy contingencies in the game
  int NumStrategyContingencies() const override;
  /// Returns the total number of strategies in the game
  int MixedProfileLength() const override;
  //@}

  /// @name Players
  //@{
  /// Returns the number of players in the game
  int NumPlayers() const override { return m_players.size(); }
  /// Returns the pl'th player in the game
  GamePlayer GetPlayer(int pl) const override { return m_players[pl]; }
  //@}

  /// @name Outcomes
  //@{
  /// Returns the number of outcomes defined in the game
  int NumOutcomes() const override { return m_outcomes.size(); }
  /// Returns the index'th outcome defined in the game
  GameOutcome GetOutcome(int index) const override { return m_outcomes[index]; }
  /// Creates a new outcome in the game
  GameOutcome NewOutcome() override;

  /// @name Writing data files
  //@{
  void Write(std::ostream &p_stream, const std::string &p_format = "native") const override;
  //@}
};

} // namespace Gambit

#endif // GAMEEXPL_H
