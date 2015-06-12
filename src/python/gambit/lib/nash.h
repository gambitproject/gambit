//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
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

#include "libgambit/libgambit.h"
#include "tools/logit/nfglogit.h"

using namespace std;
using namespace Gambit;

class NullBuffer : public std::streambuf {
public:
  int overflow(int c) { return c; }
};

LogitQREMixedStrategyProfile *logit_estimate(MixedStrategyProfile<double> *p_frequencies)
{
  LogitQREMixedStrategyProfile start(p_frequencies->GetGame());
  StrategicQREEstimator alg;
  NullBuffer null_buffer;
  std::ostream null_stream(&null_buffer);
  LogitQREMixedStrategyProfile result = alg.Estimate(start, *p_frequencies, 
						     null_stream, 1000000.0, 1.0);
  return new LogitQREMixedStrategyProfile(result);
}



