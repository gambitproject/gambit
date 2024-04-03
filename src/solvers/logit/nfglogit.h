//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: nfglogit.cc
// Computation of quantal response equilibrium correspondence for
// normal form games.
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

#ifndef NFGLOGIT_H
#define NFGLOGIT_H

#include "path.h"

namespace Gambit {

class LogitQREMixedStrategyProfile {
  friend class StrategicQREPathTracer;
  friend class StrategicQREEstimator;

public:
  explicit LogitQREMixedStrategyProfile(const Game &p_game)
    : m_profile(p_game->NewMixedStrategyProfile(0.0)), m_lambda(0.0)
  {
  }

  double GetLambda() const { return m_lambda; }
  const MixedStrategyProfile<double> &GetProfile() const { return m_profile; }
  double GetLogLike() const { return m_logLike; }

  Game GetGame() const { return m_profile.GetGame(); }
  int MixedProfileLength() const { return m_profile.MixedProfileLength(); }
  double operator[](int i) const { return m_profile[i]; }

private:
  // Construct a logit QRE with a given strategy profile and lambda value.
  // Access is restricted to classes in this module, which ensure that
  // objects so constructed are in fact QREs.
  LogitQREMixedStrategyProfile(const MixedStrategyProfile<double> &p_profile, double p_lambda,
                               double p_logLike)
    : m_profile(p_profile), m_lambda(p_lambda), m_logLike(p_logLike)
  {
  }

  const MixedStrategyProfile<double> m_profile;
  double m_lambda;
  double m_logLike;
};

class StrategicQREPathTracer : public PathTracer {
public:
  StrategicQREPathTracer() : m_fullGraph(true), m_decimals(6) {}
  ~StrategicQREPathTracer() override = default;

  List<LogitQREMixedStrategyProfile>
  TraceStrategicPath(const LogitQREMixedStrategyProfile &p_start, std::ostream &p_logStream,
                     double p_maxregret, double p_omega) const;
  LogitQREMixedStrategyProfile SolveAtLambda(const LogitQREMixedStrategyProfile &p_start,
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
  class LambdaCriterion;
  class CallbackFunction;
};

class StrategicQREEstimator : public StrategicQREPathTracer {
public:
  StrategicQREEstimator() = default;
  ~StrategicQREEstimator() override = default;

  LogitQREMixedStrategyProfile Estimate(const LogitQREMixedStrategyProfile &p_start,
                                        const MixedStrategyProfile<double> &p_frequencies,
                                        std::ostream &p_logStream, double p_maxLambda,
                                        double p_omega);

protected:
  class CriterionFunction;
  class CallbackFunction;
};

inline List<MixedStrategyProfile<double>> LogitStrategySolve(const Game &p_game, double p_regret,
                                                             double p_firstStep, double p_maxAccel)
{
  StrategicQREPathTracer tracer;
  tracer.SetMaxDecel(p_maxAccel);
  tracer.SetStepsize(p_firstStep);
  tracer.SetFullGraph(false);
  std::ostringstream ostream;
  auto result =
      tracer.TraceStrategicPath(LogitQREMixedStrategyProfile(p_game), ostream, p_regret, 1.0);
  auto ret = List<MixedStrategyProfile<double>>();
  if (!result.empty()) {
    ret.push_back(result.back().GetProfile());
  }
  return ret;
}

} // end namespace Gambit

#endif // NFGLOGIT_H
