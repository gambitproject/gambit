//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to algorithm to solve efgs via linear programming
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

#ifndef EFGLP_H
#define EFGLP_H

#include "numerical/tableau.h"
#include "numerical/lpsolve.h"
#include "efgalgorithm.h"

template <class T> class gbtEfgNashLp : public gbtEfgNashAlgorithm {
private:
  T maxpay, minpay;
  int ns1,ns2,ni1,ni2;
  gbtList<gbtBasicFeasibleSolution<T> > List;
  gbtList<gbtGameInfoset> isets1, isets2;

  void FillTableau(const gbtEfgSupport &,
		   gbtMatrix<T> &, const gbtGameNode &, T ,int ,int , int ,int );
  void GetSolutions(const gbtEfgSupport &, gbtBehavNashSet &) const;
  int Add_BFS(/*const*/ gbtLPSolver<T> &B);
  
  void GetProfile(const gbtEfgSupport &, gbtDPVector<T> &v, const gbtBasicFeasibleSolution<T> &sol,
		  const gbtGameNode &n, int s1,int s2) const;

public:
  gbtEfgNashLp(void);
  virtual ~gbtEfgNashLp() { }

  gbtText GetAlgorithm(void) const { return "Lp[EFG]"; }
  gbtBehavNashSet Solve(const gbtEfgSupport &, gbtStatus &);
};


#endif  // EFGLP_H
