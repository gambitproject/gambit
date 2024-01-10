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

#ifndef GNARRAY_H
#define GNARRAY_H

#include "gambit.h"

//
// Basic n-dimensional array
//
template <class T> class gNArray   {
  protected:
    long storage_size;
    T *storage;
    Gambit::Array<int> dim;

  public:
    gNArray();
    explicit gNArray(const Gambit::Array<int> &d);
    gNArray(const gNArray<T>& a);
    ~gNArray();

    gNArray<T> &operator=(const gNArray<T> &);

    /* not used for now
    T operator[](const Gambit::Vector<int> &) const;
    T &operator[](const Gambit::Vector<int> &);
    */

    T operator[](const Gambit::Array<int> &) const;
    T &operator[](const Gambit::Array<int> &);

    const T &operator[](long l) const;
    T &operator[](long l);

    const Gambit::Array<int> &Dimensionality() const;
};

#endif    // GNARRAY_H

