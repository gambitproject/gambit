//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/liblinear/btableau.h
// Interface to base tableau classes
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

#ifndef BTABLEAU_H  
#define BTABLEAU_H

#include "bfs.h"
#include "basis.h"

// ---------------------------------------------------------------------------
//                          BaseTableau Stuff
// ---------------------------------------------------------------------------


template <class T> class BaseTableau {
public:
  class BadPivot : public Gambit::Exception  {
  public:
    virtual ~BadPivot() throw() { }
    const char *what(void) const throw() { return "Bad Pivot in BaseTableau"; }
  };

  virtual ~BaseTableau() { }

  bool ColIndex(int) const;
  bool RowIndex(int) const;
  bool ValidIndex(int) const;
  virtual int MinRow() const = 0;
  virtual int MaxRow() const = 0;
  virtual int MinCol() const = 0;
  virtual int MaxCol() const = 0;

  virtual bool Member(int i) const = 0;
    // is variable i is a member of basis
  virtual int Label(int i) const = 0;   
    // return variable in i'th position of Tableau
  virtual int Find(int i) const = 0;  
    // return position of variable i
  
      // pivoting
  virtual int CanPivot(int outgoing,int incoming) = 0;
  virtual void Pivot(int outrow,int col) = 0;
      // perform pivot operation -- outgoing is row, incoming is column
  void CompPivot(int outlabel,int col);
  virtual long NumPivots() const = 0;
  
      // raw Tableau functions
  virtual  void Refactor() = 0;
};

// ---------------------------------------------------------------------------
//                           TableauInterface Stuff
// ---------------------------------------------------------------------------

template <class T> class TableauInterface : public BaseTableau<T>{
protected:
  const Gambit::Matrix<T> *A;  // should this be private?
  const Gambit::Vector<T> *b;  // should this be private?
  Basis basis; 
  Gambit::Vector<T> solution;  // current solution vector. should this be private?
  long npivots;
  T eps1,eps2;
  Gambit::Array<int> artificial;  // artificial variables

public:
  TableauInterface(const Gambit::Matrix<T> &A, const Gambit::Vector<T> &b); 
  TableauInterface(const Gambit::Matrix<T> &A, const Gambit::Array<int> &art, 
		   const Gambit::Vector<T> &b); 
  TableauInterface(const TableauInterface<T>&);
  virtual ~TableauInterface();

  TableauInterface<T>& operator=(const TableauInterface<T>&);

  // information

  int MinRow() const;
  int MaxRow() const;
  int MinCol() const;
  int MaxCol() const;

  Basis & GetBasis(void);
  const Gambit::Matrix<T> & Get_A(void) const;
  const Gambit::Vector<T> & Get_b(void) const;
  
  bool Member(int i) const;
  int Label(int i) const;   // return variable in i'th position of Tableau
  int Find(int i) const;  // return Tableau position of variable i

  long NumPivots() const;
  long &NumPivots();
  
  void Mark(int label);     // marks label to block it from entering basis
  void UnMark(int label);   // unmarks label
  bool IsBlocked(int label) const;   // returns true if label is blocked
  
  virtual void BasisVector(Gambit::Vector<T> &x) const = 0; // solve M x = (*b)
  void GetColumn( int , Gambit::Vector<T> &) const;  // raw column
  void GetBasis( Basis & ) const; // return Basis for current Tableau

  BFS<T> GetBFS1(void) const; 
  BFS<T> GetBFS(void);  // used in lpsolve for some reason

  virtual int CanPivot(int outgoing,int incoming) = 0;
  virtual void Pivot(int outrow,int col) = 0; // pivot -- outgoing is row, incoming is column
  virtual void SolveColumn(int, Gambit::Vector<T> &) = 0;  // column in new basis 
  virtual void Solve(const Gambit::Vector<T> &b, Gambit::Vector<T> &x) = 0;  // solve M x = b
  virtual void SolveT(const Gambit::Vector<T> &c, Gambit::Vector<T> &y) = 0;  // solve y M = c

  virtual void Refactor() = 0;
  virtual void SetRefactor(int) = 0;

      // miscellaneous functions
  bool EqZero(T x) const;
  bool LtZero(T x) const;
  bool GtZero(T x) const;
  bool LeZero(T x) const;
  bool GeZero(T x) const;
  T Epsilon(int i = 2) const;
  bool IsArtifColumn(int col) const;
};

#endif     // BTABLEAU_H
