//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/gameagg.h
// Declaration of GameAGGRep, the action-graph game representation
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

#ifndef GAMEAGG_H
#define GAMEAGG_H

#include "games/agg/agg.h"

namespace Gambit {

class GameAGGRep : public GameRep {
  template <class T> friend class AGGMixedStrategyProfileRep;
  friend class AGGPureStrategyProfileRep;

private:
  std::shared_ptr<agg::AGG> aggPtr;
  Array<GamePlayerRep *> m_players;

  /// Constructor
  explicit GameAGGRep(std::shared_ptr<agg::AGG>);

public:
  /// @name Lifecycle
  //@{
  /// Create a game from a serialized file in AGG format
  static Game ReadAggFile(std::istream &);
  /// Destructor
  ~GameAGGRep() override
  {
    for (auto player : m_players) {
      player->Invalidate();
    }
  }
  /// Create a copy of the game, as a new game
  Game Copy() const override;
  //@}

  std::shared_ptr<agg::AGG> GetUnderlyingAGG() const { return aggPtr; }
  /// @name Dimensions of the game
  //@{
  /// The number of actions in each information set
  PVector<int> NumActions() const override { throw UndefinedException(); }
  /// The number of members in each information set
  PVector<int> NumMembers() const override { throw UndefinedException(); }
  /// The number of strategies for each player
  Array<int> NumStrategies() const override;
  /// Gets the i'th strategy in the game, numbered globally
  GameStrategy GetStrategy(int p_index) const override;
  /// Returns the total number of actions in the game
  int BehavProfileLength() const override { throw UndefinedException(); }
  /// Returns the total number of strategies in the game
  int MixedProfileLength() const override { return aggPtr->getNumActions(); }
  int NumStrategyContingencies() const override { throw UndefinedException(); }
  //@}

  PureStrategyProfile NewPureStrategyProfile() const override;
  MixedStrategyProfile<double> NewMixedStrategyProfile(double) const override;
  MixedStrategyProfile<Rational> NewMixedStrategyProfile(const Rational &) const override;
  MixedStrategyProfile<double>
  NewMixedStrategyProfile(double, const StrategySupportProfile &) const override;
  MixedStrategyProfile<Rational>
  NewMixedStrategyProfile(const Rational &, const StrategySupportProfile &) const override;

  /// @name Players
  //@{
  /// Returns the number of players in the game
  int NumPlayers() const override { return aggPtr->getNumPlayers(); }
  /// Returns the pl'th player in the game
  GamePlayer GetPlayer(int pl) const override { return m_players[pl]; }
  /// Returns the chance (nature) player
  GamePlayer GetChance() const override { throw UndefinedException(); }
  /// Creates a new player in the game, with no moves
  GamePlayer NewPlayer() override { throw UndefinedException(); }
  //@}

  /// @name Information sets
  //@{
  /// Returns the iset'th information set in the game (numbered globally)
  GameInfoset GetInfoset(int iset) const override { throw UndefinedException(); }
  /// Returns the set of information sets in the game
  Array<GameInfoset> GetInfosets() const override { throw UndefinedException(); }
  /// Returns an array with the number of information sets per personal player
  Array<int> NumInfosets() const override { throw UndefinedException(); }
  /// Returns the act'th action in the game (numbered globally)
  GameAction GetAction(int act) const override { throw UndefinedException(); }
  //@}

  /// @name Outcomes
  //@{
  /// Returns the number of outcomes defined in the game
  int NumOutcomes() const override { throw UndefinedException(); }
  /// Returns the index'th outcome defined in the game
  GameOutcome GetOutcome(int index) const override { throw UndefinedException(); }
  /// Creates a new outcome in the game
  GameOutcome NewOutcome() override { throw UndefinedException(); }
  /// Deletes the specified outcome from the game
  void DeleteOutcome(const GameOutcome &) override { throw UndefinedException(); }
  //@}

  /// @name Nodes
  //@{
  /// Returns the root node of the game
  GameNode GetRoot() const override { throw UndefinedException(); }
  /// Returns the number of nodes in the game
  int NumNodes() const override { throw UndefinedException(); }
  //@}

  /// @name General data access
  //@{
  bool IsTree() const override { return false; }
  bool IsAgg() const override { return true; }
  bool IsPerfectRecall(GameInfoset &, GameInfoset &) const override { return true; }
  bool IsConstSum() const override;
  /// Returns the smallest payoff in any outcome of the game
  Rational GetMinPayoff(int) const override { return Rational(aggPtr->getMinPayoff()); }
  /// Returns the largest payoff in any outcome of the game
  Rational GetMaxPayoff(int) const override { return Rational(aggPtr->getMaxPayoff()); }

  //@}

  /// @name Modification
  //@{
  Game SetChanceProbs(const GameInfoset &, const Array<Number> &) override
  {
    throw UndefinedException();
  }
  //@}

  /// @name Writing data files
  //@{
  /// Write the game to a savefile in the specified format.
  void Write(std::ostream &p_stream, const std::string &p_format = "native") const override;
  virtual void WriteAggFile(std::ostream &) const;
  //@}
};

} // namespace Gambit

#endif // GAMEAGG_H
