//#
//# FILE: gmatrix.h -- Implementation of normal and NonRectangular
//#                    matrix classes
//#
//# $Id$
//#

#ifndef GMATRIX_H
#define GMATRIX_H

#include "gbmatrix.h"

#ifdef __GNUC__
#define INLINE inline
#elif definded(__BORLANDC__)
#define INLINE 
#else
#error Unsupported compiler type
#endif          // __GNUC__, __BORLANDC__

//
// <category lib=  sect= >
//
// A gMatrix is a class designed to allow basic manipulations of matricies.
//
// In order to use a type T in a gMatrix, it must have all the basic
// arithmetic operators defined on it.
//
// <warn> This class will not work with IBM AIX machines due to a compiler
//        bug that does not properly reference the data (second subscript)
//        correctly
//
template <class T> class gMatrix: public gBaseMatrix<T>
{
 protected:
  int minimum_col;
  int CheckCol(int col) const
    {
      int tmp = (col >= minimum_col && col <= Width());
      if(!tmp) gerr << "column is out of bounds\n";
      return tmp;
    }
  int CheckRow(int row) const
    {
      int tmp = (row >= minimum && row <= Height());
      if(!tmp) gerr << "row is out of bounds\n";
      return tmp;
    }
  int CheckDimensions(const gMatrix<T> &M) const
    {
      int tmp = (Height() == M.Height() && Width() == M.Width() && 
		 MinIndex() == MinIndex() && MinCol() == MinCol());
      if(!tmp) gerr << "matricies are not the same size\n";
      return tmp;
    }

 public:
//
// Initialize an empty gMatrix
//
  gMatrix(void) {}
//
// Initialize a gMatrix starting from a minimum index, and given a maximum
// index for the rows, and the columns
//
  gMatrix(int, int, int);
//
// Initialize a gMatrix starting from 1 and going to a maximum index for the
// rows, and a maximum index for the columns
//
  gMatrix(int, int);
//
// Initialize a gMatrix starting from a minimum index for the rows, and going
// to a maximum index for the rows, and going from a minimum index to a
// maximum index for the columns
//
  gMatrix(int, int, int, int);
//
// Copy constructor
//
  gMatrix(const gMatrix<T> &M) : minimum_col(M.MinCol())
    { minimum = M.minimum;
      data = M.data;
    }
//
// Deallocate a gMatrix
//
  ~gMatrix() {}

//
// Multiplication of matricies by other matricies and constants.  In matrix
// multiplication it is checked that the matricies are of the correct size,
// and a matrix beginning at one is released.
//+grp
  gMatrix<T> operator*(const gMatrix<T> &) const;
  gMatrix<T> operator*(T) const;
  gMatrix<T> operator*=(const gMatrix<T> &M)
    { 
      *this = *this * M;
      return *this;
    }
  gMatrix<T> operator*=(T c)
    {
      *this = *this * c;
      return *this;
    }
//-grp
  gMatrix<T> operator/(T) const;

  gMatrix<T> operator+(const gMatrix<T> &) const;
  gMatrix<T> operator-(const gMatrix<T> &) const;
  gMatrix<T> operator+=(const gMatrix<T> &M)
    {
      *this = *this + M;
      return *this;
    }
  gMatrix<T> operator-=(const gMatrix<T> &M)
    {
      *this = *this - M;
      return *this;
    }

  void AddRow(const gVector<T> &V)
    {
      assert(V.First() == MinCol() && V.Last() == Width());
      gBaseMatrix<T>::AddRow(V);
    }
  void AddColumn(const gVector<T> &);
  void RemoveColumn(int);
  gVector<T> GetColumn(int) const;
  void SwitchColumn(int, const gVector<T> &);
  void SwitchColumns(int, int);

  int NumColumns(void) const
    { return data[MinCol()].Length(); }
  int Width(void) const
    { return data[MinCol()].Last(); }
  int MinCol(void) const
    { return minimum_col; }

  gMatrix<T> GetSlice(int, int, int, int) const;
  gMatrix<T> GetSubMatrix(const gSet<int> &, const gSet<int> &)
    const;

  gMatrix<T> Invert(void) const;
  gMatrix<T> ExternalPivot(int, int) const;
  void Pivot(int, int);
  T Determinant(void) const;
};

template <class T> inline gOutput &
operator<<(gOutput &to, const gMatrix<T> &M)
{
  M.Dump(to); return to;
}

