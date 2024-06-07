//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/solvers/logit/logit.h
// Computation of quantal response equilibrium correspondence
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

#ifndef SOLVERS_LOGIT_H
#define SOLVERS_LOGIT_H

#include <functional>

namespace Gambit {

template <class T> class LogitQRE {
public:
  explicit LogitQRE(const Game &p_game);
  LogitQRE(const T &p_profile, double p_lambda, double p_logLike = 1.0)
    : m_profile(p_profile), m_lambda(p_lambda), m_logLike(p_logLike)
  {
  }
  LogitQRE(const LogitQRE &p_qre) = default;
  ~LogitQRE() = default;

  double GetLambda() const { return m_lambda; }

  const T &GetProfile() const { return m_profile; }

  double GetLogLike() const { return m_logLike; }

  Game GetGame() const { return m_profile.GetGame(); }

  size_t size() const;

  double operator[](int i) const { return m_profile[i]; }

private:
  T m_profile;
  double m_lambda;
  double m_logLike;
};

template <>
inline LogitQRE<MixedStrategyProfile<double>>::LogitQRE(const Game &p_game)
  : m_profile(p_game->NewMixedStrategyProfile(0.0)), m_lambda(0.0), m_logLike(1.0)
{
}

template <> inline size_t LogitQRE<MixedStrategyProfile<double>>::size() const
{
  return m_profile.MixedProfileLength();
}

template <>
inline LogitQRE<MixedBehaviorProfile<double>>::LogitQRE(const Game &p_game)
  : m_profile(MixedBehaviorProfile<double>(p_game)), m_lambda(0.0), m_logLike(1.0)
{
}

template <> inline size_t LogitQRE<MixedBehaviorProfile<double>>::size() const
{
  return m_profile.BehaviorProfileLength();
}

using LogitQREMixedStrategyProfile = LogitQRE<MixedStrategyProfile<double>>;

using MixedStrategyObserverFunctionType =
    std::function<void(const LogitQREMixedStrategyProfile &)>;

inline void NullMixedStrategyObserver(const LogitQREMixedStrategyProfile &) {}

List<LogitQREMixedStrategyProfile>
LogitStrategySolve(const LogitQREMixedStrategyProfile &p_start, double p_regret, double p_omega,
                   double p_firstStep, double p_maxAccel,
                   MixedStrategyObserverFunctionType p_observer = NullMixedStrategyObserver);

std::list<LogitQREMixedStrategyProfile>
LogitStrategySolveLambda(const LogitQREMixedStrategyProfile &p_start,
                         const std::list<double> &p_targetLambda, double p_omega,
                         double p_firstStep, double p_maxAccel,
                         MixedStrategyObserverFunctionType p_observer = NullMixedStrategyObserver);

LogitQREMixedStrategyProfile
LogitStrategyEstimate(const MixedStrategyProfile<double> &p_frequencies, double p_maxLambda,
                      double p_omega, double p_stopAtLocal, double p_firstStep, double p_maxAccel,
                      MixedStrategyObserverFunctionType p_observer = NullMixedStrategyObserver);

using LogitQREMixedBehaviorProfile = LogitQRE<MixedBehaviorProfile<double>>;

using MixedBehaviorObserverFunctionType =
    std::function<void(const LogitQREMixedBehaviorProfile &)>;

inline void NullMixedBehaviorObserver(const LogitQREMixedBehaviorProfile &) {}

List<LogitQREMixedBehaviorProfile>
LogitBehaviorSolve(const LogitQREMixedBehaviorProfile &p_start, double p_regret, double p_omega,
                   double p_firstStep, double p_maxAccel,
                   MixedBehaviorObserverFunctionType p_observer = NullMixedBehaviorObserver);

std::list<LogitQREMixedBehaviorProfile>
LogitBehaviorSolveLambda(const LogitQREMixedBehaviorProfile &p_start,
                         const std::list<double> &p_targetLambda, double p_omega,
                         double p_firstStep, double p_maxAccel,
                         MixedBehaviorObserverFunctionType p_observer = NullMixedBehaviorObserver);

LogitQREMixedBehaviorProfile
LogitBehaviorEstimate(const MixedBehaviorProfile<double> &p_frequencies, double p_maxLambda,
                      double p_omega, double p_stopAtLocal, double p_firstStep, double p_maxAccel,
                      MixedBehaviorObserverFunctionType p_observer = NullMixedBehaviorObserver);

} // namespace Gambit

#endif // SOLVERS_LOGIT_H
