//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria of an extensive form game by recursively
// solving subgames
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

#ifndef SUBSOLVE_H
#define SUBSOLVE_H

#include "efgalgorithm.h"
#include "nfgalgorithm.h"

class SubgameSolver : public efgNashAlgorithm  {
private:
  bool m_isPerfectRecall;
  double time;
  BehavProfile<gNumber> *solution;
  gList<BehavSolution> solutions;
  efgNashAlgorithm *m_efgAlgorithm;
  nfgNashAlgorithm *m_nfgAlgorithm;

  gArray<gArray<Infoset *> *> infosets;

  void FindSubgames(const EFSupport &, gStatus &, 
		    Node *, gList<BehavSolution> &, gList<gbtEfgOutcome> &);
  
public:
  SubgameSolver(void) : m_efgAlgorithm(0), m_nfgAlgorithm(0) { }
  virtual ~SubgameSolver();
    
  gText GetAlgorithm(void) const;
  gList<BehavSolution> Solve(const EFSupport &, gStatus &);

  void SetAlgorithm(efgNashAlgorithm *p_algorithm)
    { m_efgAlgorithm = p_algorithm; m_nfgAlgorithm = 0; }
  void SetAlgorithm(nfgNashAlgorithm *p_algorithm)
    { m_nfgAlgorithm = p_algorithm; m_efgAlgorithm = 0; }
  
  double Time(void) const   { return time; }
};


#endif   // SUBSOLVE_H



