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
  Tableau(const Gambit::Matrix<double> &A, const gbtVector<double> &b); 
  Tableau(const Gambit::Matrix<double> &A, const gbtArray<int> &art, 
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
// Tableau<Gambit::Rational> 
//  

template<>
class Tableau<Gambit::Rational> : public TableauInterface<Gambit::Rational>{
private:
  int remap(int col_index) const;  // aligns the column indexes
  Gambit::Matrix<Gambit::Rational> GetInverse();

  Gambit::Matrix<Gambit::Integer> Tabdat;  // This caries the full tableau
  gbtVector<Gambit::Integer> Coeff;   // and coeffieient vector
  Gambit::Integer totdenom;  // This carries the denominator for Q data or 1 for Z
  Gambit::Integer denom;  // This is the denominator for the simplex

  gbtVector<Gambit::Rational> tmpcol; // temporary column vector, to avoid allocation

  void MySolveColumn(int, gbtVector<Gambit::Rational> &);  // column in new basis 

protected:
  gbtArray<int> nonbasic;     //** nonbasic variables -- should be moved to Basis

public:
  class BadDenom : public gbtException  {
  public:
    virtual ~BadDenom();
    std::string GetDescription(void) const;
  };
      // constructors and destructors
  Tableau(const Gambit::Matrix<Gambit::Rational> &A, const gbtVector<Gambit::Rational> &b); 
  Tableau(const Gambit::Matrix<Gambit::Rational> &A, const gbtArray<int> &art, 
	  const gbtVector<Gambit::Rational> &b); 
  Tableau(const Tableau<Gambit::Rational>&);
  virtual ~Tableau();
  
  Tableau<Gambit::Rational>& operator=(const Tableau<Gambit::Rational>&);
  
  // pivoting
  int CanPivot(int outgoing,int incoming);
  void Pivot(int outrow,int col); // pivot -- outgoing is row, incoming is column
  void SolveColumn(int, gbtVector<Gambit::Rational> &);  // column in new basis 
  void GetColumn(int, gbtVector<Gambit::Rational> &) const;  // column in new basis 
  
  // raw Tableau functions

  void Refactor();
  void SetRefactor(int);

  void SetConst(const gbtVector<Gambit::Rational> &bnew);
  void SetBasis( const Basis &); // set new Tableau
  void Solve(const gbtVector<Gambit::Rational> &b, gbtVector<Gambit::Rational> &x);  // solve M x = b
  void SolveT(const gbtVector<Gambit::Rational> &c, gbtVector<Gambit::Rational> &y);  // solve y M = c
  
  bool IsFeasible();
  bool IsLexMin();
  void BasisVector(gbtVector<Gambit::Rational> &out) const;
  Gambit::Integer TotDenom() const;
};

#endif     // TABLEAU_H
