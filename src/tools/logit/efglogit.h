//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/logit/efglogit.h
// Computation of agent quantal response equilibrium correspondence for
// extensive games.
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

#ifndef EFGLOGIT_H
#define EFGLOGIT_H

#include "path.h"

namespace Gambit  {

class LogitQREMixedBehaviorProfile {
public:
  LogitQREMixedBehaviorProfile(const Game &p_game)
    : m_profile(p_game), m_lambda(0.0)
	{ }

  double GetLambda(void) const { return m_lambda; }
  const MixedBehaviorProfile<double> &GetProfile(void) const { return m_profile; }

  Game GetGame(void) const              { return m_profile.GetGame(); }
  int BehaviorProfileLength(void) const { return m_profile.Length(); }
  double operator[](int i) const        { return m_profile[i]; }
  
private:
  const MixedBehaviorProfile<double> m_profile;
  double m_lambda;
};


class AgentQREPathTracer : public PathTracer {
public:
  AgentQREPathTracer(void) : m_fullGraph(true), m_decimals(6) { }
  virtual ~AgentQREPathTracer() { }

  void 
  TraceAgentPath(const LogitQREMixedBehaviorProfile &p_start,
		 std::ostream &p_stream,
		 double p_maxLambda, double p_omega,
		 double p_targetLambda=-1.0);

  void SetFullGraph(bool p_fullGraph) { m_fullGraph = p_fullGraph; }
  bool GetFullGraph(void) const { return m_fullGraph; }

  void SetDecimals(int p_decimals) { m_decimals = p_decimals; }
  int GetDecimals(void) const { return m_decimals; }

private:
  bool m_fullGraph;
  int m_decimals;

  class EquationSystem;
  class CallbackFunction;
  class LambdaCriterion;
};

}  // end namespace Gambit

#endif  // EFGLOGIT_H
 
