//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/recarray.h
// Rectangular array base class
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

#include "gambit.h"

namespace Gambit {

/// This class implements a rectangular (two-dimensional) array
template <class T> class RectArray {
protected:
  int minrow, maxrow, mincol, maxcol;
  T **data;

public:
  /// @name Lifecycle
  //@{
  RectArray();
  RectArray(unsigned int nrows, unsigned int ncols);
  RectArray(int minr, int maxr, int minc, int maxc);
  RectArray(const RectArray<T> &);
  virtual ~RectArray();

  RectArray<T> &operator=(const RectArray<T> &);
  //@}

  /// @name General data access
  //@{
  int NumRows() const;
  int NumColumns() const;
  int MinRow() const;
  int MaxRow() const;
  int MinCol() const;
  int MaxCol() const;
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
  void SwitchRow(int, Array<T> &);
  void SwitchRows(int, int);
  void GetRow(int, Array<T> &) const;
  void SetRow(int, const Array<T> &);

  void SwitchColumn(int, Array<T> &);
  void SwitchColumns(int, int);
  void GetColumn(int, Array<T> &) const;
  void SetColumn(int, const Array<T> &);

  /// Returns the transpose of the rectangular array
  RectArray<T> Transpose() const;

  /// @name Range checking functions; returns true only if valid index/size
  //@{
  /// check for correct row index
  bool CheckRow(int row) const;
  /// check row vector for correct column boundaries
  bool CheckRow(const Array<T> &) const;
  /// check for correct column index
  bool CheckColumn(int col) const;
  /// check column vector for correct row boundaries
  bool CheckColumn(const Array<T> &) const;
  /// check row and column indices
  bool Check(int row, int col) const;
  /// check matrix for same row and column boundaries
  bool CheckBounds(const RectArray<T> &) const;
  //@
};


//------------------------------------------------------------------------
//            RectArray<T>: Bounds-checking member functions
//------------------------------------------------------------------------

template <class T> bool RectArray<T>::CheckRow(int row) const
{
  return (minrow <= row && row <= maxrow);
}

template <class T> bool RectArray<T>::CheckRow(const Array<T> &v) const
{
  return (v.First() == mincol && v.Last() == maxcol);
}

template <class T> bool RectArray<T>::CheckColumn(int col) const
{
  return (mincol <= col && col <= maxcol);
}

template <class T> bool RectArray<T>::CheckColumn(const Array<T> &v) const
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

template <class T> RectArray<T>::RectArray()
  : minrow(1), maxrow(0), mincol(1), maxcol(0), data(nullptr)
{ }

template <class T> RectArray<T>::RectArray(unsigned int rows,
						 unsigned int cols)
  : minrow(1), maxrow(rows), mincol(1), maxcol(cols)
{
  data = (rows > 0) ? new T *[maxrow] - 1 : nullptr;
  for (int i = 1; i <= maxrow;
       data[i++] = (cols > 0) ? new T[maxcol] - 1 : nullptr);
}

template <class T>
RectArray<T>::RectArray(int minr, int maxr, int minc, int maxc)
  : minrow(minr), maxrow(maxr), mincol(minc), maxcol(maxc)
{
  data = (maxrow >= minrow) ? new T *[maxrow - minrow + 1] - minrow : nullptr;
  for (int i = minrow; i <= maxrow;
       data[i++] = (maxcol - mincol + 1) ? new T[maxcol - mincol + 1] - mincol : nullptr);
}

template <class T> RectArray<T>::RectArray(const RectArray<T> &a)
  : minrow(a.minrow), maxrow(a.maxrow), mincol(a.mincol), maxcol(a.maxcol)
{
  data = (maxrow >= minrow) ? new T *[maxrow - minrow + 1] - minrow : nullptr;
  for (int i = minrow; i <= maxrow; i++)  {
    data[i] = (maxcol >= mincol) ? new T[maxcol - mincol + 1] - mincol : nullptr;
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
    
    data = (maxrow >= minrow) ? new T *[maxrow - minrow + 1] - minrow : nullptr;
  
    for (i = minrow; i <= maxrow; i++)  {
      data[i] = (maxcol >= mincol) ? new T[maxcol - mincol + 1] - mincol : nullptr;
      for (int j = mincol; j <= maxcol; j++)
	data[i][j] = a.data[i][j];
    }
  }
    
  return *this;
}

//------------------------------------------------------------------------
//                  RectArray<T>: Data access members
//------------------------------------------------------------------------

template <class T> int RectArray<T>::NumRows() const
{ return maxrow - minrow + 1; }

template <class T> int RectArray<T>::NumColumns() const
{ return maxcol - mincol + 1; }

template <class T> int RectArray<T>::MinRow() const    { return minrow; }
template <class T> int RectArray<T>::MaxRow() const    { return maxrow; }
template <class T> int RectArray<T>::MinCol() const { return mincol; }
template <class T> int RectArray<T>::MaxCol() const { return maxcol; }

template <class T> T &RectArray<T>::operator()(int r, int c)
{
  if (!Check(r, c))  throw IndexException();

  return data[r][c];
}

template <class T> const T &RectArray<T>::operator()(int r, int c) const
{
  if (!Check(r, c))  throw IndexException();

  return data[r][c];
}

//------------------------------------------------------------------------
//                   RectArray<T>: Row and column rotation
//------------------------------------------------------------------------

template <class T> void RectArray<T>::RotateUp(int lo, int hi)
{
  if (lo < minrow || hi < lo || maxrow < hi)  throw IndexException();

  T *temp = data[lo];
  for (int k = lo; k < hi; k++)
    data[k] = data[k + 1];
  data[hi] = temp;
}

template <class T> void RectArray<T>::RotateDown(int lo, int hi)
{
  if (lo < minrow || hi < lo || maxrow < hi)  throw IndexException();

  T *temp = data[hi];
  for (int k = hi; k > lo; k--)
    data[k] = data[k - 1];
  data[lo] = temp;
}

template <class T> void RectArray<T>::RotateLeft(int lo, int hi)
{
  if (lo < mincol || hi < lo || maxcol < hi)   throw IndexException();
  
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
  if (lo < mincol || hi < lo || maxcol < hi)   throw IndexException();

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

template <class T> void RectArray<T>::SwitchRow(int row, Array<T> &v)
{
  if (!CheckRow(row))  throw IndexException();
  if (!CheckRow(v))    throw DimensionException();

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
  if (!CheckRow(i) || !CheckRow(j))   throw IndexException();
  T *temp = data[j];
  data[j] = data[i];
  data[i] = temp;
}

template <class T> void RectArray<T>::GetRow(int row, Array<T> &v) const
{
  if (!CheckRow(row))   throw IndexException();
  if (!CheckRow(v))     throw DimensionException();

  T *rowptr = data[row];
  for (int i = mincol; i <= maxcol; i++)
    v[i] = rowptr[i];
}

template <class T> void RectArray<T>::SetRow(int row, const Array<T> &v)
{
  if (!CheckRow(row))   throw IndexException();
  if (!CheckRow(v))     throw DimensionException();

  T *rowptr = data[row];
  for (int i = mincol; i <= maxcol; i++)
    rowptr[i] = v[i];
}

//-------------------------------------------------------------------------
//                RectArray<T>: Column manipulation functions
//-------------------------------------------------------------------------

template <class T> void RectArray<T>::SwitchColumn(int col, Array<T> &v)
{
  if (!CheckColumn(col))   throw IndexException();
  if (!CheckColumn(v))     throw DimensionException();

  for (int i = minrow; i <= maxrow; i++)   {
    T tmp = data[i][col];
    data[i][col] = v[i];
    v[i] = tmp;
  }
}

template <class T> void RectArray<T>::SwitchColumns(int a, int b)
{
  if (!CheckColumn(a) || !CheckColumn(b))   throw IndexException();

  for (int i = minrow; i <= maxrow; i++)   {
    T tmp = data[i][a];
    data[i][a] = data[i][b];
    data[i][b] = tmp;
  }
}

template <class T> void RectArray<T>::GetColumn(int col, Array<T> &v) const
{
  if (!CheckColumn(col))  throw IndexException();
  if (!CheckColumn(v))    throw DimensionException();

  for (int i = minrow; i <= maxrow; i++)
    v[i] = data[i][col];
}

template <class T> void RectArray<T>::SetColumn(int col, const Array<T> &v)
{
  if (!CheckColumn(col))   throw IndexException();
  if (!CheckColumn(v))     throw DimensionException();

  for (int i = minrow; i <= maxrow; i++)
    data[i][col] = v[i];
}

//-------------------------------------------------------------------------
//                        RectArray<T>: Transpose
//-------------------------------------------------------------------------

template <class T> RectArray<T> RectArray<T>::Transpose() const
{
  RectArray<T> tmp(mincol, maxcol, minrow, maxrow);
 
  for (int i = minrow; i <= maxrow; i++)
    for (int j = mincol; j <= maxrow; j++)
      tmp(j,i) = (*this)(i,j);

  return tmp;
}

} // end namespace Gambit

#endif // LIBGAMBIT_RECARRAY_H
