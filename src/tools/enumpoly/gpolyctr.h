//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/gpolyctr.h
// Declaration of specialized containers for polynomials
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

template <class T> class gPolyArray  {
  protected:
    int mindex, maxdex;
    gPoly<T> **data;

  public:
    gPolyArray(const gSpace *, const term_order *, int len = 0);
    gPolyArray(const gSpace *, const term_order *, int lo, int hi);
    gPolyArray(const gPolyArray<T> &);
    virtual ~gPolyArray();

    gPolyArray<T> &operator=(const gPolyArray<T> &);
    int Length(void) const;

    int First(void) const;
    int Last(void) const;

    const gPoly<T> &operator[](int index) const;
    gPoly<T> &operator[](int index);
    virtual void Dump(gOutput &) const;
};

template <class T> class gPolyBlock : public gPolyArray<T>   {
  private:
    int InsertAt(const gPoly<T> &t, int where);

  public:
    gPolyBlock(const gSpace *, const term_order *, int len = 0);
    gPolyBlock(const gSpace *, const term_order *, int lo, int hi);
    gPolyBlock(const gPolyBlock<T> &);
    virtual ~gPolyBlock();

    gPolyBlock<T> &operator=(const gPolyBlock<T> &);

    bool operator==(const gPolyBlock<T> &b) const;
    bool operator!=(const gPolyBlock<T> &b) const;

    int Append(const gPoly<T> &);
    int Insert(const gPoly<T> &, int);
    gPoly<T> Remove(int);

    int Find(const gPoly<T> &) const;
    int Contains(const gPoly<T> &t) const;
    void Flush(void);
};

#endif    // GPOLYCTR_H

