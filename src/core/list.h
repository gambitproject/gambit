//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/libgambit/list.h
// A generic linked-list container class
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

#ifndef LIBGAMBIT_LIST_H
#define LIBGAMBIT_LIST_H

#include <list>

namespace Gambit {

/// @brief A linked-list container
///
/// This is a lightweight wrapper around std::list.  It exists as a transitional
/// class between Gambit's legacy linked-list implementation Gambit::List and
/// STL's list.
///
/// Gambit's List container supported the index operator [], which is not defined
/// on STL lists.  Therefore this class provides such an operator by wrapping
/// an appropriate iterator-based operation.  This is very inefficient!  However,
/// the [] operator and indexing is tightly embedded in the remaining uses of this
/// class, so this operator provides a refactoring path while the remaining uses
/// are rewritten.
///
/// Note importantly that the index operator [] is **1-based** - that is, the first
/// element of the list is 1, not 0.
template <class T> class List {
private:
  std::list<T> m_list;

public:
  using iterator = typename std::list<T>::iterator;
  using const_iterator = typename std::list<T>::const_iterator;
  using value_type = typename std::list<T>::value_type;
  using size_type = typename std::list<T>::size_type;

  List() = default;
  List(const List<T> &) = default;
  ~List() = default;

  List<T> &operator=(const List<T> &) = default;

  const T &operator[](size_type p_index) const
  {
    if (p_index < 1 || p_index > m_list.size()) {
      throw IndexException();
    }
    return *std::next(m_list.cbegin(), p_index - 1);
  }

  T &operator[](size_type p_index)
  {
    if (p_index < 1 || p_index > m_list.size()) {
      throw IndexException();
    }
    return *std::next(m_list.begin(), p_index - 1);
  }

  /// @name STL-style interface
  ///
  /// These operations forward STL-type operations to the underlying list.
  /// This does not provide all operations on std::list, only ones used in
  /// existing code.  Rather than adding new functions here, existing code
  /// should be rewritten to use std::list directly.
  ///@{
  /// Return whether the list container is empty (has size 0).
  bool empty() const { return m_list.empty(); }
  /// Return the number of elements in the list container.
  size_type size() const { return m_list.size(); }
  /// Adds a new element at the beginning of the list container
  void push_front(const T &val) { m_list.push_front(val); }
  /// Adds a new element at the end of the list container, after its
  /// current last element.
  void push_back(const T &val) { m_list.push_back(val); }
  /// Removes the last element
  void pop_back() { m_list.pop_back(); }
  /// Inserts the value at the specified position
  void insert(iterator pos, const T &value) { m_list.insert(pos, value); }
  /// Erases the element at the specified position
  void erase(iterator pos) { m_list.erase(pos); }

  /// Removes all elements from the list container (which are destroyed),
  /// leaving the container with a size of 0.
  void clear() { m_list.clear(); }
  /// Returns a reference to the first element in the list container.
  T &front() { return m_list.front(); }
  /// Returns a reference to the first element in the list container.
  const T &front() const { return m_list.front(); }
  /// Returns a reference to the last element in the list container.
  T &back() { return m_list.back(); }
  /// Returns a reference to the last element in the list container.
  const T &back() const { return m_list.back(); }

  bool operator==(const List<T> &b) const { return m_list == b.m_list; }
  bool operator!=(const List<T> &b) const { return m_list != b.m_list; }

  /// Return a forward iterator starting at the beginning of the list
  iterator begin() { return m_list.begin(); }
  /// Return a forward iterator past the end of the list
  iterator end() { return m_list.end(); }
  /// Return a const forward iterator starting at the beginning of the list
  const_iterator begin() const { return m_list.begin(); }
  /// Return a const forward iterator past the end of the list
  const_iterator end() const { return m_list.end(); }
  /// Return a const forward iterator starting at the beginning of the list
  const_iterator cbegin() const { return m_list.cbegin(); }
  /// Return a const forward iterator past the end of the list
  const_iterator cend() const { return m_list.cend(); }
  ///@}
};

} // namespace Gambit

#endif // LIBGAMBIT_LIST_H
