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
#include "gblock.h"

//---------------------------------------------------------------------------
// Class Basis
//---------------------------------------------------------------------------

template <class T> class LUdecomp;

template <class T> class Basis {

friend class LUdecomp<T>;
  
protected:

  const gMatrix<T> *A;
  gBlock<int> label;
  gBlock<int> cols;
  gBlock<int> slacks;
  gBlock<int> arts;
  gBlock<int> artUnitEntry;
  gArray<bool> colBlocked;
  gArray<bool> rowBlocked;
  bool IsBasisIdent;

public:

  //-------------------------------------------
  // Constructors, Destructor, Operators
  //-------------------------------------------
  

  Basis(const gMatrix<T> &A);
  Basis(const Basis<T> &);
  virtual ~Basis();
  
  Basis<T>& operator=(const Basis<T>&);

  //------------------------------
  // Public Members
  //------------------------------
  
  //remove outindex, insert label, return outlabel
  int Pivot(int outindex, int col); 

  // return true iff label is a Basis member
  bool Member(int label) const;

  // finds Basis index corresponding to label number,
  // fails assert if label not in Basis
  int Find(int label) const;

  // finds label of variable corresponding to Basis index
  int Label(int index) const;

  // marks/unmarks label to block it from entering basis
  void Mark(int label);
  void UnMark(int label);

  // returns true if label is blocked from entering basis
  bool IsBlocked(int label) const;

  // select Basis elements according to Tableau rows and cols
  void BasisSelect(const gVector<T>&rowv, gVector<T> &colv) const;

  // as above, but unit column elements nonzero
  void BasisSelect(const gVector<T>&unitv,
		   const gVector<T>&rowv,
		   gVector<T>&colv
		   ) const; 
  

  // returns whether the basis is the identity matrix
  bool IsIdent();

  // returns true if the column is special in some way
  bool IsSpecialColumn( int col ) const; 
  
  // returns a column from the matrix  
  void GetColumn( int col, gVector<T> & ) const;
  
  // Insert an artificial variable at column col
  void InsertArtificial( int art, int col );

  // Append an artificial variable.  Returns the last col index
  // ( ie where the artificial variable was appended ).
  int AppendArtificial( int art );

  // Remove an artificial variable, returns which artificial variable
  // was removed.
  void RemoveArtificial( int col );
  
  // Clears out all the artificial variables
  void FlushArtificial();

  // Returns the index of the last artificial variable
  int LastArtificial();

  void Dump(gOutput &) const;

  // ----------------
  // Private members
  // ----------------

  virtual void CheckBasis();

};



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










