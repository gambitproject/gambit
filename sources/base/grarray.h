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
#include "gstream.h"

template <class T> class gArray;

template <class T> class gRectArray    {
  protected:
    int minrow, maxrow, mincol, maxcol;
    T **data;

  public:
    class BadIndex : public gException  { 
      public:
        virtual ~BadIndex()  { }
        gText Description(void) const;
    };

    class BadDim : public gException  {
      public:
        virtual ~BadDim()   { }
	gText Description(void) const;
    };

       // CONSTRUCTORS, DESTRUCTOR, CONSTRUCTIVE OPERATORS
    gRectArray(void);
    gRectArray(unsigned int nrows, unsigned int ncols);
    gRectArray(int minr, int maxr, int minc, int maxc);
    gRectArray(const gRectArray<T> &);
    virtual ~gRectArray();

    gRectArray<T> &operator=(const gRectArray<T> &);

       // GENERAL DATA ACCESS
    int NumRows(void) const;
    int NumColumns(void) const;
    int MinRow(void) const;
    int MaxRow(void) const;
    int MinCol(void) const;
    int MaxCol(void) const;
    virtual void Dump(gOutput &) const;
    
       // INDEXING OPERATORS
    T &operator()(int r, int c);
    const T &operator()(int r, int c) const;

       // ROW AND COLUMN ROTATION OPERATORS
    void RotateUp(int lo, int hi);
    void RotateDown(int lo, int hi);
    void RotateLeft(int lo, int hi);
    void RotateRight(int lo, int hi);

       // ROW MANIPULATION FUNCTIONS
    void SwitchRow(int, gArray<T> &);
    void SwitchRows(int, int);
    void GetRow(int, gArray<T> &) const;
    void SetRow(int, const gArray<T> &);

       // COLUMN MANIPULATION FUNCTIONS
    void SwitchColumn(int, gArray<T> &);
    void SwitchColumns(int, int);
    void GetColumn(int, gArray<T> &) const;
    void SetColumn(int, const gArray<T> &);

      // TRANSPOSE
    gRectArray<T>       Transpose()         const;


    // originally protected functions, moved to permit compilation
    // should be moved back eventually

      // RANGE CHECKING FUNCTIONS
      // check for correct row index
    bool CheckRow(int row) const;
      // check row vector for correct column boundaries
    bool CheckRow(const gArray<T> &) const;
      // check for correct column index
    bool CheckColumn(int col) const;
      // check column vector for correct row boundaries
    bool CheckColumn(const gArray<T> &) const;
      // check row and column indices
    bool Check(int row, int col) const;
      // check matrix for same row and column boundaries
    bool CheckBounds(const gRectArray<T> &) const;


};

template <class T> gOutput &operator<<(gOutput &, const gRectArray<T> &);

#endif   // GRARRAY_H
