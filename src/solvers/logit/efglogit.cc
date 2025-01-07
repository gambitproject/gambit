//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/logit/efglogit.cc
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

#include <cmath>

#include "gambit.h"
#include "logit.h"
#include "logbehav.imp"
#include "path.h"

namespace {

double LogLike(const Vector<double> &p_frequencies, const Vector<double> &p_point)
{
  double logL = 0.0;
  for (int i = 1; i <= p_frequencies.size(); i++) {
    logL += p_frequencies[i] * log(p_point[i]);
  }
  return logL;
}

double DiffLogLike(const Vector<double> &p_frequencies, const Vector<double> &p_tangent)
{
  double diff_logL = 0.0;
  for (int i = 1; i <= p_frequencies.size(); i++) {
    diff_logL += p_frequencies[i] * p_tangent[i];
  }
  return diff_logL;
}

MixedBehaviorProfile<double> PointToProfile(const Game &p_game, const Vector<double> &p_point)
{
  MixedBehaviorProfile<double> profile(p_game);
  for (int i = 1; i < p_point.size(); i++) {
    profile[i] = exp(p_point[i]);
  }
  return profile;
}

LogBehavProfile<double> PointToLogProfile(const Game &p_game, const Vector<double> &p_point)
{
  LogBehavProfile<double> profile(p_game);
  for (size_t i = 1; i <= profile.BehaviorProfileLength(); i++) {
    profile.SetLogProb(i, p_point[i]);
  }
  return profile;
}

Vector<double> ProfileToPoint(const LogitQREMixedBehaviorProfile &p_profile)
{
  Vector<double> point(p_profile.size() + 1);
  for (size_t i = 1; i <= p_profile.size(); i++) {
    point[i] = log(p_profile[i]);
  }
  point.back() = p_profile.GetLambda();
  return point;
}

bool RegretTerminationFunction(const Game &p_game, const Vector<double> &p_point, double p_regret)
{
  return (p_point.back() < 0.0 || PointToProfile(p_game, p_point).GetMaxRegret() < p_regret);
}

class EquationSystem {
public:
  explicit EquationSystem(const Game &p_game);
  ~EquationSystem() = default;

  // Compute the value of the system of equations at the specified point.
  void GetValue(const Vector<double> &p_point, Vector<double> &p_lhs) const;

  // Compute the Jacobian matrix at the specified point.
  void GetJacobian(const Vector<double> &p_point, Matrix<double> &p_matrix) const;

private:
  //
  // This abstract base class represents one of the defining equations of the system.
  //
  class Equation {
  public:
    virtual ~Equation() = default;

    virtual double Value(const LogBehavProfile<double> &p_point, double p_lambda) const = 0;

    virtual void Gradient(const LogBehavProfile<double> &p_point, double p_lambda,
                          Vector<double> &p_gradient) const = 0;
  };

  //
  // This class represents the equation that the probabilities of actions
  // in information set (pl,iset) sums to one
  //
  class SumToOneEquation : public Equation {
  private:
    Game m_game;
    GameInfoset m_infoset;

  public:
    SumToOneEquation(const Game &p_game, const GameInfoset &p_infoset)
      : m_game(p_game), m_infoset(p_infoset)
    {
    }

    double Value(const LogBehavProfile<double> &p_profile, double p_lambda) const override;

    void Gradient(const LogBehavProfile<double> &p_profile, double p_lambda,
                  Vector<double> &p_gradient) const override;
  };

  //
  // This class represents the equation relating the probability of
  // playing the action to the probability of playing the reference action.
  //
  class RatioEquation : public Equation {
  private:
    Game m_game;
    GameInfoset m_infoset;
    GameAction m_action, m_refAction;

  public:
    RatioEquation(const Game &p_game, const GameAction &p_action, const GameAction &p_refAction)
      : m_game(p_game), m_infoset(p_action->GetInfoset()), m_action(p_action),
        m_refAction(p_refAction)
    {
    }

    double Value(const LogBehavProfile<double> &p_profile, double p_lambda) const override;

    void Gradient(const LogBehavProfile<double> &p_profile, double p_lambda,
                  Vector<double> &p_gradient) const override;
  };

