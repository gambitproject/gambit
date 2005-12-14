//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of base class for representing games
//
// This file is part of Gambit
// Copyright (c) 2005, The Gambit Project
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


#ifndef LIBGAMBIT_GAME_H
#define LIBGAMBIT_GAME_H


/// This is a base class for all game-related objects.  Primary among
/// its responsibility is maintaining a reference count.  Calling code
/// which maintains pointers to objects representing parts of a game
/// (e.g., nodes) which may be deleted should increment the reference
/// count for that object.  The library guarantees that any object
/// with a positive reference count will not have its memory deleted,
/// but will instead be marked as deleted.  Calling code should always
/// be careful to check the deleted status of the object before any
/// operations on it.
class gbtGameObject {
protected:
  int m_refCount;
  bool m_valid;

public:
  /// @name Lifecycle
  //@{
  /// Constructor; initializes reference count
  gbtGameObject(void) : m_refCount(0), m_valid(true) { }
  //@}

  /// @name Validation
  //@{
  /// Is the object still valid?
  bool IsValid(void) const { return m_valid; }
  /// Invalidate the object; delete if not referenced elsewhere
  void Invalidate(void)
  { if (!m_refCount) delete this; else m_valid = false; }
  //@}

  /// @name Reference counting
  //@{
  /// Increment the reference count
  void IncRef(void) { m_refCount++; }
  /// Decrement the reference count; delete if reference count is zero.
  void DecRef(void) { if (!--m_refCount && !m_valid) delete this; }
  //@}
};


//
// This is a base class for representing an arbitrary finite game.
//
class gbtGame {
public:
  /// Get the text label associated with the game
  virtual const std::string &GetTitle(void) const = 0;
  /// Set the text label associated with the game
  virtual void SetTitle(const std::string &s) = 0;

  /// Get the text comment associated with the game
  virtual const std::string &GetComment(void) const = 0;
  /// Set the text comment associated with the game
  virtual void SetComment(const std::string &) = 0;

  /// Returns true if the game is constant-sum
  virtual bool IsConstSum(void) const = 0; 
  /// Returns the smallest payoff in any outcome of the game
  virtual gbtRational GetMinPayoff(int pl = 0) const = 0;
  /// Returns the largest payoff in any outcome of the game
  virtual gbtRational GetMaxPayoff(int pl = 0) const = 0;
};



#endif   // LIBGAMBIT_GAME_H
