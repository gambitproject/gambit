//#
//# FILE: gmatrix.h -- Implementation of matrix classes
//#
//# $Id$
//#

#ifndef GMATRIX_H
#define GMATRIX_H

#include "gambitio.h"
#include "grarray.h"
#include "gblock.h"
#include "gvector.h"

template <class T> class gMatrix : public gRectArray<T>  {
  public:
       // CONSTRUCTORS, DESTRUCTOR, CONSTRUCTIVE OPERATORS
    gMatrix(void);
    gMatrix(int rows, int cols);
    gMatrix(int rows, int cols, int minrows);
    gMatrix(int rl, int rh, int cl, int ch);
    gMatrix(const gMatrix<T> &M);
    virtual ~gMatrix();

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
  void SwitchRow(int, const gVector<T> &); // redundant (identical to SetRow)
  void SwitchRowWithVector(int, gVector<T> &);
  void SwitchRows(int, int);
  gVector<T> GetRow(int) const;
  void GetRow(int, gVector<T>&) const;
  void SetRow(int, const gVector<T>&);
  void MakeIdent(void);  // set matrix to identity matrix

  void SwitchColumn(int, const gVector<T> &); // redundant (identical to SetColumn)
  void SwitchColumnWithVector(int, gVector<T> &);
  void SwitchColumns(int, int);
  gVector<T> GetColumn(int) const;
  void GetColumn(int, gVector<T>&) const;
  void SetColumn(int, const gVector<T>&);

  void Pivot(int, int);


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
};

template <class T> gOutput &operator<<(gOutput &to, const gMatrix<T> &M);

#endif     // GMATRIX_H




