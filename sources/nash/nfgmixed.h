//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria via Mangasarian's algorithm
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

//
// Enum implements the procedure described in 
// Mangasarian, O. L., "Equilibrium points of bimatrix games", 
// SIAM 12 (1964): 778-780 for finding all extreme points of 
// the Nash equilibrium components of a two person game. 
// 

#ifndef NFGMIXED_H
#define NFGMIXED_H

#include "nfgalgorithm.h"

template <class T> class gbtNfgNashEnumMixed : public gbtNfgNashAlgorithm {
private:
  int m_stopAfter;
  bool m_cliques;

  // Private auxiliary functions
  bool EqZero(const T &) const;

public:
  gbtNfgNashEnumMixed(void);
  virtual ~gbtNfgNashEnumMixed() { }
  
  int StopAfter(void) const { return m_stopAfter; }
  void SetStopAfter(int p_stopAfter) { m_stopAfter = p_stopAfter; }

  bool Cliques(void) const { return m_cliques; }
  void SetCliques(bool p_cliques) { m_cliques = p_cliques; }

  gText GetAlgorithm(void) const { return "EnumMixed[NFG]"; }
  gList<MixedSolution> Solve(const gbtNfgSupport &, gStatus &);
};

#endif  // NFGMIXED_H




