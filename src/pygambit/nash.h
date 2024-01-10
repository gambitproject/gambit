//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/python/gambit/lib/nash.h
// Shims for Cython wrapper for computation of equilibrium
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

#include "gambit.h"
#include "solvers/logit/nfglogit.h"

using namespace std;
using namespace Gambit;

class NullBuffer : public std::streambuf {
public:
  int overflow(int c) { return c; }
};

std::shared_ptr<LogitQREMixedStrategyProfile>
logit_estimate(std::shared_ptr<MixedStrategyProfile<double>> p_frequencies)
{
  LogitQREMixedStrategyProfile start(p_frequencies->GetGame());
  StrategicQREEstimator alg;
  NullBuffer null_buffer;
  std::ostream null_stream(&null_buffer);
  LogitQREMixedStrategyProfile result = alg.Estimate(start, *p_frequencies, 
						     null_stream, 1000000.0, 1.0);
  return make_shared<LogitQREMixedStrategyProfile>(result);
}

std::shared_ptr<LogitQREMixedStrategyProfile>
logit_atlambda(const Game &p_game, double p_lambda)
{
  LogitQREMixedStrategyProfile start(p_game);
  StrategicQREPathTracer alg;
  NullBuffer null_buffer;
  std::ostream null_stream(&null_buffer);
  return make_shared<LogitQREMixedStrategyProfile>(
    alg.SolveAtLambda(start, null_stream, p_lambda, 1.0)
  );
}

List<LogitQREMixedStrategyProfile>
logit_principal_branch(const Game &p_game, double p_maxLambda=1000000.0)
{
  LogitQREMixedStrategyProfile start(p_game);
  StrategicQREPathTracer alg;
  NullBuffer null_buffer;
  std::ostream null_stream(&null_buffer);
  return alg.TraceStrategicPath(start, null_stream, p_maxLambda, 1.0);
}

