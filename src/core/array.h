//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/core/array.h
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

#ifndef GAMBIT_CORE_ARRAY_H
#define GAMBIT_CORE_ARRAY_H

#include <vector>
#include <iterator>

#include "util.h"

namespace Gambit {

/// An extension of std::vector with an arbitrary offset index
///
/// This is a variation on std::vector, which allows for the index of the first
/// element to be something other than zero.
template <class T> class Array {
protected:
  int m_offset;
  std::vector<T> m_data;

public:
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;
  using reference = typename std::vector<T>::reference;
  using const_reference = typename std::vector<T>::const_reference;

  explicit Array(size_t len = 0) : m_offset(1), m_data(len) {}
  Array(int lo, int hi) : m_offset(lo), m_data(hi - lo + 1) {}
  Array(const Array<T> &) = default;
  ~Array() = default;

  Array<T> &operator=(const Array<T> &) = default;

  bool operator==(const Array<T> &a) const { return m_offset == a.m_offset && m_data == a.m_data; }

  bool operator!=(const Array<T> &a) const { return m_offset != a.m_offset || m_data != a.m_data; }

  const_reference operator[](int index) const
  {
    if (index < m_offset || index > back_index()) {
      throw std::out_of_range("Index out of range in Array");
    }
    return m_data.at(index - m_offset);
  }

  reference operator[](int index)
  {
    if (index < m_offset || index > back_index()) {
      throw std::out_of_range("Index out of range in Array");
    }
    return m_data[index - m_offset];
  }
  const T &front() const { return m_data.front(); }
  T &front() { return m_data.front(); }
  const T &back() const { return m_data.back(); }
  T &back() { return m_data.back(); }

  iterator begin() { return m_data.begin(); }
  const_iterator begin() const { return m_data.begin(); }
  iterator end() { return m_data.end(); }
  const_iterator end() const { return m_data.end(); }
  const_iterator cbegin() const { return m_data.cbegin(); }
  const_iterator cend() const { return m_data.cend(); }

  bool empty() const { return m_data.empty(); }
  size_t size() const { return m_data.size(); }
  int front_index() const { return m_offset; }
  int back_index() const { return m_offset + m_data.size() - 1; }

  void clear() { m_data.clear(); }
  void insert(const_iterator pos, const T &value) { m_data.insert(pos, value); }
  void erase(iterator pos) { m_data.erase(pos); }
  void push_back(const T &value) { m_data.push_back(value); }
  void pop_back() { m_data.pop_back(); }
};

/// Convenience function to erase the element at `p_index`
template <class T> void erase_atindex(Array<T> &p_array, int p_index)
{
  p_array.erase(std::next(p_array.begin(), p_index - p_array.front_index()));
}

} // end namespace Gambit

#endif // GAMBIT_CORE_ARRAY_H
