//#
//# FILE: gmatrix.h -- Implementation of normal matrix classes
//#
//# $Id$
//#

#ifndef GMATRIX_H
#define GMATRIX_H

#include "gvector.h"
#include "gambitio.h"

#ifdef __GNUC__
#define INLINE inline
#elif defined(__BORLANDC__)
#define INLINE 
#else
#error Unsupported compiler type
#endif          // __GNUC__, __BORLANDC__


template <class T> class gMatrix   {
protected:
  int minrow,maxrow,mincol,maxcol;
  T **data;

  // check for correct row index
  int CheckRow(int row) const
    {
      return(minrow<=row && row <= maxrow);
    }
  // check row vector for correct column boundaries
  int CheckRow(const gVector<T> &v) const
    {
      return( v.First()==mincol && v.Last()==maxcol );
    }
  // check for correct column index
  int CheckColumn(int col) const
    {
      return(mincol<=col && col <= maxcol); 
    }
  // check column vector for correct row boundaries
  int CheckColumn(const gVector<T> &v) const
    {
      return( v.First()==minrow && v.Last()==maxrow );
    }
  // check row and column indices
  int Check(int row,int col) const
    {
      return(CheckRow(row) && CheckColumn(col));
    }
  // check matrix for same row and column boundaries
  int CheckBounds(const gMatrix<T> &m) const
    {
      return( minrow==m.minrow && maxrow==m.maxrow
	   && mincol==m.mincol && maxcol==m.maxcol );
    }

  T** AllocateIndex(void) {
    T** p = new T*[maxrow-minrow+1];
    assert( p != NULL );
    return p-minrow;
  }
  void DeleteIndex(T** p) {
    delete [] (p+minrow);
  }
  T* AllocateRow(void) {
    T* p = new T[maxcol-mincol+1];
    assert( p != NULL );
    return p-mincol;
  }
  void DeleteRow(T* p) {
    delete [] (p+mincol);
  }
  void AllocateData(void);
  void DeleteData(void);
  void CopyData(const gMatrix<T> &);
  
public:
  // Constructors
  gMatrix(void) {
    minrow=1; maxrow=0;
    mincol=1; maxcol=0;
    data=NULL;
  }
  gMatrix(int rows, int cols)
  {
    assert( rows>=0 && cols>=0 );
    minrow=1; maxrow= rows; mincol= 1; maxcol= cols;
    AllocateData();
  }
  gMatrix(int rows, int cols, int minrows)
  {
    assert( rows>=0 && cols>=0 );
    minrow= minrows; maxrow= minrows+rows-1; mincol= 1; maxcol= cols;
    AllocateData();
  }
  gMatrix(int rl, int rh, int cl, int ch)
  {
    assert( rh>=rl-1 && ch>=rl-1 );
    minrow=rl; maxrow=rh; mincol=cl; maxcol=ch;
    AllocateData();
  }
  gMatrix(const gMatrix<T> &M)
  {
    CopyData(M);
  }

  // Destructor
  ~gMatrix() { DeleteData(); }


  // Access a gMatrix element
  T& operator()(int row, int col) { 
    assert(Check(row,col));
    return data[row][col]; 
  }
  T operator()(int row, int col) const {
    assert(Check(row,col));
    return data[row][col]; 
  }

  // = operator
  gMatrix<T> &operator=(const gMatrix<T> &M);

  // +,- operators
  gMatrix<T> operator+(const gMatrix<T> &) const;
  gMatrix<T> operator-(const gMatrix<T> &) const;
  gMatrix<T> &operator+=(const gMatrix<T> &);
  gMatrix<T> &operator-=(const gMatrix<T> &);

  // *,/ operators
  gMatrix<T> operator*(const gMatrix<T> &) const;
  gVector<T> operator*(const gVector<T> &) const;
  friend gVector<T> operator*(const gVector<T> &, const gMatrix<T>&);
  gMatrix<T> operator*(const T) const;
  gMatrix<T> &operator*=(const gMatrix<T> &M) {
    return (*this)= (*this) * M;
  }
  gMatrix<T> &operator*=(T);

  gMatrix<T> operator/(T) const;
  gMatrix<T> &operator/=(T);

  // comparison functions
  int operator==(const gMatrix<T> &) const;
  int operator!=(const gMatrix<T> &M) const {
    return !((*this)==M);
  }
  int operator==( T ) const;
  int operator!=( T s ) const {
    return !((*this)==s);
  }

  // manipulation functions
  void AddRow(const gVector<T> &);
  void RemoveRow(int);
  void SwitchRow(int, const gVector<T> &);
  void SwitchRows(int, int);
  gVector<T> GetRow(int) const;
  void GetRow(int, gVector<T>&) const;
  void SetRow(int, const gVector<T>&);

  void AddColumn(const gVector<T> &);
  void RemoveColumn(int);
  void SwitchColumn(int, const gVector<T> &);
  void SwitchColumnWithVector(int, gVector<T> &);
  void SwitchColumns(int, int);
  gVector<T> GetColumn(int) const;
  void GetColumn(int, gVector<T>&) const;
  void SetColumn(int, const gVector<T>&);

  gMatrix<T> GetSlice(int, int, int, int) const;
  gMatrix<T> GetSubMatrix(const gBlock<int> &, const gBlock<int> &) const;
  gMatrix<T> Invert(void) const;
  gMatrix<T> ExternalPivot(int, int) const;
  void Pivot(int, int);
  T Determinant(void) const;

  // parameter access functions
  int MinRow(void) const { return minrow; }
  int MaxRow(void) const { return maxrow; }
  int NumRows(void) const { return maxrow-minrow+1; }
  
  int MinCol(void) const { return mincol; }
  int MaxCol(void) const { return maxcol; }
  int NumColumns(void) const { return maxcol-mincol+1; }

  void Dump(gOutput &to) const;
  
  void SwapRows(int, int);
  void RotateUp(int lo, int hi);
  void RotateDown(int lo, int hi);

};


