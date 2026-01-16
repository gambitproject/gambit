//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/logit/nfglogit.cc
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

#include <cmath>

#include "gambit.h"
#include "logbehav.h"
#include "logit.h"
#include "path.h"

namespace Gambit {

namespace {

MixedStrategyProfile<double> PointToProfile(const Game &p_game, const Vector<double> &p_point)
{
  MixedStrategyProfile<double> profile(p_game->NewMixedStrategyProfile(0.0));
  for (size_t i = 1; i < p_point.size(); i++) {
    profile[i] = std::exp(p_point[i]);
  }
  return profile;
}

Vector<double> ProfileToPoint(const LogitQREMixedStrategyProfile &p_profile)
{
  Vector<double> point(p_profile.size() + 1);
  for (size_t i = 1; i <= p_profile.size(); i++) {
    point[i] = std::log(p_profile[i]);
  }
  point.back() = p_profile.GetLambda();
  return point;
}

double LogLike(const Vector<double> &p_frequencies, const Vector<double> &p_point)
{
  return std::inner_product(p_frequencies.begin(), p_frequencies.end(), p_point.begin(), 0.0,
                            std::plus<>(),
                            [](double freq, double prob) { return freq * std::log(prob); });
}

double DiffLogLike(const Vector<double> &p_frequencies, const Vector<double> &p_tangent)
{
  return std::inner_product(p_frequencies.begin(), p_frequencies.end(), p_tangent.begin(), 0.0);
}

bool RegretTerminationFunction(const Game &p_game, const Vector<double> &p_point, double p_regret)
{
  if (p_point.back() < 0.0) {
    return true;
  }
  return PointToProfile(p_game, p_point).GetMaxRegret() < p_regret;
}

class Equation {
public:
  virtual ~Equation() = default;

  virtual double Value(const MixedStrategyProfile<double> &p_profile,
                       const MixedStrategyProfile<double> &p_logProfile,
                       double p_lambda) const = 0;
  virtual void Gradient(const MixedStrategyProfile<double> &p_profile,
                        const MixedStrategyProfile<double> &p_logProfile, double p_lambda,
                        Vector<double> &p_gradient) const = 0;
};

class EquationSystem {
public:
  explicit EquationSystem(const Game &p_game);
  ~EquationSystem() = default;

  void GetValue(const Vector<double> &p_point, Vector<double> &p_lhs) const;
  void GetJacobian(const Vector<double> &p_point, Matrix<double> &p_jac) const;

private:
  std::vector<std::shared_ptr<Equation>> m_equations;
  const Game &m_game;
};

class SumToOneEquation final : public Equation {
  Game m_game;
  GamePlayer m_player;

public:
  explicit SumToOneEquation(const GamePlayer &p_player)
    : m_game(p_player->GetGame()), m_player(p_player)
  {
  }

  ~SumToOneEquation() override = default;
  double Value(const MixedStrategyProfile<double> &p_profile,
               const MixedStrategyProfile<double> &p_logProfile, double p_lambda) const override;
  void Gradient(const MixedStrategyProfile<double> &p_profile,
                const MixedStrategyProfile<double> &p_logProfile, double p_lambda,
                Vector<double> &p_gradient) const override;
};

double SumToOneEquation::Value(const MixedStrategyProfile<double> &p_profile,
                               const MixedStrategyProfile<double> &p_logProfile,
                               double p_lambda) const
{
  double value = -1.0;
  for (const auto &strategy : m_player->GetStrategies()) {
    value += p_profile[strategy];
  }
  return value;
}

void SumToOneEquation::Gradient(const MixedStrategyProfile<double> &p_profile,
                                const MixedStrategyProfile<double> &p_logProfile, double p_lambda,
                                Vector<double> &p_gradient) const
{
  int col = 1;
  for (const auto &player : m_game->GetPlayers()) {
    for (const auto &strategy : player->GetStrategies()) {
      p_gradient[col++] = (player == m_player) ? p_profile[strategy] : 0.0;
    }
  }
  // Derivative wrt lambda is zero
  p_gradient[col] = 0.0;
}

class RatioEquation final : public Equation {
  Game m_game;
  GamePlayer m_player;
  GameStrategy m_strategy, m_refStrategy;

public:
  RatioEquation(const GameStrategy &p_strategy, const GameStrategy &p_refStrategy)
    : m_game(p_strategy->GetGame()), m_player(p_strategy->GetPlayer()), m_strategy(p_strategy),
      m_refStrategy(p_refStrategy)
  {
  }

