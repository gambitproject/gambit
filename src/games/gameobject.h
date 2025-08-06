//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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

#include <memory>

namespace Gambit {

class GameRep;
using Game = std::shared_ptr<GameRep>;

/// An exception thrown when attempting to dereference a null pointer
class NullException final : public std::runtime_error {
public:
  NullException() : std::runtime_error("Dereferenced null pointer") {}
  explicit NullException(const std::string &s) : std::runtime_error(s) {}
  ~NullException() noexcept override = default;
};

/// An exception thrown when attempting to dereference an invalidated object
class InvalidObjectException final : public std::runtime_error {
public:
  InvalidObjectException() : std::runtime_error("Dereferencing an invalidated object") {}
  ~InvalidObjectException() noexcept override = default;
};

/// A handle object for referring to elements of a game
///
/// This class provides the facilities for safely referencing the objects representing
/// elements of a game (players, outcomes, nodes, and so on).
///
/// This addresses two issues:
///   * Holding a reference to some element of a game counts as holding a reference to the
///     whole game.  This encapsulates such reference counting such that a game is only
///     deallocated when all references to any part of it are deallocated.
///   * Because games are mutable, the element of the game referred to by this class
///     may be removed from the game.  When an element is removed from the game, it is
///     marked as no longer valid.  This class automates the checking of validity when
///     dereferencing the object, and raising an exception when appropriate.
template <class T> class GameObjectPtr {
  std::shared_ptr<T> m_rep;
  Game m_game;

public:
  GameObjectPtr() = default;
  GameObjectPtr(std::nullptr_t r) : m_rep(r), m_game(nullptr) {}
  GameObjectPtr(std::shared_ptr<T> r) : m_rep(r), m_game((r) ? r->GetGame() : nullptr) {}
  GameObjectPtr(const GameObjectPtr<T> &) = default;
  ~GameObjectPtr() = default;

  GameObjectPtr<T> &operator=(const GameObjectPtr<T> &) = default;

  /// Access the shared pointer to the object representing the game element
  ///
  /// Returns the shared pointer to the game element.  Checks for the validity of the
  /// game element held by this object and throws an exception if the object is the
  /// null object, or is no longer valid (has been removed from the game)
  ///
  /// @exception NullException if the object holds a reference to a null element
  /// @exception InvalidObjectException if the element referred to has been deleted from its game
  std::shared_ptr<T> operator->() const
  {
    if (!m_rep) {
      throw NullException();
    }
    if (!m_rep->IsValid()) {
      throw InvalidObjectException();
    }
    return m_rep;
  }

  /// Access the shared pointer to the object representing the game element
  ///
  /// Returns the shared pointer to the game element.  Checks for the validity of the
  /// game element held by this object and throws an exception if the object is the
  /// null object, or is no longer valid (has been removed from the game)
  ///
  /// @exception NullException if the object holds a reference to a null element
  /// @exception InvalidObjectException if the element referred to has been deleted from its game
  std::shared_ptr<T> get_shared() const
  {
    if (!m_rep) {
      throw NullException();
    }
    if (!m_rep->IsValid()) {
      throw InvalidObjectException();
    }
    return m_rep;
  }

  /// Access the raw pointer to the object representing the game element
  ///
  /// Returns the raw pointer to the game element.  Checks for the validity of the
  /// game element held by this object and throws an exception if the object is the
  /// null object, or is no longer valid (has been removed from the game)
  ///
  /// @exception NullException if the object holds a reference to a null element
  /// @exception InvalidObjectException if the element referred to has been deleted from its game
  T *get() const
  {
    if (!m_rep) {
      throw NullException();
    }
    if (!m_rep->IsValid()) {
      throw InvalidObjectException();
    }
    return m_rep.get();
  }

  bool operator==(const GameObjectPtr<T> &r) const { return (m_rep == r.m_rep); }
  bool operator==(const std::shared_ptr<T> r) const { return (m_rep == r); }
  bool operator==(const std::shared_ptr<const T> r) const { return (m_rep == r); }
  bool operator==(const std::nullptr_t) const { return !bool(m_rep); }
  bool operator!=(const GameObjectPtr<T> &r) const { return (m_rep != r.m_rep); }
  bool operator!=(const std::shared_ptr<T> r) const { return (m_rep != r); }
  bool operator!=(const std::shared_ptr<const T> r) const { return (m_rep != r); }
  bool operator!=(const std::nullptr_t) const { return bool(m_rep); }
  bool operator<(const GameObjectPtr<T> &r) const { return (m_rep < r.m_rep); }

  operator bool() const noexcept { return bool(m_rep); }
  operator std::shared_ptr<T>() const { return m_rep; }
};

template <class P, class T> class ElementCollection {
  P m_owner{nullptr};
  const std::vector<std::shared_ptr<T>> *m_container{nullptr};

public:
  class iterator {
    P m_owner{nullptr};
    const std::vector<std::shared_ptr<T>> *m_container{nullptr};
    size_t m_index{0};

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = GameObjectPtr<T>;
    using pointer = value_type *;
    using reference = value_type &;

    iterator() = default;
    iterator(const P &p_owner, const std::vector<std::shared_ptr<T>> *p_container,
             size_t p_index = 0)
      : m_owner(p_owner), m_container(p_container), m_index(p_index)
    {
    }
    iterator(const iterator &) = default;
    ~iterator() = default;
    iterator &operator=(const iterator &) = default;

    bool operator==(const iterator &p_iter) const
    {
      return m_owner == p_iter.m_owner && m_container == p_iter.m_container &&
             m_index == p_iter.m_index;
    }
    bool operator!=(const iterator &p_iter) const
    {
      return m_owner != p_iter.m_owner || m_container != p_iter.m_container ||
             m_index != p_iter.m_index;
    }

    iterator &operator++()
    {
      m_index++;
      return *this;
    }
    iterator &operator--()
    {
      m_index--;
      return *this;
    }
    value_type operator*() const { return m_container->at(m_index); }

    inline const P &GetOwner() const { return m_owner; }
  };

  ElementCollection() = default;
  explicit ElementCollection(const P &p_owner, const std::vector<std::shared_ptr<T>> *p_container)
    : m_owner(p_owner), m_container(p_container)
  {
  }
  ElementCollection(const ElementCollection<P, T> &) = default;
  ~ElementCollection() = default;
  ElementCollection &operator=(const ElementCollection<P, T> &) = default;

  bool operator==(const ElementCollection<P, T> &p_other) const
  {
    return m_owner == p_other.m_owner && m_container == p_other.m_container;
  }
  size_t size() const { return m_container->size(); }
  GameObjectPtr<T> front() const { return m_container->front(); }
  GameObjectPtr<T> back() const { return m_container->back(); }

  iterator begin() const { return {m_owner, m_container, 0}; }
  iterator end() const { return {m_owner, m_container, (m_owner) ? m_container->size() : 0}; }
  iterator cbegin() const { return {m_owner, m_container, 0}; }
  iterator cend() const { return {m_owner, m_container, (m_owner) ? m_container->size() : 0}; }
};

} // end namespace Gambit

#endif // GAMBIT_GAMES_GAMEOBJECT_H
