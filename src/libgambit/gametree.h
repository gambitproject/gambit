//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/gametree.h
// Declaration of extensive game representation
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

#ifndef GAMETREE_H
#define GAMETREE_H

#include "gameexpl.h"

namespace Gambit {

class GameTreeRep : public GameExplicitRep {
  friend class GameNodeRep;
  friend class GameInfosetRep;
protected:
  mutable bool m_computedValues;
  GameNodeRep *m_root;
  GamePlayerRep *m_chance;

  /// @name Private auxiliary functions
  //@{
  void NumberNodes(GameNodeRep *, int &);
  //@}

public: 
  /// @name Lifecycle
  //@{
  GameTreeRep(void);
  virtual ~GameTreeRep();
  virtual Game Copy(void) const;
  //@}

  /// @name General data access
  //@{
  virtual bool IsTree(void) const { return true; }
  virtual bool IsConstSum(void) const;
  virtual bool IsPerfectRecall(GameInfoset &, GameInfoset &) const;
  //@}

  /// @name Players
  //@{
  /// Returns the chance (nature) player
  virtual GamePlayer GetChance(void) const { return m_chance; } 
  /// Creates a new player in the game, with no moves
  virtual GamePlayer NewPlayer(void);
  //@}

  /// @name Nodes
  //@{
  /// Returns the root node of the game
  virtual GameNode GetRoot(void) const { return m_root; } 
  /// Returns the number of nodes in the game
  int NumNodes(void) const;
  //@}

  virtual void DeleteOutcome(const GameOutcome &);

  /// @name Managing the representation
  //@{
  virtual void Canonicalize(void);
  virtual void BuildComputedValues(void);
  virtual void ClearComputedValues(void) const;
  /// Have computed values been built?
  virtual bool HasComputedValues(void) const { return m_computedValues; }
  //@}

  /// @name Writing data files
  //@{
  virtual void WriteEfgFile(std::ostream &) const;
  virtual void WriteEfgFile(std::ostream &, const GameNode &p_node) const;
  virtual void WriteNfgFile(std::ostream &) const;
  //@}

  /// @name Dimensions of the game
  //@{
  /// The number of actions in each information set
  virtual PVector<int> NumActions(void) const;
  /// The number of members in each information set
  virtual PVector<int> NumMembers(void) const;
  /// Returns the total number of actions in the game
  virtual int BehavProfileLength(void) const;
  //@}

  /// @name Information sets
  //@{
  /// Returns the iset'th information set in the game (numbered globally)
  virtual GameInfoset GetInfoset(int iset) const;
  /// Returns an array with the number of information sets per personal player
  virtual Array<int> NumInfosets(void) const;
  /// Returns the act'th action in the game (numbered globally)
  virtual GameAction GetAction(int act) const;
  //@}

  virtual PureStrategyProfile NewPureStrategyProfile(void) const;
  virtual MixedStrategyProfile<double> NewMixedStrategyProfile(double) const;
  virtual MixedStrategyProfile<Rational> NewMixedStrategyProfile(const Rational &) const; 
};

}



#endif  // GAMETREE_H
