//#
//# FILE: ludecomp.h -- Implementation of LU decomposition
//#
//# $Id$
//#

#ifndef LUDECOMP_H
#define LUDECOMP_H


#include "gmatrix.h"
#include "gvector.h"
#include "glist.h"
#include "gambitio.h"
#include "gblock.h"


template <class T> class EtaMatrix {
 public:
  int col;
  gVector<T> etadata;
  
  
  EtaMatrix(int c, gVector<T> &v) : col(c), etadata(v) {};

// required for list class
bool operator==(const EtaMatrix<T> &) const;
bool operator!=(const EtaMatrix<T> &) const;

};

template <class T> gOutput &operator<<( gOutput &, const EtaMatrix<T> &);

template <class T> class LUdecomp {

private:
 public:         // for debugging purposes only
  const gMatrix<T> *A;
  gBlock<int> basis;
  bool IsBasisIdent;     //set true if matrix is the identity matrix
  gList< EtaMatrix<T> > L;
  gList< EtaMatrix<T> > U;
  gList< EtaMatrix<T> > E;
  gList< int > P;

public:

  // ------------------------
  // Constructors, Destructor
  // ------------------------
    
  // copy constructor
  LUdecomp( const LUdecomp<T> & );

  // Decompose given matrix
  LUdecomp( const gMatrix<T> & ); 

  // Decompose the selected columns of the given matrix
  LUdecomp( const gMatrix<T> &, const gBlock<int> & );

  // Destructor
  ~LUdecomp();

  // Equality operator
  LUdecomp<T>& operator=(const LUdecomp<T>&);


  // --------------------
  // Public Members
  // --------------------

  // replace (update) the column given with the vector given.
  void update( int, const gVector<T> & );
  void update( int, int matcol ); // matcol is the column number in the matrix

  // refactor 
  void refactor();
  
  // factor a new matrix.
  void refactor( const gMatrix<T> & );

  // reinitialize with selected indices -- rows (neg), columns (pos)
  void refactor( const gMatrix<T> &, const gBlock<int> & );
  
  // solve: Bk d = a
  void solve (const gVector<T> &, gVector<T> & ) const;

  // solve: y Bk = c
  void solveT( const gVector<T> &, gVector <T> & ) const;

  // compute Determinant
  T Determinant() const; 

  //-------------------
  // Private Members
  //-------------------

private:
    
  void FactorBasis();
  void GaussElem( gMatrix<T> &, int, int );
  bool CheckBasis( const gBlock<int> & );
  bool CheckBasis();

  void VectorEtaSolve( const gVector<T> &v,  
		      const EtaMatrix<T> &, 
		      gVector<T> &y ) const;

  void EtaVectorSolve( const gVector<T> &v, 
		      const EtaMatrix<T> &,
		      gVector<T> &d ) const;

  void yLP_mult( const gVector<T> &y, int j, gVector<T> &) const;
  void LPd_mult( const gVector<T> &d, int j, gVector<T> &) const;


};  // end of class LUdecomp
    
#endif     // LUDECOMP_H







