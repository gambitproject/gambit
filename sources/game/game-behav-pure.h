//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Definition of pure behavior profile for trees
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

#ifndef GAME_BEHAV_PURE_H
#define GAME_BEHAV_PURE_H

#include "game.h"

class gbtGameBehavContingencyRep : public gbtGameObject  {
public:
  //!
  //! @name Constructors and destructor
  //!
  //@{
  /// Destructor
  virtual ~gbtGameBehavContingencyRep() { }
  /// Create a copy of the contingency.
  virtual gbtGameBehavContingencyRep *Copy(void) const = 0;
  //@}
  
  //!
  //! @name Accessing the state
  //!
  //@{
  /// Return the action specified at the information set
  virtual gbtGameAction GetAction(const gbtGameInfoset &) const = 0;

  /// Set the action at an information set
  virtual void SetAction(const gbtGameAction &) = 0;

  /// Returns the payoff to a player at the contingency
  virtual gbtRational GetPayoff(const gbtGamePlayer &) const = 0;
  /// Compute the probability the node is reached, given the contingency
  virtual gbtRational GetRealizProb(const gbtGameNode &) const = 0;

  //@}

};

typedef
gbtGameSingleHandle<gbtGameBehavContingencyRep> gbtGameBehavContingency;

//!
//! This class sequentially visits all the behavior contingencies
//! of a game.
//!
class gbtGameBehavProfileIteratorRep : public gbtGameObject {
public:
  //!
  //! @name Constructors and destructor
  //!
  //@[
  /// Destructor
  virtual ~gbtGameBehavProfileIteratorRep() { }
  /// Creates a copy of the iterator
  virtual gbtGameBehavProfileIteratorRep *Copy(void) const = 0;
  //@}

  //!
  //! @name Iteration
  //!
  //@{
  /// Sets the iterator to the first contingency
  virtual void First(void) = 0;
  /// Advances the iterator to the next contingency; returns true if done
  virtual bool NextContingency(void) = 0;
  //@}

  //!
  //! @name Accessing the state
  //!
  //@{
  /// Get the action at the information set for the current contingency
  virtual gbtGameAction GetAction(const gbtGameInfoset &) const = 0;

  /// Get the payoff to the player at the current contingency
  virtual gbtRational GetPayoff(const gbtGamePlayer &) const = 0;
  /// Compute the probability the node is reached, given the contingency
  virtual gbtRational GetRealizProb(const gbtGameNode &) const = 0;
  //@}
};

typedef 
gbtGameSingleHandle<gbtGameBehavProfileIteratorRep> gbtGameBehavProfileIterator;

#endif  // GAME_BEHAV_PURE_H
