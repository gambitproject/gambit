//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/logit/nfglogit.cc
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

#include <algorithm>
#include <cmath>
#include <utility>

#include "games.h"
#include "logit.h"
#include "logitdriver.h"

namespace Gambit {

namespace {

using LogitDriver::PointToProfile;

void PointToLogProfile(MixedStrategyProfile<double> &p_profile, const Vector<double> &p_point)
{
  for (size_t i = 1; i < p_point.size(); i++) {
    p_profile[i] = p_point[i];
  }
}

class Equation {
public:
  virtual ~Equation() = default;

  virtual double Value(const MixedStrategyProfile<double> &p_profile,
                       const MixedStrategyProfile<double> &p_logProfile,
                       const Vector<double> &p_strategyValues, double p_lambda) const = 0;
};

class SumToOneEquation final : public Equation {
  Game m_game;
  GamePlayer m_player;
  int m_firstIndex{0}, m_lastIndex{0};

public:
  explicit SumToOneEquation(const GamePlayer &p_player)
    : m_game(p_player->GetGame()), m_player(p_player)
  {
    int col = 1;
    for (const auto &player : m_game->GetPlayers()) {
      if (player != m_player) {
        col += player->GetStrategies().size();
        continue;
      }
      m_firstIndex = col;
      m_lastIndex = col + player->GetStrategies().size();
      return;
    }
  }

  ~SumToOneEquation() override = default;
  double Value(const MixedStrategyProfile<double> &p_profile,
               const MixedStrategyProfile<double> &p_logProfile,
               const Vector<double> &p_strategyValues, double p_lambda) const override;
};

double SumToOneEquation::Value(const MixedStrategyProfile<double> &p_profile,
                               const MixedStrategyProfile<double> & /*p_logProfile*/,
                               const Vector<double> & /*p_strategyValues*/,
                               double /*p_lambda*/) const
{
  double value = -1.0;
  for (int col = m_firstIndex; col < m_lastIndex; col++) {
    value += p_profile[col];
  }
  return value;
}

class RatioEquation final : public Equation {
  Game m_game;
  GamePlayer m_player;
  GameStrategy m_strategy, m_refStrategy;
  int m_strategyIndex{0}, m_refStrategyIndex{0};

public:
  RatioEquation(const GameStrategy &p_strategy, const GameStrategy &p_refStrategy)
    : m_game(p_strategy->GetGame()), m_player(p_strategy->GetPlayer()), m_strategy(p_strategy),
      m_refStrategy(p_refStrategy)
  {
    int col = 1;
    for (const auto &strategy : m_game->GetStrategies()) {
      if (strategy == p_strategy) {
        m_strategyIndex = col;
      }
      else if (strategy == p_refStrategy) {
        m_refStrategyIndex = col;
      }
      col++;
    }
  }

  ~RatioEquation() override = default;
  double Value(const MixedStrategyProfile<double> &p_profile,
               const MixedStrategyProfile<double> &p_logProfile,
               const Vector<double> &p_strategyValues, double p_lambda) const override;
};

double RatioEquation::Value(const MixedStrategyProfile<double> & /*p_profile*/,
                            const MixedStrategyProfile<double> &p_logProfile,
                            const Vector<double> &p_strategyValues, double p_lambda) const
{
  return p_logProfile[m_strategyIndex] - p_logProfile[m_refStrategyIndex] -
         p_lambda * (p_strategyValues[m_strategyIndex] - p_strategyValues[m_refStrategyIndex]);
}

class EquationSystem {
public:
  explicit EquationSystem(const Game &p_game);
  ~EquationSystem() = default;

