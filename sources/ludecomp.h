//
// FILE: ludecomp.h -- Definition of the LUdecomp class
//
// 
// $Id$
//


#ifndef LUDECOMP_H
#define LUDECOMP_H

#include "gmatrix.h"
#include "gvector.h"
#include "glist.h"
#include "gambitio.h"
#include "gblock.h"
#include "gwatch.h"

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
public:  // just for debugging.
  const gMatrix<T> *A;
  gBlock<int> basis;

  bool IsBasisIdent;     //set true if basis is the identity matrix

  gList< EtaMatrix<T> > L;
  gList< EtaMatrix<T> > U;
  gList< EtaMatrix<T> > E;
  gList< int > P;

  gVector<T> scratch1; // scratch vectors so we don't reallocate them
  gVector<T> scratch2; // everytime we do something.

  int refactor_number;
  int iterations;
  int total_operations;

  const LUdecomp<T> *parent;
  int copycount;

public:

  // ------------------------
  // Constructors, Destructor
  // ------------------------
    
  // copy constructor
  // note:  This will fail an assertion if you try to update or delete
  //        the original before the copy has been deleted, refactored
  //        Or set to something else.
  LUdecomp( const LUdecomp<T> & );

  // Decompose given matrix
  LUdecomp( const gMatrix<T> &, int rfac = 0 ); 

  // Decompose the selected columns of the given matrix
  LUdecomp( const gMatrix<T> &, const gBlock<int> &, int rfac = 0 );

  // Destructor
  ~LUdecomp();

  // Equality operator
  // note:  This will fail an assertion if you try to delete or update
  //        the original before the copy has been deleted, refactored, or set 
  //        equal to something else.
  LUdecomp<T>& operator=(const LUdecomp<T>&);


  // --------------------
  // Public Members
  // --------------------

  // replace (update) the column given with the vector given.
  void update( int, int matcol ); // matcol is the column number in the matrix

  // refactor 
  void refactor();
  
  // factor a new matrix.
  void refactor( const gMatrix<T> &, int rfac = 0 );

  // reinitialize with selected columns 
  // a negative index specifies that the column is an identity column
  // with the one in the abs(index) position.
  void refactor( const gMatrix<T> &, const gBlock<int> &, int rfac = 0 );
  
  // solve: Bk d = a
  void solve (const gVector<T> &, gVector<T> & ) const;

  // solve: y Bk = c
  void solveT( const gVector<T> &, gVector <T> & ) const;

  // set number of etamatrices added before refactoring;
  // if number is set to zero, refactoring is done automatically.
  // if number is < 0, no refactoring is done;
  void SetRefactor( int );

  //-------------------
  // Private Members
  //-------------------

private:
  
  void FactorBasis();

  void GaussElem( gMatrix<T> &, int, int );

  bool CheckBasis();
  bool RefactorCheck();

  void BTransE( gVector<T> & ) const;
  void FTransE( gVector<T> & ) const;
  void BTransU( gVector<T> & ) const;
  void FTransU( gVector<T> & ) const;
  void LPd_Trans( gVector<T> & ) const;
  void yLP_Trans( gVector<T> & ) const;

  void VectorEtaSolve( const gVector<T> &v,  
		      const EtaMatrix<T> &, 
		      gVector<T> &y ) const;

  void EtaVectorSolve( const gVector<T> &v, 
		      const EtaMatrix<T> &,
		      gVector<T> &d ) const;

  void yLP_mult( const gVector<T> &y, int j, gVector<T> &) const;

  void LPd_mult( gVector<T> &d, int j, gVector<T> &) const;


};  // end of class LUdecomp
    
#endif // LUDECOMP_H