template <class T>
gMatrix<T>::gMatrix(int rows, int columns, int min)
{
  minimum = min;
  minimum_col = min;
  gVector<gVecFoo> newdata(min, rows);
  data = newdata;
  gVector<T> tmp(MinCol(), columns);
  for(int i = data.First(); i <= data.Last(); i++)
    data[i] = tmp;
}

template <class T>
gMatrix<T>::gMatrix(int rows, int cols)
{
  minimum = 1;
  minimum_col = 1;
  gVector<gVecFoo> newdata(MinIndex(), rows);
  gVector<T> tmp(MinCol(), cols);
  data = newdata;
  for(int i = data.First(); i <= data.Last(); i++)
    data[i] = tmp;
}

template <class T>
gMatrix<T>::gMatrix(int minrow, int maxrow, int mincol, int maxcol)
{
  minimum = minrow;
  minimum_col = mincol;
  gVector<gVecFoo> newdata(MinIndex(), maxrow);
  gVector<T> tmp(MinCol(), maxcol);
  data = newdata;
  for(int i = data.First(); i <= data.Last(); i++)
    data[i] = tmp;
}

template <class T> gMatrix<T>
gMatrix<T>::operator*(const gMatrix<T> &M) const
{
  assert(NumColumns() == M.NumRows());
  gMatrix<T> out(NumRows(), M.NumColumns());
  for(int i = 0; i < NumRows(); i++)
    for(int j = 0; j < M.NumColumns(); j++)
      for(int k = 0; k < NumColumns(); k++)
	out(i + out.MinIndex(), j + out.MinCol()) += 
	  (*this)(i + MinIndex(), k + MinCol()) *
	    M(k + M.MinIndex(), j + M.MinCol());
  return out;
}

template <class T> gMatrix<T>
gMatrix<T>::operator*(T c) const
{
  gMatrix<T> out(*this);
  for(int i = MinIndex(); i <= Height(); i++)
    out.data[i] = data[i] * c;
  return out;
}

template <class T> gMatrix<T>
gMatrix<T>::operator/(T c) const
{
  gMatrix<T> out(*this);
  for(int i = MinIndex(); i <= Height(); i++)
    out.data[i] = data[i]/c;
  return out;
}

template <class T> INLINE gMatrix<T>
gMatrix<T>::operator+(const gMatrix<T> &M) const
{
  assert(CheckDimensions(M));
  gMatrix<T> out(*this);
  for(int i = MinIndex(); i <= Height(); i++)
    for(int j = data[i].First(); j <= data[i].Last(); j++)
      out(i,j) += M(i,j);
  return out;
}

template <class T> INLINE gMatrix<T>
gMatrix<T>::operator-(const gMatrix<T> &M) const
{
  assert(CheckDimensions(M));
  gMatrix<T> out(*this);
  for(int i = MinIndex(); i <= Height(); i++)
    for(int j = MinCol(); i <= Width(); i++)
      out(i,j) -= M(i,j);
  return out;
}

template <class T> void
gMatrix<T>::AddColumn(const gVector<T> &V)
{
  assert(V.Length() == Height());
  int width = Width();
  gVector<T> newrow(MinCol(), width+1);
  for(int i = MinIndex(); i <= Height(); i++)
    {
      for(int j = MinCol(); j <= width; j++)
	newrow[j] = data[i][j];
      newrow[j] = V[i];
      data[i] = newrow;
    }
}

template <class T> void
gMatrix<T>::RemoveColumn(int col)
{
  assert(CheckCol(col));
  int width = Width();
  gVector<T> newrow(MinCol(), width-1);
  for(int i = MinIndex(); i <= Height(); i++)
    {
      int j = MinCol();
      while(j != col)
	{
	  newrow[j] = data[i][j];
	  j++;
	}
      j++;
      while(j <= width)
	{
	  newrow[j-1] = data[i][j];
	  j++;
	}
      data[i] = newrow;
    }
}

template <class T> void
gMatrix<T>::SwitchColumn(int col, const gVector<T> &V)
{
  assert(CheckCol(col));
//  assert(V.First() == MinCol() && V.Last() == Width()+MinCol());
  for(int i = data.First(); i <= data.Last(); i++)
    data[i][col] = V[i];
}

template <class T> void
gMatrix<T>::SwitchColumns(int col1, int col2)
{
  assert(CheckCol(col1) && CheckCol(col2));
  gVector<T> tmp(GetColumn(col1));
  SwitchColumn(col1, GetColumn(col2));
  SwitchColumn(col2, tmp);
}

