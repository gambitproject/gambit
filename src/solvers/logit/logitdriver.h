//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/logit/logitdriver.h
// Tracing of the logit quantal response equilibrium correspondence, written once for
// all representations of profiles
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

#ifndef GAMBIT_SOLVERS_LOGIT_LOGITDRIVER_H
#define GAMBIT_SOLVERS_LOGIT_LOGITDRIVER_H

#include <cmath>
#include <concepts>
#include <functional>
#include <list>
#include <numeric>
#include <utility>
#include <vector>

#include "games.h"
#include "logit.h"
#include "solvers/path/path.h"

namespace Gambit::LogitDriver {

/// @brief What a representation of profiles supplies to the logit drivers.
///
/// A point on the path holds the logarithm of each probability of the profile, in the
/// profile's own order, followed by lambda.
template <class R>
concept Representation =
    requires(const Game &p_game, const typename R::Profile &p_profile) {
      { R::NewProfile(p_game) } -> std::same_as<typename R::Profile>;
      { R::MaxRegret(p_profile) } -> std::convertible_to<double>;
      { R::Probabilities(p_profile) } -> std::convertible_to<const Vector<double> &>;
    } &&
    requires(const typename R::System &p_system, const Vector<double> &p_point,
             Vector<double> &p_lhs, Matrix<double> &p_jac) {
      p_system.GetValue(p_point, p_lhs);
      p_system.GetJacobian(p_point, p_jac);
    } &&
    std::constructible_from<typename R::System, const Game &>;

template <class R> using QRE = LogitQRE<typename R::Profile>;

/// @brief Set the probabilities of \p p_profile from the point \p p_point
template <class Profile> void PointToProfile(Profile &p_profile, const Vector<double> &p_point)
{
  for (size_t i = 1; i < p_point.size(); i++) {
    p_profile[i] = std::exp(p_point[i]);
  }
}

template <Representation R> Vector<double> ProfileToPoint(const QRE<R> &p_qre)
{
  Vector<double> point(p_qre.size() + 1);
  for (size_t i = 1; i <= p_qre.size(); i++) {
    point[i] = std::log(p_qre[i]);
  }
  point.back() = p_qre.GetLambda();
  return point;
}

inline double LogLike(const Vector<double> &p_frequencies, const Vector<double> &p_probs)
{
  return std::inner_product(
      p_frequencies.begin(), p_frequencies.end(), p_probs.begin(), 0.0, std::plus<>(),
      [](const double freq, const double prob) { return freq * std::log(prob); });
}

inline double DiffLogLike(const Vector<double> &p_frequencies, const Vector<double> &p_tangent)
{
  return std::inner_product(p_frequencies.begin(), p_frequencies.end(), p_tangent.begin(), 0.0);
}

template <Representation R>
bool RegretTerminationFunction(const Game &p_game, const Vector<double> &p_point, double p_regret)
{
  if (p_point.back() < 0.0) {
    return true;
  }
  auto profile = R::NewProfile(p_game);
  PointToProfile(profile, p_point);
  return R::MaxRegret(profile) < p_regret;
}

/// @brief Records each point traced as a QRE and reports it as an event
template <Representation R> class TracingCallbackFunction {
public:
  TracingCallbackFunction(const Game &p_game, LogitEventCallbackType<QRE<R>> p_onEvent)
    : m_game(p_game), m_onEvent(std::move(p_onEvent))
  {
  }

  void AppendPoint(const Vector<double> &p_point)
  {
    m_profiles.push_back(PointToQRE(p_point));
    m_onEvent(LogitPathEvent<QRE<R>>{m_profiles.back()});
  }
  void OnBifurcation(const Vector<double> &p_before, const Vector<double> &p_after)
  {
    m_bifurcations.push_back({.before = PointToQRE(p_before), .after = PointToQRE(p_after)});
    const auto &bifurcation = m_bifurcations.back();
    m_onEvent(
        LogitBifurcationEvent<QRE<R>>{.before = bifurcation.before, .after = bifurcation.after});
  }
  void OnPerturbation(bool p_active, const Vector<double> &p_point)
  {
    const auto qre = PointToQRE(p_point);
    m_onEvent(LogitPerturbationEvent<QRE<R>>{.active = p_active, .qre = qre});
  }

  const std::list<QRE<R>> &GetProfiles() const { return m_profiles; }
  const std::vector<LogitBifurcation<QRE<R>>> &GetBifurcations() const { return m_bifurcations; }

private:
  QRE<R> PointToQRE(const Vector<double> &p_point) const
  {
    auto profile = R::NewProfile(m_game);
    PointToProfile(profile, p_point);
    return {profile, p_point.back(), 1.0};
  }

  Game m_game;
  LogitEventCallbackType<QRE<R>> m_onEvent;
  std::list<QRE<R>> m_profiles;
  std::vector<LogitBifurcation<QRE<R>>> m_bifurcations;
};

/// @brief Reports each point traced as an event, keeping the one that best fits the
///        frequencies
template <Representation R> class EstimatorCallbackFunction {
public:
  EstimatorCallbackFunction(const Game &p_game, const Vector<double> &p_frequencies,
                            LogitEventCallbackType<QRE<R>> p_onEvent)
    : m_game(p_game), m_frequencies(p_frequencies), m_onEvent(std::move(p_onEvent)),
      m_bestProfile(Centroid(p_game, p_frequencies))
  {
  }

  void EvaluatePoint(const Vector<double> &p_point)
  {
    const auto qre = PointToQRE(p_point);
    m_onEvent(LogitPathEvent<QRE<R>>{qre});
    if (qre.GetLogLike() > m_bestProfile.GetLogLike()) {
      m_bestProfile = qre;
    }
  }
  void OnBifurcation(const Vector<double> &p_before, const Vector<double> &p_after)
  {
    const auto before = PointToQRE(p_before);
    const auto after = PointToQRE(p_after);
    m_onEvent(LogitBifurcationEvent<QRE<R>>{.before = before, .after = after});
  }
  void OnPerturbation(bool p_active, const Vector<double> &p_point)
  {
    const auto qre = PointToQRE(p_point);
    m_onEvent(LogitPerturbationEvent<QRE<R>>{.active = p_active, .qre = qre});
  }

  const QRE<R> &GetMaximizer() const { return m_bestProfile; }

private:
  static QRE<R> Centroid(const Game &p_game, const Vector<double> &p_frequencies)
  {
    const auto profile = R::NewProfile(p_game);
    return {profile, 0.0, LogLike(p_frequencies, R::Probabilities(profile))};
  }
  QRE<R> PointToQRE(const Vector<double> &p_point) const
  {
    auto profile = R::NewProfile(m_game);
    PointToProfile(profile, p_point);
    return {profile, p_point.back(), LogLike(m_frequencies, R::Probabilities(profile))};
  }

  Game m_game;
  const Vector<double> &m_frequencies;
  LogitEventCallbackType<QRE<R>> m_onEvent;
  QRE<R> m_bestProfile;
};

inline PathTracer MakeTracer(double p_firstStep, double p_maxAccel)
{
  PathTracer tracer;
  tracer.SetMaxDecel(p_maxAccel);
  tracer.SetStepsize(p_firstStep);
  return tracer;
}

/// @brief Trace the branch from \p p_start until it reaches a profile with regret below
///        \p p_regret, scaled by the game's payoff range
template <Representation R>
LogitTrace<QRE<R>> Solve(const QRE<R> &p_start, double p_regret,
                         PathTracer::TraceDirection p_direction, double p_firstStep,
                         double p_maxAccel,
                         const std::function<void(const typename R::Profile &)> &p_onEquilibrium,
                         LogitEventCallbackType<QRE<R>> p_onEvent, const CancelToken &p_cancel)
{
  Nash::RequirePerfectRecall(p_start.GetGame());
  if (p_start.size() == 0) {
    return {.profiles = {p_start}, .bifurcations = {}};
  }
  const PathTracer tracer = MakeTracer(p_firstStep, p_maxAccel);
  const Game game = p_start.GetGame();
  if (const auto scale = static_cast<double>(game->GetMaxPayoff() - game->GetMinPayoff());
      scale != 0.0) {
    p_regret *= scale;
  }

  Vector<double> x(ProfileToPoint<R>(p_start));
  TracingCallbackFunction<R> callback(game, std::move(p_onEvent));
  const typename R::System system(game);
  const auto result = tracer.TracePath(
      [&system](const Vector<double> &p_point, Vector<double> &p_lhs) {
        system.GetValue(p_point, p_lhs);
      },
      [&system](const Vector<double> &p_point, Matrix<double> &p_jac) {
        system.GetJacobian(p_point, p_jac);
      },
      x, p_direction, x.size(),
      {.terminate =
           [game, p_regret](const Vector<double> &p_point) {
             return RegretTerminationFunction<R>(game, p_point, p_regret);
           },
       .callback = [&callback](const Vector<double> &p_point) { callback.AppendPoint(p_point); },
       .onBifurcation =
           [&callback](const Vector<double> &p_before, const Vector<double> &p_after) {
             callback.OnBifurcation(p_before, p_after);
           },
       .onPerturbation =
           [&callback](bool p_active, const Vector<double> &p_point) {
             callback.OnPerturbation(p_active, p_point);
           },
       .cancel = p_cancel});
  const auto &profiles = callback.GetProfiles();
  if (R::MaxRegret(profiles.back().GetProfile()) < p_regret) {
    p_onEquilibrium(profiles.back().GetProfile());
  }
  return {
      .profiles = profiles, .bifurcations = callback.GetBifurcations(), .stats = {result.stats}};
}

/// @brief Trace the branch from \p p_start to each of the values of lambda in turn
template <Representation R>
LogitLambdaResult<QRE<R>>
SolveLambda(const QRE<R> &p_start, const std::list<double> &p_targetLambda,
            PathTracer::TraceDirection p_direction, double p_firstStep, double p_maxAccel,
            LogitEventCallbackType<QRE<R>> p_onEvent, const CancelToken &p_cancel)
{
  Nash::RequirePerfectRecall(p_start.GetGame());
  if (p_start.size() == 0) {
    return {.profiles = {p_start}, .stats = {}};
  }
  const PathTracer tracer = MakeTracer(p_firstStep, p_maxAccel);
  const Game game = p_start.GetGame();
  Vector<double> x(ProfileToPoint<R>(p_start));
  TracingCallbackFunction<R> callback(game, std::move(p_onEvent));
  const typename R::System system(game);
  LogitLambdaResult<QRE<R>> ret;
  for (const auto &lam : p_targetLambda) {
    const auto result = tracer.TracePath(
        [&system](const Vector<double> &p_point, Vector<double> &p_lhs) {
          system.GetValue(p_point, p_lhs);
        },
        [&system](const Vector<double> &p_point, Matrix<double> &p_jac) {
          system.GetJacobian(p_point, p_jac);
        },
        x, p_direction, x.size(),
        {.terminate = LambdaPositiveTerminationFunction,
         .callback = [&callback](const Vector<double> &p_point) { callback.AppendPoint(p_point); },
         .criterion = [lam](const Vector<double> &p_point, const Vector<double> &) -> double {
           return p_point.back() - lam;
         },
         .onBifurcation =
             [&callback](const Vector<double> &p_before, const Vector<double> &p_after) {
               callback.OnBifurcation(p_before, p_after);
             },
         .onPerturbation =
             [&callback](bool p_active, const Vector<double> &p_point) {
               callback.OnPerturbation(p_active, p_point);
             },
         .cancel = p_cancel});
    ret.profiles.push_back(callback.GetProfiles().back());
    ret.stats.push_back(result.stats);
  }
  return ret;
}

/// @brief Trace the principal branch for the QRE that maximises the likelihood of
///        \p p_frequencies
template <Representation R>
QRE<R> Estimate(const typename R::Profile &p_frequencies, double p_maxLambda,
                PathTracer::TraceDirection p_direction, double p_stopAtLocal, double p_firstStep,
                double p_maxAccel, LogitEventCallbackType<QRE<R>> p_onEvent,
                const CancelToken &p_cancel)
{
  Nash::RequirePerfectRecall(p_frequencies.GetGame());
  QRE<R> start(p_frequencies.GetGame());
  if (start.size() == 0) {
    return start;
  }
  const PathTracer tracer = MakeTracer(p_firstStep, p_maxAccel);
  const Game game = start.GetGame();
  Vector<double> x(ProfileToPoint<R>(start)), restart(x);
  const Vector<double> frequencies(R::Probabilities(p_frequencies));
  EstimatorCallbackFunction<R> callback(game, frequencies, std::move(p_onEvent));
  const typename R::System system(game);
  while (true) {
    p_cancel.Check();
    tracer.TracePath(
        [&system](const Vector<double> &p_point, Vector<double> &p_lhs) {
          system.GetValue(p_point, p_lhs);
        },
        [&system](const Vector<double> &p_point, Matrix<double> &p_jac) {
          system.GetJacobian(p_point, p_jac);
        },
        x, p_direction, x.size(),
        {.terminate =
             [p_maxLambda](const Vector<double> &p_point) {
               return LambdaRangeTerminationFunction(p_point, 0, p_maxLambda);
             },
         .callback =
             [&callback](const Vector<double> &p_point) { callback.EvaluatePoint(p_point); },
         .criterion = [&frequencies](const Vector<double> &, const Vector<double> &p_tangent)
             -> double { return DiffLogLike(frequencies, p_tangent); },
         .criterionBracket = [&restart](const Vector<double> &,
                                        const Vector<double> &p_restart) { restart = p_restart; },
         .onBifurcation =
             [&callback](const Vector<double> &p_before, const Vector<double> &p_after) {
               callback.OnBifurcation(p_before, p_after);
             },
         .onPerturbation =
             [&callback](bool p_active, const Vector<double> &p_point) {
               callback.OnPerturbation(p_active, p_point);
             },
         .cancel = p_cancel});
    if (p_stopAtLocal || x.back() >= p_maxLambda) {
      break;
    }
    x = restart;
  }
  return callback.GetMaximizer();
}

} // namespace Gambit::LogitDriver

#endif // GAMBIT_SOLVERS_LOGIT_LOGITDRIVER_H
