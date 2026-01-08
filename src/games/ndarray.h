//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/games/ndarray.h
// A simple N-dimensional array class
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

#ifndef GAMBIT_GAMES_NDARRAY_H
#define GAMBIT_GAMES_NDARRAY_H

#include <numeric>

#include "gambit.h"

namespace Gambit {

/// A representation of an (N+1)-dimensional array
///
/// This class provides an (N+1)-dimensional array, or, if you prefer, an N-dimensional
/// array of vectors (where all vectors are of the same size).
/// Elements of the array are indexed using a pair of an Array (the index into the N-dimensional
/// part) and an integer (the index into the vector part).
///
/// Internally, this is implemented as a single contiguous std::vector.  The N+1 dimensions are
/// mapped into this vector using offsets (in a quite standard way); these offsets are pre-computed
/// and stored to make indexing efficient.
///
/// @tparam T The data type of elements of the array
template <class T> class NDArray {
  using reference = typename std::vector<T>::reference;
  using const_reference = typename std::vector<T>::const_reference;

protected:
  Array<int> m_index_dim;
  int m_vector_dim{0};
  Array<int> m_offsets;
  int m_offsets_sum{0};
  std::vector<T> m_storage;

  int ComputeOffset(const Array<int> &p_array_index, int p_vector_index) const
  {
    // Gambit Array indices are 1-based but std::vector is 0-based.
    // As a result to index into the storage array we would need to deduct 1 from each
    // element of the array index.  This is the same as subtracting the sum of the offsets.
    return (std::inner_product(p_array_index.begin(), p_array_index.end(), m_offsets.begin(), 0) +
            p_vector_index * m_offsets.back() - m_offsets_sum);
  }

public:
  NDArray() = default;
  explicit NDArray(const Array<int> &p_index_dim, const int p_vector_dim)
    : m_index_dim(p_index_dim), m_vector_dim(p_vector_dim), m_offsets(p_index_dim.size() + 1),
      m_storage(std::accumulate(m_index_dim.begin(), m_index_dim.end(), 1, std::multiplies<>()) *
                m_vector_dim)
  {
    m_offsets.front() = 1;
    std::partial_sum(m_index_dim.begin(), m_index_dim.end(), std::next(m_offsets.begin()),
                     std::multiplies<>());
    // NOLINTBEGIN(cppcoreguidelines-prefer-member-initializer)
    m_offsets_sum = std::accumulate(m_offsets.begin(), m_offsets.end(), 0);
    // NOLINTEND(cppcoreguidelines-prefer-member-initializer)
  }

  NDArray(const NDArray &) = default;
  ~NDArray() = default;

  NDArray &operator=(const NDArray &) = default;

  const_reference at(const Array<int> &v, const int index) const
  {
    return m_storage.at(ComputeOffset(v, index));
  }
  reference at(const Array<int> &v, const int index)
  {
    return m_storage.at(ComputeOffset(v, index));
  }

  const Array<int> &GetIndexDimension() const { return m_index_dim; }
  int GetVectorDimension() const { return m_vector_dim; }
};

} // end namespace Gambit

#endif // GAMBIT_GAMES_NDARRAY_H
