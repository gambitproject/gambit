//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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
#include "logit.h"
#include "path.h"

namespace Gambit {

namespace {

MixedStrategyProfile<double> PointToProfile(const Game &p_game, const Vector<double> &p_point)
{
  MixedStrategyProfile<double> profile(p_game->NewMixedStrategyProfile(0.0));
  for (int i = 1; i < p_point.size(); i++) {
    profile[i] = exp(p_point[i]);
  }
  return profile;
}

Vector<double> ProfileToPoint(const LogitQREMixedStrategyProfile &p_profile)
{
  Vector<double> point(p_profile.size() + 1);
  for (size_t i = 1; i <= p_profile.size(); i++) {
    point[i] = log(p_profile[i]);
  }
  point.back() = p_profile.GetLambda();
  return point;
}

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

bool RegretTerminationFunction(const Game &p_game, const Vector<double> &p_point, double p_regret)
{
  if (p_point.back() < 0.0) {
    return true;
  }
  return PointToProfile(p_game, p_point).GetMaxRegret() < p_regret;
}

void GetValue(const Game &p_game, const Vector<double> &p_point, Vector<double> &p_lhs)
{
  MixedStrategyProfile<double> profile(PointToProfile(p_game, p_point)),
      logprofile(p_game->NewMixedStrategyProfile(0.0));
  for (size_t i = 1; i <= profile.MixedProfileLength(); i++) {
    logprofile[i] = p_point[i];
  }
  double lambda = p_point.back();
  p_lhs = 0.0;
  for (int rowno = 0, pl = 1; pl <= p_game->NumPlayers(); pl++) {
    GamePlayer player = p_game->GetPlayer(pl);
    for (size_t st = 1; st <= player->GetStrategies().size(); st++) {
      rowno++;
      if (st == 1) {
        // This is a sum-to-one equation
        p_lhs[rowno] = -1.0;
        for (size_t j = 1; j <= player->GetStrategies().size(); j++) {
          p_lhs[rowno] += profile[player->GetStrategy(j)];
        }
      }
      else {
        // This is a ratio equation
        p_lhs[rowno] = (logprofile[player->GetStrategy(st)] - logprofile[player->GetStrategy(1)] -
                        lambda * (profile.GetPayoff(player->GetStrategy(st)) -
                                  profile.GetPayoff(player->GetStrategy(1))));
      }
    }
  }
}

void GetJacobian(const Game &p_game, const Vector<double> &p_point, Matrix<double> &p_matrix)
{
  MixedStrategyProfile<double> profile(PointToProfile(p_game, p_point)),
      logprofile(p_game->NewMixedStrategyProfile(0.0));
  for (size_t i = 1; i <= profile.MixedProfileLength(); i++) {
    logprofile[i] = p_point[i];
  }
  double lambda = p_point.back();

  p_matrix = 0.0;

  for (int rowno = 0, i = 1; i <= p_game->NumPlayers(); i++) {
    GamePlayer player = p_game->GetPlayer(i);
    for (size_t j = 1; j <= player->GetStrategies().size(); j++) {
      rowno++;
      if (j == 1) {
        // This is a sum-to-one equation
        for (int colno = 0, ell = 1; ell <= p_game->NumPlayers(); ell++) {
          GamePlayer player2 = p_game->GetPlayer(ell);
          for (size_t m = 1; m <= player2->GetStrategies().size(); m++) {
            colno++;
            if (i == ell) {
              p_matrix(colno, rowno) = profile[player2->GetStrategy(m)];
            }
            // Otherwise, entry is zero
          }
        }
        // The last column is derivative wrt lambda, which is zero
      }
      else {
        // This is a ratio equation
        for (int colno = 0, ell = 1; ell <= p_game->NumPlayers(); ell++) {
          GamePlayer player2 = p_game->GetPlayer(ell);
          for (size_t m = 1; m <= player2->GetStrategies().size(); m++) {
            colno++;
            if (i == ell) {
              if (m == 1) {
                p_matrix(colno, rowno) = -1.0;
              }
              else if (m == j) {
                p_matrix(colno, rowno) = 1.0;
              }
              // Entry is zero for all other strategy pairs
            }
            else {
              p_matrix(colno, rowno) =
                  -lambda * profile[player2->GetStrategy(m)] *
                  (profile.GetPayoffDeriv(i, player->GetStrategy(j), player2->GetStrategy(m)) -
                   profile.GetPayoffDeriv(i, player->GetStrategy(1), player2->GetStrategy(m)));
            }
          }
        }
        // Fill the last column, the derivative wrt lambda
        p_matrix(p_matrix.NumRows(), rowno) = (profile.GetPayoff(player->GetStrategy(1)) -
                                               profile.GetPayoff(player->GetStrategy(j)));
      }
    }
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
  const List<LogitQREMixedStrategyProfile> &GetProfiles() const { return m_profiles; }

private:
  Game m_game;
  MixedStrategyObserverFunctionType m_observer;
  List<LogitQREMixedStrategyProfile> m_profiles;
};

void TracingCallbackFunction::AppendPoint(const Vector<double> &p_point)
{
  MixedStrategyProfile<double> profile(PointToProfile(m_game, p_point));
  m_profiles.push_back(LogitQREMixedStrategyProfile(profile, p_point.back(), 1.0));
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
                  LogLike(p_frequencies, static_cast<const Vector<double> &>(
                                             p_game->NewMixedStrategyProfile(0.0))))
{
}

void EstimatorCallbackFunction::EvaluatePoint(const Vector<double> &p_point)
{
  MixedStrategyProfile<double> profile(PointToProfile(m_game, p_point));
  auto qre = LogitQREMixedStrategyProfile(
      profile, p_point.back(),
      LogLike(m_frequencies, static_cast<const Vector<double> &>(profile)));
  m_observer(qre);
  if (qre.GetLogLike() > m_bestProfile.GetLogLike()) {
    m_bestProfile = qre;
  }
}

} // namespace

List<LogitQREMixedStrategyProfile> LogitStrategySolve(const LogitQREMixedStrategyProfile &p_start,
                                                      double p_regret, double p_omega,
                                                      double p_firstStep, double p_maxAccel,
                                                      MixedStrategyObserverFunctionType p_observer)
{
  PathTracer tracer;
  tracer.SetMaxDecel(p_maxAccel);
  tracer.SetStepsize(p_firstStep);

  double scale = p_start.GetGame()->GetMaxPayoff() - p_start.GetGame()->GetMinPayoff();
  if (scale != 0.0) {
    p_regret *= scale;
  }

  Vector<double> x(ProfileToPoint(p_start));
  TracingCallbackFunction callback(p_start.GetGame(), p_observer);
  tracer.TracePath(
      [&p_start](const Vector<double> &p_point, Vector<double> &p_lhs) {
        GetValue(p_start.GetGame(), p_point, p_lhs);
      },
      [&p_start](const Vector<double> &p_point, Matrix<double> &p_jac) {
        GetJacobian(p_start.GetGame(), p_point, p_jac);
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

  Vector<double> x(ProfileToPoint(p_start));
  TracingCallbackFunction callback(p_start.GetGame(), p_observer);
  std::list<LogitQREMixedStrategyProfile> ret;
  for (auto lam : p_targetLambda) {
    tracer.TracePath(
        [&p_start](const Vector<double> &p_point, Vector<double> &p_lhs) {
          GetValue(p_start.GetGame(), p_point, p_lhs);
        },
        [&p_start](const Vector<double> &p_point, Matrix<double> &p_jac) {
          GetJacobian(p_start.GetGame(), p_point, p_jac);
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

  Vector<double> x(ProfileToPoint(start)), restart(x);
  Vector<double> freq_vector(static_cast<const Vector<double> &>(p_frequencies));
  EstimatorCallbackFunction callback(
      start.GetGame(), static_cast<const Vector<double> &>(p_frequencies), p_observer);
  while (true) {
    tracer.TracePath(
        [&start](const Vector<double> &p_point, Vector<double> &p_lhs) {
          GetValue(start.GetGame(), p_point, p_lhs);
        },
        [&start](const Vector<double> &p_point, Matrix<double> &p_jac) {
          GetJacobian(start.GetGame(), p_point, p_jac);
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