  void GetValue(const Vector<double> &p_point, Vector<double> &p_lhs) const;
  void GetJacobian(const Vector<double> &p_point, Matrix<double> &p_jac) const;

private:
  std::vector<std::shared_ptr<Equation>> m_equations;
  Game m_game;
  mutable MixedStrategyProfile<double> m_profile, m_logProfile;
  mutable Vector<double> m_strategyValues;
  // Payoffs of one player's strategies and their derivatives, reused across players
  mutable Vector<double> m_blockValues;
  mutable Matrix<double> m_blockDerivs;
};

EquationSystem::EquationSystem(const Game &p_game)
  : m_game(p_game), m_profile(p_game->NewMixedStrategyProfile(0.0)),
    m_logProfile(p_game->NewMixedStrategyProfile(0.0)),
    m_strategyValues(m_profile.MixedProfileLength())
{
  m_equations.reserve(m_profile.MixedProfileLength());
  for (const auto &player : m_game->GetPlayers()) {
    m_equations.push_back(std::make_shared<SumToOneEquation>(player));
    const auto strategies = player->GetStrategies();
    for (auto strategy = std::next(strategies.begin()); strategy != strategies.end(); ++strategy) {
      m_equations.push_back(std::make_shared<RatioEquation>(*strategy, strategies.front()));
    }
  }
}

void EquationSystem::GetValue(const Vector<double> &p_point, Vector<double> &p_lhs) const
{
  PointToProfile(m_profile, p_point);
  PointToLogProfile(m_logProfile, p_point);
  const double lambda = p_point.back();
  auto value = m_strategyValues.begin();
  for (const auto &player : m_game->GetPlayers()) {
    m_profile.GetStrategyValues(player, m_blockValues);
    value = std::ranges::copy(m_blockValues, value).out;
  }
  std::ranges::transform(m_equations, p_lhs.begin(), [this, lambda](const auto &e) {
    return e->Value(m_profile, m_logProfile, m_strategyValues, lambda);
  });
}

void EquationSystem::GetJacobian(const Vector<double> &p_point, Matrix<double> &p_jac) const
{
  // Rows of p_jac are variables (log probabilities, then lambda); columns are equations: for
  // each player, that their probabilities sum to one, then one for each strategy but the first
  // relating its log probability to that of the first.
  PointToProfile(m_profile, p_point);
  const double lambda = p_point.back();
  const size_t length = p_point.size() - 1;
  p_jac = 0.0;
  size_t column = 1;
  size_t first = 1; // position in the profile of the current player's first strategy
  for (const auto &player : m_game->GetPlayers()) {
    m_profile.GetPayoffDerivBlock(player, m_blockValues, m_blockDerivs);
    const size_t last = first + m_blockValues.size() - 1;
    for (size_t var = first; var <= last; var++) {
      p_jac(var, column) = m_profile[var];
    }
    column++;
    for (size_t r = 2; r <= m_blockValues.size(); r++, column++) {
      for (size_t var = 1; var <= length; var++) {
        if (var < first || var > last) {
          p_jac(var, column) =
              -lambda * m_profile[var] * (m_blockDerivs(r, var) - m_blockDerivs(1, var));
        }
      }
      p_jac(first + r - 1, column) = 1.0;
      p_jac(first, column) = -1.0;
      p_jac(length + 1, column) = m_blockValues[1] - m_blockValues[r];
    }
    first = last + 1;
  }
}

struct StrategyRepresentation {
  using Profile = MixedStrategyProfile<double>;
  using System = EquationSystem;

  static Profile NewProfile(const Game &p_game) { return p_game->NewMixedStrategyProfile(0.0); }
  static double MaxRegret(const Profile &p_profile) { return p_profile.GetMaxRegret(); }
  static const Vector<double> &Probabilities(const Profile &p_profile)
  {
    return p_profile.GetProbVector();
  }
};

} // namespace

LogitTrace<LogitQREMixedStrategyProfile>
LogitStrategySolve(const LogitQREMixedStrategyProfile &p_start, double p_regret,
                   PathTracer::TraceDirection p_direction, double p_firstStep, double p_maxAccel,
                   const Nash::StrategyCallbackType<double> &p_onEquilibrium,
                   LogitEventCallbackType<LogitQREMixedStrategyProfile> p_onEvent,
                   const CancelToken &p_cancel)
{
  return LogitDriver::Solve<StrategyRepresentation>(p_start, p_regret, p_direction, p_firstStep,
                                                    p_maxAccel, p_onEquilibrium,
                                                    std::move(p_onEvent), p_cancel);
}

LogitLambdaResult<LogitQREMixedStrategyProfile> LogitStrategySolveLambda(
    const LogitQREMixedStrategyProfile &p_start, const std::list<double> &p_targetLambda,
    PathTracer::TraceDirection p_direction, double p_firstStep, double p_maxAccel,
    LogitEventCallbackType<LogitQREMixedStrategyProfile> p_onEvent, const CancelToken &p_cancel)
{
  return LogitDriver::SolveLambda<StrategyRepresentation>(p_start, p_targetLambda, p_direction,
                                                          p_firstStep, p_maxAccel,
                                                          std::move(p_onEvent), p_cancel);
}

LogitQREMixedStrategyProfile
LogitStrategyEstimate(const MixedStrategyProfile<double> &p_frequencies, double p_maxLambda,
                      PathTracer::TraceDirection p_direction, double p_stopAtLocal,
                      double p_firstStep, double p_maxAccel,
                      LogitEventCallbackType<LogitQREMixedStrategyProfile> p_onEvent,
                      const CancelToken &p_cancel)
{
  return LogitDriver::Estimate<StrategyRepresentation>(p_frequencies, p_maxLambda, p_direction,
                                                       p_stopAtLocal, p_firstStep, p_maxAccel,
                                                       std::move(p_onEvent), p_cancel);
}

} // end namespace Gambit
