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

#ifndef NFGLP_H
#define NFGLP_H

#include "nfgalgorithm.h"
#include "numerical/lpsolve.h"

template <class T> class gbtNfgNashLp : public gbtNfgNashAlgorithm {
private:
  int Add_BFS(const gbtNfgSupport &, /*const*/ LPSolve<T> &B,
	      gList<BFS<T> > &);
  void GetSolutions(const gbtNfgSupport &, const gList<BFS<T> > &,
		    gList<MixedSolution > &,
		    const T &) const;

public:
  gbtNfgNashLp(void);
  virtual ~gbtNfgNashLp() { }

  gText GetAlgorithm(void) const { return "Lp[NFG]"; }
  gList<MixedSolution> Solve(const gbtNfgSupport &, gStatus &);
};

#endif  // NFGLP_H