template <class T> gVector<T>
gMatrix<T>::GetColumn(int col) const
{
  assert(CheckCol(col));
  gVector<T> out(MinIndex(), Height());
  for(int i = data.First(); i <= data.Last(); i++)
    out[i] = data[i][col];
  return out;
}

template <class T> gMatrix<T>
gMatrix<T>::GetSlice(int minrow, int maxrow,
		     int mincol, int maxcol) const
{
  assert(CheckRow(minrow) && CheckRow(maxrow));
  assert(CheckCol(mincol) && CheckCol(maxcol));
  assert(minrow <= maxrow && mincol <= maxcol);
  gMatrix<T> out(maxrow-minrow+1, maxcol-mincol+1);
  for(int i = 1, row = minrow; row <= maxrow; i++, row++)
    for(int j = 1, col = mincol; col <= maxcol; j++, col++)
      out(i,j) = (*this)(row, col);
  return out;
}

template <class T> gMatrix<T>
gMatrix<T>::GetSubMatrix(const gSet<int> &Rows,
			 const gSet<int> &Cols) const
{
  int rows = Rows.Length();
  int cols = Cols.Length();
  for(int i = 1; i <= rows; i++)
    assert(CheckRow(Rows[i]));
  for(i = 1; i <= rows; i++)
    assert(CheckCol(Cols[i]));
  gMatrix<T> out(rows, cols, MinIndex());
  for(i = 1; i <= rows; i++)
    for(int j = 1; j <= cols; j++)
      out(i,j) = (*this)(Rows[i],Cols[j]);
  return out;
}

template <class T> gMatrix<T>
gMatrix<T>::ExternalPivot(int row, int col) const
{
  assert(CheckRow(row) && CheckCol(col));
  assert(data[row][col] != 0);
  int i;
  T tmp = (*this)(row,col);
  gMatrix<T> out(*this);
  out.data[row] = data[row]/tmp;
  for(i = data.First(); i <= data.Last(); i++)
    if(i != row)
      {
	out.data[i] = data[i] - (out.data[row] * data[i][col]);
      }
  return out;
}

template <class T> void
gMatrix<T>::Pivot(int row, int col)
{
  assert(CheckRow(row) && CheckCol(col));
  assert(data[row][col] != 0);
  int i;
  T tmp = (*this)(row,col);
  gVector<T> temp(data[row]);
  data[row] = temp/tmp;
  for(i = data.First(); i <= data.Last(); i++)
    if(i != row)
      {
	temp = data[i];
	data[i] = temp - (data[row] * data[i][col]);
      }
}

template <class T> gMatrix<T>
gMatrix<T>::Invert(void) const
{
  assert(NumRows() == NumColumns());
  int i, j;
  T temp;
  gMatrix<T> out(NumRows(), NumColumns());
  gMatrix<T> tmp(*this);
  int oRa = out.MinIndex(), oCa = out.MinCol();
  int tRa = tmp.MinIndex(), tCa = tmp.MinCol();
  for(i = out.MinIndex(); i <= out.Width(); i++)
    out(i,i) = 1;
  for(i = 0; i < NumRows(); i++)
    {
      temp = tmp(i + tRa, i + tCa);
      out.data[i + oRa] = out.data[i + oRa]/temp;
      tmp.data[i + tRa] = tmp.data[i + tRa]/temp;
      for(j = 0; j < NumColumns(); j++)
	if(j != i)
	  {
	    temp = tmp(j + tRa,i + tCa);
	    out.data[j + oRa] = out.data[j + oRa] - (out.data[j + oRa] * temp);
	    tmp.data[j + tRa] = tmp.data[j + tRa] - (tmp.data[j + tRa] * temp);
	  }
    }
  return out;
}

template <class T> T
gMatrix<T>::Determinant(void) const
{
  assert(NumRows() == NumColumns());
  T result = 0;
  if(NumRows() == 2)
    result = (*this)(MinIndex(),MinCol()) * (*this)(MinIndex()+1,MinCol()+1) -
      (*this)(MinIndex(),MinCol()+1) * (*this)(MinIndex()+1,MinCol());
  else if(NumRows() == 1) result = (*this)(MinIndex(),MinCol());
  else
    {
      gMatrix<T> tmp;
      int i, j;
      int l = 1;
      gSet<int> rows, cols;
      for(i = MinIndex() + 1; i <= Width(); i++)
        rows.Append(i);
     for(i = MinIndex(); i <= Width(); i++)
	{
          for(j = 1; j <= Width(); j++)
            if(j != i) cols.Append(j);
	  tmp = GetSubMatrix(rows,cols);
	  result += (tmp.Determinant() * l * (*this)(1,i));
	  l *= -1;
          cols.Flush();
	}
    }
  return result;
}

