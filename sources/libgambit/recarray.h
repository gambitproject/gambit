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

#ifndef LIBGAMBIT_RECARRAY_H
#define LIBGAMBIT_RECARRAY_H

#include "libgambit.h"

template <class T> class gbtArray;

namespace Gambit {

/// This implements a rectangular (two-dimensional) array
template <class T> class RectArray {
protected:
  int minrow, maxrow, mincol, maxcol;
  T **data;

  /// @name Range checking functions; returns true only if valid index/size
  //@{
  /// check for correct row index
  bool CheckRow(int row) const;
  /// check row vector for correct column boundaries
  bool CheckRow(const gbtArray<T> &) const;
  /// check for correct column index
  bool CheckColumn(int col) const;
  /// check column vector for correct row boundaries
  bool CheckColumn(const gbtArray<T> &) const;
  /// check row and column indices
  bool Check(int row, int col) const;
  /// check matrix for same row and column boundaries
  bool CheckBounds(const RectArray<T> &) const;
  //@

public:
  /// @name Lifecycle
  //@{
  RectArray(void);
  RectArray(unsigned int nrows, unsigned int ncols);
  RectArray(int minr, int maxr, int minc, int maxc);
  RectArray(const RectArray<T> &);
  virtual ~RectArray();

  RectArray<T> &operator=(const RectArray<T> &);
  //@}

  /// @name General data access
  //@{
  int NumRows(void) const;
  int NumColumns(void) const;
  int MinRow(void) const;
  int MaxRow(void) const;
  int MinCol(void) const;
  int MaxCol(void) const;
  //@}
    
  /// @name Indexing operations
  //@{
  T &operator()(int r, int c);
  const T &operator()(int r, int c) const;
  //@}

  /// @name Row and column rotation operators
  //@{
  void RotateUp(int lo, int hi);
  void RotateDown(int lo, int hi);
  void RotateLeft(int lo, int hi);
  void RotateRight(int lo, int hi);
  //@}

  /// @name Row and column manipulation functions
  //@{
  void SwitchRow(int, gbtArray<T> &);
  void SwitchRows(int, int);
  void GetRow(int, gbtArray<T> &) const;
  void SetRow(int, const gbtArray<T> &);

  void SwitchColumn(int, gbtArray<T> &);
  void SwitchColumns(int, int);
  void GetColumn(int, gbtArray<T> &) const;
  void SetColumn(int, const gbtArray<T> &);

  /// Returns the transpose of the rectangular array
  RectArray<T> Transpose(void) const;
};


//------------------------------------------------------------------------
//            RectArray<T>: Private/protected member functions
//------------------------------------------------------------------------

template <class T> bool RectArray<T>::CheckRow(int row) const
{
  return (minrow <= row && row <= maxrow);
}

template <class T> bool RectArray<T>::CheckRow(const gbtArray<T> &v) const
{
  return (v.First() == mincol && v.Last() == maxcol);
}

template <class T> bool RectArray<T>::CheckColumn(int col) const
{
  return (mincol <= col && col <= maxcol);
}

template <class T> bool RectArray<T>::CheckColumn(const gbtArray<T> &v) const
{
  return (v.First() == minrow && v.Last() == maxrow);
}

template <class T> bool RectArray<T>::Check(int row, int col) const
{
  return (CheckRow(row) && CheckColumn(col));
}

template <class T>
bool RectArray<T>::CheckBounds(const RectArray<T> &m) const
{
  return (minrow == m.minrow && maxrow == m.maxrow &&
	  mincol == m.mincol && maxcol == m.maxcol);
}

//------------------------------------------------------------------------
//     RectArray<T>: Constructors, destructor, constructive operators
//------------------------------------------------------------------------

template <class T> RectArray<T>::RectArray(void)
  : minrow(1), maxrow(0), mincol(1), maxcol(0), data(0)
{ }

template <class T> RectArray<T>::RectArray(unsigned int rows,
						 unsigned int cols)
  : minrow(1), maxrow(rows), mincol(1), maxcol(cols)
{
  data = (rows > 0) ? new T *[maxrow] - 1 : 0;
  for (int i = 1; i <= maxrow;
       data[i++] = (cols > 0) ? new T[maxcol] - 1 : 0);
}

template <class T>
RectArray<T>::RectArray(int minr, int maxr, int minc, int maxc)
  : minrow(minr), maxrow(maxr), mincol(minc), maxcol(maxc)
{
  data = (maxrow >= minrow) ? new T *[maxrow - minrow + 1] - minrow : 0;
  for (int i = minrow; i <= maxrow;
       data[i++] = (maxcol - mincol + 1) ? new T[maxcol - mincol + 1] - mincol : 0);
}

template <class T> RectArray<T>::RectArray(const RectArray<T> &a)
  : minrow(a.minrow), maxrow(a.maxrow), mincol(a.mincol), maxcol(a.maxcol)
{
  data = (maxrow >= minrow) ? new T *[maxrow - minrow + 1] - minrow : 0;
  for (int i = minrow; i <= maxrow; i++)  {
    data[i] = (maxcol >= mincol) ? new T[maxcol - mincol + 1] - mincol : 0;
    for (int j = mincol; j <= maxcol; j++)
      data[i][j] = a.data[i][j];
  }
}

template <class T> RectArray<T>::~RectArray()
{
  for (int i = minrow; i <= maxrow; i++)
    if (data[i])  delete [] (data[i] + mincol);
  if (data)  delete [] (data + minrow);
}

template <class T>
RectArray<T> &RectArray<T>::operator=(const RectArray<T> &a)
{
  if (this != &a)   {
    int i;
    for (i = minrow; i <= maxrow; i++)
      if (data[i])  delete [] (data[i] + mincol);
    if (data)  delete [] (data + minrow);

    minrow = a.minrow;
    maxrow = a.maxrow;
    mincol = a.mincol;
    maxcol = a.maxcol;
    
    data = (maxrow >= minrow) ? new T *[maxrow - minrow + 1] - minrow : 0;
  
    for (i = minrow; i <= maxrow; i++)  {
      data[i] = (maxcol >= mincol) ? new T[maxcol - mincol + 1] - mincol : 0;
      for (int j = mincol; j <= maxcol; j++)
	data[i][j] = a.data[i][j];
    }
  }
    
  return *this;
}

//------------------------------------------------------------------------
//                  RectArray<T>: Data access members
//------------------------------------------------------------------------

template <class T> int RectArray<T>::NumRows(void) const
{ return maxrow - minrow + 1; }

template <class T> int RectArray<T>::NumColumns(void) const
{ return maxcol - mincol + 1; }

template <class T> int RectArray<T>::MinRow(void) const    { return minrow; }
template <class T> int RectArray<T>::MaxRow(void) const    { return maxrow; }
template <class T> int RectArray<T>::MinCol(void) const { return mincol; }
template <class T> int RectArray<T>::MaxCol(void) const { return maxcol; }

template <class T> T &RectArray<T>::operator()(int r, int c)
{
  if (!Check(r, c))  throw gbtIndexException();

  return data[r][c];
}

template <class T> const T &RectArray<T>::operator()(int r, int c) const
{
  if (!Check(r, c))  throw gbtIndexException();

  return data[r][c];
}

//------------------------------------------------------------------------
//                   RectArray<T>: Row and column rotation
//------------------------------------------------------------------------

template <class T> void RectArray<T>::RotateUp(int lo, int hi)
{
  if (lo < minrow || hi < lo || maxrow < hi)  throw gbtIndexException();

  T *temp = data[lo];
  for (int k = lo; k < hi; k++)
    data[k] = data[k + 1];
  data[hi] = temp;
}

template <class T> void RectArray<T>::RotateDown(int lo, int hi)
{
  if (lo < minrow || hi < lo || maxrow < hi)  throw gbtIndexException();

  T *temp = data[hi];
  for (int k = hi; k > lo; k--)
    data[k] = data[k - 1];
  data[lo] = temp;
}

template <class T> void RectArray<T>::RotateLeft(int lo, int hi)
{
  if (lo < mincol || hi < lo || maxcol < hi)   throw gbtIndexException();
  
  T temp;
  for (int i = minrow; i <= maxrow; i++)  {
    T *row = data[i];
    temp = row[lo];
    for (int j = lo; j < hi; j++)
      row[j] = row[j + 1];
    row[hi] = temp;
  }
}

template <class T> void RectArray<T>::RotateRight(int lo, int hi)
{
  if (lo < mincol || hi < lo || maxcol < hi)   throw gbtIndexException();

  for (int i = minrow; i <= maxrow; i++)  {
    T *row = data[i];
    T temp = row[hi];
    for (int j = hi; j > lo; j--)
      row[j] = row[j - 1];
    row[lo] = temp;
  }
}

//-------------------------------------------------------------------------
//                 RectArray<T>: Row manipulation functions
//-------------------------------------------------------------------------

template <class T> void RectArray<T>::SwitchRow(int row, gbtArray<T> &v)
{
  if (!CheckRow(row))  throw gbtIndexException();
  if (!CheckRow(v))    throw gbtDimensionException();

  T *rowptr = data[row];
  T tmp;
  for (int i = mincol; i <= maxcol; i++)  {
    tmp = rowptr[i];
    rowptr[i] = v[i];
    v[i] = tmp;
  }
}

template <class T> void RectArray<T>::SwitchRows(int i, int j)
{
  if (!CheckRow(i) || !CheckRow(j))   throw gbtIndexException();
  T *temp = data[j];
  data[j] = data[i];
  data[i] = temp;
}

template <class T> void RectArray<T>::GetRow(int row, gbtArray<T> &v) const
{
  if (!CheckRow(row))   throw gbtIndexException();
  if (!CheckRow(v))     throw gbtDimensionException();

  T *rowptr = data[row];
  for (int i = mincol; i <= maxcol; i++)
    v[i] = rowptr[i];
}

template <class T> void RectArray<T>::SetRow(int row, const gbtArray<T> &v)
{
  if (!CheckRow(row))   throw gbtIndexException();
  if (!CheckRow(v))     throw gbtDimensionException();

  T *rowptr = data[row];
  for (int i = mincol; i <= maxcol; i++)
    rowptr[i] = v[i];
}

//-------------------------------------------------------------------------
//                RectArray<T>: Column manipulation functions
//-------------------------------------------------------------------------

template <class T> void RectArray<T>::SwitchColumn(int col, gbtArray<T> &v)
{
  if (!CheckColumn(col))   throw gbtIndexException();
  if (!CheckColumn(v))     throw gbtDimensionException();

  for (int i = minrow; i <= maxrow; i++)   {
    T tmp = data[i][col];
    data[i][col] = v[i];
    v[i] = tmp;
  }
}

template <class T> void RectArray<T>::SwitchColumns(int a, int b)
{
  if (!CheckColumn(a) || !CheckColumn(b))   throw gbtIndexException();

  for (int i = minrow; i <= maxrow; i++)   {
    T tmp = data[i][a];
    data[i][a] = data[i][b];
    data[i][b] = tmp;
  }
}

template <class T> void RectArray<T>::GetColumn(int col, gbtArray<T> &v) const
{
  if (!CheckColumn(col))  throw gbtIndexException();
  if (!CheckColumn(v))    throw gbtDimensionException();

  for (int i = minrow; i <= maxrow; i++)
    v[i] = data[i][col];
}

template <class T> void RectArray<T>::SetColumn(int col, const gbtArray<T> &v)
{
  if (!CheckColumn(col))   throw gbtIndexException();
  if (!CheckColumn(v))     throw gbtDimensionException();

  for (int i = minrow; i <= maxrow; i++)
    data[i][col] = v[i];
}

//-------------------------------------------------------------------------
//                        RectArray<T>: Transpose
//-------------------------------------------------------------------------

template <class T> RectArray<T> RectArray<T>::Transpose(void) const
{
  RectArray<T> tmp(mincol, maxcol, minrow, maxrow);
 
  for (int i = minrow; i <= maxrow; i++)
    for (int j = mincol; j <= maxrow; j++)
      tmp(j,i) = (*this)(i,j);

  return tmp;
}

} // end namespace Gambit

#endif // LIBGAMBIT_RECARRAY_H
