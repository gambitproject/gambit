//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/pygambit/nash.h
// Shims for Cython wrapper for computation of equilibrium
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

#include "gambit.h"
#include "solvers/logit/logit.h"

using namespace std;
using namespace Gambit;

List<MixedBehaviorProfile<double>> LogitBehaviorSolveWrapper(const Game &p_game, double p_regret,
                                                             double p_firstStep, double p_maxAccel)
{
  List<MixedBehaviorProfile<double>> ret;
  ret.push_back(LogitBehaviorSolve(LogitQREMixedBehaviorProfile(p_game), p_regret, 1.0,
                                   p_firstStep, p_maxAccel)
                    .back()
                    .GetProfile());
  return ret;
}

inline List<LogitQREMixedBehaviorProfile> LogitBehaviorPrincipalBranchWrapper(const Game &p_game,
                                                                              double p_regret,
                                                                              double p_firstStep,
                                                                              double p_maxAccel)
{
  return LogitBehaviorSolve(LogitQREMixedBehaviorProfile(p_game), p_regret, 1.0, p_firstStep,
                            p_maxAccel);
}

std::shared_ptr<LogitQREMixedBehaviorProfile>
LogitBehaviorEstimateWrapper(std::shared_ptr<MixedBehaviorProfile<double>> p_frequencies,
                             bool p_stopAtLocal, double p_firstStep, double p_maxAccel)
{
  return make_shared<LogitQREMixedBehaviorProfile>(LogitBehaviorEstimate(
      *p_frequencies, 1000000.0, 1.0, p_stopAtLocal, p_firstStep, p_maxAccel));
}

std::list<std::shared_ptr<LogitQREMixedBehaviorProfile>>
LogitBehaviorAtLambdaWrapper(const Game &p_game, const std::list<double> &p_targetLambda,
                             double p_firstStep, double p_maxAccel)
{
  LogitQREMixedBehaviorProfile start(p_game);
  std::list<std::shared_ptr<LogitQREMixedBehaviorProfile>> ret;
  for (auto &qre : LogitBehaviorSolveLambda(start, p_targetLambda, 1.0, p_firstStep, p_maxAccel)) {
    ret.push_back(std::make_shared<LogitQREMixedBehaviorProfile>(qre));
  }
  return ret;
}

List<MixedStrategyProfile<double>> LogitStrategySolveWrapper(const Game &p_game, double p_regret,
                                                             double p_firstStep, double p_maxAccel)
{
  List<MixedStrategyProfile<double>> ret;
  ret.push_back(LogitStrategySolve(LogitQREMixedStrategyProfile(p_game), p_regret, 1.0,
                                   p_firstStep, p_maxAccel)
                    .back()
                    .GetProfile());
  return ret;
}

inline List<LogitQREMixedStrategyProfile> LogitStrategyPrincipalBranchWrapper(const Game &p_game,
                                                                              double p_regret,
                                                                              double p_firstStep,
                                                                              double p_maxAccel)
{
  return LogitStrategySolve(LogitQREMixedStrategyProfile(p_game), p_regret, 1.0, p_firstStep,
                            p_maxAccel);
}

std::list<std::shared_ptr<LogitQREMixedStrategyProfile>>
LogitStrategyAtLambdaWrapper(const Game &p_game, const std::list<double> &p_targetLambda,
                             double p_firstStep, double p_maxAccel)
{
  LogitQREMixedStrategyProfile start(p_game);
  std::list<std::shared_ptr<LogitQREMixedStrategyProfile>> ret;
  for (auto &qre : LogitStrategySolveLambda(start, p_targetLambda, 1.0, p_firstStep, p_maxAccel)) {
    ret.push_back(std::make_shared<LogitQREMixedStrategyProfile>(qre));
  }
  return ret;
}

std::shared_ptr<LogitQREMixedStrategyProfile>
LogitStrategyEstimateWrapper(std::shared_ptr<MixedStrategyProfile<double>> p_frequencies,
                             bool p_stopAtLocal, double p_firstStep, double p_maxAccel)
{
  return make_shared<LogitQREMixedStrategyProfile>(LogitStrategyEstimate(
      *p_frequencies, 1000000.0, 1.0, p_stopAtLocal, p_firstStep, p_maxAccel));
}
