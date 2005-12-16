//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Normal form iterator class
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

#ifndef NFGITER_H
#define NFGITER_H

template <class T> class gbtArray;

class gbtStrategyProfile;
#include "nfstrat.h"

class gbtNfgContingencyIterator;

/// This class iterates through the contingencies in a strategic game.
/// It visits each strategy profile in turn, advancing one contingency
/// on each call of NextContingency().  Optionally, the strategy of
/// one player may be held fixed during the iteration (by the use of the
/// second constructor).
class gbtNfgContingencyIterator {
  friend class gbtNfgGame;
private:
  gbtNfgSupport support;
  gbtArray<int> m_currentStrat;
  gbtStrategyProfile profile;
  int m_frozen1, m_frozen2;
  
public:
  /// @name Lifecycle
  //@{
  /// Construct a new iterator on the support, with no strategies held fixed
  gbtNfgContingencyIterator(const gbtNfgSupport &);
  /// Construct a new iterator on the support, fixing player pl's strategy
  gbtNfgContingencyIterator(const gbtNfgSupport &s, int pl, int st);
  /// Construct a new iterator on the support, fixing two players' strategies
  gbtNfgContingencyIterator(const gbtNfgSupport &s, 
			    int pl1, int st1, int pl2, int st2);
  //@}

  /// @name Iteration
  //@{
  /// Reset the iterator to the first contingency (this is called by ctors)
  void First(void);
  /// Advance to the next contingency.  Returns false if already at last one.
  bool NextContingency(void);
  //@}  
  
  /// @name Data access
  //@{
  /// Get the current strategy profile
  const gbtStrategyProfile &GetProfile(void) const { return profile; }
  
  /// Get the outcome assigned to the current contingency
  gbtNfgOutcome GetOutcome(void) const;
  /// Set the outcome assigned to the current contingency
  void SetOutcome(gbtNfgOutcome);
  /// Get the payoff to player 'pl' at the current contingency
  gbtRational GetPayoff(int pl) const;
  //@}
};

#endif   // NFGITER_H




