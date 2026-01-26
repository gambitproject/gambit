//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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
  /// @exception InvalidObjectException if the element referred to has been deleted from its game
  std::shared_ptr<T> get_shared() const
  {
    if (m_rep && !m_rep->IsValid()) {
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
  bool operator==(const std::shared_ptr<T> &r) const { return (m_rep == r); }
  bool operator==(const std::shared_ptr<const T> &r) const { return (m_rep == r); }
  bool operator==(const std::nullptr_t &) const { return m_rep == nullptr; }
  bool operator!=(const GameObjectPtr<T> &r) const { return (m_rep != r.m_rep); }
  bool operator!=(const std::shared_ptr<T> &r) const { return (m_rep != r); }
  bool operator!=(const std::shared_ptr<const T> &r) const { return (m_rep != r); }
  bool operator!=(const std::nullptr_t &) const { return m_rep != nullptr; }
  bool operator<(const GameObjectPtr<T> &r) const { return (m_rep < r.m_rep); }

  operator bool() const noexcept { return m_rep != nullptr; }
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

    const P &GetOwner() const { return m_owner; }
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

/// @brief A view on a nested collection of objects (e.g. infosets of players or strategies of
/// players)
template <class T, auto OuterMemFn, auto InnerMemFn> class NestedElementCollection {
  T m_owner;

public:
  class iterator {
    using OuterRange = decltype((m_owner.get()->*OuterMemFn)());
    using OuterIter = decltype(std::begin(std::declval<OuterRange &>()));

    using OuterElem = decltype(*std::declval<OuterIter>());
    using InnerRange = decltype((std::declval<OuterElem>().get()->*InnerMemFn)());
    using InnerIter = decltype(std::begin(std::declval<InnerRange &>()));

    T m_iterOwner;
    OuterRange m_outerRange;
    OuterIter m_outerIt, m_outerEnd;
    InnerRange m_innerRange;
    InnerIter m_innerIt, m_innerEnd;

    void update_inner()
    {
      if (m_outerIt != m_outerEnd) {
        m_innerRange = ((*m_outerIt).get()->*InnerMemFn)();
        m_innerIt = m_innerRange.begin();
        m_innerEnd = m_innerRange.end();
      }
    }

    void skip_empty()
    {
      while (m_outerIt != m_outerEnd && m_innerIt == m_innerEnd) {
        ++m_outerIt;
        if (m_outerIt != m_outerEnd) {
          update_inner();
        }
      }
    }

  public:
    using iterator_category = std::forward_iterator_tag;
    using reference = decltype(*std::declval<InnerIter>());
    using pointer = std::add_pointer_t<reference>;
    using value_type = std::remove_reference_t<reference>;
    using difference_type = std::ptrdiff_t;

    iterator() = default;

    iterator(T p_owner, const bool p_isEnd)
      : m_iterOwner(p_owner), m_outerRange((m_iterOwner.get()->*OuterMemFn)()),
        m_outerIt(std::begin(m_outerRange)), m_outerEnd(std::end(m_outerRange))
    {
      if (p_isEnd) {
        m_outerIt = m_outerEnd;
        return;
      }
      if (m_outerIt != m_outerEnd) {
        update_inner();
      }
      skip_empty();
    }

    reference operator*() const { return *m_innerIt; }
    pointer operator->() const { return *m_innerIt; }

    iterator &operator++()
    {
      ++m_innerIt;
      skip_empty();
      return *this;
    }
    iterator operator++(int)
    {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const iterator &p_other) const
    {
      return m_outerIt == p_other.m_outerIt &&
             (m_outerIt == m_outerEnd || m_innerIt == p_other.m_innerIt);
    }
    bool operator!=(const iterator &p_other) const { return !(*this == p_other); }
  };

  explicit NestedElementCollection(T owner) : m_owner(owner) {}

  /// @brief Return the total number of elements summed across all inner ranges
  std::size_t size() const
  {
    auto outer = (m_owner.get()->*OuterMemFn)();
    return std::accumulate(outer.begin(), outer.end(), static_cast<std::size_t>(0),
                           [](std::size_t acc, const auto &element) {
                             return acc + (element.get()->*InnerMemFn)().size();
                           });
  }

  /// @brief Returns the shape, a vector of the sizes of the inner ranges
  std::vector<std::size_t> shape() const
  {
    std::vector<std::size_t> result;
    auto outer = (m_owner.get()->*OuterMemFn)();
    result.reserve(outer.size());
    std::transform(outer.begin(), outer.end(), std::back_inserter(result),
                   [](const auto &element) { return (element.get()->*InnerMemFn)().size(); });
    return result;
  }

  /// @brief Returns the shape, a Gambit Array of the sizes of the inner ranges
  /// @deprecated This is for backwards compatibility; uses should be migrated to shape().
  Array<std::size_t> shape_array() const
  {
    Array<std::size_t> result;
    auto outer = (m_owner.get()->*OuterMemFn)();
    result.reserve(outer.size());
    std::transform(outer.begin(), outer.end(), std::back_inserter(result),
                   [](const auto &element) { return (element.get()->*InnerMemFn)().size(); });
    return result;
  }

  /// @brief Returns the Cartesian product of the sizes of the inner ranges
  std::size_t extent_product() const
  {
    auto outer = (m_owner.get()->*OuterMemFn)();
    return std::accumulate(outer.begin(), outer.end(), static_cast<std::size_t>(1),
                           [](std::size_t acc, const auto &element) {
                             return acc * (element.get()->*InnerMemFn)().size();
                           });
  }

  iterator begin() const { return {m_owner, false}; }
  iterator end() const { return {m_owner, true}; }
};

} // end namespace Gambit

#endif // GAMBIT_GAMES_GAMEOBJECT_H