// method implementations follow:


// stream output

template <class T> gOutput &
operator<<(gOutput &to, const gMatrix<T> &M)
{
  M.Dump(to); return to;
}

template <class T> void
gMatrix<T>::Dump(gOutput &to) const
{
  to<<"matrix dump: "<<NumRows()<<" x "<<NumColumns()<<" :: "
    <<MinRow()<<".."<<MaxRow()<<" x "<<MinCol()<<".."<<MaxCol()<<"\n";
  for(int i = minrow; i <= maxrow; i++)
    {
      to << "[ ";
      for(int j = mincol; j <= maxcol; j++)
	to << (*this)(i,j) << " ";
      to << "]\n";
    }
}


// internal data operations

template <class T> void
gMatrix<T>::DeleteData(void)
{
  if(data) {
    for(int i=minrow;i<=maxrow;i++)
      DeleteRow( data[i] );
    DeleteIndex( data );
  }
}

template <class T> void
gMatrix<T>::AllocateData(void)
{
  typedef T* foo;

  data= AllocateIndex();
  for(int i=minrow;i<=maxrow;i++) {
    data[i]= AllocateRow();
  }
}

template <class T> void
gMatrix<T>::CopyData(const gMatrix<T> &M)
{
  minrow=M.MinRow(); maxrow=M.MaxRow();
  mincol=M.MinCol(); maxcol=M.MaxCol();
  AllocateData();
  for(int i=minrow;i<=maxrow;i++) {
    for(int j=mincol;j<=maxcol;j++)
      (*this)(i,j)=M(i,j);
/*
    T *src= M.data[i]+mincol;
    T *dst= data[i]+mincol;
    int j= maxcol-mincol+1;
    while( j-- )
      *(dst++)= *(src++);
*/
  }
}


// copy operator
template <class T> gMatrix<T>&
gMatrix<T>::operator=(const gMatrix<T> &M)
{ if(this != &M)
    {
      DeleteData();
      CopyData(M);
    }
  return (*this);
}


// arithmetic operators
template <class T> gMatrix<T>
gMatrix<T>::operator+(const gMatrix<T> &M) const
{
  assert( CheckBounds(M) );
  gMatrix<T> tmp(minrow,maxrow, mincol, maxcol);
  for(int i=minrow; i<=maxrow; i++)
    {
      for(int j=mincol; j<=maxcol; j++)
	tmp(i,j)= (*this)(i,j) + M(i,j);
    }
  return tmp;
}

