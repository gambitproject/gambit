//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/ndarray.h
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

#ifndef NDARRAY_H
#define NDARRAY_H

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
protected:
  Array<int> m_index_dim;
  int m_vector_dim;
  Array<int> m_offsets;
  int m_offsets_sum;
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
  NDArray() : m_vector_dim(0) {}
  explicit NDArray(const Array<int> &p_index_dim, int p_vector_dim)
    : m_index_dim(p_index_dim), m_vector_dim(p_vector_dim), m_offsets(p_index_dim.size() + 1),
      m_storage(
          std::accumulate(m_index_dim.begin(), m_index_dim.end(), 1, std::multiplies<int>()) *
          m_vector_dim)
  {
    m_offsets.front() = 1;
    std::partial_sum(m_index_dim.begin(), m_index_dim.end(), std::next(m_offsets.begin()),
                     std::multiplies<int>());
    m_offsets_sum = std::accumulate(m_offsets.begin(), m_offsets.end(), 0);
  }

  NDArray(const NDArray<T> &) = default;
  ~NDArray() = default;

  NDArray<T> &operator=(const NDArray<T> &) = default;

  const T &at(const Array<int> &v, int index) const
  {
    return m_storage.at(ComputeOffset(v, index));
  }
  T &at(const Array<int> &v, int index) { return m_storage.at(ComputeOffset(v, index)); }

  const Array<int> &GetIndexDimension() const { return m_index_dim; }
  int GetVectorDimension() { return m_vector_dim; }
};

} // end namespace Gambit

#endif // NDARRAY_H
