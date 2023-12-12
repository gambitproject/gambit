//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/game.h
// Declaration of base class for representing games
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

#ifndef GAMBIT_GAMES_GAMEOBJECT_H
#define GAMBIT_GAMES_GAMEOBJECT_H

namespace Gambit {

/// This is a base class for all game-related objects.  Primary among
/// its responsibility is maintaining a reference count.  Calling code
/// which maintains pointers to objects representing parts of a game
/// (e.g., nodes) which may be deleted should increment the reference
/// count for that object.  The library guarantees that any object
/// with a positive reference count will not have its memory deleted,
/// but will instead be marked as deleted.  Calling code should always
/// be careful to check the deleted status of the object before any
/// operations on it.
class GameObject {
protected:
  int m_refCount;
  bool m_valid;

public:
  /// @name Lifecycle
  //@{
  /// Constructor; initializes reference count
  GameObject() : m_refCount(0), m_valid(true)
  {}

  /// Destructor
  virtual ~GameObject() = default;
  //@}

  /// @name Validation
  //@{
  /// Is the object still valid?
  bool IsValid() const
  { return m_valid; }

  /// Invalidate the object; delete if not referenced elsewhere
  void Invalidate()
  { if (!m_refCount) delete this; else m_valid = false; }
  //@}

  /// @name Reference counting
  //@{
  /// Increment the reference count
  void IncRef()
  { m_refCount++; }

  /// Decrement the reference count; delete if reference count is zero.
  void DecRef()
  { if (!--m_refCount && !m_valid) delete this; }

  /// Returns the reference count
  int RefCount() const
  { return m_refCount; }
  //@}
};


class BaseGameRep {
protected:
  int m_refCount;

public:
  /// @name Lifecycle
  //@{
  /// Constructor; initializes reference count
  BaseGameRep() : m_refCount(0)
  {}

  /// Destructor
  virtual ~BaseGameRep() = default;
  //@}

  /// @name Validation
  //@{
  /// Is the object still valid?
  bool IsValid() const
  { return true; }

  /// Invalidate the object; delete if not referenced elsewhere
  void Invalidate()
  { if (!m_refCount) delete this; }
  //@}

  /// @name Reference counting
  //@{
  /// Increment the reference count
  void IncRef()
  { m_refCount++; }

  /// Decrement the reference count; delete if reference count is zero.
  void DecRef()
  { if (!--m_refCount) delete this; }

  /// Returns the reference count
  int RefCount() const
  { return m_refCount; }
  //@}
};

/// An exception thrown when attempting to dereference an invalidated object
class InvalidObjectException : public Exception {
public:
  ~InvalidObjectException() noexcept override = default;

  const char *what() const noexcept override
  { return "Dereferencing an invalidated object"; }
};


//
// This is a handle class that is used by all calling code to refer to
// member objects of games.  It takes care of all the reference-counting
// considerations.
//
template<class T>
class GameObjectPtr {
private:
  T *rep;

public:
  GameObjectPtr(T *r = nullptr) : rep(r)
  { if (rep) rep->IncRef(); }

  GameObjectPtr(const GameObjectPtr<T> &r) : rep(r.rep)
  { if (rep) rep->IncRef(); }

  ~GameObjectPtr()
  { if (rep) rep->DecRef(); }

  GameObjectPtr<T> &operator=(const GameObjectPtr<T> &r)
  {
    if (&r != this) {
      if (rep) rep->DecRef();
      rep = r.rep;
      if (rep) rep->IncRef();
    }
    return *this;
  }

  T *operator->() const
  {
    if (!rep) throw NullException();
    if (!rep->IsValid()) throw InvalidObjectException();
    return rep;
  }

  bool operator==(const GameObjectPtr<T> &r) const
  { return (rep == r.rep); }

  bool operator==(T *r) const
  { return (rep == r); }

  bool operator!=(const GameObjectPtr<T> &r) const
  { return (rep != r.rep); }

  bool operator!=(T *r) const
  { return (rep != r); }

  operator T *() const
  { return rep; }

  bool operator!() const
  { return !rep; }
};

}  // end namespace Gambit

#endif  // GAMBIT_GAMES_GAMEOBJECT_H
