//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to LP tableaus
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

// ---------------------------------------------------------------------------
//  LPTableau Stuff (For Linear Programming code)
// ---------------------------------------------------------------------------

#ifndef LPTAB_H  
#define LPTAB_H

#include "tableau.h"

template <class T> class LPTableau : public Tableau<T> {
private:
  gVector<T> dual;
  gBlock<T> unitcost;
  gBlock<T> cost;
  gBlock<bool> UB,LB;  // does col have upper/lower bound?
  gBlock<T> ub,lb;   // upper/lower bound
  
  void SolveDual();
public:
  class BadDim : public gException  {
  public:
    virtual ~BadDim();
    gText Description(void) const;
  };
  class BadPivot : public gException  {
  public:
    virtual ~BadPivot();
    gText Description(void) const;
  };
  LPTableau(const gMatrix<T> &A, const gVector<T> &b); 
  LPTableau(const gMatrix<T> &A, const gBlock<int> &art, const gVector<T> &b); 
  LPTableau(const LPTableau<T>&);
  virtual ~LPTableau();
  
  LPTableau<T>& operator=(const LPTableau<T>&);
  
      // cost information
  void SetCost(const gVector<T>& ); // unit column cost := 0
  void SetCost(const gVector<T>&, const gVector<T>& );
  gVector<T> GetCost() const;
  gVector<T> GetUnitCost() const;
  T TotalCost(); // cost of current solution
  T RelativeCost(int) const; // negative index convention
  void RelativeCostVector(gVector<T> &, gVector<T> &); 
  void DualVector(gVector<T> &) const; // column vector
      // Redefined functions
  void Refactor();
  void Pivot(int outrow,int col);
  void ReversePivots(gList<gArray<int> > &);
  bool IsReversePivot(int i, int j);
  void DualReversePivots(gList<gArray<int> > &);
  bool IsDualReversePivot(int i, int j);
  BFS<T> DualBFS(void) const;

  // returns the label of the index of the last artificial variable
  int LastLabel( void );

  // select Basis elements according to Tableau rows and cols
  void BasisSelect(const gBlock<T>&rowv, gVector<T> &colv) const;

  // as above, but unit column elements nonzero
  void BasisSelect(const gBlock<T>&unitv, const gBlock<T>&rowv,
		   gVector<T>&colv) const; 

  void BigDump(gOutput &);
};

#endif     // LPTAB_H
