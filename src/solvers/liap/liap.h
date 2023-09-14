//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/liap/efgliap.h
// Compute Nash equilibria by minimizing Liapunov function on extensive game
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

#ifndef GAMBIT_NASH_LIAP_H
#define GAMBIT_NASH_LIAP_H

#include "games/nash.h"

using namespace Gambit;
using namespace Gambit::Nash;

class NashLiapBehaviorSolver : public BehavSolver<double> {
public:
  explicit NashLiapBehaviorSolver(int p_maxitsN, bool p_verbose = false,
                                  std::shared_ptr<StrategyProfileRenderer<double> > p_onEquilibrium = nullptr)
    : BehavSolver<double>(p_onEquilibrium),
      m_maxitsN(p_maxitsN), m_verbose(p_verbose)
  { }
  ~NashLiapBehaviorSolver() override = default;

  List<MixedBehaviorProfile<double> > Solve(const MixedBehaviorProfile<double> &p_start) const;
  List<MixedBehaviorProfile<double> > Solve(const BehaviorSupportProfile &p_support) const override
    { return Solve(MixedBehaviorProfile<double>(p_support)); }

private:
  int m_maxitsN;
  bool m_verbose;
};

inline List<MixedBehaviorProfile<double> > LiapBehaviorSolve(const Game &p_game, int p_maxitsN)
{
  return NashLiapBehaviorSolver(p_maxitsN).Solve(BehaviorSupportProfile(p_game));
}

class NashLiapStrategySolver : public StrategySolver<double> {
public:
  explicit NashLiapStrategySolver(int p_maxitsN, bool p_verbose = false,
                                  std::shared_ptr<StrategyProfileRenderer<double> > p_onEquilibrium = nullptr)
    : StrategySolver<double>(std::move(p_onEquilibrium)),
      m_maxitsN(p_maxitsN), m_verbose(p_verbose)
  { }
  ~NashLiapStrategySolver() override = default;

  List<MixedStrategyProfile<double> > Solve(const MixedStrategyProfile<double> &p_start) const;
  List<MixedStrategyProfile<double> > Solve(const Game &p_game) const override
  { return Solve(p_game->NewMixedStrategyProfile(0.0)); }

private:
  int m_maxitsN;
  bool m_verbose;
};

inline List<MixedStrategyProfile<double> > LiapStrategySolve(const Game &p_game, int p_maxitsN)
{
  return NashLiapStrategySolver(p_maxitsN).Solve(p_game);
}


#endif  // GAMBIT_NASH_LIAP_H
