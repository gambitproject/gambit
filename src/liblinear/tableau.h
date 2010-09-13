//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/liblinear/tableau.h
// Interface to tableau class
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
  Gambit::Vector<double> tmpcol; // temporary column vector, to avoid allocation

public:
      // constructors and destructors
  Tableau(const Gambit::Matrix<double> &A, const Gambit::Vector<double> &b); 
  Tableau(const Gambit::Matrix<double> &A, const Gambit::Array<int> &art, 
	  const Gambit::Vector<double> &b); 
  Tableau(const Tableau<double>&);
  virtual ~Tableau();
  
  Tableau<double>& operator=(const Tableau<double>&);
  
  // pivoting
  int CanPivot(int outgoing,int incoming);
  void Pivot(int outrow,int col); // pivot -- outgoing is row, incoming is column
  void BasisVector(Gambit::Vector<double> &x) const; // solve M x = (*b)
  void SolveColumn(int, Gambit::Vector<double> &);  // column in new basis 
  void Solve(const Gambit::Vector<double> &b, Gambit::Vector<double> &x);  // solve M x = b
  void SolveT(const Gambit::Vector<double> &c, Gambit::Vector<double> &y);  // solve y M = c
  
  // raw Tableau functions

  void Refactor();
  void SetRefactor(int);

  void SetConst(const Gambit::Vector<double> &bnew);
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
  Gambit::Vector<Gambit::Integer> Coeff;   // and coeffieient vector
  Gambit::Integer totdenom;  // This carries the denominator for Q data or 1 for Z
  Gambit::Integer denom;  // This is the denominator for the simplex

  Gambit::Vector<Gambit::Rational> tmpcol; // temporary column vector, to avoid allocation

  void MySolveColumn(int, Gambit::Vector<Gambit::Rational> &);  // column in new basis 

protected:
  Gambit::Array<int> nonbasic;     //** nonbasic variables -- should be moved to Basis

public:
  class BadDenom : public Gambit::Exception  {
  public:
    virtual ~BadDenom() throw() { }
    const char *what(void) const throw() { return "Bad denominator in Tableau"; } 
  };
      // constructors and destructors
  Tableau(const Gambit::Matrix<Gambit::Rational> &A, const Gambit::Vector<Gambit::Rational> &b); 
  Tableau(const Gambit::Matrix<Gambit::Rational> &A, const Gambit::Array<int> &art, 
	  const Gambit::Vector<Gambit::Rational> &b); 
  Tableau(const Tableau<Gambit::Rational>&);
  virtual ~Tableau();
  
  Tableau<Gambit::Rational>& operator=(const Tableau<Gambit::Rational>&);
  
  // pivoting
  int CanPivot(int outgoing,int incoming);
  void Pivot(int outrow,int col); // pivot -- outgoing is row, incoming is column
  void SolveColumn(int, Gambit::Vector<Gambit::Rational> &);  // column in new basis 
  void GetColumn(int, Gambit::Vector<Gambit::Rational> &) const;  // column in new basis 
  
  // raw Tableau functions

  void Refactor();
  void SetRefactor(int);

  void SetConst(const Gambit::Vector<Gambit::Rational> &bnew);
  void SetBasis( const Basis &); // set new Tableau
  void Solve(const Gambit::Vector<Gambit::Rational> &b, Gambit::Vector<Gambit::Rational> &x);  // solve M x = b
  void SolveT(const Gambit::Vector<Gambit::Rational> &c, Gambit::Vector<Gambit::Rational> &y);  // solve y M = c
  
  bool IsFeasible();
  bool IsLexMin();
  void BasisVector(Gambit::Vector<Gambit::Rational> &out) const;
  Gambit::Integer TotDenom() const;
};

#endif     // TABLEAU_H