template <class T> gMatrix<T>
gMatrix<T>::operator-(const gMatrix<T> &M) const
{
  assert( CheckBounds(M) );
  gMatrix<T> tmp(minrow,maxrow, mincol, maxcol);
  for(int i=minrow; i<=maxrow; i++)
    {
      for(int j=mincol; j<=maxcol; j++)
	tmp(i,j)= (*this)(i,j) - M(i,j);
    }
  return tmp;
}

template <class T> gMatrix<T> &
gMatrix<T>::operator+=(const gMatrix<T> &M)
{
  assert( CheckBounds(M) );
  for(int i=minrow; i<=maxrow; i++)
    {
      for(int j=mincol; j<=maxcol; j++)
	(*this)(i,j)+= M(i,j);
    }
  return (*this);
}

template <class T> gMatrix<T> &
gMatrix<T>::operator-=(const gMatrix<T> &M)
{
  assert( CheckBounds(M) );
  for(int i=minrow; i<=maxrow; i++)
    {
      for(int j=mincol; j<=maxcol; j++)
	(*this)(i,j)-= M(i,j);
    }
  return (*this);
}

template <class T> gMatrix<T>
gMatrix<T>::operator*(const gMatrix<T> &M) const
{
  assert( mincol==M.minrow && maxcol==M.maxrow );
  gMatrix<T> tmp(minrow,maxrow, M.mincol, M.maxcol);
  for(int i=minrow; i<=maxrow; i++)
  for(int j=M.mincol; j<=M.maxcol; j++)
    {
      T sum= (T)0;
      for(int k=mincol; k<=maxcol; k++)
	sum += (*this)(i,k) * M(k,j);
      tmp(i,j)= sum;
    }
  return tmp;
}

template <class T> gVector<T>
gMatrix<T>::operator*(const gVector<T> &v) const
{
  assert( CheckRow(v) );
  gVector<T> tmp(minrow, maxrow);
  for(int i=minrow; i<=maxrow; i++)
    {
      T sum= (T)0;
      for(int j=mincol; j<=maxcol; j++)
	sum += (*this)(i,j) * v[j];
      tmp[i]= sum;
    }
  return tmp;
}

// for the sake of completeness, here is
// vector*matrix, a friend function of gMatrix
template <class T> gVector<T>
operator*(const gVector<T> &v, const gMatrix<T> &M)
{
  assert( CheckColumn(v) );
  gVector<T> tmp(mincol, maxcol);
  for(int i=mincol; i<=maxcol; i++)
    {
      T sum= (T)0;
      for(int j=minrow; j<=maxrow; j++)
	sum += v[j] * (*this)(j,i);
      tmp[i]= sum;
    }
  return tmp;
}

template <class T> gMatrix<T>
gMatrix<T>::operator*(T s) const
{
  gMatrix<T> tmp(minrow,maxrow, mincol,maxcol);
  for(int i=minrow; i<=maxrow; i++)
    {
      for(int j=mincol; j<=maxcol; j++)
	tmp(i,j)= (*this)(i,j) * s;
    }
  return tmp;
}

// matrix*=matrix is defined in class declaration

template <class T> gMatrix<T> &
gMatrix<T>::operator*=( T s )
{
  for(int i=minrow; i<=maxrow; i++)
    {
      for(int j=mincol; j<=maxcol; j++)
	(*this)(i,j) *= s;
    }
}

template <class T> gMatrix<T>
gMatrix<T>::operator/(T s) const
{
  assert( s != (T)0 );
  gMatrix<T> tmp(minrow,maxrow, mincol,maxcol);
  for(int i=minrow; i<=maxrow; i++)
    {
      for(int j=mincol; j<=maxcol; j++)
	tmp(i,j)= (*this)(i,j) / s;
    }
  return tmp;
}

template <class T> gMatrix<T> &
gMatrix<T>::operator/=( T s )
{
  assert( s != (T)0 );
  for(int i=minrow; i<=maxrow; i++)
    {
      for(int j=mincol; j<=maxcol; j++)
	(*this)(i,j) /= s;
    }
}

