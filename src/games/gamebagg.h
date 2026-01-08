//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/libgambit/gameagg.h
// Declaration of the Bayesian action-graph game representation
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

#ifndef GAMEBAGG_H
#define GAMEBAGG_H

#include "games/agg/bagg.h"

namespace Gambit {

class GameBAGGRep : public GameRep {
  template <class T> friend class BAGGMixedStrategyProfileRep;
  friend class BAGGPureStrategyProfileRep;

private:
  std::shared_ptr<agg::BAGG> baggPtr;
  Array<int> agent2baggPlayer;

public:
  /// @name Lifecycle
  //@{
  /// Constructor
  explicit GameBAGGRep(std::shared_ptr<agg::BAGG> _baggPtr);
  /// Destructor
  ~GameBAGGRep() override
  {
    for (auto player : m_players) {
      player->Invalidate();
    }
  }
  /// Create a copy of the game, as a new game
  Game Copy() const override;
  //@}

  /// @name Dimensions of the game
  //@{
  /// Returns the total number of actions in the game
  int BehavProfileLength() const override { throw UndefinedException(); }
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
  /// Returns the chance (nature) player
  GamePlayer GetChance() const override { throw UndefinedException(); }
  /// Creates a new player in the game, with no moves
  GamePlayer NewPlayer() override { throw UndefinedException(); }
  //@}

  /// @name Nodes
  //@{
  /// Returns the root node of the game
  GameNode GetRoot() const override { throw UndefinedException(); }
  /// Returns the number of nodes in the game
  size_t NumNodes() const override { throw UndefinedException(); }
  /// Returns the number of non-terminal nodes in the game
  size_t NumNonterminalNodes() const override { throw UndefinedException(); }
  //@}

  /// @name General data access
  //@{
  bool IsTree() const override { return false; }
  virtual bool IsBagg() const { return true; }
  bool IsPerfectRecall() const override { return true; }
  bool IsConstSum() const override { throw UndefinedException(); }
  /// Returns the smallest payoff to any player in any outcome of the game
  Rational GetMinPayoff() const override { return Rational(baggPtr->getMinPayoff()); }
  /// Returns the smallest payoff to the player in any outcome of the game
  Rational GetPlayerMinPayoff(const GamePlayer &) const override { throw UndefinedException(); }
  /// Returns the largest payoff to any player in any outcome of the game
  Rational GetMaxPayoff() const override { return Rational(baggPtr->getMaxPayoff()); }
  /// Returns the largest payoff to the player in any outcome of the game
  Rational GetPlayerMaxPayoff(const GamePlayer &) const override { throw UndefinedException(); }
  //@}

  /// @name Writing data files
  //@{
  /// Write the game to a savefile in the specified format.
  void Write(std::ostream &p_stream, const std::string &p_format = "native") const override;
  void WriteNfgFile(std::ostream &) const override { throw UndefinedException(); }
  virtual void WriteBaggFile(std::ostream &) const;
  //@}

  /// @name Modification
  //@{
  Game SetChanceProbs(const GameInfoset &, const Array<Number> &) override
  {
    throw UndefinedException();
  }
  //@}
};

/// @brief Reads a game representation in .bagg format
/// @param[in] p_stream An input stream, positioned at the start of the text in .bagg format
/// @return A handle to the game representation constructed
/// @throw InvalidFileException If the stream does not contain a valid serialisation
///                             of a game in .bagg format.
inline Game ReadBaggFile(std::istream &in)
{
  try {
    return std::make_shared<GameBAGGRep>(agg::BAGG::makeBAGG(in));
  }
  catch (std::runtime_error &ex) {
    throw InvalidFileException(ex.what());
  }
}

} // end namespace Gambit

#endif // GAMEBAGG_H