  ~RatioEquation() override = default;
  double Value(const MixedStrategyProfile<double> &p_profile,
               const MixedStrategyProfile<double> &p_logProfile, double p_lambda) const override;
  void Gradient(const MixedStrategyProfile<double> &p_profile,
                const MixedStrategyProfile<double> &p_logProfile, double p_lambda,
                Vector<double> &p_gradient) const override;
};

double RatioEquation::Value(const MixedStrategyProfile<double> &p_profile,
                            const MixedStrategyProfile<double> &p_logProfile,
                            double p_lambda) const
{
  return (p_logProfile[m_strategy] - p_logProfile[m_refStrategy] -
          p_lambda * (p_profile.GetPayoff(m_strategy) - p_profile.GetPayoff(m_refStrategy)));
}

void RatioEquation::Gradient(const MixedStrategyProfile<double> &p_profile,
                             const MixedStrategyProfile<double> &p_logProfile, double p_lambda,
                             Vector<double> &p_gradient) const
{
  int col = 1;
  for (const auto &player : m_game->GetPlayers()) {
    for (const auto &strategy : player->GetStrategies()) {
      if (strategy == m_refStrategy) {
        p_gradient[col] = -1.0;
      }
      else if (strategy == m_strategy) {
        p_gradient[col] = 1.0;
      }
      else if (player == m_player) {
        p_gradient[col] = 0.0;
      }
      else {
        p_gradient[col] =
            -p_lambda * p_profile[strategy] *
            (p_profile.GetPayoffDeriv(m_player->GetNumber(), m_strategy, strategy) -
             p_profile.GetPayoffDeriv(m_player->GetNumber(), m_refStrategy, strategy));
      }
      col++;
    }
  }
  p_gradient[col] = (p_profile.GetPayoff(m_refStrategy) - p_profile.GetPayoff(m_strategy));
}

EquationSystem::EquationSystem(const Game &p_game) : m_game(p_game)
{
  for (const auto &player : m_game->GetPlayers()) {
    m_equations.push_back(std::make_shared<SumToOneEquation>(player));
    auto strategies = player->GetStrategies();
    for (auto strategy = std::next(strategies.begin()); strategy != strategies.end(); ++strategy) {
      m_equations.push_back(std::make_shared<RatioEquation>(*strategy, strategies.front()));
    }
  }
}

void EquationSystem::GetValue(const Vector<double> &p_point, Vector<double> &p_lhs) const
{
  MixedStrategyProfile<double> profile(PointToProfile(m_game, p_point)),
      logprofile(m_game->NewMixedStrategyProfile(0.0));
  for (size_t i = 1; i <= profile.MixedProfileLength(); i++) {
    logprofile[i] = p_point[i];
  }
  const double lambda = p_point.back();
  std::transform(
      m_equations.begin(), m_equations.end(), p_lhs.begin(),
      [&profile, &logprofile, lambda](auto e) { return e->Value(profile, logprofile, lambda); });
}

void EquationSystem::GetJacobian(const Vector<double> &p_point, Matrix<double> &p_matrix) const
{
  MixedStrategyProfile<double> profile(PointToProfile(m_game, p_point)),
      logprofile(m_game->NewMixedStrategyProfile(0.0));
  for (size_t i = 1; i <= profile.MixedProfileLength(); i++) {
    logprofile[i] = p_point[i];
  }
  const double lambda = p_point.back();
  Vector<double> column(p_point.size());
  for (size_t i = 1; i <= m_equations.size(); i++) {
    m_equations[i - 1]->Gradient(profile, logprofile, lambda, column);
    p_matrix.SetColumn(i, column);
  }
}

class TracingCallbackFunction {
public:
  TracingCallbackFunction(const Game &p_game, MixedStrategyObserverFunctionType p_observer)
    : m_game(p_game), m_observer(p_observer)
  {
  }
  ~TracingCallbackFunction() = default;

  void AppendPoint(const Vector<double> &p_point);
  const std::list<LogitQREMixedStrategyProfile> &GetProfiles() const { return m_profiles; }

private:
  Game m_game;
  MixedStrategyObserverFunctionType m_observer;
  std::list<LogitQREMixedStrategyProfile> m_profiles;
};

void TracingCallbackFunction::AppendPoint(const Vector<double> &p_point)
{
  const MixedStrategyProfile<double> profile(PointToProfile(m_game, p_point));
  m_profiles.emplace_back(profile, p_point.back(), 1.0);
  m_observer(m_profiles.back());
}

class EstimatorCallbackFunction {
public:
  EstimatorCallbackFunction(const Game &p_game, const Vector<double> &p_frequencies,
                            MixedStrategyObserverFunctionType p_observer);
  ~EstimatorCallbackFunction() = default;

  void EvaluatePoint(const Vector<double> &p_point);

