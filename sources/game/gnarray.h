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
template <class T> class gbtNDArray   {
  protected:
    long storage_size;
    T *storage;
    gbtArray<int> dim;

    void DumpFrom(gbtOutput &, int, gbtVector<int> &) const;
//    void ReadFrom(gbtInput &, const gbtVector<int> &, gbtVector<int> &, int);

  public:
    gbtNDArray(void);
    gbtNDArray(const gbtArray<int> &d);
    gbtNDArray(const gbtNDArray<T>& a);
    ~gbtNDArray();

    gbtNDArray<T> &operator=(const gbtNDArray<T> &);

    /* not used for now
    T operator[](const gbtVector<int> &) const;
    T &operator[](const gbtVector<int> &);
    */

    T operator[](const gbtArray<int> &) const;
    T &operator[](const gbtArray<int> &);

    const T &operator[](long l) const;
    T &operator[](long l);

    const gbtArray<int> &Dimensionality(void) const;

//    void Input(gbtInput &, const gbtVector<int> &, int);
    void Output(gbtOutput &) const;
};

#ifdef UNUSED
template <class T> class gIndexedNArray : private gbtNDArray<T>   {
  private:
    gbtArray<long> *index;
    
  public:
    gIndexedNArray(void);
    gIndexedNArray(const gbtVector<int> &d);
    gIndexedNArray(const gIndexedNArray<T> &);
	 ~gIndexedNArray();

	 gIndexedNArray<T> &operator=(const gIndexedNArray<T> &);

    T operator[](const gbtVector<int> &) const;
	 T &operator[](const gbtVector<int> &);
    
    const gbtVector<int> &Dimensionality(void) const   { return dim; }

    void Input(gbtInput &f, const gbtVector<int> &v, int i)
      { gbtNDArray<T>::Input(f, v, i); }
    void Output(gbtOutput &f) const
      { gbtNDArray<T>::Output(f); }
};
#endif   // UNUSED

#endif    // GNARRAY_H
