//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// A basic bounds-checked array type
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#ifndef GARRAY_H
#define GARRAY_H

#include "gmisc.h"
#include <stdlib.h>

template <class T> class gbtArray  {
  protected:
    int mindex, maxdex;
    T *data;

  public:
/// Constructs a gbtArray of length 'len', starting at '1'
    gbtArray(unsigned int len = 0);

/// Constructs a gbtArray starting at lo and ending at hi
    gbtArray(int lo, int hi);

/// Copy constructor
    gbtArray(const gbtArray<T> &);

/// Destruct and deallocate gbtArray
    virtual ~gbtArray();

/// Copies data from input gbtArray
    gbtArray<T> &operator=(const gbtArray<T> &);

/// Return length in the invoking gbtArray<T>
    int Length(void) const throw () { return maxdex - mindex + 1; }

/// Return first index
    int First(void) const throw () { return mindex; }

/// Return last index
    int Last(void) const throw () { return maxdex; }

/// Subscripting operator; throws exception if index is out of bounds
    const T &operator[](int index) const throw (gbtIndexException);
    T &operator[](int index) throw (gbtIndexException);
};

template <class T> bool operator==(const gbtArray<T> &, const gbtArray<T> &);
template <class T> bool operator!=(const gbtArray<T> &, const gbtArray<T> &);

#endif	// GARRAY_H
