//
// FILE: ludecomp.h -- Definition of the LUdecomp class
//
// $Id$
//


#ifndef LUDECOMP_H
#define LUDECOMP_H

#include "gmatrix.h"
#include "gvector.h"
#include "glist.h"
#include "gstream.h"
#include "basis.h"

// ---------------------------------------------------------------------------
// Class EtaMatrix
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Class LUdecomp
// ---------------------------------------------------------------------------

template <class T> class LUdecomp {

private:

  Basis<T> &basis;

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
    

  // don't use this copy constructor
  LUdecomp( const LUdecomp<T> &a) : basis(a.basis) { assert(0); };

  // copy constructor
  // note:  Copying will fail an assertion if you try to update or delete
  //        the original before the copy has been deleted, refactored
  //        Or set to something else.
  LUdecomp( const LUdecomp<T> &, Basis<T> & );

  // Decompose given matrix
  LUdecomp( Basis<T> &, int rfac = -1 ); 

  // Destructor
  ~LUdecomp();

  // don't use the equals operator, use the Copy function instead
  LUdecomp<T>& operator=(const LUdecomp<T>&) { assert(0); return *this; };


  // --------------------
  // Public Members
  // --------------------

  // copies the LUdecomp given (expect for the basis &).
  void Copy( const LUdecomp<T> &, Basis<T> & );

  // replace (update) the column given with the vector given.
  void update( int, int matcol ); // matcol is the column number in the matrix

  // refactor 
  void refactor();
  
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










