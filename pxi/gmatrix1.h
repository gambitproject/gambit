#ifndef GMATRIX1_H
#define GMATRIX1_H

#include "gvector1.h"
#include <stdio.h>
#include "gset.h"

template <class T> class gMatrix1{
 private:
	typedef gVector1<T> gVecFoo;

 public:
		gVector1<gVecFoo> data;

  // CONSTRUCTORS AND DESTRUCTORS
	// initialize an empty gMatrix1
	gMatrix1(void) { ;}
	gMatrix1(int, int);
	gMatrix1(const gMatrix1<T> &M) { data = M.data; }
	~gMatrix1() { ; }

  // OPERATOR OVERLOADING
  // assignment
	void operator=(const gMatrix1<T> &M)
    { if(this != &M) data = M.data; }

  // equality
	int operator==(const gMatrix1<T> &M) const
    { return (data == M.data); }
	int operator!=(const gMatrix1<T> &M) const
    { return !(data == M.data); }
  int operator==(T c) const;
  int operator!=(T c) const
    { return !(*this == c); }

	// ADDING AND DELETING ELEMENTS
	void AddRow(const gVector1<T> &);
  void RemoveRow(int);
	void AddColumn(const gVector1<T> &);
  void RemoveColumn(int);
  void SwitchRows(int,int);
	void SwitchRow(int,const gVector1<T> &);
  void SwitchColumns(int,int);
	void SwitchColumn(int,const gVector1<T> &);

  // DATA ACCESS
  T& operator()(int i,int j);
	T operator()(int,int) const;
	gVector1<T>& operator[](int i)
			{ if(i<=Height()) return data[i]; }
	gVector1<T>& GetRow(int i)
		{ if(i<=Height()) return data[i]; }
	gVector1<T> GetColumn(int) const;
	gMatrix1<T> GetSlice(int, int, int, int) const;
//  gMatrix1<T> GetSubMatrix(const gSet<int> &,const gSet<int> &) const;

  // GENERAL INFORMATION
  int NumRows() { return data.Length(); }
  int NumColumns() { return (data[1]).Length(); }
  int Width() const { return (data[1]).Length(); }
  int Height() const { return data.Length(); }

  // DEBUGGING OUTPUT
  void Dump(output &) const;
};

#ifdef __GNUC__
#define INLINE inline
#elif defined(__BORLANDC__)
#define INLINE 
#else
#error Unsupported compiler type
#endif    // __GNUC__, __BORLANDC__

template <class T> inline output& operator<<(output &to,const gMatrix1<T> &M)
{
  M.Dump(to); return to;
}

template <class T> gMatrix1<T>::gMatrix1(int row, int col)
{
	gVector1<gVecFoo> tmp(1,row);
  data = tmp;
	gVector1<T> rows(1,col);
  for(int i= data.First();i<=data.Last();i++)
    data[i] = rows;
}

template <class T> int gMatrix1<T>::operator==(T c) const
{
  int out=0;
  for(int i=data.First();i<=data.Last();i++)
    out += (data[i] == c);
}

template <class T> void gMatrix1<T>::AddRow(const gVector1<T> &V)
{
  assert(Width() == V.Length());
	gMatrix1<T> tmp(Height()+1,Width());
  for(int i=1;i<=Height();i++)
    tmp.data[i] = data[i];
  tmp.data[i] = V;
  (*this) = tmp;
}

template <class T> void gMatrix1<T>::RemoveRow(int k)
{
  assert(k <= Height());
  int height = Height();
	gMatrix1<T> tmp(height-1,Width());
  int i=1;
  while(i!=k) {
    tmp.data[i] = data[i];
    i++;
  }
  i++;
  while(i<=height) {
    tmp.data[i-1] = data[i];
    i++;
  }
  *this = tmp;
}

template <class T> void gMatrix1<T>::AddColumn(const gVector1<T> &V)
{
  assert(Height() == V.Length());
  int width = Width();
	gVector1<T> tmp(1,width+1);
  for(int i=1;i<=Height();i++) {
    for(int j=1;j<=width;j++)
      tmp[j] = data[i][j];
    tmp[j] = V[i];
    data[i] = tmp;
  }
}

template <class T> void gMatrix1<T>::RemoveColumn(int k)
{
  assert(k <= Width());
  int width = Width();
	gVector1<T> tmp(1,width-1);
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

template <class T> void gMatrix1<T>::SwitchRows(int k, int l)
{ //switch row k with row l
  assert((k<=Height())&&(l<=Height()));
	gVector1<T> tmp(data[k]);
  data[k] = data[l];
  data[l] = tmp;
}

template <class T> void gMatrix1<T>::SwitchRow(int k, const gVector1<T> &row)
{
  assert((k<=Height())&&(row.Length()==Width()));
  data[k] = row;
}

template <class T> void gMatrix1<T>::SwitchColumns(int k, int l)
{
  assert((k<=Width())&&(l<=Width()));
	gVector1<T> tmp;
  tmp = GetColumn(k);
  SwitchColumn(k,GetColumn(l));
  SwitchColumn(l,tmp);
}

template <class T> void gMatrix1<T>::SwitchColumn(int k, const gVector1<T> &col)
{
  assert((k<=Width())&&(col.Length()==Height()));
  for(int i=1;i<=Height();i++)
    (data[i])[k] = col[i];
}

template <class T> inline T& gMatrix1<T>::operator()(int i, int j)
{
  return data[i][j];
}

template <class T> inline T gMatrix1<T>::operator()(int i, int j) const
{
  return (data[i])[j];
}

template <class T> gVector1<T> gMatrix1<T>::GetColumn(int k) const
{
  assert(k<=Width());
	gVector1<T> tmp(1,Height());
  for(int i=1;i<=Height();i++)
    tmp[i] = (data[i])[k];
  return tmp;
}

template <class T> gMatrix1<T> gMatrix1<T>::GetSlice(int minrow, int mincolumn,
						   int maxrow, int maxcolumn)
     const
{
  assert((minrow<Height())&&(maxrow<=Height())&&
	 (mincolumn<Width())&&(maxcolumn<=Width()));
	gMatrix1<T> out(maxrow-minrow+1,maxcolumn-mincolumn+1);
  for(int i=1, k=minrow;k<=maxrow;i++,k++)
    for(int j=1, l=mincolumn;l<=maxcolumn;j++,l++)
      out(i,j) = (*this)(k,l);
  return out;
}

template <class T> void gMatrix1<T>::Dump(output &to) const
{
  for(int i=1;i<=Height();i++) {
    to << "[ ";
    for(int j=1;j<=Width();j++)
      to << (*this)(i,j) << " ";
    to << "]\n";
  }
}

#endif     //GMATRIX_H