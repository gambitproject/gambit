//
// FILE: gmatrix.h -- Implementation of matrix classes
//
// $Id$
//

#ifndef GMATRIX_H
#define GMATRIX_H

#include "grarray.h"
#include "gblock.h"
#include "gvector.h"

template <class T> class gMatrix : public gRectArray<T>  {
  public:
#ifdef USE_EXCEPTIONS
    class DivideByZero : public gException  {
      public:
        DivideByZero(int, char *);

        virtual ~DivideByZero()   { }
        gText Description(void) const;
    };
#endif   // USE_EXCEPTIONS

       // CONSTRUCTORS, DESTRUCTOR, CONSTRUCTIVE OPERATORS
    gMatrix(void);
    gMatrix(unsigned int rows, unsigned int cols);
    gMatrix(unsigned int rows, unsigned int cols, int minrows);
    gMatrix(int rl, int rh, int cl, int ch);
    gMatrix(const gMatrix<T> &);
    virtual ~gMatrix();

    gMatrix<T> &operator=(const gMatrix<T> &);
    gMatrix<T> &operator=(const T &);
    gMatrix<T> operator-(void);

       // ADDITIVE OPERATORS
    gMatrix<T> operator+(const gMatrix<T> &) const;
    gMatrix<T> operator-(const gMatrix<T> &) const;
    gMatrix<T> &operator+=(const gMatrix<T> &);
    gMatrix<T> &operator-=(const gMatrix<T> &);


       // MULTIPLICATIVE OPERATORS
       // "in-place" column multiply
    void CMultiply(const gVector<T> &, gVector<T> &) const;
       // "in-place" row (transposed) multiply
    void RMultiply(const gVector<T> &, gVector<T> &) const;
    gMatrix<T> operator*(const gMatrix<T> &) const;
    gVector<T> operator*(const gVector<T> &) const;
    friend gVector<T> operator*(const gVector<T> &, const gMatrix<T> &);
    gMatrix<T> operator*(const T &) const;
    gMatrix<T> &operator*=(const gMatrix<T> &);
    gMatrix<T> &operator*=(const T &);

    gMatrix<T> operator/(const T &) const;
    gMatrix<T> &operator/=(const T &);

       // KRONECKER PRODUCT
    gMatrix<T> operator&(const gMatrix<T> &) const;

      // TRANSPOSE
    gMatrix<T>       Transpose()         const;

       // COMPARISON OPERATORS
    bool operator==(const gMatrix<T> &) const;
    bool operator!=(const gMatrix<T> &) const;
    bool operator==(const T &) const;
    bool operator!=(const T &) const;

       // INFORMATION
    gVector<T> Row   (const int&) const;
    gVector<T> Column(const int&) const;

  void MakeIdent(void);  // set matrix to identity matrix
  void Pivot(int, int);
};

template <class T> gOutput &operator<<(gOutput &, const gMatrix<T> &);

#endif     // GMATRIX_H