template <class T> class gNonRectangularMatrix: public gBaseMatrix<T>
{
 private:
  int CheckRow(int row) const
    {
      int tmp = (row >= data.First() && row <= data.Last());
      if(tmp) gerr << "row is out of bounds\n";
      return tmp;
    }
  int CheckCol(int col, int row) const
    {
      int tmp = (col >= data[row].First() && col <= data[row].Last());
      if(tmp) gerr << "column is out of bounds\n";
      return tmp;
    }

 public:
  gNonRectangularMatrix(void) {}
  gNonRectangularMatrix(int height) : gBaseMatrix<T>::gBaseMatrix(height,1) {}
  gNonRectangularMatrix(const gNonRectangularMatrix<T> &M)
    : minimum(M.minimum), data(M.data) {}
  ~gNonRectangularMatrix() {}

  void AddColumn(const gVector<T> &);
  void RemoveColumn(int);
  void SwitchColumn(int, const gVector<T> &);
  void SwitchColumns(int, int);
  void RemoveElement(int, int);
  void AddElement(int, T);
  int NumColumns(int row) const
    { return data[row].Length(); }
  int Width(int row) const
    { return data[row].Length(); }
};

template <class T> inline gOutput &
  operator<<(gOutput &to, const gNonRectangularMatrix<T> &M)
{
  M.Dump(to); return to;
}

template <class T> void
gNonRectangularMatrix<T>::AddColumn(const gVector<T> &V)
{
  assert(V.Length() <= Height());
  int i, j;
  for(i = V.First(); i <= V.Last(); i++)
    {
      gVector<T> tmp(MinIndex(), Width(i)+1);
      for(j = data[i].First(); j <= data[i].Last(); j++)
	tmp[j] = data[i][j];
      tmp[j] = V[i];
      data[i] = tmp;
    }
}

template <class T> void
  gNonRectangularMatrix<T>::RemoveColumn(int col)
{
  int i, j;
  for(i = data.First(); i <= data.Last(); i++)
    {
      if(CheckCol(col,i))
	{
	  gVector<T> tmp(data[i].First(), data[i].Last()-1);
	  j = data[i].First();
	  while(j != col)
	    {
	      tmp[j] = data[i][j];
	      j++;
	    }
	  j++;
	  while(j <= Width(i))
	    {
	      tmp[j-1] = data[i][j];
	      j++;
	    }
	}
      data[i] = tmp;
    }
}

template <class T> void
  gNonRectangularMatrix<T>::SwitchColumn(int col, const gVector<T> &V)
{
  assert(V.Length() <= Height());
  for(int i = data.First(); i <= data.Last(); i++)
    {
      assert(CheckCol(col,i));
      (*this)(i,k) = V[i];
    }
}

template <class T> void
  gNonRectangularMatrix<T>::SwitchColumns(int col1, int col2)
{
  T tmp;
  for(int i = data.First(); i <= data.Last(); i++)
      if(CheckCol(col1,i) && CheckCol(col2,i))
	{
	  tmp = data[i][col1];
	  (*this)(i,col1) = (*this)(i,col2);
	  (*this)(i,col2) = tmp;
	}
}

template <class T> gVector<T>
  gNonRectangularMatrix<T>::GetColumn(int col) const
{
  gVector<T> tmp(data.First(), data.Last());
  for(int i = data.First(); i <= data.Last(); i++)
    {
      if(CheckCol(col,i)) tmp[i] = (*this)(i,col);
      else tmp[i] = 0;
    }
  return tmp;
}

template <class T> void
  gNonRectangularMatrix<T>::RemoveElement(int row, int col)
{
  assert(CheckRow(row));
  assert(CheckCol(col,row));
  gVector<T> tmp(data[row].First(), data[row].Last()-1);
  int i = data[row].First();
  while(i != col)
    {
      tmp[i] = data[row][i];
      i++;
    }
  i++;
  while(i <= Width(i))
    {
      tmp[i-1] = data[row][i];
      i++;
    }
  data[row] = tmp;
}

template <class T> void
  gNonRectangularMatrix<T>::AddElement(int row, T to_add)
{
  assert(CheckRow(row));
  gVector<T> tmp(data[row].First(), data[row].Last()+1);
  for(int i = data[row].First(); i <= data[row].Last(); i++)
    tmp[i] = data[row][i];
  tmp[i] = to_add;
  data[k] = tmp;
}

#endif             //GMATRIX_H
