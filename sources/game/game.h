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

class gbtGameUndefinedOperation : public gbtException {
public:
  virtual ~gbtGameUndefinedOperation() { }

  gbtText Description(void) const
    { return "Undefined operation"; } 
};

//
// A base class for all game object types, providing interfaces common
// to all.
//
class gbtGameObject {
protected:
  int m_refCount;

  void Reference(void) { m_refCount++; }
  bool Dereference(void) { return (--m_refCount == 0); }

public:
  gbtGameObject(void) : m_refCount(1) { }
  virtual ~gbtGameObject() { }

  virtual gbtText GetLabel(void) const = 0;
  virtual void SetLabel(const gbtText &) = 0;
};

#endif   // GAME_H
