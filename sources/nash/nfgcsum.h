//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface of algorithm to compute mixed strategy equilibria
// of constant sum normal form games via linear programming
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

#ifndef NFGCSUM_H
#define NFGCSUM_H

#include "nfgalgorithm.h"
#include "numerical/lpsolve.h"

template <class T> class nfgLp : public nfgNashAlgorithm {
private:
  int Add_BFS(const NFSupport &, /*const*/ LPSolve<T> &B,
	      gList<BFS<T> > &);
  void GetSolutions(const NFSupport &, const gList<BFS<T> > &,
		    gList<MixedSolution > &,
		    const T &) const;

public:
  nfgLp(void);
  virtual ~nfgLp() { }

  gText GetAlgorithm(void) const { return "Lp"; }
  gList<MixedSolution> Solve(const NFSupport &, gStatus &);
};

#endif    // NFGCSUM_H



