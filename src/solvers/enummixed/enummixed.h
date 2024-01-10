//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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

namespace Gambit {
namespace Nash {

template <class T> class EnumMixedStrategySolver;

///
/// This encapsulates the data output by a run of enumeration of
/// mixed strategies.
///
template <class T> class EnumMixedStrategySolution {
  friend class EnumMixedStrategySolver<T>;
public:
  explicit EnumMixedStrategySolution(const Game &p_game) : m_game(p_game) { }
  ~EnumMixedStrategySolution() = default;

  const Game &GetGame() const { return m_game; }
  const List<MixedStrategyProfile<T> > &GetExtremeEquilibria() const
  { return m_extremeEquilibria; }
  
  List<List<MixedStrategyProfile<T> > > GetCliques() const;

private:
  Game m_game;
  List<MixedStrategyProfile<T> > m_extremeEquilibria;

  /// Representation of the graph connecting the extreme equilibria
  ///@{
  List<Vector<T> > m_key1, m_key2;  
  List<int> m_node1, m_node2; // IDs of each component of the extreme equilibria
  int m_v1, m_v2;
  ///@}

  /// Representation of the connectedness of the extreme equilibria
  /// These are generated only on demand
  mutable List<Array<int> > m_cliques1, m_cliques2;
};


template <class T> class EnumMixedStrategySolver : public StrategySolver<T> {
public:
  explicit EnumMixedStrategySolver(std::shared_ptr<StrategyProfileRenderer<T> > p_onEquilibrium = 0)
    : StrategySolver<T>(p_onEquilibrium) {}
  virtual ~EnumMixedStrategySolver() = default;

  std::shared_ptr<EnumMixedStrategySolution<T> > SolveDetailed(const Game &p_game) const;
  List<MixedStrategyProfile<T> > Solve(const Game &p_game) const
  { return SolveDetailed(p_game)->GetExtremeEquilibria(); }
  
  
private:
  /// Implement fuzzy equality for floating-point version when testing Nashness
  static bool EqZero(const T &x);
};


inline List<MixedStrategyProfile<double> > EnumMixedStrategySolveDouble(const Game &p_game)
{
  return EnumMixedStrategySolver<double>().Solve(p_game);
}

inline List<MixedStrategyProfile<Rational> > EnumMixedStrategySolveRational(const Game &p_game)
{
  return EnumMixedStrategySolver<Rational>().Solve(p_game);
}


 
//
// Enumerate all mixed-strategy Nash equilibria of a two-player game
// using the lrslib backend.
//
class EnumMixedLrsStrategySolver : public StrategySolver<Rational> {
public:
  explicit EnumMixedLrsStrategySolver(std::shared_ptr<StrategyProfileRenderer<Rational> > p_onEquilibrium = nullptr)
    : StrategySolver<Rational>(p_onEquilibrium) { }
  ~EnumMixedLrsStrategySolver() override = default;

  List<MixedStrategyProfile<Rational> > Solve(const Game &p_game) const override;
};

inline List<MixedStrategyProfile<Rational> > EnumMixedStrategySolveLrs(const Game &p_game)
{
  return EnumMixedLrsStrategySolver().Solve(p_game);
}

}  // end namespace Gambit::Nash
}  // end namespace Gambit
    
#endif  // GAMBIT_NASH_ENUMMIXED_H
