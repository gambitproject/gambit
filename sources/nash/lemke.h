//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria via Lemke-Howson algorithm
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

#ifndef LEMKE_H
#define LEMKE_H

#include "lhtab.h"
#include "nfgalgorithm.h"

template <class T> class nfgLcp : public nfgNashAlgorithm  {
private:
  int m_stopAfter, m_maxDepth;

  int AddBfs(LHTableau<T> &, gList<BFS<T> > &);
  gList<MixedSolution> AddSolutions(const NFSupport &,
				    const gList<BFS<T> > &, const T &);
  void AllLemke(const NFSupport &, int, LHTableau<T> &B, gList<BFS<T> > &,
		int depth, gStatus &);
  
public:
  nfgLcp(void);
  virtual ~nfgLcp() { }
  
  int StopAfter(void) const { return m_stopAfter; }
  void SetStopAfter(int p_stopAfter) { m_stopAfter = p_stopAfter; }

  int MaxDepth(void) const { return m_maxDepth; }
  void SetMaxDepth(int p_maxDepth) { m_maxDepth = p_maxDepth; }

  gText GetAlgorithm(void) const { return "Lcp[NFG]"; }
  gList<MixedSolution> Solve(const NFSupport &, gStatus &);
};


#endif    // LEMKE_H





