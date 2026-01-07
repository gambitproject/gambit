//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: library/include/gambit/nash/enummixed.h
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

#ifndef GAMBIT_NASH_ENUMMIXED_H
#define GAMBIT_NASH_ENUMMIXED_H

#include "games/nash.h"

namespace Gambit::Nash {

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

  Array<Array<MixedStrategyProfile<T>>> GetCliques() const;

  Game m_game;
  std::list<MixedStrategyProfile<T>> m_extremeEquilibria;

  /// Representation of the graph connecting the extreme equilibria
  ///@{
  Array<Vector<T>> m_key1, m_key2;
  Array<int> m_node1, m_node2; // IDs of each component of the extreme equilibria
  int m_v1{0}, m_v2{0};
  ///@}

  /// Representation of the connectedness of the extreme equilibria
  /// These are generated only on demand
  mutable Array<Array<int>> m_cliques1, m_cliques2;
};

template <class T>
std::shared_ptr<EnumMixedStrategySolution<T>>
EnumMixedStrategySolveDetailed(const Game &p_game,
                               StrategyCallbackType<T> p_onEquilibrium = NullStrategyCallback<T>);

template <class T>
std::list<MixedStrategyProfile<T>>
EnumMixedStrategySolve(const Game &p_game,
                       StrategyCallbackType<T> p_onEquilibrium = NullStrategyCallback<T>)
{
  return EnumMixedStrategySolveDetailed<T>(p_game, p_onEquilibrium)->m_extremeEquilibria;
}

} // end namespace Gambit::Nash

#endif // GAMBIT_NASH_ENUMMIXED_H
