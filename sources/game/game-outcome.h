//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to outcome representation classes
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

#ifndef GAME_OUTCOME_H
#define GAME_OUTCOME_H

#include "game.h"

class gbtGamePlayerRep;
typedef gbtGameObjectHandle<gbtGamePlayerRep> gbtGamePlayer;

class gbtGameOutcomeRep : public gbtGameObject {
  friend class gbtGameObjectHandle<gbtGameOutcomeRep>;
  friend struct gbt_efg_game_rep;
  friend struct gbt_nfg_game_rep;

public:
  //!
  //! @name General information about the outcome
  //!
  //@{
  /// Returns the ID number of the outcome (unique within a game)
  virtual int GetId(void) const = 0;
  /// Set the text label associated with the player
  virtual void SetLabel(const std::string &) = 0;
  /// Get the text label associated with the player
  virtual std::string GetLabel(void) const = 0;

  /// Returns true if the outcome has been deleted from its game
  virtual bool IsDeleted(void) const = 0;
  //@}

  //!
  //! @name Accessing payoff information about the outcome
  //!
  //@{
  /// Returns the payoff at the outcome to the player
  virtual gbtRational GetPayoff(const gbtGamePlayer &) const = 0;
  /// Returns the payoff at the outcome to all players as a vector
  virtual gbtArray<gbtRational> GetPayoff(void) const = 0;
  /// Sets the payoff at the outcome to the player
  virtual void SetPayoff(const gbtGamePlayer &, const gbtRational &) = 0;
  //@}

  //!
  //! @name Editing the outcomes in a game
  //!
  //@{
  /// Delete the outcome from the game
  virtual void DeleteOutcome(void) = 0;
  //@}
};

typedef gbtGameObjectHandle<gbtGameOutcomeRep> gbtGameOutcome;


#endif  // GAME_OUTCOME_H
