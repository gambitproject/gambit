//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
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

  /// @name Writing data files
  //@{
  /// Write the game in .efg format to the specified stream
  virtual void WriteEfgFile(std::ostream &) const
  { throw UndefinedException(); }
  /// Write the game in .nfg format to the specified stream
  virtual void WriteNfgFile(std::ostream &) const
  { throw UndefinedException(); }
  //@}

public:
  /// @name Lifecycle
  //@{
  /// Destructor
  virtual ~GameExplicitRep();
  //@}

  /// @name General data access
  //@{
  /// Returns the smallest payoff in any outcome of the game
  virtual Rational GetMinPayoff(int pl = 0) const;
  /// Returns the largest payoff in any outcome of the game
  virtual Rational GetMaxPayoff(int pl = 0) const;
  //@}

  /// @name Dimensions of the game
  //@{
  /// The number of strategies for each player
  virtual Array<int> NumStrategies(void) const;
  /// Gets the i'th strategy in the game, numbered globally
  virtual GameStrategy GetStrategy(int p_index) const;
  /// Returns the number of strategy contingencies in the game
  virtual int NumStrategyContingencies(void) const;
  /// Returns the total number of strategies in the game
  virtual int MixedProfileLength(void) const;
  //@}

  /// @name Players
  //@{
  /// Returns the number of players in the game
  virtual int NumPlayers(void) const { return m_players.Length(); }
  /// Returns the pl'th player in the game
  virtual GamePlayer GetPlayer(int pl) const { return m_players[pl]; }
  /// Returns the set of players in the game
  virtual const GamePlayers &Players(void) const { return m_players; }
  //@}

  /// @name Outcomes
  //@{
  /// Returns the number of outcomes defined in the game
  virtual int NumOutcomes(void) const { return m_outcomes.Length(); }
  /// Returns the index'th outcome defined in the game
  virtual GameOutcome GetOutcome(int index) const { return m_outcomes[index]; }
  /// Creates a new outcome in the game
  virtual GameOutcome NewOutcome(void);

  /// @name Writing data files
  //@{
  virtual void Write(std::ostream &p_stream,
		     const std::string &p_format="native") const;
  //@}
};

}

#endif  // GAMEEXPL_H
