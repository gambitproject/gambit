//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: library/include/gambit/nash/lcp.h
// Compute Nash equilibria via linear complementarity programming
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

#ifndef GAMBIT_NASH_LCP_H
#define GAMBIT_NASH_LCP_H

#include "games/nash.h"

namespace Gambit {

namespace linalg {
template <class T> class LHTableau;
template <class T> class LemkeTableau;
} // namespace linalg

namespace Nash {

template <class T> class NashLcpStrategySolver : public StrategySolver<T> {
public:
  NashLcpStrategySolver(int p_stopAfter, int p_maxDepth,
                        std::shared_ptr<StrategyProfileRenderer<T>> p_onEquilibrium = nullptr)
    : StrategySolver<T>(p_onEquilibrium), m_stopAfter(p_stopAfter), m_maxDepth(p_maxDepth)
  {
  }
  ~NashLcpStrategySolver() override = default;

  List<MixedStrategyProfile<T>> Solve(const Game &) const override;

private:
  int m_stopAfter, m_maxDepth;

  class Solution;

  bool OnBFS(const Game &, linalg::LHTableau<T> &, Solution &) const;
  void AllLemke(const Game &, int j, linalg::LHTableau<T> &, Solution &, int) const;
};

inline List<MixedStrategyProfile<double>> LcpStrategySolveDouble(const Game &p_game,
                                                                 int p_stopAfter, int p_maxDepth)
{
  return NashLcpStrategySolver<double>(p_stopAfter, p_maxDepth).Solve(p_game);
}

inline List<MixedStrategyProfile<Rational>>
LcpStrategySolveRational(const Game &p_game, int p_stopAfter, int p_maxDepth)
{
  return NashLcpStrategySolver<Rational>(p_stopAfter, p_maxDepth).Solve(p_game);
}

template <class T> class NashLcpBehaviorSolver : public BehavSolver<T> {
public:
  NashLcpBehaviorSolver(int p_stopAfter, int p_maxDepth,
                        std::shared_ptr<StrategyProfileRenderer<T>> p_onEquilibrium = nullptr)
    : BehavSolver<T>(p_onEquilibrium), m_stopAfter(p_stopAfter), m_maxDepth(p_maxDepth)
  {
  }
  ~NashLcpBehaviorSolver() override = default;

  List<MixedBehaviorProfile<T>> Solve(const Game &) const override;

private:
  int m_stopAfter, m_maxDepth;

  class Solution;

  void FillTableau(Matrix<T> &, const GameNode &, T, int, int, Solution &) const;
  void AllLemke(const Game &, int dup, linalg::LemkeTableau<T> &B, int depth, Matrix<T> &,
                Solution &) const;
  void GetProfile(const linalg::LemkeTableau<T> &tab, MixedBehaviorProfile<T> &, const Vector<T> &,
                  const GameNode &n, int, int, Solution &) const;
};

inline List<MixedBehaviorProfile<double>> LcpBehaviorSolveDouble(const Game &p_game,
                                                                 int p_stopAfter, int p_maxDepth)
{
  return NashLcpBehaviorSolver<double>(p_stopAfter, p_maxDepth).Solve(p_game);
}

inline List<MixedBehaviorProfile<Rational>>
LcpBehaviorSolveRational(const Game &p_game, int p_stopAfter, int p_maxDepth)
{
  return NashLcpBehaviorSolver<Rational>(p_stopAfter, p_maxDepth).Solve(p_game);
}

} // end namespace Nash
} // end namespace Gambit

#endif // GAMBIT_NASH_LCP_H
