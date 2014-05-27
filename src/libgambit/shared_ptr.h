//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/shared_ptr.h
// Lightweight implementation of shared pointer.
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

#ifndef LIBGAMBIT_SHARED_PTR_H
#define LIBGAMBIT_SHARED_PTR_H

#include <algorithm>
#include <functional>
#include <new>

namespace Gambit  {

// A reference-counted shared-pointer implementation, which should be
// compatible with Boost / C++11.   This is intended entirely as a transitional
// implementation, until a full migration to C++11 is done.
template <class T> class shared_ptr {
public:
  shared_ptr(T *p = 0) : value(p)
  {
    try {
      count = new long(1);
    }
    catch (...)  {
      delete p;
      throw;
    }
  }

  shared_ptr(const shared_ptr &r) : value(r.value) 
  { count = r.count; ++*count; }

  ~shared_ptr() 
  {
    if (--*count == 0) {
      delete value;
      delete count;
    }
  }

  shared_ptr &operator=(const shared_ptr &r) 
  {
    shared_ptr(r).swap(*this);
    return *this;
  }

  void reset(T *p = 0)       { shared_ptr(p).swap(*this); }
  T &operator*(void) const   { return *value; }
  T *operator->(void) const  { return value; }
  T *get(void) const         { return value; }
  long use_count(void) const { return *count; }
  bool unique(void) const    { return (*count == 1); }
  void swap(shared_ptr<T> & other)  // never throws
  {
    std::swap(value, other.value);
    std::swap(count, other.count);
  }

private:
  T *value; 
  long *count; 
};

template <class T, class U> 
inline bool operator==(const shared_ptr<T> &a, const shared_ptr<U> &b)
{
  return a.get() == b.get();
}

template <class T, class U> 
inline bool operator!=(const shared_ptr<T> &a, const shared_ptr<U> &b)
{
  return a.get() != b.get();
}

}  // namespace Gambit

#endif // LIBGAMBIT_SHARED_PTR_H
