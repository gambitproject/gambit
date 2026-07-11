//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/hp/hp.cc
// Computation of a Nash equilibria using a differentiable homotopy
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

#include <iostream>
#include "gambit.h"
#include "solvers/hp/hp.h"
#include "solvers/hp/hpsystem.h"

namespace Gambit {
std::list<MixedStrategyProfile<double>>
HPStrategySolve(const MixedStrategyProfile<double> &p_prior)
{
  const std::list<MixedStrategyProfile<double>> result;

  const HPEquationSystem system(p_prior);
  Vector<double> init_pt = system.ComputeInitialPoint();

  std::cout << "Computed starting vector (alpha form): ";
  for (size_t i = 1; i <= init_pt.size(); ++i) {
    std::cout << init_pt[i] << " ";
  }
  std::cout << std::endl;

  std::list<MixedStrategyProfile<double>> ret;
  const MixedStrategyProfile<double> T_0 = system.ExtractEquilibrium(init_pt);
  ret.push_back(T_0);

  return ret;
}
} // namespace Gambit
