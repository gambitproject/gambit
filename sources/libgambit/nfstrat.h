//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to strategy classes for normal forms
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#ifndef NFSTRAT_H
#define NFSTRAT_H

#include "libgambit.h"

/// This class represents a strategy profile on a normal form game.
/// It specifies exactly one strategy for each player defined on the
/// game.
class gbtStrategyProfile  {
  friend class Gambit::GameRep;
private:
  long m_index;
  Gambit::Game m_nfg;
  gbtArray<Gambit::GameStrategy> m_profile;
  
public:
  /// @name Lifecycle
  //@{
  /// Construct a new strategy profile on the specified game
  gbtStrategyProfile(const Gambit::Game &);
  //@}

  /// @name Data access and manipulation
  //@{
  /// Get the strategy played by player pl  
  Gambit::GameStrategy GetStrategy(int pl) const { return m_profile[pl]; }
  /// Set the strategy for a player
  void SetStrategy(const Gambit::GameStrategy &);

  /// Get the outcome that results from the profile
  Gambit::GameOutcome GetOutcome(void) const;
  /// Set the outcome that results from the profile
  void SetOutcome(Gambit::GameOutcome p_outcome); 

  /// Get the payoff to player pl that results from the profile
  gbtRational GetPayoff(int pl) const;
  //@}
};


/// This class represents a subset of the strategies in a normal form game.
/// It is enforced that each player has at least one strategy; thus,
/// the strategies in a support can be viewed as a restriction of a game
/// to a subset of its strategies.  This is useful for eliminating
/// dominated strategies from consideration, and in computational
/// approaches that enumerate possible equilibrium supports.
///
/// Within the support, strategies are maintained in the same order
/// in which they appear in the underlying game.
class gbtNfgSupport {
protected:
  Gambit::Game m_nfg;
  gbtArray<gbtArray<Gambit::GameStrategy> > m_support;
  
  bool Undominated(gbtNfgSupport &newS, int pl, bool strong,
		   std::ostream &tracefile) const;

public:
  /// @name Lifecycle
  //@{
  /// Constructor.  By default, a support contains all strategies.
  gbtNfgSupport(const Gambit::Game &);
  //@}

  /// @name Operator overloading
  //@{
  /// Test for the equality of two supports (same strategies for all players)
  bool operator==(const gbtNfgSupport &p_support) const
    { return (m_support == p_support.m_support); }
  /// Test for the inequality of two supports
  bool operator!=(const gbtNfgSupport &p_support) const
    { return (m_support != p_support.m_support); }
  //@}

  /// @name General information
  //@{
  /// Returns the game on which the support is defined.
  Gambit::Game GetGame(void) const { return m_nfg; }

  /// Returns the number of strategies in the support for player pl.
  int NumStrats(int pl) const  { return m_support[pl].Length(); }

  /// Returns the number of strategies in the support for all players.
  gbtArray<int> NumStrats(void) const;

  /// Returns the total number of strategies in the support.
  int ProfileLength(void) const;

  /// Returns the strategy in the st'th position for player pl.
  Gambit::GameStrategy GetStrategy(int pl, int st) const 
    { return m_support[pl][st]; }

  /// Retuns the index of the strategy in the support.
  int GetIndex(const Gambit::GameStrategy &s) const
    { return m_support[s->GetPlayer()->GetNumber()].Find(s); }

  /// Returns true exactly when the strategy is in the support.
  bool Contains(const Gambit::GameStrategy &s) const
    { return m_support[s->GetPlayer()->GetNumber()].Contains(s); }

  /// Returns true iff this support is a (weak) subset of the specified support
  bool IsSubsetOf(const gbtNfgSupport &) const;

  //@}

  /// @name Modifying the support
  //@{
  /// Add a strategy to the support.
  void AddStrategy(Gambit::GameStrategy);

  /// Remove a strategy from the support; return true if successful.

  /// Removes a strategy from the support.  If the strategy is
  /// not present, or if the strategy is the only strategy for that
  /// player, it is not removed.  Returns true if the removal was
  /// executed, and false if not.
  bool RemoveStrategy(Gambit::GameStrategy);
  //@}

  /// @name Identification of dominated strategies
  //@{
  bool Dominates(Gambit::GameStrategy s, Gambit::GameStrategy t, bool strong) const;
  bool IsDominated(Gambit::GameStrategy s, bool strong) const; 

  gbtNfgSupport Undominated(bool strong, const gbtArray<int> &players,
			    std::ostream &tracefile) const;
  //@}
};

#endif  // NFSTRAT_H


