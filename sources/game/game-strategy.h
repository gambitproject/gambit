//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of class to represent a normal form (pure) strategy
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

#ifndef GAME_STRATEGY_H
#define GAME_STRATEGY_H

#include "game.h"

class gbtGamePlayerRep;
typedef gbtGameObjectHandle<gbtGamePlayerRep> gbtGamePlayer;

class gbtGameStrategyRep : public gbtGameObject {
  friend class gbtGameObjectHandle<gbtGameStrategyRep>;

public:
  //!
  //! @name General information about the strategy
  //!
  //@{
  /// Returns the ID number of the strategy (unique for a player)
  virtual int GetId(void) const = 0;
  /// Set the text label associated with the strategy
  virtual void SetLabel(const std::string &) = 0;
  /// Get the text label associated with the strategy
  virtual std::string GetLabel(void) const = 0;
  //@}

  //!
  //! @name Accessing information about the player
  //!
  //@{
  /// Returns the player to whom the strategy belongs
  virtual gbtGamePlayer GetPlayer(void) const = 0;
  /// Returns the action chosen at the information set in the strategy
  virtual gbtGameAction GetBehavior(const gbtGameInfoset &) const = 0;
  //@}

  //!
  //! @name Dominance properties
  //!
  //@{
  /// Returns true if this strategy dominates the given strategy
  virtual bool Dominates(const gbtGameStrategy &, bool p_strict) const = 0;
  /// Returns true if this strategy is dominated by another pure strategy
  virtual bool IsDominated(bool p_strict) const = 0;
  //@}
};


typedef gbtGameObjectHandle<gbtGameStrategyRep> gbtGameStrategy;

#endif  // GAME_STRATEGY_H
