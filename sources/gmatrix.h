//#
//# FILE: gmatrix.h -- Implementation of normal matrix classes
//#
//# $Id$
//#

#ifndef GMATRIX_H
#define GMATRIX_H

#include "gambitio.h"
#include "gvector.h"
#include "gblock.h"

template <class T> class gMatrix   {
protected:
  int minrow,maxrow,mincol,maxcol;
  T **data;


  T** AllocateIndex(void); 
  void DeleteIndex(T** p);
  T* AllocateRow(void);
  void DeleteRow(T* p);
  void AllocateData(void);
  void DeleteData(void);
  void CopyData(const gMatrix<T> &);

public:
  // Constructors
  gMatrix(void);
  gMatrix(int rows, int cols);
  gMatrix(int rows, int cols, int minrows);
  gMatrix(int rl, int rh, int cl, int ch);
  gMatrix(const gMatrix<T> &M);

  // Destructor
  ~gMatrix();

  // Access a gMatrix element
  T& operator()(int row, int col);
  const T& operator()(int row, int col) const;

  // = operator
  gMatrix<T> &operator=(const gMatrix<T> &M);

  // +,- operators
  gMatrix<T> operator+(const gMatrix<T> &) const;
  gMatrix<T> operator-(const gMatrix<T> &) const;
  gMatrix<T> &operator+=(const gMatrix<T> &);
  gMatrix<T> &operator-=(const gMatrix<T> &);

  // *,/ operators
  // "in-place" column multiply
  void CMultiply(const gVector<T>&, gVector<T>&) const;
  // "in-place" row (transposed) multiply
  void RMultiply(const gVector<T>&, gVector<T>&) const;
  gMatrix<T> operator*(const gMatrix<T> &) const;
  gVector<T> operator*(const gVector<T> &) const;
  friend gVector<T> operator*(const gVector<T> &, const gMatrix<T>&);
  gMatrix<T> operator*(T) const;
  gMatrix<T> &operator*=(const gMatrix<T> &M);
  gMatrix<T> &operator*=(T);

  gMatrix<T> operator/(T) const;
  gMatrix<T> &operator/=(T);

  // comparison functions
  int operator==(const gMatrix<T> &) const;
  int operator!=(const gMatrix<T> &M) const;
  int operator==( T ) const;
  int operator!=( T s ) const;

  // manipulation functions
  void AddRow(const gVector<T> &);
  void RemoveRow(int);
  void SwitchRow(int, const gVector<T> &); // redundant (identical to SetRow)
  void SwitchRowWithVector(int, gVector<T> &);
  void SwitchRows(int, int);
  gVector<T> GetRow(int) const;
  void GetRow(int, gVector<T>&) const;
  void SetRow(int, const gVector<T>&);

  void AddColumn(const gVector<T> &);
  void RemoveColumn(int);
  void SwitchColumn(int, const gVector<T> &); // redundant (identical to SetColumn)
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


  // check functions
    // check for correct row index
  int CheckRow(int row) const;
    // check row vector for correct column boundaries
  int CheckRow(const gVector<T> &v) const;
    // check for correct column index
  int CheckColumn(int col) const;
    // check column vector for correct row boundaries
  int CheckColumn(const gVector<T> &v) const;
    // check row and column indices
  int Check(int row,int col) const;
    // check matrix for same row and column boundaries
  int CheckBounds(const gMatrix<T> &m) const;

  // parameter access functions
  const int& MinRow(void) const;
  const int& MaxRow(void) const;
  int NumRows(void) const;
  
  const int& MinCol(void) const;
  const int& MaxCol(void) const;
  int NumColumns(void) const;

  void Dump(gOutput &to) const;
  
//  void SwapRows(int, int); // use SwitchRows
  void RotateUp(int lo, int hi);
  void RotateDown(int lo, int hi);

};

template <class T> gOutput &operator<<(gOutput &to, const gMatrix<T> &M);


#ifdef __BORLANDC__
#include "gmatrix.imp"
#endif   // __BORLANDC__


#endif     // GMATRIX_H



