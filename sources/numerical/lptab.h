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
//  gbtLPTableau Stuff (For Linear Programming code)
// ---------------------------------------------------------------------------

#ifndef LPTAB_H  
#define LPTAB_H

#include "tableau.h"

template <class T> class gbtLPTableau : public gbtTableau<T> {
private:
  gbtVector<T> dual;
  gbtBlock<T> unitcost;
  gbtBlock<T> cost;
  gbtBlock<bool> UB,LB;  // does col have upper/lower bound?
  gbtBlock<T> ub,lb;   // upper/lower bound
  
  void SolveDual();
public:
  gbtLPTableau(const gbtMatrix<T> &A, const gbtVector<T> &b); 
  gbtLPTableau(const gbtMatrix<T> &A, const gbtBlock<int> &art, const gbtVector<T> &b); 
  gbtLPTableau(const gbtLPTableau<T>&);
  virtual ~gbtLPTableau();
  
  gbtLPTableau<T>& operator=(const gbtLPTableau<T>&);
  
      // cost information
  void SetCost(const gbtVector<T>& ); // unit column cost := 0
  void SetCost(const gbtVector<T>&, const gbtVector<T>& );
  gbtVector<T> GetCost() const;
  gbtVector<T> GetUnitCost() const;
  T TotalCost(); // cost of current solution
  T RelativeCost(int) const; // negative index convention
  void RelativeCostVector(gbtVector<T> &, gbtVector<T> &); 
  void DualVector(gbtVector<T> &) const; // column vector
      // Redefined functions
  void Refactor();
  void Pivot(int outrow,int col);
  void ReversePivots(gbtList<gbtArray<int> > &);
  bool IsReversePivot(int i, int j);
  void DualReversePivots(gbtList<gbtArray<int> > &);
  bool IsDualReversePivot(int i, int j);
  gbtBasicFeasibleSolution<T> DualBFS(void) const;

  // returns the label of the index of the last artificial variable
  int LastLabel( void );

  // select Basis elements according to gbtTableau rows and cols
  void BasisSelect(const gbtBlock<T>&rowv, gbtVector<T> &colv) const;

  // as above, but unit column elements nonzero
  void BasisSelect(const gbtBlock<T>&unitv, const gbtBlock<T>&rowv,
		   gbtVector<T>&colv) const; 
};

#endif     // LPTAB_H