  const LogitQREMixedStrategyProfile &GetMaximizer() const { return m_bestProfile; }

private:
  Game m_game;
  const Vector<double> &m_frequencies;
  MixedStrategyObserverFunctionType m_observer;
  LogitQREMixedStrategyProfile m_bestProfile;
};

EstimatorCallbackFunction::EstimatorCallbackFunction(const Game &p_game,
                                                     const Vector<double> &p_frequencies,
                                                     MixedStrategyObserverFunctionType p_observer)
  : m_game(p_game), m_frequencies(p_frequencies), m_observer(p_observer),
    m_bestProfile(p_game->NewMixedStrategyProfile(0.0), 0.0,
                  LogLike(p_frequencies, p_game->NewMixedStrategyProfile(0.0).GetProbVector()))
{
}

void EstimatorCallbackFunction::EvaluatePoint(const Vector<double> &p_point)
{
  const MixedStrategyProfile<double> profile(PointToProfile(m_game, p_point));
  auto qre = LogitQREMixedStrategyProfile(profile, p_point.back(),
                                          LogLike(m_frequencies, profile.GetProbVector()));
  m_observer(qre);
  if (qre.GetLogLike() > m_bestProfile.GetLogLike()) {
    m_bestProfile = qre;
  }
}

} // namespace

std::list<LogitQREMixedStrategyProfile>
LogitStrategySolve(const LogitQREMixedStrategyProfile &p_start, double p_regret, double p_omega,
                   double p_firstStep, double p_maxAccel,
                   MixedStrategyObserverFunctionType p_observer)
{
  PathTracer tracer;
  tracer.SetMaxDecel(p_maxAccel);
  tracer.SetStepsize(p_firstStep);

  const double scale = p_start.GetGame()->GetMaxPayoff() - p_start.GetGame()->GetMinPayoff();
  if (scale != 0.0) {
    p_regret *= scale;
  }

  const Game game = p_start.GetGame();
  Vector<double> x(ProfileToPoint(p_start));
  TracingCallbackFunction callback(game, p_observer);
  EquationSystem system(game);
  tracer.TracePath(
      [&system](const Vector<double> &p_point, Vector<double> &p_lhs) {
        system.GetValue(p_point, p_lhs);
      },
      [&system](const Vector<double> &p_point, Matrix<double> &p_jac) {
        system.GetJacobian(p_point, p_jac);
      },
      x, p_omega,
      [p_start, p_regret](const Vector<double> &p_point) {
        return RegretTerminationFunction(p_start.GetGame(), p_point, p_regret);
      },
      [&callback](const Vector<double> &p_point) -> void { callback.AppendPoint(p_point); });
  return callback.GetProfiles();
}

std::list<LogitQREMixedStrategyProfile>
LogitStrategySolveLambda(const LogitQREMixedStrategyProfile &p_start,
                         const std::list<double> &p_targetLambda, double p_omega,
                         double p_firstStep, double p_maxAccel,
                         MixedStrategyObserverFunctionType p_observer)
{
  PathTracer tracer;
  tracer.SetMaxDecel(p_maxAccel);
  tracer.SetStepsize(p_firstStep);

  const Game game = p_start.GetGame();
  Vector<double> x(ProfileToPoint(p_start));
  TracingCallbackFunction callback(game, p_observer);
  std::list<LogitQREMixedStrategyProfile> ret;
  EquationSystem system(game);
  for (auto lam : p_targetLambda) {
    tracer.TracePath(
        [&system](const Vector<double> &p_point, Vector<double> &p_lhs) {
          system.GetValue(p_point, p_lhs);
        },
        [&system](const Vector<double> &p_point, Matrix<double> &p_jac) {
          system.GetJacobian(p_point, p_jac);
        },
        x, p_omega, LambdaPositiveTerminationFunction,
        [&callback](const Vector<double> &p_point) -> void { callback.AppendPoint(p_point); },
        [lam](const Vector<double> &x, const Vector<double> &) -> double {
          return x.back() - lam;
        });
    ret.push_back(callback.GetProfiles().back());
  }
  return ret;
}

LogitQREMixedStrategyProfile
LogitStrategyEstimate(const MixedStrategyProfile<double> &p_frequencies, double p_maxLambda,
                      double p_omega, double p_stopAtLocal, double p_firstStep, double p_maxAccel,
                      MixedStrategyObserverFunctionType p_observer)
{
  LogitQREMixedStrategyProfile start(p_frequencies.GetGame());
  PathTracer tracer;
  tracer.SetMaxDecel(p_maxAccel);
  tracer.SetStepsize(p_firstStep);

  const Game game;
  Vector<double> x(ProfileToPoint(start)), restart(x);
  const Vector<double> freq_vector(p_frequencies.GetProbVector());
  EstimatorCallbackFunction callback(start.GetGame(), p_frequencies.GetProbVector(), p_observer);
  EquationSystem system(game);
  while (true) {
    tracer.TracePath(
        [&system](const Vector<double> &p_point, Vector<double> &p_lhs) {
          system.GetValue(p_point, p_lhs);
        },
        [&system](const Vector<double> &p_point, Matrix<double> &p_jac) {
          system.GetJacobian(p_point, p_jac);
        },
        x, p_omega,
        [p_maxLambda](const Vector<double> &p_point) {
          return LambdaRangeTerminationFunction(p_point, 0, p_maxLambda);
        },
        [&callback](const Vector<double> &p_point) -> void { callback.EvaluatePoint(p_point); },
        [freq_vector](const Vector<double> &, const Vector<double> &p_tangent) -> double {
          return DiffLogLike(freq_vector, p_tangent);
        },
        [&restart](const Vector<double> &, const Vector<double> &p_restart) -> void {
          restart = p_restart;
        });
    if (p_stopAtLocal || x.back() >= p_maxLambda) {
      break;
    }
    x = restart;
  }
  return callback.GetMaximizer();
}

} // end namespace Gambit
