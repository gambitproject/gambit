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
// We have different implementations of Tableau for double and gbtRational, 
// but with the same interface
// ---------------------------------------------------------------------------

// 
// Tableau<double>
//  

template<>
class Tableau<double> : public TableauInterface<double>{
private:
  LUdecomp<double> B;     // LU decomposition
  gbtVector<double> tmpcol; // temporary column vector, to avoid allocation

public:
      // constructors and destructors
  Tableau(const gbtMatrix<double> &A, const gbtVector<double> &b); 
  Tableau(const gbtMatrix<double> &A, const gbtBlock<int> &art, 
	  const gbtVector<double> &b); 
  Tableau(const Tableau<double>&);
  virtual ~Tableau();
  
  Tableau<double>& operator=(const Tableau<double>&);
  
  // pivoting
  int CanPivot(int outgoing,int incoming);
  void Pivot(int outrow,int col); // pivot -- outgoing is row, incoming is column
  void BasisVector(gbtVector<double> &x) const; // solve M x = (*b)
  void SolveColumn(int, gbtVector<double> &);  // column in new basis 
  void Solve(const gbtVector<double> &b, gbtVector<double> &x);  // solve M x = b
  void SolveT(const gbtVector<double> &c, gbtVector<double> &y);  // solve y M = c
  
  // raw Tableau functions

  void Refactor();
  void SetRefactor(int);

  void SetConst(const gbtVector<double> &bnew);
  void SetBasis( const Basis &); // set new Tableau
  
  bool IsFeasible();
  bool IsLexMin();
};

// 
// Tableau<gbtRational> 
//  

template<>
class Tableau<gbtRational> : public TableauInterface<gbtRational>{
private:
  int remap(int col_index) const;  // aligns the column indexes
  gbtMatrix<gbtRational> GetInverse();

  gbtMatrix<gbtInteger> Tabdat;  // This caries the full tableau
  gbtVector<gbtInteger> Coeff;   // and coeffieient vector
  gbtInteger totdenom;  // This carries the denominator for Q data or 1 for Z
  gbtInteger denom;  // This is the denominator for the simplex

  gbtVector<gbtRational> tmpcol; // temporary column vector, to avoid allocation

  void MySolveColumn(int, gbtVector<gbtRational> &);  // column in new basis 

protected:
  gbtBlock<int> nonbasic;     //** nonbasic variables -- should be moved to Basis

public:
  class BadDenom : public gbtException  {
  public:
    virtual ~BadDenom();
    gbtText Description(void) const;
  };
      // constructors and destructors
  Tableau(const gbtMatrix<gbtRational> &A, const gbtVector<gbtRational> &b); 
  Tableau(const gbtMatrix<gbtRational> &A, const gbtBlock<int> &art, 
	  const gbtVector<gbtRational> &b); 
  Tableau(const Tableau<gbtRational>&);
  virtual ~Tableau();
  
  Tableau<gbtRational>& operator=(const Tableau<gbtRational>&);
  
  // pivoting
  int CanPivot(int outgoing,int incoming);
  void Pivot(int outrow,int col); // pivot -- outgoing is row, incoming is column
  void SolveColumn(int, gbtVector<gbtRational> &);  // column in new basis 
  void GetColumn(int, gbtVector<gbtRational> &) const;  // column in new basis 
  
  // raw Tableau functions

  void Refactor();
  void SetRefactor(int);

  void SetConst(const gbtVector<gbtRational> &bnew);
  void SetBasis( const Basis &); // set new Tableau
  void Solve(const gbtVector<gbtRational> &b, gbtVector<gbtRational> &x);  // solve M x = b
  void SolveT(const gbtVector<gbtRational> &c, gbtVector<gbtRational> &y);  // solve y M = c
  
  bool IsFeasible();
  bool IsLexMin();
  void BigDump(gbtOutput &);
  void BasisVector(gbtVector<gbtRational> &out) const;
  gbtInteger TotDenom() const;
};

#if defined(__GNUG__) && !defined(__APPLE_CC__)
#include "math/rational.h"
gbtOutput &operator<<(gbtOutput &, const Tableau<double> &);
gbtOutput &operator<<(gbtOutput &, const Tableau<gbtRational> &);
#elif defined __BORLANDC__
template <class T> gbtOutput &operator<<(gbtOutput &, const Tableau<T> &);
#endif   // __GNUG__, __BORLANDC__

#endif     // TABLEAU_H
