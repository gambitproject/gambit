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

/// An exception thrown when attempting to dereference an invalidated object
class InvalidObjectException : public Exception {
public:
  ~InvalidObjectException() noexcept override = default;

  const char *what() const noexcept override { return "Dereferencing an invalidated object"; }
};

template <class T> class GameObjectPtr {
  std::shared_ptr<T> m_rep;

public:
  GameObjectPtr() = default;
  GameObjectPtr(std::nullptr_t r) : m_rep(r) {}
  GameObjectPtr(std::shared_ptr<T> r) : m_rep(r) {}
  GameObjectPtr(const GameObjectPtr<T> &) = default;
  ~GameObjectPtr() = default;

  GameObjectPtr<T> &operator=(const GameObjectPtr<T> &) = default;

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
