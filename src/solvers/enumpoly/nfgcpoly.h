//
// This file is part of Gambit
// Copyright (c) 1994-2024, Litao Wei and The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/nfgcpoly.h
// Compute Nash equilibria via heuristic search on game supports
// (Porter, Nudelman & Shoham, 2004)
// Implemented by Litao Wei
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

#ifndef NFGCPOLY_H
#define NFGCPOLY_H

#include "gambit.h"
#include "nfgensup.h"
#include "gpoly.h"
#include "gpolylst.h"
#include "rectangl.h"
#include "quiksolv.h"

using namespace Gambit;

class HeuristicPolEnumModule {
private:
  int m_stopAfter;
  double eps{0.0};
  Game NF;
  const StrategySupportProfile &support;
  gSpace Space;
  term_order Lex;
  int num_vars;
  long nevals;
  double time{0.0};
  Gambit::List<MixedStrategyProfile<double>> solutions;
  bool is_singular;

  bool EqZero(double x) const;

  // p_i_j as a gPoly, with last prob in terms of previous probs
  gPoly<double> Prob(int i, int j) const;

  // equation for when player i sets strat1 = strat2
  // with last probs for each player substituted out.
  gPoly<double> IndifferenceEquation(int i, int strat1, int strat2) const;
  gPolyList<double> IndifferenceEquations() const;
  gPolyList<double> LastActionProbPositiveInequalities() const;
  gPolyList<double> NashOnSupportEquationsAndInequalities() const;
  Gambit::List<Vector<double>> NashOnSupportSolnVectors(const gPolyList<double> &equations,
                                                        const gRectangle<double> &Cube);

  int SaveSolutions(const Gambit::List<Vector<double>> &list);

public:
  HeuristicPolEnumModule(const StrategySupportProfile &, int p_stopAfter);

  int PolEnum();

  long NumEvals() const;
  double Time() const;

  const Gambit::List<MixedStrategyProfile<double>> &GetSolutions() const;
  Vector<double> SolVarsFromMixedStrategyProfile(const MixedStrategyProfile<double> &) const;

  int PolishKnownRoot(Vector<double> &) const;

  MixedStrategyProfile<double> ReturnPolishedSolution(const Vector<double> &) const;

  bool IsSingular() const;
};

#endif // NFGCPOLY_H