// comparison operators

template <class T> int
gMatrix<T>::operator==(const gMatrix<T> &M) const
{
  assert( CheckBounds(M) );
  for(int i=minrow; i<=maxrow; i++)
    {
      for(int j=mincol; j<=maxcol; j++)
	if( M(i,j) != (*this)(i,j) )
	  return 0;
    }
  return 1;
}

template <class T> int
gMatrix<T>::operator==(T s) const
{
  for(int i=minrow; i<=maxrow; i++)
    {
      for(int j=mincol; j<=maxcol; j++)
	if( (*this)(i,j) != s )
	  return 0;
    }
  return 1;
}


// row manipulation
template <class T> void
gMatrix<T>::AddRow(const gVector<T> &v)
{
  assert( CheckRow(v) );
  maxrow++;
  T* newrow= AllocateRow();
  for( int i=mincol; i<=maxcol; i++ )
    newrow[i]= v[i];
  T** newidx= AllocateIndex();
  for( i=minrow; i<maxrow; i++ )
    newidx[i]= data[i];
  newidx[maxrow]= newrow;
  DeleteIndex(data);
  data= newidx;
}

template <class T> void
gMatrix<T>::RemoveRow(int row)
{
  assert( CheckRow(row) );
  maxrow--;
  T** newidx= AllocateIndex();
  for(int i=minrow; i<row; i++ )
    newidx[i]= data[i];
  for( ; i<=maxrow; i++ )
    newidx[i]= data[i+1];
  DeleteIndex(data);
  data= newidx;
}

template <class T> void
gMatrix<T>::SwitchRow(int row, const gVector<T> &v)
{
  assert( CheckRow(row) && CheckRow(v) );
  T* rowptr= data[row];
  for(int i=mincol; i<=maxcol; i++)
    rowptr[i]= v[i];
}

template <class T> void
gMatrix<T>::SwitchRows(int i, int j)
{
  // note: SwapRows has been changed to SwitchRows,
  // since that's the name in the original gMatrix class
  assert( CheckRow(i) && CheckRow(j) );
  T *temp;
      // Swap data rows
  temp=data[j];
  data[j]=data[i];
  data[i]=temp;
}

template <class T> gVector<T>
gMatrix<T>::GetRow(int row) const
{
  assert( CheckRow(row) );
  gVector<T> v(mincol, maxcol);
  for(int i=mincol; i<=maxcol; i++)
    v[i]= (*this)(row,i);
  return v;
}

template <class T> void
gMatrix<T>::GetRow(int row, gVector<T> &v) const
{
  assert( CheckRow(row) );
  assert( CheckRow(v) );
  for(int i=mincol; i<=maxcol; i++)
    v[i]= (*this)(row,i);
}

template <class T> void
gMatrix<T>::SetRow(int row, const gVector<T> &v)
{
  assert( CheckRow(row) );
  assert( CheckRow(v) );
  for(int i=mincol; i<=maxcol; i++)
    (*this)(row,i)= v[i];
}

template <class T> void
gMatrix<T>::RotateUp(int lo, int hi)
{
  assert( CheckRow(lo) && CheckRow(hi) );
  assert(lo<=hi);
      // Rotate data rows
  T *temp;
  temp=data[hi];
  for(int k=hi;k>lo;k--) data[k]=data[k-1];
  data[lo]=temp;
}

template <class T> void
gMatrix<T>::RotateDown(int lo, int hi)
{
  assert( CheckRow(lo) && CheckRow(hi) );
  assert(lo<=hi);
      // Rotate data rows
  T *temp;
  temp=data[lo];
  for(int k=lo;k<hi;k++) data[k]=data[k+1];
  data[hi]=temp;
}


// column manipulation

template<class T> void
gMatrix<T>::AddColumn(const gVector<T> &v)
{
  assert( CheckColumn(v) );
  maxcol++;
  for(int i=minrow; i<=maxrow; i++)
    {
      T* newrow= AllocateRow();
      T* oldrow= data[i];
      for(int j=mincol; j<=maxcol; j++)
	newrow[j]= oldrow[j];
      newrow[maxcol]= v[i];
      DeleteRow(oldrow);
      data[i]= newrow;
    }
}

