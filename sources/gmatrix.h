#ifndef GMATRIX_H
#define GMATRIX_H

#include "gbmatrix.h"
#include <stdio.h>

//
// Notice.  This class will not work with IBM Aix machines due to a compiler
// bug that does not properly reference the data (second subscript) correctly.
//
template <class T> class gMatrix: private gBMatrix<T> {
 public:
  gMatrix(void) { ; }
  gMatrix(int,int);
  gMatrix(const gMatrix<T> &M)
    { data = M.data; }
  ~gMatrix() { ; }

  // OPERATOR OVERLOADING
  // assignment
  void operator=(const gMatrix<T> &M)
    { if(Height()==M.Height() && Width()==M.Width())
	for(int i=1;i<=Height();i++)
	  for(int j=1;j<=Width();j++)
	    (*this)(i,j) = M(i,j);
      else gBMatrix<T>::operator=(M);
    }
      

  // equality
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

  gMatrix<T> Invert(void) const;
  gMatrix<T> Pivot(int, int) const;
  T Determinant() const;

  gBMatrix<T>::Dump;
};

template <class T> class gNRMatrix: private gBMatrix<T> {
 public:
  gNRMatrix(void) { ; }
  gNRMatrix(int k)
    { gBMatrix<T>::gBMatrix(k); }
  gNRMatrix(const gNRMatrix<T> &M)
    { data = M.data; }
  ~gNRMatrix() { ; }

  // OPERATOR OVERLOADING
  // assignment
  gBMatrix<T>::operator=;

  // equality
  gBMatrix<T>::operator==;
  gBMatrix<T>::operator!=;

  // ADDING AND DELETING ELEMENTS
  gBMatrix<T>::AddRow;
  gBMatrix<T>::RemoveRow;
  void AddColumn(const gVector<T> &);
  void RemoveColumn(int);
  gBMatrix<T>::SwitchRows;
  gBMatrix<T>::SwitchRow;
  void SwitchColumn(int,const gVector<T>&);
  void SwitchColumns(int, int);
  void RemoveElement(int,int);
  void AddElement(int,T&);

  // DATA ACCESS
  gBMatrix<T>::operator();
  gBMatrix<T>::GetRow;
  gVector<T> GetColumn(int) const;
  gBMatrix<T>::NumRows;
  gBMatrix<T>::Height;
  int NumColumns(int k) const
    { return Width(k); }
  int Width(int k) const
    { return data[k].Length(); }

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
	for(int k=1;k<=Width();k++)
	  out(i,j) += (*this)(i,k) * M(k,j);
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

template <class T> gMatrix<T> gMatrix<T>::Pivot(int k, int l) const
{
  assert((k<=Height())&&(l<=Width()));
  assert((*this)(k,l)!=0);
  int j;
  gMatrix<T> tmp(*this);
  tmp.data[k] = data[k]/tmp(k,l);
  for(int i=1;i<=Height();i++)
    if(i!=k) {
      tmp.data[i] = tmp.data[i]-(tmp.data[k] * tmp(i,l));
    }
  return tmp;
}

template <class T> gMatrix<T> gMatrix<T>::Invert(void) const
{ 
  assert(Height()==Width()); // square matricies
  int i,j;
  T factor;
  gMatrix<T> tmp1(*this);
  gMatrix<T> tmp(Width(),Height());
  for(i=1;i<=Height();i++) // make identity matirx
    tmp(i,i) = 1;
  for(i=1;i<=Width();i++) {
    tmp.data[i] = tmp.data[i]/tmp1(i,i);
    tmp1.data[i] = tmp1.GetRow(i)/tmp1(i,i);
    for(j=1;j<=Height();j++)
      if(j!=i) {
	tmp.data[j] = tmp.data[j] - (tmp.data[i]*tmp1(j,i));
	tmp1.data[j] = tmp1.data[j] - (tmp1.data[i]*tmp1(j,i));
      }
  }
  return tmp;
}

template <class T> T gMatrix<T>::Determinant() const
{
  assert(Height()==Width());
  T result = 0;
  if(Height()==2)
    result = (*this)(1,1) * (*this)(2,2) - (*this)(1,2) * (*this)(2,1);
  else if(Height()==1) result = (*this)(1,1);
  else {
    int l=1;
    gSet<int> rows, cols;
    for(int i=2;i<=Height();i++)
      rows.Append(i);
    for(i=1;i<=Width();i++) {
      for(int j=1;j<=Width();j++)
	if(j!=i) cols.Append(j);
      gMatrix tmp = GetSubMatrix(rows,cols);
      result += tmp.Determinant() * l * (*this)(1,i);
      l *= -1;
      cols.Flush();
    }
  }
  return result;
}

//*****************************************************************************
//These are some of the member fuctions for gNRMatrix<T>.
//*****************************************************************************

template <class T> output& operator<<(output &to,const gNRMatrix<T> M)
{
  M.Dump(to); return to;
}

template <class T> void gNRMatrix<T>::AddColumn(const gVector<T> &V)
{
  assert(V.Length()<=Height());
  int j;
  for(int i=1;i<=V.Length();i++) {
    gVector<T> tmp(1,data[i].Length()+1);
    for(j=data[i].First();j<=data[i].Last();j++)
      tmp[j]=(data[i])[j];
    tmp[j] = V[i];
    data[i] = tmp;
  }
}

template <class T> void gNRMatrix<T>::RemoveColumn(int k)
{
  int j,width;
  for(int i=1;i<=Height();i++)
    width = Width(i);
    if(k<=Width(i)) {
      gVector<T> tmp(1,Width(i)-1);
      j = 1;
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

template <class T> void gNRMatrix<T>::SwitchColumns(int k, int l)
{
  T tmp;
  for(int i=1;i<=Height();i++)
    if(k<=Width(i)&&l<=Width(i)) {
      tmp = (*this)(i,k);
      (*this)(i,k) = (*this)(i,l);
      (*this)(i,l) = tmp;
    }
}

template <class T> void gNRMatrix<T>::SwitchColumn(int k,const gVector<T> &V)
{
  assert(V.Length()<=Height());
  for(int i=1;i<=Height();i++) {
    assert(k<=Width(i));
    (*this)(i,k) = V[i];
  }
}

template <class T> gVector<T> gNRMatrix<T>::GetColumn(int k) const
{
  gVector<T> tmp(1,Height());
  for(int i=1;i<=Height();i++) {
    if(k<=Width(i))
      tmp[i] = (*this)(i,k);
    else tmp[i] = 0;
  }
  return tmp;
}

template <class T> void gNRMatrix<T>::RemoveElement(int i, int j)
{
  assert(i<=Height()); assert(j<=Width(i));
  gVector<T> tmp(1,Width(i)-1);
  int k = 1;
  while(k!=j) {
    tmp[k] = (data[i])[k];
    k++;
  }
  k++;
  while(k<=Width(i)) {
    tmp[k-1] = (data[i])[k];
    k++;
  }
  data[i] = tmp;
}

template <class T> void gNRMatrix<T>::AddElement(int k,T& c)
{
  assert(k<=Height());
  gVector<T> tmp(1,Width(k)+1);
  for(int i=1;i<=Width(k);i++)
    tmp[i] = (*this)(k,i);
  tmp[i] = c;
  data[k] = tmp;
}

#endif     //GMATRIX_H
