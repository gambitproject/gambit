//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of class to represent a normal form contingency
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

#ifndef GAME_CONTINGENCY_H
#define GAME_CONTINGENCY_H

//!
//! Abstract class representing a contingency, i.e., a list of
//! strategies, one per player in the game.
//!
//! New contingencies are created using gbtGameRep::NewContingency()
//! Contingencies are manipulated using the gbtGameSingleHandle idiom.
//!
class gbtNfgContingencyRep  {
  friend class gbtGameSingleHandle<gbtNfgContingencyRep>;

public:
  virtual ~gbtNfgContingencyRep() { }
  
  /// Create a copy of the contingency.  Equivalent to a copy constructor.
  virtual gbtNfgContingencyRep *Copy(void) const = 0;

  /// Return the strategy specified for the player
  virtual gbtGameStrategy GetStrategy(const gbtGamePlayer &) const = 0;

  /// Set the strategy for a player.
  virtual void SetStrategy(const gbtGameStrategy &) = 0;

  /// Get the outcome associated with the contingency, if defined.
  virtual void SetOutcome(const gbtGameOutcome &) = 0;

  /// Set the outcome associated with the contingency, if defined.
  virtual gbtGameOutcome GetOutcome(void) const = 0;

  /// Returns the payoff to a player at the contingency.
  virtual gbtRational GetPayoff(const gbtGamePlayer &) const = 0;
};

typedef gbtGameSingleHandle<gbtNfgContingencyRep> gbtNfgContingency;

#endif  // GAME_CONTINGENCY_H
