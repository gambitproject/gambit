//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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

  bool is_in_range(const int p_index) const
  {
    return p_index >= m_offset && p_index <= back_index();
  }

public:
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;
  using value_type = typename std::vector<T>::value_type;
  using reference = typename std::vector<T>::reference;
  using const_reference = typename std::vector<T>::const_reference;

  Array() : m_offset(1) {}
  explicit Array(size_t p_length) : m_offset(1), m_data(p_length) {}
  /// Construct an array with the given index bounds.
  /// If p_high is less than p_low, the resulting array will have
  /// front_index equal to p_low, and size 0.
  explicit Array(const int p_low, const int p_high) : m_offset(p_low)
  {
    if (p_high >= p_low) {
      m_data.resize(p_high - p_low + 1);
    }
  }
  Array(const Array &) = default;
  Array(Array &&) noexcept = default;
  ~Array() = default;

  Array &operator=(const Array &) = default;
  Array &operator=(Array &&) noexcept = default;

  bool operator==(const Array &p_other) const
  {
    return m_offset == p_other.m_offset && m_data == p_other.m_data;
  }
  bool operator!=(const Array &p_other) const { return !(*this == p_other); }

  const_reference operator[](const int p_index) const
  {
    if (!is_in_range(p_index)) {
      throw std::out_of_range("Index out of range in Array");
    }
    return m_data.at(p_index - m_offset);
  }

  reference operator[](const int p_index)
  {
    if (!is_in_range(p_index)) {
      throw std::out_of_range("Index out of range in Array");
    }
    return m_data.at(p_index - m_offset);
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

  [[nodiscard]] bool empty() const noexcept { return m_data.empty(); }
  [[nodiscard]] size_t size() const noexcept { return m_data.size(); }
  [[nodiscard]] int front_index() const noexcept { return m_offset; }
  [[nodiscard]] int back_index() const { return m_offset + m_data.size() - 1; }

  void clear() { m_data.clear(); }
  void insert(const_iterator pos, const T &value) { m_data.insert(pos, value); }
  void erase(iterator pos) { m_data.erase(pos); }
  void erase_at(const int p_index)
  {
    if (!is_in_range(p_index)) {
      throw std::out_of_range("Index out of range in Array");
    }
    erase(std::next(begin(), p_index - front_index()));
  }
  template <class Pred> void remove_if(Pred p)
  {
    auto it = std::remove_if(m_data.begin(), m_data.end(), p);
    m_data.erase(it, m_data.end());
  }

  void push_back(const T &value) { m_data.push_back(value); }
  template <class... Args> T &emplace_back(Args &&...args)
  {
    return m_data.emplace_back(std::forward<Args>(args)...);
  }
  void pop_back() { m_data.pop_back(); }
  void reserve(size_t len) { m_data.reserve(len); }
};

} // end namespace Gambit

#endif // GAMBIT_CORE_ARRAY_H
