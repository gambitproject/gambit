//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//                          Albert Xin Jiang <albertjiang@gmail.com>
//
// FILE: library/src/gtracer/aggame.cc
// Implement GNM-specific routines for action graph games
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

#include "aggame.h"

namespace Gambit::gametracer {

// The payoff Jacobian: dest(i, j) is the payoff to the owner of action i if it deviates to i
// while the owner of action j simultaneously deviates to j, everyone else playing s. Computed a
// full row at a time via AGG::getPayoffJacobianRow -- see its comment for the algorithm -- so
// this only touches AGG's public API and avoids redoing the projection/convolution work
// independently for every (row, column) pair.
void aggame::payoffMatrix(cmatrix &dest, const cvector &s, double fuzz) const
{
  const std::vector<double> sp(s.values(), s.values() + s.getm());

  // deal with the diagonal
  for (int rown = 0; rown < aggPtr->getNumPlayers(); ++rown) {
    double fuzzcount = fuzz;
    for (int rowi = firstAction(rown); rowi < lastAction(rown); rowi++) {
      for (int coli = firstAction(rown); coli < lastAction(rown); coli++) {
        dest(rowi, coli) = fuzzcount;
        fuzzcount += fuzz;
      }
    }
  }

  // off-diagonal entries
  std::vector<double> row(aggPtr->getNumActions());
  for (int rown = 0; rown < aggPtr->getNumPlayers(); ++rown) {
    for (int act1 = 0; act1 < aggPtr->getNumActions(rown); act1++) {
      aggPtr->getPayoffJacobianRow(rown, act1, sp, row);
      for (int coln = 0; coln < aggPtr->getNumPlayers(); ++coln) {
        if (coln == rown) {
          continue;
        }
        for (int act2 = 0; act2 < aggPtr->getNumActions(coln); act2++) {
          dest(act1 + firstAction(rown), act2 + firstAction(coln)) = row[act2 + firstAction(coln)];
        }
      }
    }
  }
}

void aggame::KSymPayoffMatrix(cmatrix &dest, const cvector &s, double fuzz) const
{
  const std::vector<double> sp(s.values(), s.values() + s.getm());
  // simple implementation using expected payoffs:
  for (int rowcls = 0; rowcls < getNumPlayerClasses(); ++rowcls) {
    for (int rowa = 0; rowa < getNumKSymActions(rowcls); ++rowa) {
      for (int colcls = 0; colcls < getNumPlayerClasses(); ++colcls) {
        int multiplier = aggPtr->getPlayerClass(colcls).size();
        if (rowcls == colcls) {
          multiplier -= 1;
        }
        for (int cola = 0; cola < getNumKSymActions(colcls); ++cola) {

          dest(rowa + firstKSymAction(rowcls), cola + firstKSymAction(colcls)) =
              (double)multiplier * aggPtr->getKSymMixedPayoff(sp, rowcls, rowa, colcls, cola);
        }
      }
    }
  }

  double fuzzcount;
  for (int rown = 0; rown < getNumPlayerClasses(); ++rown) {
    fuzzcount = fuzz;
    for (int rowi = firstKSymAction(rown); rowi < lastKSymAction(rown); rowi++) {
      if (getNumPlayerClasses() == 1) { // symmetric case
        dest(rowi, rowi) += fuzzcount;
        fuzzcount += fuzz;
      }
      else {
        for (int coli = firstKSymAction(rown); coli < lastKSymAction(rown); coli++) {
          dest(rowi, coli) += fuzzcount;
          fuzzcount += fuzz;
        }
      }
    }
  }
}

} // end namespace Gambit::gametracer
