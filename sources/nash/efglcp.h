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

#ifndef EFGLCP_H
#define EFGLCP_H

#include "efgalgorithm.h"
#include "numerical/lemketab.h"

template <class T> class gbtEfgNashLcp : public gbtEfgNashAlgorithm {
private:
  int m_stopAfter, m_maxDepth;

  int ns1,ns2,ni1,ni2;
  T maxpay,eps;
  gbtList<gbtBasicFeasibleSolution<T> > List;
  gbtList<gbtEfgInfoset> isets1, isets2;

  void FillTableau(const gbtEfgSupport &, gbtMatrix<T> &, const gbtEfgNode &, T,
		   int, int, int, int);
  int Add_BFS(const gbtLemkeTableau<T> &tab);
  int All_Lemke(const gbtEfgSupport &, int dup, gbtLemkeTableau<T> &B,
		int depth, gbtMatrix<T> &, gbtList<BehavSolution> &, gbtStatus &);
  
  void GetProfile(const gbtEfgSupport &, const gbtLemkeTableau<T> &tab, 
		  gbtDPVector<T> &, const gbtVector<T> &, 
		  const gbtEfgNode &n, int,int);

public:
  gbtEfgNashLcp(void);
  virtual ~gbtEfgNashLcp();
  
  int StopAfter(void) const { return m_stopAfter; }
  void SetStopAfter(int p_stopAfter) { m_stopAfter = p_stopAfter; }

  int MaxDepth(void) const { return m_maxDepth; }
  void SetMaxDepth(int p_maxDepth) { m_maxDepth = p_maxDepth; }

  gbtText GetAlgorithm(void) const { return "Lcp[EFG]"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

#endif  // EFGLCP_H
