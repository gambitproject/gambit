//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/core/segment.h
// A container segmenter, with versions for array and vector
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

#ifndef GAMBIT_CORE_SEGMENT_H
#define GAMBIT_CORE_SEGMENT_H

#include "vector.h"

namespace Gambit {

template <class Storage> class Segmented {
  Storage m_values;
  Array<size_t> m_offsets;
  Array<size_t> m_shape;

public:
  using value_type = typename Storage::value_type;

  template <class U> class SegmentView {
    U *m_data{nullptr};
    size_t m_size{0};

  public:
    using value_type = U;
    using pointer = U *;
    using reference = U &;
    using iterator = U *;
    using const_iterator = const U *;

    SegmentView() = default;
    SegmentView(U *data, const size_t size) : m_data(data), m_size(size) {}

    size_t size() const { return m_size; }
    bool empty() const { return m_size == 0; }

    U *data() { return m_data; }
    const U *data() const { return m_data; }

    U &operator[](const size_t i) { return m_data[i - 1]; }
    const U &operator[](const size_t i) const { return m_data[i - 1]; }

    iterator begin() { return m_data; }
    iterator end() { return m_data + m_size; }
    const_iterator begin() const { return m_data; }
    const_iterator end() const { return m_data + m_size; }
  };

  template <bool IsConst> class SegmentRange {
    using parent_type = std::conditional_t<IsConst, const Segmented, Segmented>;
    using elem_type = std::conditional_t<IsConst, const value_type, value_type>;
    using view_type = SegmentView<elem_type>;

    parent_type *m_parent{nullptr};

  public:
    class iterator {
      parent_type *m_parent{nullptr};
      std::size_t m_index{1};

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = view_type;
      using difference_type = std::ptrdiff_t;
      using reference = value_type;

      iterator() = default;
      iterator(parent_type *p_parent, std::size_t p_index) : m_parent(p_parent), m_index(p_index)
      {
      }

      reference operator*() const { return m_parent->segment(m_index); }

      iterator &operator++()
      {
        ++m_index;
        return *this;
      }
      iterator operator++(int)
      {
        auto tmp = *this;
        ++(*this);
        return tmp;
      }

      bool operator==(const iterator &p_other) const
      {
        return m_parent == p_other.m_parent && m_index == p_other.m_index;
      }
      bool operator!=(const iterator &p_other) const { return !(*this == p_other); }
    };

    SegmentRange() = default;
    explicit SegmentRange(parent_type *p_parent) : m_parent(p_parent) {}

    std::size_t size() const { return m_parent->GetShape().size(); }
    bool empty() const { return m_parent->GetShape().empty(); }

    iterator begin() const { return iterator(m_parent, 1); }
    iterator end() const { return iterator(m_parent, size() + 1); }
  };

  Segmented() = delete;
  explicit Segmented(const Array<size_t> &p_shape)
    : m_values(std::accumulate(p_shape.begin(), p_shape.end(), 0)), m_offsets(p_shape.size()),
      m_shape(p_shape)
  {
    for (size_t index = 0, i = 1; i <= m_shape.size(); i++) {
      m_offsets[i] = index;
      index += m_shape[i];
    }
  }
  Segmented(const Segmented &) = default;
  Segmented(Segmented &&) noexcept = default;
  ~Segmented() = default;

  Segmented &operator=(const Segmented &) = default;
  Segmented &operator=(Segmented &&) noexcept = default;
  Segmented &operator=(const value_type &c)
  {
    m_values = c;
    return *this;
  }

  const Array<size_t> &GetShape() const { return m_shape; }

  SegmentView<value_type> segment(const size_t a)
  {
    return SegmentView<value_type>(&m_values[m_offsets[a] + 1], m_shape[a]);
  }
  SegmentView<const value_type> segment(const size_t a) const
  {
    return SegmentView<const value_type>(&m_values[m_offsets[a] + 1], m_shape[a]);
  }
  SegmentRange<false> segments() { return SegmentRange<false>(this); }
  SegmentRange<true> segments() const { return SegmentRange<true>(this); }

  void SetFlattened(const Storage &v) { m_values = v; }
  const Storage &GetFlattened() const { return m_values; }
  Storage &GetFlattened() { return m_values; }
};

template <class T> using SegmentedArray = Segmented<Array<T>>;
template <class T> using SegmentedVector = Segmented<Vector<T>>;

} // namespace Gambit

#endif // GAMBIT_CORE_SEGMENT_H