  Array<std::shared_ptr<Equation>> m_equations;
  const Game &m_game;
};

EquationSystem::EquationSystem(const Game &p_game) : m_game(p_game)
{
  for (const auto &player : m_game->GetPlayers()) {
    for (const auto &infoset : player->GetInfosets()) {
      m_equations.push_back(std::make_shared<SumToOneEquation>(m_game, infoset));
      const auto &actions = infoset->GetActions();
      for (auto action = std::next(actions.begin()); action != actions.end(); ++action) {
        m_equations.push_back(std::make_shared<RatioEquation>(m_game, *action, actions.front()));
      }
    }
  }
}

double EquationSystem::SumToOneEquation::Value(const LogBehavProfile<double> &p_profile,
                                               double p_lambda) const
{
  double value = -1.0;
  for (const auto &action : m_infoset->GetActions()) {
    value += p_profile.GetProb(action);
  }
  return value;
}

void EquationSystem::SumToOneEquation::Gradient(const LogBehavProfile<double> &p_profile,
                                                double p_lambda, Vector<double> &p_gradient) const
{
  int i = 1;
  for (const auto &player : m_game->GetPlayers()) {
    for (const auto &infoset : player->GetInfosets()) {
      for (const auto &action : infoset->GetActions()) {
        p_gradient[i++] = (infoset == m_infoset) ? p_profile.GetProb(action) : 0.0;
      }
    }
  }
  // Derivative wrt lambda is zero
  p_gradient[i] = 0.0;
}

double EquationSystem::RatioEquation::Value(const LogBehavProfile<double> &p_profile,
                                            double p_lambda) const
{
  return (p_profile.GetLogProb(m_action) - p_profile.GetLogProb(m_refAction) -
          p_lambda * (p_profile.GetPayoff(m_action) - p_profile.GetPayoff(m_refAction)));
}

void EquationSystem::RatioEquation::Gradient(const LogBehavProfile<double> &p_profile,
                                             double p_lambda, Vector<double> &p_gradient) const
{
  int i = 1;
  for (const auto &player : m_game->GetPlayers()) {
    for (const auto &infoset : player->GetInfosets()) {
      for (const auto &action : infoset->GetActions()) {
        if (action == m_refAction) {
          p_gradient[i] = -1.0;
        }
        else if (action == m_action) {
          p_gradient[i] = 1.0;
        }
        else if (infoset == m_infoset) {
          p_gradient[i] = 0.0;
        }
        else {
          p_gradient[i] = -p_lambda * (p_profile.DiffActionValue(m_action, action) -
                                       p_profile.DiffActionValue(m_refAction, action));
        }
        i++;
      }
    }
  }

  p_gradient[i] = (p_profile.GetPayoff(m_refAction) - p_profile.GetPayoff(m_action));
}

void EquationSystem::GetValue(const Vector<double> &p_point, Vector<double> &p_lhs) const
{
  LogBehavProfile<double> profile(PointToLogProfile(m_game, p_point));
  double lambda = p_point.back();
  for (int i = 1; i <= p_lhs.size(); i++) {
    p_lhs[i] = m_equations[i]->Value(profile, lambda);
  }
}

void EquationSystem::GetJacobian(const Vector<double> &p_point, Matrix<double> &p_matrix) const
{
  LogBehavProfile<double> profile(PointToLogProfile(m_game, p_point));
  double lambda = p_point.back();

  for (int i = 1; i <= m_equations.size(); i++) {
    Vector<double> column(p_point.size());
    m_equations[i]->Gradient(profile, lambda, column);
    p_matrix.SetColumn(i, column);
  }
}

class TracingCallbackFunction {
public:
  TracingCallbackFunction(const Game &p_game, MixedBehaviorObserverFunctionType p_observer)
    : m_game(p_game), m_observer(p_observer)
  {
  }
  ~TracingCallbackFunction() = default;

  void AppendPoint(const Vector<double> &p_point);
  const List<LogitQREMixedBehaviorProfile> &GetProfiles() const { return m_profiles; }

private:
  Game m_game;
  MixedBehaviorObserverFunctionType m_observer;
  List<LogitQREMixedBehaviorProfile> m_profiles;
};

void TracingCallbackFunction::AppendPoint(const Vector<double> &p_point)
{
  MixedBehaviorProfile<double> profile(PointToProfile(m_game, p_point));
  m_profiles.push_back(LogitQREMixedBehaviorProfile(profile, p_point.back(), 1.0));
  m_observer(m_profiles.back());
}

class EstimatorCallbackFunction {
public:
  EstimatorCallbackFunction(const Game &p_game, const Vector<double> &p_frequencies,
                            MixedBehaviorObserverFunctionType p_observer);
  ~EstimatorCallbackFunction() = default;

