//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// gbtTableau class for Lemke-Howson algorithm
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
#include "game/game.h"

template <class T> class gbtLemkeHowsonTableau : public gbtBaseTableau<T>{
protected:
  gbtLemkeTableau<T> T1,T2;
  gbtVector<T> tmp1,tmp2; // temporary column vectors, to avoid allocation
  gbtVector<T> solution;
public:
      // constructors and destructors
  gbtLemkeHowsonTableau(const gbtMatrix<T> &A1, const gbtMatrix<T> &A2, 
	    const gbtVector<T> &b1, const gbtVector<T> &b2); 
  gbtLemkeHowsonTableau(const gbtLemkeHowsonTableau<T>&);
  virtual ~gbtLemkeHowsonTableau();
  
  gbtLemkeHowsonTableau<T>& operator=(const gbtLemkeHowsonTableau<T>&);
  
      // information
  int MinRow() const;
  int MaxRow() const;
  int MinCol() const;
  int MaxCol() const;
  T Epsilon() const;
  
  bool Member(int i) const;
  int Label(int i) const;   // return variable in i'th position of gbtTableau
  int Find(int i) const;  // return gbtTableau position of variable i
  
      // pivoting
  int CanPivot(int outgoing,int incoming);
  void Pivot(int outrow,int inlabel);
      // perform pivot operation -- outgoing is row, incoming is column
  long NumPivots() const;
  
      // raw gbtTableau functions
  void Refactor();
  
      // miscellaneous functions
  gbtBasicFeasibleSolution<T> GetBFS(void);

  int PivotIn(int i);
  int ExitIndex(int i);
  int LemkePath(int dup); // follow a path of ACBFS's from one CBFS to another
};



#endif     // LHTAB_H
