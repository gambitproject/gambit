//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to tableau class
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#ifndef TABLEAU_H  
#define TABLEAU_H

#include "btableau.h"
#include "ludecomp.h"

template <class T> class Tableau;
template <class T> class LPTableau;


// ---------------------------------------------------------------------------
// We have different implementations of Tableau for double and gRational, 
// but with the same interface
// ---------------------------------------------------------------------------

// 
// Tableau<double>
//  

template<>
class Tableau<double> : public TableauInterface<double>{
private:
  LUdecomp<double> B;     // LU decomposition
  gVector<double> tmpcol; // temporary column vector, to avoid allocation

public:
      // constructors and destructors
  Tableau(const gMatrix<double> &A, const gVector<double> &b); 
  Tableau(const gMatrix<double> &A, const gBlock<int> &art, 
	  const gVector<double> &b); 
  Tableau(const Tableau<double>&);
  virtual ~Tableau();
  
  Tableau<double>& operator=(const Tableau<double>&);
  
  // pivoting
  int CanPivot(int outgoing,int incoming);
  void Pivot(int outrow,int col); // pivot -- outgoing is row, incoming is column
  void BasisVector(gVector<double> &x) const; // solve M x = (*b)
  void SolveColumn(int, gVector<double> &);  // column in new basis 
  void Solve(const gVector<double> &b, gVector<double> &x);  // solve M x = b
  void SolveT(const gVector<double> &c, gVector<double> &y);  // solve y M = c
  
  // raw Tableau functions

  void Refactor();
  void SetRefactor(int);

  void SetConst(const gVector<double> &bnew);
  void SetBasis( const Basis &); // set new Tableau
  
  bool IsFeasible();
  bool IsLexMin();
};

// 
// Tableau<gRational> 
//  

template<>
class Tableau<gRational> : public TableauInterface<gRational>{
private:
  int remap(int col_index) const;  // aligns the column indexes
  gMatrix<gRational> GetInverse();

  gMatrix<gInteger> Tabdat;  // This caries the full tableau
  gVector<gInteger> Coeff;   // and coeffieient vector
  gInteger totdenom;  // This carries the denominator for Q data or 1 for Z
  gInteger denom;  // This is the denominator for the simplex

  gVector<gRational> tmpcol; // temporary column vector, to avoid allocation

  void MySolveColumn(int, gVector<gRational> &);  // column in new basis 

protected:
  gBlock<int> nonbasic;     //** nonbasic variables -- should be moved to Basis

public:
  class BadDenom : public gException  {
  public:
    virtual ~BadDenom();
    gText Description(void) const;
  };
      // constructors and destructors
  Tableau(const gMatrix<gRational> &A, const gVector<gRational> &b); 
  Tableau(const gMatrix<gRational> &A, const gBlock<int> &art, 
	  const gVector<gRational> &b); 
  Tableau(const Tableau<gRational>&);
  virtual ~Tableau();
  
  Tableau<gRational>& operator=(const Tableau<gRational>&);
  
  // pivoting
  int CanPivot(int outgoing,int incoming);
  void Pivot(int outrow,int col); // pivot -- outgoing is row, incoming is column
  void SolveColumn(int, gVector<gRational> &);  // column in new basis 
  void GetColumn(int, gVector<gRational> &) const;  // column in new basis 
  
  // raw Tableau functions

  void Refactor();
  void SetRefactor(int);

  void SetConst(const gVector<gRational> &bnew);
  void SetBasis( const Basis &); // set new Tableau
  void Solve(const gVector<gRational> &b, gVector<gRational> &x);  // solve M x = b
  void SolveT(const gVector<gRational> &c, gVector<gRational> &y);  // solve y M = c
  
  bool IsFeasible();
  bool IsLexMin();
  void BigDump(gOutput &);
  void BasisVector(gVector<gRational> &out) const;
  gInteger TotDenom() const;
};

#ifdef __GNUG__
#include "math/rational.h"
gOutput &operator<<(gOutput &, const Tableau<double> &);
gOutput &operator<<(gOutput &, const Tableau<gRational> &);
#elif defined __BORLANDC__
template <class T> gOutput &operator<<(gOutput &, const Tableau<T> &);
#endif   // __GNUG__, __BORLANDC__

#endif     // TABLEAU_H
