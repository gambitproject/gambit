//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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

#include <vector>
#include <iterator>

namespace Gambit {

/// \brief An extension of std::vector to have arbitrary offset index
///
/// This is a variation on std::vector, which allows for the index of the first
/// element to be something other than zero.
template <class T> class Array {
protected:
  int m_offset, m_length;
  T *m_data;

  int InsertAt(const T &t, int n)
  {
    if (this->m_offset > n || n > this->last_index() + 1) {
      throw IndexException();
    }

    T *new_data = new T[++this->m_length] - this->m_offset;

    int i;
    for (i = this->m_offset; i <= n - 1; i++) {
      new_data[i] = this->m_data[i];
    }
    new_data[i++] = t;
    for (; i <= this->last_index(); i++) {
      new_data[i] = this->m_data[i - 1];
    }

    if (this->m_data) {
      delete[] (this->m_data + this->m_offset);
    }
    this->m_data = new_data;

    return n;
  }

  T Remove(int n)
  {
    if (n < this->m_offset || n > this->last_index()) {
      throw IndexException();
    }

    T ret(this->m_data[n]);
    T *new_data = (--this->m_length > 0) ? new T[this->m_length] - this->m_offset : nullptr;

    int i;
    for (i = this->m_offset; i < n; i++) {
      new_data[i] = this->m_data[i];
    }
    for (; i <= this->last_index(); i++) {
      new_data[i] = this->m_data[i + 1];
    }

    delete[] (this->m_data + this->m_offset);
    this->m_data = new_data;

    return ret;
  }

public:
  class iterator {
    friend class Array;

  private:
    Array *m_array;
    int m_index;

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = value_type *;
    using reference = value_type &;

    iterator(Array *p_array = 0, int p_index = 0) : m_array(p_array), m_index(p_index) {}
    reference operator*() { return (*m_array)[m_index]; }
    pointer operator->() { return &(*m_array)[m_index]; }
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
    iterator operator++(int)
    {
      auto ret = *this;
      m_index++;
      return ret;
    }
    bool operator==(const iterator &it) const
    {
      return (m_array == it.m_array) && (m_index == it.m_index);
    }
    bool operator!=(const iterator &it) const { return !(*this == it); }
  };

  class const_iterator {
    friend class Array;

  private:
    const Array *m_array;
    int m_index;

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = value_type *;
    using reference = value_type &;

    const_iterator(const Array *p_array, int p_index) : m_array(p_array), m_index(p_index) {}
    const T &operator*() const { return (*m_array)[m_index]; }
    const T &operator->() const { return (*m_array)[m_index]; }
    const_iterator &operator++()
    {
      m_index++;
      return *this;
    }
    const_iterator &operator--()
    {
      m_index--;
      return *this;
    }
    const_iterator operator++(int)
    {
      auto ret = *this;
      m_index++;
      return ret;
    }
    bool operator==(const const_iterator &it) const
    {
      return (m_array == it.m_array) && (m_index == it.m_index);
    }
    bool operator!=(const const_iterator &it) const { return !(*this == it); }
  };

  explicit Array(unsigned int len = 0)
    : m_offset(1), m_length(len), m_data((len) ? new T[len] - 1 : nullptr)
  {
  }
  Array(int lo, int hi) : m_offset(lo), m_length(hi - lo + 1)
  {
    if (m_length < 0) {
      throw RangeException();
    }
    m_data = (m_length > 0) ? new T[m_length] - m_offset : nullptr;
  }
  Array(const Array<T> &a)
    : m_offset(a.m_offset), m_length(a.m_length),
      m_data((m_length > 0) ? new T[m_length] - m_offset : nullptr)
  {
    for (int i = m_offset; i <= last_index(); i++) {
      m_data[i] = a.m_data[i];
    }
  }
  virtual ~Array()
  {
    if (m_length > 0) {
      delete[] (m_data + m_offset);
    }
  }

  Array<T> &operator=(const Array<T> &a)
  {
    if (this != &a) {
      // We only reallocate if necessary.
      if (!m_data || (m_data && (m_offset != a.m_offset || m_length != a.m_length))) {
        if (m_data) {
          delete[] (m_data + m_offset);
        }
        m_offset = a.m_offset;
        m_length = a.m_length;
        m_data = (m_length > 0) ? new T[m_length] - m_offset : nullptr;
      }

      for (int i = m_offset; i <= last_index(); i++) {
        m_data[i] = a.m_data[i];
      }
    }

    return *this;
  }

  bool operator==(const Array<T> &a) const
  {
    if (m_offset != a.m_offset || m_length != a.m_length) {
      return false;
    }
    for (int i = m_offset; i <= last_index(); i++) {
      if ((*this)[i] != a[i]) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(const Array<T> &a) const { return !(*this == a); }

  const T &operator[](int index) const
  {
    if (index < m_offset || index > last_index()) {
      throw IndexException();
    }
    return m_data[index];
  }

  T &operator[](int index)
  {
    if (index < m_offset || index > last_index()) {
      throw IndexException();
    }
    return m_data[index];
  }
  const T &front() const { return m_data[m_offset]; }
  T &front() { return m_data[m_offset]; }
  const T &back() const { return m_data[last_index()]; }
  T &back() { return m_data[last_index()]; }

  iterator begin() { return {this, m_offset}; }
  const_iterator begin() const { return {this, m_offset}; }
  iterator end() { return {this, m_offset + m_length}; }
  const_iterator end() const { return {this, m_offset + m_length}; }
  const_iterator cbegin() const { return {this, m_offset}; }
  const_iterator cend() const { return {this, m_offset + m_length}; }

  bool empty() const { return this->m_length == 0; }
  size_t size() const { return m_length; }
  int first_index() const { return m_offset; }
  int last_index() const { return m_offset + m_length - 1; }

  void clear()
  {
    if (m_length > 0) {
      delete[] (m_data + m_offset);
    }
    m_data = nullptr;
    m_length = 0;
  }
  void insert(const_iterator pos, const T &value) { InsertAt(value, pos.m_index); }
  void erase(iterator pos) { Remove(pos.m_index); }
  void push_back(const T &val) { InsertAt(val, this->last_index() + 1); }
  void pop_back() { Remove(last_index()); }
};

/// Convenience function to erase the element at `p_index`
template <class T> void erase_atindex(Array<T> &p_array, int p_index)
{
  p_array.erase(std::next(p_array.begin(), p_index - p_array.first_index()));
}

} // end namespace Gambit

#endif // LIBGAMBIT_ARRAY_H
