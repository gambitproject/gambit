//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of class to represent a sequence form sequence
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

#ifndef GAME_SEQUENCE_H
#define GAME_SEQUENCE_H

#include "game.h"

class gbtGamePlayerRep;
typedef gbtGameObjectHandle<gbtGamePlayerRep> gbtGamePlayer;

class gbtGameSequenceRep : public gbtGameObject {
  friend class gbtGameObjectHandle<gbtGameSequenceRep>;

public:
  //!
  //! @name General information about the sequence
  //!
  //@{
  /// Returns the ID number of the sequence (unique for a player)
  virtual int GetId(void) const = 0;
  /// Set the text label associated with the sequence
  virtual void SetLabel(const std::string &) = 0;
  /// Get the text label associated with the sequence
  virtual std::string GetLabel(void) const = 0;
  //@}

  //!
  //! @name Accessing information about the player
  //!
  //@{
  /// Returns the player to whom the sequence belongs
  virtual gbtGamePlayer GetPlayer(void) const = 0;
  /// Returns the action defining the sequence
  virtual gbtGameAction GetAction(void) const = 0;
  /// Returns the parent sequence of the sequence
  virtual gbtGameSequence GetParent(void) const = 0;
  /// Determines whether the action is part of the sequence
  virtual bool ContainsAction(const gbtGameAction &) const = 0;
  //@}
};

typedef gbtGameObjectHandle<gbtGameSequenceRep> gbtGameSequence;

#endif  // GAME_SEQUENCE_H
