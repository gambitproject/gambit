//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to algorithm to solve extensive forms using linear
// complementarity program from sequence form
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

#ifndef SEQFORM_H
#define SEQFORM_H

#include "efgalgorithm.h"
#include "numerical/lemketab.h"

template <class T> class efgLcp : public efgNashAlgorithm {
private:
  int m_stopAfter, m_maxDepth;

  int ns1,ns2,ni1,ni2;
  T maxpay,eps;
  gList<BFS<T> > List;
  gList<Infoset *> isets1, isets2;

  void FillTableau(const EFSupport &, gMatrix<T> &, const Node *, T,
		   int, int, int, int);
  int Add_BFS(const LTableau<T> &tab);
  int All_Lemke(const EFSupport &, int dup, LTableau<T> &B,
		int depth, gMatrix<T> &, gList<BehavSolution> &, gStatus &);
  
  void GetProfile(const EFSupport &, const LTableau<T> &tab, 
		  gDPVector<T> &, const gVector<T> &, 
		  const Node *n, int,int);

public:
  efgLcp(void);
  virtual ~efgLcp();
  
  int StopAfter(void) const { return m_stopAfter; }
  void SetStopAfter(int p_stopAfter) { m_stopAfter = p_stopAfter; }

  int MaxDepth(void) const { return m_maxDepth; }
  void SetMaxDepth(int p_maxDepth) { m_maxDepth = p_maxDepth; }

  gText GetAlgorithm(void) const { return "Lcp[EFG]"; }
  gList<BehavSolution> Solve(const EFSupport &, gStatus &);
};

#endif    // SEQFORM_H





