//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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

namespace Gambit {

class LogitQREMixedBehaviorProfile {
public:
  explicit LogitQREMixedBehaviorProfile(const Game &p_game) : m_profile(p_game), m_lambda(0.0) {}
  LogitQREMixedBehaviorProfile(const MixedBehaviorProfile<double> &p_profile, double p_lambda,
                               double p_logLike)
    : m_profile(p_profile), m_lambda(p_lambda), m_logLike(p_logLike)
  {
  }
  double GetLambda() const { return m_lambda; }
  const MixedBehaviorProfile<double> &GetProfile() const { return m_profile; }
  double GetLogLike() const { return m_logLike; }

  Game GetGame() const { return m_profile.GetGame(); }
  size_t BehaviorProfileLength() const { return m_profile.BehaviorProfileLength(); }
  double operator[](int i) const { return m_profile[i]; }

private:
  const MixedBehaviorProfile<double> m_profile;
  double m_lambda;
  double m_logLike;
};

class AgentQREPathTracer : public PathTracer {
public:
  AgentQREPathTracer() : m_fullGraph(true), m_decimals(6) {}
  ~AgentQREPathTracer() override = default;

  List<LogitQREMixedBehaviorProfile> TraceAgentPath(const LogitQREMixedBehaviorProfile &p_start,
                                                    std::ostream &p_stream, double p_regret,
                                                    double p_omega) const;
  LogitQREMixedBehaviorProfile SolveAtLambda(const LogitQREMixedBehaviorProfile &p_start,
                                             std::ostream &p_logStream, double p_targetLambda,
                                             double p_omega) const;

  void SetFullGraph(bool p_fullGraph) { m_fullGraph = p_fullGraph; }
  bool GetFullGraph() const { return m_fullGraph; }

  void SetDecimals(int p_decimals) { m_decimals = p_decimals; }
  int GetDecimals() const { return m_decimals; }

protected:
  bool m_fullGraph;
  int m_decimals;

  class EquationSystem;
  class CallbackFunction;
  class LambdaCriterion;
};

class AgentQREEstimator : public AgentQREPathTracer {
public:
  AgentQREEstimator() = default;
  ~AgentQREEstimator() override = default;

  LogitQREMixedBehaviorProfile Estimate(const LogitQREMixedBehaviorProfile &p_start,
                                        const MixedBehaviorProfile<double> &p_frequencies,
                                        std::ostream &p_logStream, double p_maxLambda,
                                        double p_omega);

protected:
  class CriterionFunction;
  class CallbackFunction;
};

inline LogitQREMixedBehaviorProfile
LogitBehaviorEstimate(const MixedBehaviorProfile<double> &p_frequencies, double p_firstStep,
                      double p_maxAccel)
{
  LogitQREMixedBehaviorProfile start(p_frequencies.GetGame());
  AgentQREEstimator alg;
  alg.SetMaxDecel(p_maxAccel);
  alg.SetStepsize(p_firstStep);
  std::ostringstream ostream;
  return alg.Estimate(start, p_frequencies, ostream, 2.0, 1.0);
}

inline List<MixedBehaviorProfile<double>> LogitBehaviorSolve(const Game &p_game, double p_epsilon,
                                                             double p_firstStep, double p_maxAccel)
{
  AgentQREPathTracer tracer;
  tracer.SetMaxDecel(p_maxAccel);
  tracer.SetStepsize(p_firstStep);
  tracer.SetFullGraph(false);
  std::ostringstream ostream;
  auto result =
      tracer.TraceAgentPath(LogitQREMixedBehaviorProfile(p_game), ostream, p_epsilon, 1.0);
  auto ret = List<MixedBehaviorProfile<double>>();
  if (!result.empty()) {
    ret.push_back(result.back().GetProfile());
  }
  return ret;
}

} // end namespace Gambit

#endif // EFGLOGIT_H
