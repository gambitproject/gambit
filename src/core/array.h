//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/array.h
// A basic bounds-checked array type
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

#ifndef LIBGAMBIT_ARRAY_H
#define LIBGAMBIT_ARRAY_H

#include <iostream>

#include <vector>
#include <iterator>

namespace Gambit {

/// A basic bounds-checked array
template <class T> class Array {
protected:
  int m_offset;
  std::vector<T> m_data;

public:
  using size_type = typename std::vector<T>::size_type;
  using reference = typename std::vector<T>::reference;
  using const_reference = typename std::vector<T>::const_reference;
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;
  using reverse_iterator = typename std::vector<T>::reverse_iterator;
  using const_reverse_iterator = typename std::vector<T>::const_reverse_iterator;

  explicit Array(size_t len = 0) : m_offset(1), m_data(len) {}
  Array(int first_index, int last_index) : m_offset(first_index)
  {
    if (last_index < first_index - 1) {
      throw RangeException();
    }
    m_data.resize(last_index - first_index + 1);
  }
  Array(const Array<T> &) = default;
  virtual ~Array() = default;
  Array<T> &operator=(const Array<T> &) = default;

  bool operator==(const Array<T> &a) const { return m_offset == a.m_offset && m_data == a.m_data; }
  bool operator!=(const Array<T> &a) const { return m_offset != a.m_offset || m_data != a.m_data; }

  int Length() const { return m_data.size(); }
  int First() const { return m_offset; }
  int Last() const { return m_offset + static_cast<int>(m_data.size()) - 1; }

  iterator begin() { return m_data.begin(); }
  const_iterator begin() const { return m_data.begin(); }
  const_iterator cbegin() const { return m_data.cbegin(); }

  iterator end() { return m_data.end(); }
  const_iterator end() const { return m_data.end(); }
  const_iterator cend() const { return m_data.cend(); }

  reverse_iterator rbegin() { return m_data.rbegin(); }
  const_reverse_iterator rbegin() const { return m_data.rbegin(); }
  const_reverse_iterator crbegin() const { return m_data.crbegin(); }

  reverse_iterator rend() { return m_data.rend(); }
  const_reverse_iterator rend() const { return m_data.rend(); }
  const_reverse_iterator crend() const { return m_data.crend(); }

  const_reference operator[](int index) const
  {
    if (index < First() || index > Last()) {
      throw IndexException();
    }
    return m_data.at(index - m_offset);
  }
  reference operator[](int index)
  {
    if (index < First() || index > Last()) {
      throw IndexException();
    }
    return m_data.at(index - m_offset);
  }

  const_reference front() const { return m_data.front(); }
  reference front() { return m_data.front(); }
  const_reference back() const { return m_data.back(); }
  reference back() { return m_data.back(); }

  bool empty() const { return m_data.empty(); }
  size_type size() const { return m_data.size(); }
  size_type max_size() const { return m_data.max_size(); }
  void reserve(size_type new_cap) { m_data.reserve(new_cap); }
  size_type capacity() const { return m_data.capacity(); }
  void shrink_to_fit() { m_data.shrink_to_fit(); }

  void clear() { m_data.clear(); }
  iterator insert(const_iterator pos, const T &value) { return m_data.insert(pos, value); }
  void erase(iterator pos) { m_data.erase(pos); }
  void push_back(const T &val) { m_data.push_back(val); }
  void pop_back() { m_data.pop_back(); }

  /// Return the index at which a given element resides in the array.
  int Find(const T &t) const
  {
    int i = m_offset;
    for (const auto &value : m_data) {
      if (value == t) {
        return i;
      }
      i++;
    }
    return m_offset - 1;
  }
  void Insert(const T &t, int n)
  {
    if (n < First()) {
      throw IndexException();
    }
    if (n > Last()) {
      m_data.push_back(t);
    }
    else {
      auto pos = std::next(m_data.begin(), n - m_offset);
      m_data.insert(pos, t);
    }
  }
  T Remove(int n)
  {
    if (n < First() || n > Last()) {
      throw IndexException();
    }
    auto pos = std::next(m_data.begin(), n - m_offset);
    std::cout << "removing\n";
    T ret = *pos;
    m_data.erase(pos);
    return ret;
  }
};

} // end namespace Gambit

#endif // LIBGAMBIT_ARRAY_H
