#ifndef GMATRIX_H
#define GMATRIX_H

#include "gbmatrix.h"
#include <stdio.h>

template <class T> class gMatrix: private gBMatrix<T> {
 public:
  gMatrix(void) { ; }
  gMatrix(int,int);
  gMatrix(const gMatrix<T> &M)
    { data = M.data; }
  ~gMatrix() { ; }

  // OPERATOR OVERLOADING
  // assignment
  gBMatrix<T>::operator=;

  // equality
  gBMatrix<T>::operator==;
  gBMatrix<T>::operator!=;
  gBMatrix<T>::operator==;
  gBMatrix<T>::operator!=;

  // multiplication
  gMatrix<T> operator*(const gMatrix<T> &) const;
  gMatrix<T> operator*(T) const;
  void operator*=(const gMatrix<T> &);
  void operator*=(T);

  // division (by a scalar)
  gMatrix<T> operator/(T) const;

  // addition and subtraction
  gMatrix<T> operator+(const gMatrix<T> &) const;
  gMatrix<T> operator-(const gMatrix<T> &) const;
  void operator+=(const gMatrix<T> &);
  void operator-=(const gMatrix<T> &);

  // ADDING AND DELETING ELEMENTS
  void AddRow(const gVector<T> &V)
    { assert(V.Length() == Width()); gBMatrix<T>::AddRow(V); }
  gBMatrix<T>::RemoveRow;
  void AddColumn(const gVector<T> &);
  void RemoveColumn(int);
  gBMatrix<T>::SwitchRows;
  void SwitchColumns(int, int);
  void SwitchRow(int row, const gVector<T> &V)
    { assert(V.Length() == Width()); gBMatrix<T>::SwitchRow(row, V); }
  void SwitchColumn(int, const gVector<T> &);
  void SwitchColumn(int, int);

  // DATA ACCESS
  gBMatrix<T>::operator();
  gBMatrix<T>::GetRow;
  gBMatrix<T>::NumRows;
  int NumColumns() const
    { return Width(); }
  int Width() const
    { return data[1].Length(); }
  gBMatrix<T>::Height;
  gVector<T> GetColumn(int) const;
  gMatrix<T> GetSlice(int,int,int,int) const;
  gMatrix<T> GetSubMatrix(const gSet<int> &, const gSet<int> &) const;

  gBMatrix<T>::Dump;
};

#ifdef __GNUC__
#define INLINE inline
#elif defined(__BORLANDC__)
#define INLINE 
#else
#error Unsupported compiler type
#endif    // __GNUC__, __BORLANDC__

template <class T> inline output& operator<<(output &to,const gMatrix<T> &M)
{
  M.Dump(to); return to;
}

template <class T> gMatrix<T>::gMatrix(int row, int col)
{
  gVector<gVecFoo> tmp(1,row);
  gVector<T> tmp1(1,col);
  for(int i=1;i<=row;i++)
    tmp[i] = tmp1;
  (*this).data = tmp;
}

template <class T> gMatrix<T> gMatrix<T>::operator*(const gMatrix<T> &M) const
{
  assert(Width() == M.Height());
  gMatrix out(Height(),M.Width());
  for(int i=1;i<=Height();i++)
    for(int j=1;j<=M.Width();j++)
      {
	for(int k=1;k<=Width();k++)
	  out(i,j) += (*this)(i,k) * M(k,j);
      }
  return out;
}

template <class T> gMatrix<T> gMatrix<T>::operator*(T c) const
{
  gMatrix<T> tmp(*this);
  for(int i=data.First();i<=data.Last();i++)
    tmp.data[i] = (tmp.data[i])*c;
  return tmp;
}

template <class T> void gMatrix<T>::operator*=(const gMatrix<T> &M)
{
  (*this) = (*this)*M;
}

template <class T> void gMatrix<T>::operator*=(T c)
{
  (*this) = (*this)*c;
}

