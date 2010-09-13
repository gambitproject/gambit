//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/liblinear/ludecomp.h
// Interface to LU decomposition classes
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef LUDECOMP_H
#define LUDECOMP_H

#include "libgambit/libgambit.h"
#include "basis.h"

template <class T> class Tableau;

// ---------------------------------------------------------------------------
// Class EtaMatrix
// ---------------------------------------------------------------------------

template <class T> class EtaMatrix {
  public:
  int col;
  Gambit::Vector<T> etadata;
  
  
  EtaMatrix(int c, Gambit::Vector<T> &v) : col(c), etadata(v) {};

// required for list class
bool operator==(const EtaMatrix<T> &) const;
bool operator!=(const EtaMatrix<T> &) const;

};

// ---------------------------------------------------------------------------
// Class LUdecomp
// ---------------------------------------------------------------------------

template <class T> class LUdecomp {

private:

  Tableau<T> &tab;
  Basis &basis;

  Gambit::List< EtaMatrix<T> > L;
  Gambit::List< EtaMatrix<T> > U;
  Gambit::List< EtaMatrix<T> > E;
  Gambit::List< int > P;

  Gambit::Vector<T> scratch1; // scratch vectors so we don't reallocate them
  Gambit::Vector<T> scratch2; // everytime we do something.

  int refactor_number;
  int iterations;
  int total_operations;

  const LUdecomp<T> *parent;
  int copycount;

  // don't use this copy constructor
  LUdecomp( const LUdecomp<T> &a);
  // don't use the equals operator, use the Copy function instead
  LUdecomp<T>& operator=(const LUdecomp<T>&);



public:
  class BadPivot : public Gambit::Exception  {
  public:
    virtual ~BadPivot() throw() { }
    const char *what(void) const throw() { return "Bad pivot in LUdecomp"; }
  };
  class BadCount : public Gambit::Exception  {
  public:
    virtual ~BadCount() throw() { }
    const char *what(void) const throw() { return "Bad reference count in LUdecomp"; }
  };

  // ------------------------
  // Constructors, Destructor
  // ------------------------
    

  // copy constructor
  // note:  Copying will fail an assertion if you try to update or delete
  //        the original before the copy has been deleted, refactored
  //        Or set to something else.
  LUdecomp( const LUdecomp<T> &, Tableau<T> & );

  // Decompose given matrix
  LUdecomp( Tableau<T> &, int rfac = 0 ); 

  // Destructor
  ~LUdecomp();


  // --------------------
  // Public Members
  // --------------------

  // copies the LUdecomp given (expect for the basis &).
  void Copy( const LUdecomp<T> &, Tableau<T> & );

  // replace (update) the column given with the vector given.
  void update( int, int matcol ); // matcol is the column number in the matrix

  // refactor 
  void refactor();
  
  // solve: Bk d = a
  void solve (const Gambit::Vector<T> &, Gambit::Vector<T> & ) const;

  // solve: y Bk = c
  void solveT( const Gambit::Vector<T> &, Gambit::Vector <T> & ) const;

  // set number of etamatrices added before refactoring;
  // if number is set to zero, refactoring is done automatically.
  // if number is < 0, no refactoring is done;
  void SetRefactor( int );

  //-------------------
  // Private Members
  //-------------------

private:
  
  void FactorBasis();

  void GaussElem( Gambit::Matrix<T> &, int, int );

  bool CheckBasis();
  bool RefactorCheck();

  void BTransE( Gambit::Vector<T> & ) const;
  void FTransE( Gambit::Vector<T> & ) const;
  void BTransU( Gambit::Vector<T> & ) const;
  void FTransU( Gambit::Vector<T> & ) const;
  void LPd_Trans( Gambit::Vector<T> & ) const;
  void yLP_Trans( Gambit::Vector<T> & ) const;

  void VectorEtaSolve( const Gambit::Vector<T> &v,  
		      const EtaMatrix<T> &, 
		      Gambit::Vector<T> &y ) const;

  void EtaVectorSolve( const Gambit::Vector<T> &v, 
		      const EtaMatrix<T> &,
		      Gambit::Vector<T> &d ) const;

  void yLP_mult( const Gambit::Vector<T> &y, int j, Gambit::Vector<T> &) const;

  void LPd_mult( Gambit::Vector<T> &d, int j, Gambit::Vector<T> &) const;


};  // end of class LUdecomp
    
#endif // LUDECOMP_H