template<class T> void
gMatrix<T>::RemoveColumn(int col)
{
  // note: RemoveColumn does not reallocate memory --
  // it shrinks the rows in place.
  assert( CheckColumn(col) );
  maxcol--;
  for(int i=minrow; i<=maxrow; i++)
    {
      T* row= data[i];
      for(int j=col; j<=maxcol; j++)
	row[j]= row[j+1];
    }
}

template<class T> void
gMatrix<T>::SwitchColumn(int col, const gVector<T> &v)
{
  assert( CheckColumn(col) && CheckColumn(v) );
  for(int i=minrow; i<=maxrow; i++)
    (*this)(i,col)= v[i];
}

template<class T> void
gMatrix<T>::SwitchColumnWithVector(int col, gVector<T> &v)
{
  assert( CheckColumn(col) && CheckColumn(v) );
  for(int i=minrow; i<=maxrow; i++)
    {
      T tmp= (*this)(i,col);
      (*this)(i,col)= v[i];
      v[i]= tmp;
    }
}

template<class T> void
gMatrix<T>::SwitchColumns(int a, int b)
{
  assert( CheckColumn(a) && CheckColumn(b) );
  for(int i=minrow; i<=maxrow; i++)
    {
      T* row= data[i];
      T tmp= row[a];
      row[a]= row[b];
      row[b]= tmp;
    }
}

template<class T> gVector<T>
gMatrix<T>::GetColumn(int col) const
{
  assert( CheckColumn(col) );
  gVector<T> v(minrow, maxrow);
  for(int i=minrow; i<=maxrow; i++)
    v[i]= (*this)(i,col);
  return v;
}

template <class T> void
gMatrix<T>::GetColumn(int col, gVector<T> &v) const
{
  assert( CheckColumn(col) );
  assert( CheckColumn(v) );
  for(int i=minrow; i<=maxrow; i++)
    v[i]= (*this)(i,col);
}

template <class T> void
gMatrix<T>::SetColumn(int col, const gVector<T> &v)
{
  assert( CheckColumn(col) );
  assert( CheckColumn(v) );
  for(int i=minrow; i<=maxrow; i++)
    (*this)(i,col)= v[i];
}


// more complex functions

template<class T> gMatrix<T>
gMatrix<T>::GetSlice(int rl, int rh, int cl, int ch) const
{
  assert( CheckRow(rl) && CheckRow(rh) && CheckColumn(cl) && CheckColumn(ch) );
  assert( rh >= rl-1 && ch >= cl-1 );
  gMatrix<T> m(rh-rl+1, ch-cl+1);
  for(int i=rl; i<=rh; i++)
    {
      for(int j=cl; j<=ch; j++)
	m(i-rl,j-cl)= (*this)(i,j);
    }
  return m;
}

template<class T> gMatrix<T>
gMatrix<T>::GetSubMatrix(const gBlock<int> &rowV,
			  const gBlock<int> &colV) const
{
  int rows= rowV.Length();
  int cols= colV.Length();
  for(int i=1; i<=rows; i++)
    assert( CheckRow(rowV[i]) );
  for(int j=1; j<=cols; j++)
    assert( CheckColumn(colV[j]) );

  gMatrix<T> m( rows, cols );
  for(i=1; i<=rows; i++)
    {
      for(j=1; j<=cols; j++)
	m(i,j)= (*this)(rowV[i],colV[j]);
    }
  return m;
}

