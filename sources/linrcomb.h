//
// FILE: linrcomb.h -- Find weights in linear dependency of rows of matrix
//
// $Id$ 
//

/*
   This file contains the file for a class that performs a very specific
computation, namely asking whether the last row of a matrix is a linear
combination of the other rows, and if so computing the coefficients of
a linear dependence.
   All computation is done in the constructor.  The constructor should
only be called when there is a guarantee (which this class may be used
to compute!) that the rows other than the last are linearly
independent.
*/

#ifndef LINRCOMB_H
#define LINRCOMB_H


#include "gmatrix.h"
#include "rational.h"

template <class T> class LinearCombination  {
  private:
    gMatrix<T> scrambled;
    gVector<T> weights;
    bool       last_row_is_spanned;

    void AddMultipleOfRowiToRowj(const int& i, 
				 const int& j, 
				 const T& scalar);
    void AddMultipleOfRowiToRowj(const int& i, 
				 const int& j, 
				 const T& scalar,
				 gMatrix<T>& B);
  
  public:
       // Constructors, Destructor, Constructive Operators
    LinearCombination(const gMatrix<T> &);
    LinearCombination(const LinearCombination<T> &);

    virtual ~LinearCombination();

    LinearCombination<T>& operator=(const LinearCombination<T> &);

       // Comparison Operators
    bool operator==(const LinearCombination<T> &) const;
    bool operator!=(const LinearCombination<T> &) const;

       // Information
    bool       LastRowIsSpanned() const;
    gVector<T> LinearDependence() const;

friend gOutput &operator<<(gOutput &, const LinearCombination<T> &);
};



#endif     // LinearCombination_H




