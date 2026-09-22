//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (http://www.gambit-project.org)
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

#ifndef GAMBIT_SOLVERS_LOGIT_LOGIT_H
#define GAMBIT_SOLVERS_LOGIT_LOGIT_H

#include <functional>
#include <list>
#include <optional>
#include <variant>
#include <vector>

#include "solvers/nash.h"
#include "solvers/path/path.h"

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

template <class QRE> struct LogitPathEvent {
  const QRE &qre;
};

/// @brief The last-accepted point and the rejected point bracketing a detected bifurcation,
///        held by value since a LogitBifurcation may outlive the trace that produced it
template <class QRE> struct LogitBifurcation {
  QRE before;
  QRE after;
};

/// @brief Reports a bifurcation the moment it is detected, bracketed by the points in
///        \p LogitBifurcation
template <class QRE> struct LogitBifurcationEvent {
  const QRE &before;
  const QRE &after;
};

/// @brief Reports the tracer switching its symmetry-breaking perturbation on or off while
///        attempting to cross a suspected bifurcation
template <class QRE> struct LogitPerturbationEvent {
  bool active;
  const QRE &qre;
};

template <class QRE>
using LogitEvent =
    std::variant<LogitPathEvent<QRE>, LogitBifurcationEvent<QRE>, LogitPerturbationEvent<QRE>>;
template <class QRE> using LogitEventCallbackType = std::function<void(const LogitEvent<QRE> &)>;

template <class QRE> void NullLogitEventCallback(const LogitEvent<QRE> &) {}

/// @brief The points traced along a branch of the logit QRE correspondence, together with any
///        bifurcations detected along the way
template <class QRE> struct LogitTrace {
  std::list<QRE> profiles;
  std::vector<LogitBifurcation<QRE>> bifurcations;
};

/// @brief Why a call to LogitStrategySolveEquilibrium()/LogitBehaviorSolveEquilibrium() did
///        not return an accepted equilibrium
enum class LogitTerminationReason {
  Converged,              // the traced point satisfies the requested regret criterion
  RegretTargetNotReached, // tracing along the principal branch ended without reaching that
                          // criterion, whether from stalling near a bifurcation or simply running
                          // out of path
};

LogitTrace<LogitQREMixedStrategyProfile> LogitStrategySolve(
    const LogitQREMixedStrategyProfile &p_start, double p_regret,
    PathTracer::TraceDirection p_direction, double p_firstStep, double p_maxAccel,
    Nash::StrategyCallbackType<double> p_onEquilibrium = Nash::NullStrategyCallback<double>,
    LogitEventCallbackType<LogitQREMixedStrategyProfile> p_onEvent =
        NullLogitEventCallback<LogitQREMixedStrategyProfile>,
    const CancelToken &p_cancel = CancelToken());

/// @brief The result of tracing the logit QRE correspondence for a strategic game to an
///        accepted approximate Nash equilibrium
struct LogitStrategyResult {
  std::optional<MixedStrategyProfile<double>> equilibrium;
  bool success{false};
  LogitTerminationReason reason{LogitTerminationReason::RegretTargetNotReached};
  std::vector<LogitBifurcation<LogitQREMixedStrategyProfile>> bifurcations;
};

/// @brief Trace the principal branch of the logit QRE correspondence for a strategic game,
///        starting at lambda=0, to an approximate Nash equilibrium satisfying \p p_regret
inline LogitStrategyResult LogitStrategySolveEquilibrium(
    const LogitQREMixedStrategyProfile &p_start, double p_regret, double p_firstStep,
    double p_maxAccel,
    LogitEventCallbackType<LogitQREMixedStrategyProfile> p_onEvent =
        NullLogitEventCallback<LogitQREMixedStrategyProfile>,
    PathTracer::TraceDirection p_direction = PathTracer::TraceDirection::Positive,
    const CancelToken &p_cancel = CancelToken())
{
  const auto trace = LogitStrategySolve(p_start, p_regret, p_direction, p_firstStep, p_maxAccel,
                                        Nash::NullStrategyCallback<double>, p_onEvent, p_cancel);
  const MixedStrategyProfile<double> &candidate = trace.profiles.back().GetProfile();
  const double scale = p_start.GetGame()->GetMaxPayoff() - p_start.GetGame()->GetMinPayoff();
  const double regret = (scale != 0.0) ? p_regret * scale : p_regret;
  if (candidate.GetMaxRegret() > regret) {
    return {std::nullopt, false, LogitTerminationReason::RegretTargetNotReached,
            trace.bifurcations};
  }
  return {candidate, true, LogitTerminationReason::Converged, trace.bifurcations};
}

