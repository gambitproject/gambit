//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/liblinear/lptab.h
// Interface to LP tableaus
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

// ---------------------------------------------------------------------------
//  LPTableau Stuff (For Linear Programming code)
// ---------------------------------------------------------------------------

#ifndef LPTAB_H  
#define LPTAB_H

#include "tableau.h"

template <class T> class LPTableau : public Tableau<T> {
private:
  Gambit::Vector<T> dual;
  Gambit::Array<T> unitcost;
  Gambit::Array<T> cost;
  Gambit::Array<bool> UB,LB;  // does col have upper/lower bound?
  Gambit::Array<T> ub,lb;   // upper/lower bound
  
  void SolveDual();
public:
  class BadPivot : public Gambit::Exception  {
  public:
    virtual ~BadPivot() throw() { }
    const char *what(void) const throw() { return "Bad pivot in LPTableau."; }
  };
  LPTableau(const Gambit::Matrix<T> &A, const Gambit::Vector<T> &b); 
  LPTableau(const Gambit::Matrix<T> &A, const Gambit::Array<int> &art, const Gambit::Vector<T> &b); 
  LPTableau(const LPTableau<T>&);
  virtual ~LPTableau();
  
  LPTableau<T>& operator=(const LPTableau<T>&);
  
      // cost information
  void SetCost(const Gambit::Vector<T>& ); // unit column cost := 0
  void SetCost(const Gambit::Vector<T>&, const Gambit::Vector<T>& );
  Gambit::Vector<T> GetCost() const;
  Gambit::Vector<T> GetUnitCost() const;
  T TotalCost(); // cost of current solution
  T RelativeCost(int) const; // negative index convention
  void RelativeCostVector(Gambit::Vector<T> &, Gambit::Vector<T> &); 
  void DualVector(Gambit::Vector<T> &) const; // column vector
      // Redefined functions
  void Refactor();
  void Pivot(int outrow,int col);
  void ReversePivots(Gambit::List<Gambit::Array<int> > &);
  bool IsReversePivot(int i, int j);
  void DualReversePivots(Gambit::List<Gambit::Array<int> > &);
  bool IsDualReversePivot(int i, int j);
  BFS<T> DualBFS(void) const;

  // returns the label of the index of the last artificial variable
  int LastLabel( void );

  // select Basis elements according to Tableau rows and cols
  void BasisSelect(const Gambit::Array<T>&rowv, Gambit::Vector<T> &colv) const;

  // as above, but unit column elements nonzero
  void BasisSelect(const Gambit::Array<T>&unitv, const Gambit::Array<T>&rowv,
		   Gambit::Vector<T>&colv) const; 
};

#endif     // LPTAB_H