  void EvaluatePoint(const Vector<double> &p_point);
  const LogitQREMixedBehaviorProfile &GetMaximizer() const { return m_bestProfile; }

private:
  Game m_game;
  const Vector<double> &m_frequencies;
  MixedBehaviorObserverFunctionType m_observer;
  LogitQREMixedBehaviorProfile m_bestProfile;
};

EstimatorCallbackFunction::EstimatorCallbackFunction(const Game &p_game,
                                                     const Vector<double> &p_frequencies,
                                                     MixedBehaviorObserverFunctionType p_observer)
  : m_game(p_game), m_frequencies(p_frequencies), m_observer(p_observer),
    m_bestProfile(MixedBehaviorProfile<double>(p_game), 0.0,
                  LogLike(p_frequencies, static_cast<const Vector<double> &>(
                                             MixedBehaviorProfile<double>(p_game))))
{
}

void EstimatorCallbackFunction::EvaluatePoint(const Vector<double> &p_point)
{
  MixedBehaviorProfile<double> profile(PointToProfile(m_game, p_point));
  auto qre = LogitQREMixedBehaviorProfile(
      profile, p_point.back(),
      LogLike(m_frequencies, static_cast<const Vector<double> &>(profile)));
  m_observer(qre);
  if (qre.GetLogLike() > m_bestProfile.GetLogLike()) {
    m_bestProfile = qre;
  }
}

} // namespace

namespace Gambit {

List<LogitQREMixedBehaviorProfile> LogitBehaviorSolve(const LogitQREMixedBehaviorProfile &p_start,
                                                      double p_regret, double p_omega,
                                                      double p_firstStep, double p_maxAccel,
                                                      MixedBehaviorObserverFunctionType p_observer)
{
  PathTracer tracer;
  tracer.SetMaxDecel(p_maxAccel);
  tracer.SetStepsize(p_firstStep);

  double scale = p_start.GetGame()->GetMaxPayoff() - p_start.GetGame()->GetMinPayoff();
  if (scale != 0.0) {
    p_regret *= scale;
  }

  Game game = p_start.GetGame();
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
      [game, p_regret](const Vector<double> &p_point) {
        return RegretTerminationFunction(game, p_point, p_regret);
      },
      [&callback](const Vector<double> &p_point) -> void { callback.AppendPoint(p_point); });
  return callback.GetProfiles();
}

std::list<LogitQREMixedBehaviorProfile>
LogitBehaviorSolveLambda(const LogitQREMixedBehaviorProfile &p_start,
                         const std::list<double> &p_targetLambda, double p_omega,
                         double p_firstStep, double p_maxAccel,
                         MixedBehaviorObserverFunctionType p_observer)
{
  PathTracer tracer;
  tracer.SetMaxDecel(p_maxAccel);
  tracer.SetStepsize(p_firstStep);

  Game game = p_start.GetGame();
  Vector<double> x(ProfileToPoint(p_start));
  TracingCallbackFunction callback(game, p_observer);
  EquationSystem system(game);
  std::list<LogitQREMixedBehaviorProfile> ret;
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

LogitQREMixedBehaviorProfile
LogitBehaviorEstimate(const MixedBehaviorProfile<double> &p_frequencies, double p_maxLambda,
                      double p_omega, double p_stopAtLocal, double p_firstStep, double p_maxAccel,
                      MixedBehaviorObserverFunctionType p_observer)
{
  LogitQREMixedBehaviorProfile start(p_frequencies.GetGame());
  PathTracer tracer;
  tracer.SetMaxDecel(p_maxAccel);
  tracer.SetStepsize(p_firstStep);

  Vector<double> x(ProfileToPoint(start)), restart(x);
  Vector<double> freq_vector(static_cast<const Vector<double> &>(p_frequencies));
  EstimatorCallbackFunction callback(
      start.GetGame(), static_cast<const Vector<double> &>(p_frequencies), p_observer);
  EquationSystem system(start.GetGame());
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
