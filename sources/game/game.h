//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to game representation library
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

#ifndef GAME_H
#define GAME_H

#include "base/base.h"

class gbtGameObjectDeleted : public gbtException {
public:
  virtual ~gbtGameObjectDeleted() { }

  gbtText Description(void) const
    { return "Game object deleted"; }
};

//
// A base class for all game object types, providing interfaces common
// to all.
//
class gbtGameObject {
public:
  virtual ~gbtGameObject() { }

  virtual gbtText GetLabel(void) const = 0;
  virtual void SetLabel(const gbtText &) = 0;
};

//
// An action at a move
//
class gbtGameAction : public gbtGameObject {
public:
  virtual ~gbtGameAction() { }

  virtual int GetId(void) const = 0;
};

//
// An information set: a move made by a player under certain information
// conditions
//
class gbtGameInfoset : public gbtGameObject {
public:
  virtual ~gbtGameInfoset() { }
};

//
// A player in a game
//
class gbtGamePlayer : public gbtGameObject {
public:
  virtual ~gbtGamePlayer() { }

  virtual int NumInfosets(void) const = 0;
};

//
// An outcome in a game
//
class gbtGameOutcome : public gbtGameObject {
public:
  virtual ~gbtGameOutcome() { }
};

//
// This is an abstract base class that represents a game.  It defines
// all the operations expected to be provided on games.  Some operations
// may not make sense with some derived classes (e.g., are undefined);
// in these cases, exceptions are thrown.
//
class gbtGame : public gbtGameObject {
public:
  virtual ~gbtGame() { }

  // DATA ACCESS -- GENERAL INFORMATION
  virtual gbtText GetComment(void) const = 0;
  virtual void SetComment(const gbtText &) = 0;
  
  virtual bool IsConstSum(void) const = 0;

  // DATA ACCESS -- PLAYERS
  virtual int NumPlayers(void) const = 0;

  // DATA ACCESS -- OUTCOMES
  virtual int NumOutcomes(void) const = 0;
};


#endif   // GAME_H
