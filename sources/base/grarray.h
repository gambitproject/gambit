//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Rectangular array base class
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

#ifndef GRARRAY_H
#define GRARRAY_H

#include "gmisc.h"

template <class T> class gbtArray;

template <class T> class gbtRectArray    {
  protected:
    int minrow, maxrow, mincol, maxcol;
    T **data;

  public:
    // CONSTRUCTORS, DESTRUCTOR, CONSTRUCTIVE OPERATORS
    gbtRectArray(void);
    gbtRectArray(unsigned int nrows, unsigned int ncols);
    gbtRectArray(int minr, int maxr, int minc, int maxc);
    gbtRectArray(const gbtRectArray<T> &);
    virtual ~gbtRectArray();

    gbtRectArray<T> &operator=(const gbtRectArray<T> &);

    // GENERAL DATA ACCESS
    int NumRows(void) const;
    int NumColumns(void) const;
    int MinRow(void) const;
    int MaxRow(void) const;
    int MinCol(void) const;
    int MaxCol(void) const;
    
    // INDEXING OPERATORS
    T &operator()(int r, int c);
    const T &operator()(int r, int c) const;

    // ROW AND COLUMN ROTATION OPERATORS
    void RotateUp(int lo, int hi);
    void RotateDown(int lo, int hi);
    void RotateLeft(int lo, int hi);
    void RotateRight(int lo, int hi);

    // ROW MANIPULATION FUNCTIONS
    void SwitchRow(int, gbtArray<T> &);
    void SwitchRows(int, int);
    void GetRow(int, gbtArray<T> &) const;
    void SetRow(int, const gbtArray<T> &);

    // COLUMN MANIPULATION FUNCTIONS
    void SwitchColumn(int, gbtArray<T> &);
    void SwitchColumns(int, int);
    void GetColumn(int, gbtArray<T> &) const;
    void SetColumn(int, const gbtArray<T> &);

    // TRANSPOSE
    gbtRectArray<T> Transpose(void) const;


    // originally protected functions, moved to permit compilation
    // should be moved back eventually

    // RANGE CHECKING FUNCTIONS
    // check for correct row index
    bool CheckRow(int row) const;
    // check row vector for correct column boundaries
    bool CheckRow(const gbtArray<T> &) const;
    // check for correct column index
    bool CheckColumn(int col) const;
    // check column vector for correct row boundaries
    bool CheckColumn(const gbtArray<T> &) const;
    // check row and column indices
    bool Check(int row, int col) const;
    // check matrix for same row and column boundaries
    bool CheckBounds(const gbtRectArray<T> &) const;
};

#endif   // GRARRAY_H
