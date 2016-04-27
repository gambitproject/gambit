//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
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
  Game m_unrestricted;

  /// @name Private auxiliary functions
  //@{
  void IndexStrategies(void);
  void RebuildTable(void);
  //@}

public:
  /// @name Lifecycle
  //@{
  /// Construct a new table game with the given dimension
  /// If p_sparseOutcomes = true, outcomes for all contingencies are left null
  GameTableRep(const Array<int> &p_dim, bool p_sparseOutcomes = false);
  virtual Game Copy(void) const;
  //@}

  /// @name General data access
  //@{
  virtual bool IsTree(void) const { return false; }
  virtual bool IsConstSum(void) const;
  virtual bool IsPerfectRecall(GameInfoset &, GameInfoset &) const
  { return true; }
  //@}

  /// @name Interface with restricted game mechanism
  //@{
  virtual bool IsRestriction(void) const { return (m_unrestricted != 0); }
  virtual Game Unrestrict(void) const 
  { if (m_unrestricted) return m_unrestricted; else throw UndefinedException(); }
  //@}

  /// @name Dimensions of the game
  //@{
  /// The number of actions in each information set
  virtual PVector<int> NumActions(void) const
  { throw UndefinedException(); }
  /// The number of members in each information set
  virtual PVector<int> NumMembers(void) const
  { throw UndefinedException(); }
  /// Returns the total number of actions in the game
  virtual int BehavProfileLength(void) const
  { throw UndefinedException(); }
  //@}

  /// @name Players
  //@{
  /// Returns the chance (nature) player
  virtual GamePlayer GetChance(void) const { throw UndefinedException(); }
  /// Creates a new player in the game, with no moves
  virtual GamePlayer NewPlayer(void);
  //@}

  /// @name Information sets
  //@{
  /// Returns the iset'th information set in the game (numbered globally)
  virtual GameInfoset GetInfoset(int iset) const 
  { throw UndefinedException(); }
  /// Returns an array with the number of information sets per personal player
  virtual Array<int> NumInfosets(void) const
  { throw UndefinedException(); }
  /// Returns the act'th action in the game (numbered globally)
  virtual GameAction GetAction(int act) const
  { throw UndefinedException(); }
  //@}

  /// @name Nodes
  //@{
  /// Returns the root node of the game
  virtual GameNode GetRoot(void) const { throw UndefinedException(); } 
  /// Returns the number of nodes in the game
  virtual int NumNodes(void) const { throw UndefinedException(); }
  //@}

  /// @name Outcomes
  //@{
  /// Deletes the specified outcome from the game
  virtual void DeleteOutcome(const GameOutcome &);
  //@}

  /// @name Writing data files
  //@{
  virtual void WriteNfgFile(std::ostream &) const;
  //@}

  virtual PureStrategyProfile NewPureStrategyProfile(void) const;
  virtual MixedStrategyProfile<double> NewMixedStrategyProfile(double) const;
  virtual MixedStrategyProfile<Rational> NewMixedStrategyProfile(const Rational &) const; 
  virtual MixedStrategyProfile<double> NewMixedStrategyProfile(double, const StrategySupportProfile&) const;
  virtual MixedStrategyProfile<Rational> NewMixedStrategyProfile(const Rational &, const StrategySupportProfile&) const;

};

}



#endif  // GAMETABLE_H
