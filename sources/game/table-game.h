//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Internal representation structs for normal form
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

#ifndef TABLE_GAME_H
#define TABLE_GAME_H

#include "game.h"

class gbtTablePlayerRep;
class gbtTableInfosetRep;
class gbtTableStrategyRep;
class gbtTableGameRep;

#include "table-outcome.h"
#include "table-player.h"

//!
//! Implementation of game representation via an normal form table
//! representation.  Extensive form members are undefined and throw
//! the gbtGameUndefined exception.
//!
class gbtTableGameRep : public gbtGameRep {
public:
  std::string m_label, m_comment;
  gbtArray<int> m_dimensions;

  gbtBlock<gbtTablePlayerRep *> m_players;
  gbtBlock<gbtTableOutcomeRep *> m_outcomes;

  gbtArray<gbtTableOutcomeRep *> m_results;
  gbtTreeGameRep *m_efg;

  /// @name Constructors and destructor
  //@{
  /// Constructor, creating a table game with the given shape
  gbtTableGameRep(const gbtArray<int> &);
  /// Destructor
  ~gbtTableGameRep();
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
  bool IsPerfectRecall(void) const { return true; }
  gbtRational GetMinPayoff(void) const;
  gbtRational GetMaxPayoff(void) const;
  //@}

  /// @name Information about the game tree
  //@{
  bool HasTree(void) const { return false; }
  gbtGameNode GetRoot(void) const { throw gbtGameUndefinedException(); }
  //@}

  /// @name Information about the game table
  //@{
  gbtNfgContingency NewContingency(void) const;
  //@}

  /// @name Manipulation of players in the game
  //@{
  int NumPlayers(void) const;
  gbtGamePlayer GetChance(void) const { throw gbtGameUndefinedException(); }
  gbtGamePlayer NewPlayer(void) { throw gbtGameUndefinedException(); }
  gbtGamePlayer GetPlayer(int) const;
  //@}

  /// @name Manipulation of outcomes in the game
  //@{
  int NumOutcomes(void) const;
  gbtGameOutcome NewOutcome(void);
  gbtGameOutcome GetOutcome(int) const;
  //@}

  /// @name Information about the dimensions of the game
  //@{
  int BehaviorProfileLength(void) const { throw gbtGameUndefinedException(); }
  gbtArray<int> NumInfosets(void) const { throw gbtGameUndefinedException(); }
  gbtPVector<int> NumActions(void) const { throw gbtGameUndefinedException(); }
  gbtPVector<int> NumMembers(void) const { throw gbtGameUndefinedException(); }

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
  void WriteEfg(std::ostream &) const { throw gbtGameUndefinedException(); }
  void WriteNfg(std::ostream &) const;
  //@}

  /// @name Member functions private to the implementation
  //@{
  void DeleteOutcome(gbtTableOutcomeRep *);
  void IndexStrategies(void);
  //@}
};

#endif // TABLE_GAME_H
