//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface declaration for N-dimensional arrays
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

#ifndef GNARRAY_H
#define GNARRAY_H

#include <assert.h>
#include "base/gstream.h"
#include "math/gvector.h"

//
// Basic n-dimensional array
//
template <class T> class gNArray   {
  protected:
    long storage_size;
    T *storage;
    gArray<int> dim;

    void DumpFrom(gOutput &, int, gVector<int> &) const;
//    void ReadFrom(gInput &, const gVector<int> &, gVector<int> &, int);

  public:
    gNArray(void);
    gNArray(const gArray<int> &d);
    gNArray(const gNArray<T>& a);
    ~gNArray();

    gNArray<T> &operator=(const gNArray<T> &);

    /* not used for now
    T operator[](const gVector<int> &) const;
    T &operator[](const gVector<int> &);
    */

    T operator[](const gArray<int> &) const;
    T &operator[](const gArray<int> &);

    const T &operator[](long l) const;
    T &operator[](long l);

    const gArray<int> &Dimensionality(void) const;

//    void Input(gInput &, const gVector<int> &, int);
    void Output(gOutput &) const;
};

#ifdef UNUSED
template <class T> class gIndexedNArray : private gNArray<T>   {
  private:
    gArray<long> *index;
    
  public:
    gIndexedNArray(void);
    gIndexedNArray(const gVector<int> &d);
    gIndexedNArray(const gIndexedNArray<T> &);
	 ~gIndexedNArray();

	 gIndexedNArray<T> &operator=(const gIndexedNArray<T> &);

    T operator[](const gVector<int> &) const;
	 T &operator[](const gVector<int> &);
    
    const gVector<int> &Dimensionality(void) const   { return dim; }

    void Input(gInput &f, const gVector<int> &v, int i)
      { gNArray<T>::Input(f, v, i); }
    void Output(gOutput &f) const
      { gNArray<T>::Output(f); }
};
#endif   // UNUSED

#endif    // GNARRAY_H

