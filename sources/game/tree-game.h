//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Representation of game for explicit game trees
//
// This file is part of Gambit
// Copyright (c) 2003, The Gambit Project
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

#ifndef TREE_GAME_H
#define TREE_GAME_H

#include "game.h"

class gbtTreeGameRep;
class gbtTreeOutcomeRep;
class gbtTreeActionRep;
class gbtTreeInfosetRep;
class gbtTreeNodeRep;
class gbtTreePlayerRep;

#include "tree-outcome.h"
#include "tree-player.h"
#include "tree-infoset.h"
#include "tree-node.h"

//!
//! Implementation of game representation via an explicit game
//! tree.  Normal form members of the class represent the
//! reduced normal form of the game.
//!
class gbtTreeGameRep : public gbtGameRep {
public:
  int m_refCount;
  mutable bool m_hasComputed;
  std::string m_label, m_comment;
  gbtBlock<gbtTreePlayerRep *> m_players;
  gbtBlock<gbtTreeOutcomeRep *> m_outcomes;
  gbtTreeNodeRep *m_root;
  int m_numNodes, m_nextNodeId;
  gbtTreePlayerRep *m_chance;

  /// @name Constructor and destructor
  //@{
  /// Constructor, creating a trivial game (one node, no players).
  gbtTreeGameRep(void);
  /// Destructor
  virtual ~gbtTreeGameRep();
  //@}

  /// @name Mechanism for reference counting
  //@{
  void Reference(void);
  bool Dereference(void);
  //@}

  /// @name Manipulation of titles and comments
  //@{
  void SetLabel(const std::string &);
  std::string GetLabel(void) const;
  void SetComment(const std::string &);
  std::string GetComment(void) const;
  //@}

  /// @name General information about the game
  //@{
  bool IsConstSum(void) const;
  bool IsPerfectRecall(void) const;
  gbtRational GetMinPayoff(void) const;
  gbtRational GetMaxPayoff(void) const;
  bool IsDeleted(void) const { return false; }
  void Canonicalize(void);
  //@}

  /// @name Information about the game tree
  //@{
  bool HasTree(void) const { return true; }
  gbtGameNode GetRoot(void) const;
  int NumNodes(void) const;
  gbtGameBehavContingency NewBehavContingency(void) const;
  gbtGameBehavProfileIterator NewBehavProfileIterator(void) const;
  //@}

  /// @name Information about the game table
  //@{
  gbtGameContingency NewContingency(void) const;
  gbtGameContingencyIterator NewContingencyIterator(void) const;
  gbtGameContingencyIterator NewContingencyIterator(const gbtGameStrategy &) const;
  //@}

  /// @name Manipulation of players in the game
  //@{
  int NumPlayers(void) const;
  gbtGamePlayer GetChance(void) const;
  gbtGamePlayer NewPlayer(void);
  gbtGamePlayer GetPlayer(int) const;
  //@}

  /// @name Manipulation of outcomes in the game
  //@{
  int NumOutcomes(void) const;
  gbtGameOutcome GetOutcome(int) const;
  gbtGameOutcome NewOutcome(void);
  //@}

  /// @name Information about the dimensions of the game
  //@{
  int BehaviorProfileLength(void) const;
  gbtArray<int> NumInfosets(void) const;
  gbtPVector<int> NumActions(void) const;
  gbtPVector<int> NumMembers(void) const;

  int StrategyProfileLength(void) const;
  gbtArray<int> NumStrategies(void) const;
  //@}

  /// @name Creating strategy profiles on the game
  //@{
  gbtMixedProfile<double> NewMixedProfile(double) const;
  gbtMixedProfile<gbtRational> NewMixedProfile(const gbtRational &) const;
  gbtBehavProfile<double> NewBehavProfile(double) const;
  gbtBehavProfile<gbtRational> NewBehavProfile(const gbtRational &) const;
  //@}

  /// @name Writing data files
  //@{
  void WriteEfg(std::ostream &) const;
  void WriteNfg(std::ostream &) const { throw gbtGameUndefinedException(); }
  //@}

  /// @name Managing computed elements of the game
  //@{
  /// Clears out computed elements
  void ClearComputedElements(void) const;
  /// Generates computed elements
  void BuildComputedElements(void) const;
  //@}

  /// @name Member functions private to the implementation
  //@{
  void NumberNodes(gbtTreeNodeRep *);

  gbtRational GetPayoff(gbtTreePlayerRep *, gbtTreeNodeRep *n, 
			const gbtArray<gbtTreeStrategyRep *> &profile) const;
  gbtRational GetPayoff(gbtTreePlayerRep *, 
			const gbtArray<gbtTreeStrategyRep *> &) const; 

  bool IsPerfectRecall(gbtGameInfoset &, gbtGameInfoset &) const;
  //@}
};

#endif  // TREE_GAME_H
