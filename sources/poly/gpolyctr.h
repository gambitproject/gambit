//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of specialized containers for polynomials
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

#ifndef GPOLYCTR_H
#define GPOLYCTR_H

#include "gpoly.h"

template <class T> class gbtPolyMultiArray  {
  protected:
    int mindex, maxdex;
    gbtPolyMulti<T> **data;

  public:
    gbtPolyMultiArray(const gbtPolySpace *, const gbtPolyTermOrder *, int len = 0);
    gbtPolyMultiArray(const gbtPolySpace *, const gbtPolyTermOrder *, int lo, int hi);
    gbtPolyMultiArray(const gbtPolyMultiArray<T> &);
    virtual ~gbtPolyMultiArray();

    gbtPolyMultiArray<T> &operator=(const gbtPolyMultiArray<T> &);
    int Length(void) const;

    int First(void) const;
    int Last(void) const;

    const gbtPolyMulti<T> &operator[](int index) const;
    gbtPolyMulti<T> &operator[](int index);
    virtual void Dump(gbtOutput &) const;
};

template <class T> class gbtPolyMultiBlock : public gbtPolyMultiArray<T>   {
  private:
    int InsertAt(const gbtPolyMulti<T> &t, int where);

  public:
    gbtPolyMultiBlock(const gbtPolySpace *, const gbtPolyTermOrder *, int len = 0);
    gbtPolyMultiBlock(const gbtPolySpace *, const gbtPolyTermOrder *, int lo, int hi);
    gbtPolyMultiBlock(const gbtPolyMultiBlock<T> &);
    virtual ~gbtPolyMultiBlock();

    gbtPolyMultiBlock<T> &operator=(const gbtPolyMultiBlock<T> &);

    bool operator==(const gbtPolyMultiBlock<T> &b) const;
    bool operator!=(const gbtPolyMultiBlock<T> &b) const;

    int Append(const gbtPolyMulti<T> &);
    int Insert(const gbtPolyMulti<T> &, int);
    gbtPolyMulti<T> Remove(int);

    int Find(const gbtPolyMulti<T> &) const;
    int Contains(const gbtPolyMulti<T> &t) const;
    void Flush(void);
};

#endif    // GPOLYCTR_H
