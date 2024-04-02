//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: library/include/gambit/nash/simpdiv.h
// Compute Nash equilibria via simplicial subdivision on the normal form
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

#ifndef GAMBIT_NASH_SIMPDIV_H
#define GAMBIT_NASH_SIMPDIV_H

#include "games/nash.h"

namespace Gambit {
namespace Nash {

///
/// This is a simplicial subdivision algorithm with restart, for finding
/// mixed strategy solutions to general finite n-person games.  It is based on
/// van Der Laan, Talman and van Der Heyden, Math of Oper Res, 1987.
///
class NashSimpdivStrategySolver : public StrategySolver<Rational> {
public:
  explicit NashSimpdivStrategySolver(
      int p_gridResize = 2, int p_leashLength = 0,
      const Rational &p_maxregret = Rational(1, 1000000), bool p_verbose = false,
      std::shared_ptr<StrategyProfileRenderer<Rational>> p_onEquilibrium = nullptr)
    : StrategySolver<Rational>(p_onEquilibrium), m_gridResize(p_gridResize),
      m_leashLength((p_leashLength > 0) ? p_leashLength : 32000), m_maxregret(p_maxregret),
      m_verbose(p_verbose)
  {
  }
  ~NashSimpdivStrategySolver() override = default;

  List<MixedStrategyProfile<Rational>> Solve(const MixedStrategyProfile<Rational> &p_start) const;
  List<MixedStrategyProfile<Rational>> Solve(const Game &p_game) const override;

private:
  int m_gridResize, m_leashLength;
  Rational m_maxregret;
  bool m_verbose;

  class State;

  Rational Simplex(MixedStrategyProfile<Rational> &, const Rational &d) const;
  void update(State &, RectArray<int> &, RectArray<int> &, PVector<Rational> &,
              const PVector<int> &, int j, int i) const;
  void getY(State &, MixedStrategyProfile<Rational> &x, PVector<Rational> &, const PVector<int> &,
            const PVector<int> &, const PVector<Rational> &, const RectArray<int> &, int k) const;
  void getnexty(State &, MixedStrategyProfile<Rational> &x, const RectArray<int> &,
                const PVector<int> &, int i) const;
  int get_c(int j, int h, int nstrats, const PVector<int> &) const;
  int get_b(int j, int h, int nstrats, const PVector<int> &) const;
};

inline List<MixedStrategyProfile<Rational>>
SimpdivStrategySolve(const MixedStrategyProfile<Rational> &p_start,
                     const Rational &p_maxregret = Rational(1, 1000000), int p_gridResize = 2,
                     int p_leashLength = 0)
{
  return NashSimpdivStrategySolver(p_gridResize, p_leashLength, p_maxregret).Solve(p_start);
}

} // namespace Nash
} // end namespace Gambit

#endif // GAMBIT_NASH_SIMPDIV_H
