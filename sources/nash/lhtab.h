//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Tableau class for Lemke-Howson algorithm
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

#ifndef LHTAB_H
#define LHTAB_H

#include "numerical/lemketab.h"
#include "game/nfg.h"

template <class T> class LHTableau : public BaseTableau<T>{
protected:
  LTableau<T> T1,T2;
  gVector<T> tmp1,tmp2; // temporary column vectors, to avoid allocation
  gVector<T> solution;
public:
      // constructors and destructors
  LHTableau(const gMatrix<T> &A1, const gMatrix<T> &A2, 
	    const gVector<T> &b1, const gVector<T> &b2); 
  LHTableau(const LHTableau<T>&);
  virtual ~LHTableau();
  
  LHTableau<T>& operator=(const LHTableau<T>&);
  
      // information
  int MinRow() const;
  int MaxRow() const;
  int MinCol() const;
  int MaxCol() const;
  T Epsilon() const;
  
  bool Member(int i) const;
  int Label(int i) const;   // return variable in i'th position of Tableau
  int Find(int i) const;  // return Tableau position of variable i
  
      // pivoting
  int CanPivot(int outgoing,int incoming);
  void Pivot(int outrow,int inlabel);
      // perform pivot operation -- outgoing is row, incoming is column
  long NumPivots() const;
  
      // raw Tableau functions
  void Refactor();
  
      // miscellaneous functions
  BFS<T> GetBFS(void);
  void Dump(gOutput &) const;

  int PivotIn(int i);
  int ExitIndex(int i);
  int LemkePath(int dup); // follow a path of ACBFS's from one CBFS to another
};



#endif     // LHTAB_H
