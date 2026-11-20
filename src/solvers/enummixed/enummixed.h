//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/enummixed/enummixed.h
// Enumerate all mixed strategy equilibria of two-player games
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

#ifndef GAMBIT_SOLVERS_ENUMMIXED_ENUMMIXED_H
#define GAMBIT_SOLVERS_ENUMMIXED_ENUMMIXED_H

#include <utility>
#include <vector>

#include "solvers/nash.h"

namespace Gambit::Nash {

struct Biclique;

template <class T> class EnumMixedStrategySolver;

///
/// This encapsulates the data output by a run of enumeration of
/// mixed strategies.
///
template <class T> class EnumMixedStrategySolution {
public:
  explicit EnumMixedStrategySolution(const Game &p_game) : m_game(p_game) {}
  ~EnumMixedStrategySolution() = default;

  const Game &GetGame() const { return m_game; }
  const std::list<MixedStrategyProfile<T>> &GetExtremeEquilibria() const
  {
    return m_extremeEquilibria;
  }

  Array<Array<Array<MixedStrategyProfile<T>>>> GetComponents() const;

  Game m_game;
  std::list<MixedStrategyProfile<T>> m_extremeEquilibria;
  bool success{true};

  /// Representation of the graph connecting the extreme equilibria: the mixed strategy of each
  /// node, and one edge (player 1's node, player 2's node) for each extreme equilibrium.
  /// Nodes are numbered from 1, and index the keys.
  ///@{
  Array<Vector<T>> m_key1, m_key2;
  std::vector<std::pair<int, int>> m_edges;
  ///@}

private:
  /// Builds the profiles of the vertices of one maximal biclique of the graph.
  Array<MixedStrategyProfile<T>> BuildCliqueProfiles(const Biclique &p_biclique) const;
};

template <class T>
std::shared_ptr<EnumMixedStrategySolution<T>> EnumMixedStrategySolveDetailed(
    const Game &p_game, const StrategyCallbackType<T> &p_onEquilibrium = NullStrategyCallback<T>,
    const CancelToken &p_cancel = CancelToken());

/// @brief The result of enumerating all extreme mixed-strategy Nash equilibria of a
///        two-player game
template <class T> struct EnumMixedStrategyResult {
  std::list<MixedStrategyProfile<T>> equilibria;
  bool success{true};
};

template <class T>
EnumMixedStrategyResult<T>
EnumMixedStrategySolve(const Game &p_game,
                       const StrategyCallbackType<T> &p_onEquilibrium = NullStrategyCallback<T>,
                       const CancelToken &p_cancel = CancelToken())
{
  const auto solution = EnumMixedStrategySolveDetailed<T>(p_game, p_onEquilibrium, p_cancel);
  return {solution->m_extremeEquilibria, solution->success};
}

} // end namespace Gambit::Nash

#endif // GAMBIT_SOLVERS_ENUMMIXED_ENUMMIXED_H
