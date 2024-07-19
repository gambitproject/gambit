//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/gnarray.h
// Interface declaration for N-dimensional arrays
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

#include "gambit.h"

namespace Gambit {
//
// Basic n-dimensional array
//
template <class T> class NDArray {
protected:
  Array<int> dim;
  std::vector<T> storage;

  static size_t ComputeSize(const Array<int> &dim)
  {
    size_t size = 1;
    for (auto element : dim) {
      size *= element;
    }
    return size;
  }

  int ComputeOffset(const Array<int> &v) const
  {
    int location = 0;
    for (int i = 1, offset = 1; i <= dim.Length(); i++) {
      location += (v[i] - 1) * offset;
      offset *= dim[i];
    }
    return location;
  }

public:
  NDArray() = default;
  explicit NDArray(const Array<int> &d) : dim(d), storage(ComputeSize(dim)) {}
  NDArray(const NDArray<T> &a) = default;
  ~NDArray() = default;

  NDArray<T> &operator=(const NDArray<T> &) = default;

  const T &operator[](const Array<int> &v) const { return storage[ComputeOffset(v)]; }
  T &operator[](const Array<int> &v) { return storage[ComputeOffset(v)]; }

  const Array<int> &GetDimension() const { return dim; }
};

} // end namespace Gambit

#endif // NDARRAY_H
