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

#include <vector>
#include <iterator>

namespace Gambit {

/// A basic bounds-checked array
template <class T> class Array {
protected:
  int mindex, maxdex;
  T *data;

  /// Private helper function that accomplishes the insertion of an object
  int InsertAt(const T &t, int n)
  {
    if (this->mindex > n || n > this->maxdex + 1) {
      throw IndexException();
    }

    T *new_data = new T[++this->maxdex - this->mindex + 1] - this->mindex;

    int i;
    for (i = this->mindex; i <= n - 1; i++) {
      new_data[i] = this->data[i];
    }
    new_data[i++] = t;
    for (; i <= this->maxdex; i++) {
      new_data[i] = this->data[i - 1];
    }

    if (this->data) {
      delete[] (this->data + this->mindex);
    }
    this->data = new_data;

    return n;
  }

public:
  class iterator {
  private:
    Array *m_array;
    int m_index;

  public:
    using iterator_category = std::input_iterator_tag;
    using difference_type = typename std::vector<T>::iterator::difference_type;
    using value_type = T;
    using pointer = value_type *;
    using reference = value_type &;

    iterator(Array *p_array, int p_index) : m_array(p_array), m_index(p_index) {}
    reference operator*() { return (*m_array)[m_index]; }
    pointer operator->() { return &(*m_array)[m_index]; }
    iterator &operator++()
    {
      m_index++;
      return *this;
    }
    bool operator==(const iterator &it) const
    {
      return (m_array == it.m_array) && (m_index == it.m_index);
    }
    bool operator!=(const iterator &it) const { return !(*this == it); }
  };

  class const_iterator {
  private:
    const Array *m_array;
    int m_index;

  public:
    using iterator_category = std::input_iterator_tag;
    using difference_type = typename std::vector<T>::iterator::difference_type;
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
    bool operator==(const const_iterator &it) const
    {
      return (m_array == it.m_array) && (m_index == it.m_index);
    }
    bool operator!=(const const_iterator &it) const { return !(*this == it); }
  };

  /// @name Lifecycle
  //@{
  /// Constructs an array of length 'len', starting at '1'
  explicit Array(unsigned int len = 0)
    : mindex(1), maxdex(len), data((len) ? new T[len] - 1 : nullptr)
  {
  }
  /// Constructs an array starting at lo and ending at hi
  Array(int lo, int hi) : mindex(lo), maxdex(hi)
  {
    if (maxdex + 1 < mindex) {
      throw RangeException();
    }
    data = (maxdex >= mindex) ? new T[maxdex - mindex + 1] - mindex : nullptr;
  }
  /// Copy the contents of another array
  Array(const Array<T> &a)
    : mindex(a.mindex), maxdex(a.maxdex),
      data((maxdex >= mindex) ? new T[maxdex - mindex + 1] - mindex : nullptr)
  {
    for (int i = mindex; i <= maxdex; i++) {
      data[i] = a.data[i];
    }
  }
  /// Destruct and deallocates the array
  virtual ~Array()
  {
    if (maxdex >= mindex) {
      delete[] (data + mindex);
    }
  }

  /// Copy the contents of another array
  Array<T> &operator=(const Array<T> &a)
  {
    if (this != &a) {
      // We only reallocate if necessary.  This should be somewhat faster
      // if many objects are of the same length.  Furthermore, it is
      // _essential_ for the correctness of the PVector and DVector
      // assignment operator, since it assumes the value of data does
      // not change.
      if (!data || (data && (mindex != a.mindex || maxdex != a.maxdex))) {
        if (data) {
          delete[] (data + mindex);
        }
        mindex = a.mindex;
        maxdex = a.maxdex;
        data = (maxdex >= mindex) ? new T[maxdex - mindex + 1] - mindex : nullptr;
      }

      for (int i = mindex; i <= maxdex; i++) {
        data[i] = a.data[i];
      }
    }

    return *this;
  }

  //@}

  /// @name Operator overloading
  //@{
  /// Test the equality of two arrays
  bool operator==(const Array<T> &a) const
  {
    if (mindex != a.mindex || maxdex != a.maxdex) {
      return false;
    }
    for (int i = mindex; i <= maxdex; i++) {
      if ((*this)[i] != a[i]) {
        return false;
      }
    }
    return true;
  }

  /// Test the inequality of two arrays
  bool operator!=(const Array<T> &a) const { return !(*this == a); }
  //@}

  /// @name General data access
  //@{
  /// Return the length of the array
  int Length() const { return maxdex - mindex + 1; }

  /// Return the first index
  int First() const { return mindex; }

  /// Return the last index
  int Last() const { return maxdex; }

  /// Return a forward iterator starting at the beginning of the array
  iterator begin() { return iterator(this, mindex); }
  /// Return a forward iterator past the end of the array
  iterator end() { return iterator(this, maxdex + 1); }
  /// Return a const forward iterator starting at the beginning of the array
  const_iterator begin() const { return const_iterator(this, mindex); }
  /// Return a const forward iterator past the end of the array
  const_iterator end() const { return const_iterator(this, maxdex + 1); }
  /// Return a const forward iterator starting at the beginning of the array
  const_iterator cbegin() const { return const_iterator(this, mindex); }
  /// Return a const forward iterator past the end of the array
  const_iterator cend() const { return const_iterator(this, maxdex + 1); }

  /// Access the index'th entry in the array
  const T &operator[](int index) const
  {
    if (index < mindex || index > maxdex) {
      throw IndexException();
    }
    return data[index];
  }

  /// Access the index'th entry in the array
  T &operator[](int index)
  {
    if (index < mindex || index > maxdex) {
      throw IndexException();
    }
    return data[index];
  }

  /// Return the index at which a given element resides in the array.
  int Find(const T &t) const
  {
    int i;
    for (i = this->mindex; i <= this->maxdex && this->data[i] != t; i++)
      ;
    return (i <= this->maxdex) ? i : (mindex - 1);
  }

  /// Return true if the element is currently residing in the array
  bool Contains(const T &t) const { return Find(t) != mindex - 1; }
  //@}

  /// @name Modifying the contents of the array
  //@{
  /// \brief Insert a new element into the array at a given index.
  ///
  /// Insert a new element into the array at a given index.  If the index is
  /// less than the lowest index, the element is inserted at the beginning;
  /// if the index is greater than the highest index, the element is appended.
  /// Returns the index at which the element actually is placed.
  int Insert(const T &t, int n)
  {
    return InsertAt(t, (n < this->mindex) ? this->mindex
                                          : ((n > this->maxdex + 1) ? this->maxdex + 1 : n));
  }

  /// \brief Remove an element from the array.
  ///
  /// Remove the element at a given index from the array.  Returns the value
  /// of the element removed.
  T Remove(int n)
  {
    if (n < this->mindex || n > this->maxdex) {
      throw IndexException();
    }

    T ret(this->data[n]);
    T *new_data = (--this->maxdex >= this->mindex)
                      ? new T[this->maxdex - this->mindex + 1] - this->mindex
                      : nullptr;

    int i;
    for (i = this->mindex; i < n; i++) {
      new_data[i] = this->data[i];
    }
    for (; i <= this->maxdex; i++) {
      new_data[i] = this->data[i + 1];
    }

    delete[] (this->data + this->mindex);
    this->data = new_data;

    return ret;
  }
  //@}

  /// @name STL-style interface
  ///
  /// These operations are a partial implementation of operations on
  /// STL-style list containers.  It is suggested that future code be
  /// written to use these, and existing code ported to use them as
  /// possible.
  ///@{
  /// Return whether the array container is empty (has size 0).
  bool empty() const { return (this->maxdex < this->mindex); }
  /// Return the number of elements in the array container.
  size_t size() const { return maxdex - mindex + 1; }
  /// Access first element.
  const T &front() const { return data[mindex]; }
  /// Access first element.
  T &front() { return data[mindex]; }
  /// Access last element.
  const T &back() const { return data[maxdex]; }
  /// Access last element.
  T &back() { return data[maxdex]; }

  /// Adds a new element at the end of the array container, after its
  /// current last element.
  void push_back(const T &val) { InsertAt(val, this->maxdex + 1); }
  /// Removes all elements from the array container (which are destroyed),
  /// leaving the container with a size of 0.
  void clear()
  {
    delete[] (this->data + this->mindex);
    this->data = 0;
    this->maxdex = this->mindex - 1;
  }
  ///@}
};

} // end namespace Gambit

#endif // LIBGAMBIT_ARRAY_H
