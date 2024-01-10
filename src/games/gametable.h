//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/gametable.h
// Declaration of strategic game representation
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

#ifndef GAMETABLE_H
#define GAMETABLE_H

#include "gameexpl.h"

namespace Gambit {

class GameTableRep : public GameExplicitRep {
  friend class StrategySupportProfile;
  friend class GamePlayerRep;
  friend class TablePureStrategyProfileRep;
  friend class PureStrategyProfileRep;
  template <class T> friend class MixedStrategyProfile;
  template <class T> friend class TableMixedStrategyProfileRep;
private:
  Array<GameOutcomeRep *> m_results;

  /// @name Private auxiliary functions
  //@{
  void IndexStrategies();
  void RebuildTable();
  //@}

public:
  /// @name Lifecycle
  //@{
  /// Construct a new table game with the given dimension
  /// If p_sparseOutcomes = true, outcomes for all contingencies are left null
  explicit GameTableRep(const Array<int> &p_dim, bool p_sparseOutcomes = false);
  Game Copy() const override;
  //@}

  /// @name General data access
  //@{
  bool IsTree() const override { return false; }
  bool IsConstSum() const override;
  bool IsPerfectRecall(GameInfoset &, GameInfoset &) const override
  { return true; }
  //@}

  /// @name Dimensions of the game
  //@{
  /// The number of actions in each information set
  PVector<int> NumActions() const override
  { throw UndefinedException(); }
  /// The number of members in each information set
  PVector<int> NumMembers() const override
  { throw UndefinedException(); }
  /// Returns the total number of actions in the game
  int BehavProfileLength() const override
  { throw UndefinedException(); }
  //@}

  /// @name Players
  //@{
  /// Returns the chance (nature) player
  GamePlayer GetChance() const override { throw UndefinedException(); }
  /// Creates a new player in the game, with no moves
  GamePlayer NewPlayer() override;
  //@}

  /// @name Information sets
  //@{
  /// Returns the iset'th information set in the game (numbered globally)
  GameInfoset GetInfoset(int iset) const override 
  { throw UndefinedException(); }
  /// Returns an array with the number of information sets per personal player
  Array<int> NumInfosets() const override
  { throw UndefinedException(); }
  /// Returns the act'th action in the game (numbered globally)
  GameAction GetAction(int act) const override
  { throw UndefinedException(); }
  //@}

  /// @name Nodes
  //@{
  /// Returns the root node of the game
  GameNode GetRoot() const override { throw UndefinedException(); } 
  /// Returns the number of nodes in the game
  int NumNodes() const override { throw UndefinedException(); }
  //@}

  /// @name Outcomes
  //@{
  /// Deletes the specified outcome from the game
  void DeleteOutcome(const GameOutcome &) override;
  //@}

  /// @name Writing data files
  //@{
  /// Write the game to a file in .nfg outcome format
  void WriteNfgFile(std::ostream &) const override;

  /// @name Modification
  //@{
  Game SetChanceProbs(const GameInfoset &, const Array<Number> &) override { throw UndefinedException(); }
  //@}

  PureStrategyProfile NewPureStrategyProfile() const override;
  MixedStrategyProfile<double> NewMixedStrategyProfile(double) const override;
  MixedStrategyProfile<Rational> NewMixedStrategyProfile(const Rational &) const override; 
  MixedStrategyProfile<double> NewMixedStrategyProfile(double, const StrategySupportProfile&) const override;
  MixedStrategyProfile<Rational> NewMixedStrategyProfile(const Rational &, const StrategySupportProfile&) const override;

};

}



#endif  // GAMETABLE_H