std::list<LogitQREMixedStrategyProfile> LogitStrategySolveLambda(
    const LogitQREMixedStrategyProfile &p_start, const std::list<double> &p_targetLambda,
    PathTracer::TraceDirection p_direction, double p_firstStep, double p_maxAccel,
    LogitEventCallbackType<LogitQREMixedStrategyProfile> p_onEvent =
        NullLogitEventCallback<LogitQREMixedStrategyProfile>);

LogitQREMixedStrategyProfile
LogitStrategyEstimate(const MixedStrategyProfile<double> &p_frequencies, double p_maxLambda,
                      PathTracer::TraceDirection p_direction, double p_stopAtLocal,
                      double p_firstStep, double p_maxAccel,
                      LogitEventCallbackType<LogitQREMixedStrategyProfile> p_onEvent =
                          NullLogitEventCallback<LogitQREMixedStrategyProfile>);

using LogitQREMixedBehaviorProfile = LogitQRE<MixedBehaviorProfile<double>>;

LogitTrace<LogitQREMixedBehaviorProfile> LogitBehaviorSolve(
    const LogitQREMixedBehaviorProfile &p_start, double p_regret,
    PathTracer::TraceDirection p_direction, double p_firstStep, double p_maxAccel,
    Nash::BehaviorCallbackType<double> p_onEquilibrium = Nash::NullBehaviorCallback<double>,
    LogitEventCallbackType<LogitQREMixedBehaviorProfile> p_onEvent =
        NullLogitEventCallback<LogitQREMixedBehaviorProfile>,
    const CancelToken &p_cancel = CancelToken());

/// @brief The result of tracing the logit QRE correspondence for an extensive game to an
///        accepted approximate Nash equilibrium
struct LogitBehaviorResult {
  std::optional<MixedBehaviorProfile<double>> equilibrium;
  bool success{false};
  LogitTerminationReason reason{LogitTerminationReason::RegretTargetNotReached};
  std::vector<LogitBifurcation<LogitQREMixedBehaviorProfile>> bifurcations;
};

/// @brief Trace the principal branch of the logit QRE correspondence for an extensive game,
///        starting at lambda=0, to an approximate Nash equilibrium satisfying \p p_regret
inline LogitBehaviorResult LogitBehaviorSolveEquilibrium(
    const LogitQREMixedBehaviorProfile &p_start, double p_regret, double p_firstStep,
    double p_maxAccel,
    LogitEventCallbackType<LogitQREMixedBehaviorProfile> p_onEvent =
        NullLogitEventCallback<LogitQREMixedBehaviorProfile>,
    PathTracer::TraceDirection p_direction = PathTracer::TraceDirection::Positive,
    const CancelToken &p_cancel = CancelToken())
{
  const auto trace = LogitBehaviorSolve(p_start, p_regret, p_direction, p_firstStep, p_maxAccel,
                                        Nash::NullBehaviorCallback<double>, p_onEvent, p_cancel);
  const MixedBehaviorProfile<double> &candidate = trace.profiles.back().GetProfile();
  const double scale = p_start.GetGame()->GetMaxPayoff() - p_start.GetGame()->GetMinPayoff();
  const double regret = (scale != 0.0) ? p_regret * scale : p_regret;
  if (candidate.GetAgentMaxRegret() > regret) {
    return {std::nullopt, false, LogitTerminationReason::RegretTargetNotReached,
            trace.bifurcations};
  }
  return {candidate, true, LogitTerminationReason::Converged, trace.bifurcations};
}

std::list<LogitQREMixedBehaviorProfile> LogitBehaviorSolveLambda(
    const LogitQREMixedBehaviorProfile &p_start, const std::list<double> &p_targetLambda,
    PathTracer::TraceDirection p_direction, double p_firstStep, double p_maxAccel,
    LogitEventCallbackType<LogitQREMixedBehaviorProfile> p_onEvent =
        NullLogitEventCallback<LogitQREMixedBehaviorProfile>);

LogitQREMixedBehaviorProfile
LogitBehaviorEstimate(const MixedBehaviorProfile<double> &p_frequencies, double p_maxLambda,
                      PathTracer::TraceDirection p_direction, double p_stopAtLocal,
                      double p_firstStep, double p_maxAccel,
                      LogitEventCallbackType<LogitQREMixedBehaviorProfile> p_onEvent =
                          NullLogitEventCallback<LogitQREMixedBehaviorProfile>);

} // namespace Gambit

#endif // GAMBIT_SOLVERS_LOGIT_LOGIT_H