template<class T> gMatrix<T>
gMatrix<T>::Invert(void) const
{
  assert( mincol==minrow && maxcol==maxrow );
  gMatrix<T> copy(*this);
  gMatrix<T> inv(minrow,maxrow, mincol,maxcol);

  // initialize inverse matrix and prescale row vectors
  for(int i=minrow; i<=maxrow; i++)
    {
      T max= (T)0;
      for(int j=mincol; j<=maxcol; j++)
	{
	  T abs= copy(i,j);
	  if( abs < (T)0 )
	    abs= -abs;
	  if( abs > max )
	    max= abs;
	}
      assert( max!= 0 );
      T scale= (T)1/max;
      for(j=mincol; j<=maxcol; j++)
	{
	  copy(i,j)*= scale;
	  if(i==j)
	    inv(i,j)= scale;
	  else
	    inv(i,j)= (T)0;
	}
    }

//gout<<"debug: Invert:\ncopy:\n"<<copy<<"inv:\n"<<inv<<"\n";
//char c; gin.get(c);
  for(i=mincol; i<=maxcol; i++)
    {
      // find pivot row
      T max= copy(i,i);
      if(max<(T)0)
	max= -max;
      int row= i;
      for(int j=i+1; j<=maxrow; j++)
	{
	  T abs= copy(j,i);
	  if( abs < (T)0 )
	    abs= -abs;
	  if( abs > max )
	    { max= abs; row= j; }
	}
      assert( max > (T)0 );
      copy.SwitchRows(i,row);
      inv.SwitchRows(i,row);
//gout<<"debug: swapped rows "<<i<<" and "<<row
//  <<":\ncopy:\n"<<copy<<"inv:\n"<<inv<<"\n";
      // scale pivot row
      T factor= (T)1/copy(i,i);
      for(int k=mincol; k<=maxcol; k++)
	{
	  copy(i,k)*= factor;
	  inv(i,k)*= factor;
	}

      // reduce other rows
      for(j=minrow; j<=maxrow; j++)
	{
	  if(j!=i)
	    {
	      T mult= copy(j,i);
	      for(k=mincol; k<=maxcol; k++)
		{
		  copy(j,k)-= copy(i,k)*mult;
		  inv(j,k)-= inv(i,k)*mult;
		}
	    }
	} // end for(j)
//gout<<"debug: after pivot #"<<i<<":\ncopy:\n"<<copy<<"inv:\n"<<inv<<"\n";
//gin.get(c);
    } // end for(i)

  return inv;
}

template<class T> gMatrix<T>
gMatrix<T>::ExternalPivot(int row, int col) const
{
  assert( CheckRow(row) && CheckColumn(col) );
  assert( (*this)(row,col)!=(T)0 );

  gMatrix<T> m(minrow,maxrow,mincol,maxcol);

  T mult= (T)1/(*this)(row,col);
  for(int j=mincol; j<=maxcol; j++)
    m(row,j)= (*this)(row,j)*mult;
  for(int i=minrow; i<=maxrow; i++)
    {
      mult= (*this)(i,col);
      if( i!=row)
	for(j=mincol; j<=maxcol; j++)
	  m(i,j)= (*this)(i,j) - m(row,j)*mult;
    }
  return m;
}

template<class T> void
gMatrix<T>::Pivot(int row, int col)
{
  assert( CheckRow(row) && CheckColumn(col) );
  assert( (*this)(row,col)!=(T)0 );

  T mult= (T)1/(*this)(row,col);
  for(int j=mincol; j<=maxcol; j++)
    (*this)(row,j)*= mult;
  for(int i=minrow; i<=maxrow; i++)
    {
      mult= (*this)(i,col);
      if(i!=row)
	for(j=mincol; j<=maxcol; j++)
	  (*this)(i,j)-= (*this)(row,j)*mult;
    }
}

template<class T> T
gMatrix<T>::Determinant(void) const
{
  assert(minrow==mincol && maxrow==maxcol);
  assert(NumRows()>0);
  if(NumRows()==1)
    return (*this)(minrow,mincol);
  if(NumRows()==2)
    return (*this)(minrow,mincol) * (*this)(maxrow,maxcol)
         - (*this)(minrow,maxcol) * (*this)(maxrow,mincol);

  T result= (T)0;
  int i,j;
  T l=(T)1;
  gBlock<int> rows, cols;
  for(i=minrow+1; i<=maxrow; i++)
    rows.Append(i);
  for(i=minrow; i<=maxrow; i++)
    {
      for(j=mincol; j<=maxcol; j++)
	if(j!=i) cols.Append(j);
      gMatrix<T> tmp= GetSubMatrix(rows,cols);
      result+= (tmp.Determinant()*l*(*this)(1,i));
      l*=(T)(-1);
      cols.Flush();
    }
  return result;
}


#endif     // GMATRIX_H