template <class T> gMatrix<T> gMatrix<T>::operator/(T c) const
{
  gMatrix<T> tmp(Height(),Width());
  for(int i=1;i<=Height();i++)
    tmp.data[i] = data[i]/c;
  return tmp;
}

template <class T> gMatrix<T> gMatrix<T>::operator+(const gMatrix<T> &M) const
{
  //make sure they are the same size
  assert((Width() == M.Width())&&(Height() == M.Height()));
  gMatrix<T> tmp(Width(),Height());
  tmp.data = data + M.data;
  return tmp;
}

template <class T> gMatrix<T> gMatrix<T>::operator-(const gMatrix<T> &M) const
{
  assert((Width()==M.Width())&&(Height() == M.Height()));
  gMatrix<T> tmp(Height(),Width());
  tmp.data = data - M.data;
  return tmp;
}

template <class T> void gMatrix<T>::operator+=(const gMatrix<T> &M)
{
  *this = (*this)+M;
}

template <class T> void gMatrix<T>::operator-=(const gMatrix<T> &M)
{
  *this = (*this)-M;
}

template <class T> void gMatrix<T>::AddColumn(const gVector<T> &V)
{
  assert(Height() == V.Length());
  int width = Width();
  gVector<T> tmp(1,width+1);
  for(int i=1;i<=Height();i++) {
    for(int j=1;j<=width;j++)
      tmp[j] = (data[i])[j];
    tmp[j] = V[i];
    data[i] = tmp;
  }
}

template <class T> void gMatrix<T>::RemoveColumn(int k)
{
  assert(k <= Width());
  int width = Width();
  gVector<T> tmp(1,width-1);
  for(int i=1;i<=Height();i++) {
    int j=1;
    while(j!=k) {
      tmp[j] = (data[i])[j];
      j++;
    }
    j++;
    while(j<=width) {
      tmp[j-1] = (data[i])[j];
      j++;
    }
    data[i] = tmp;
  }
}

template <class T> void gMatrix<T>::SwitchColumns(int k, int l)
{
  assert((k<=Width())&&(l<=Width()));
  gVector<T> tmp;
  tmp = GetColumn(k);
  SwitchColumn(k,GetColumn(l));
  SwitchColumn(l,tmp);
}

template <class T> void gMatrix<T>::SwitchColumn(int k, const gVector<T> &col)
{
  assert((k<=Width())&&(col.Length()==Height()));
  for(int i=1;i<=Height();i++)
    (data[i])[k] = col[i];
}

template <class T> gVector<T> gMatrix<T>::GetColumn(int k) const
{
  assert(k<=Width());
  gVector<T> tmp(1,Height());
  for(int i=1;i<=Height();i++)
    tmp[i] = (data[i])[k];
  return tmp;
}

template <class T> gMatrix<T> gMatrix<T>::GetSlice(int minrow, int mincolumn,
						   int maxrow, int maxcolumn)
     const
{
  assert((minrow<Height())&&(maxrow<=Height())&&
	 (mincolumn<Width())&&(maxcolumn<=Width()));
  gMatrix<T> out(maxrow-minrow+1,maxcolumn-mincolumn+1);
  for(int i=1, k=minrow;k<=maxrow;i++,k++)
    for(int j=1, l=mincolumn;l<=maxcolumn;j++,l++)
      out(i,j) = (*this)(k,l);
  return out;
}

template <class T> gMatrix<T> gMatrix<T>::GetSubMatrix(const gSet<int> &Rows,
					const gSet<int> &Columns) const
{
  int rlength = Rows.Length();
  int height = Height();
  int width = Width();
  int clength = Columns.Length();
  for(int i=1;i<=rlength;i++)
    assert(Rows[i]<=height);
  for(i=1;i<=clength;i++)
    assert(Columns[i]<=width);
  gMatrix<T> tmp(rlength,clength);
  for(i=1;i<=rlength;i++)
    for(int j=1;j<=clength;j++)
      tmp(i,j) = (*this)(Rows[i],Columns[j]);
  return tmp;
}
#endif     //GMATRIX_H
