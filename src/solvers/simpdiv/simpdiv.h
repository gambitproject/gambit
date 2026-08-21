//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/simpdiv/simpdiv.h
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

#ifndef GAMBIT_SOLVERS_SIMPDIV_SIMPDIV_H
#define GAMBIT_SOLVERS_SIMPDIV_SIMPDIV_H

#include <variant>
#include "solvers/nash.h"

namespace Gambit::Nash {

struct SimpdivStartEvent {
  const MixedStrategyProfile<Rational> &profile;
};

struct SimpdivRefinementEvent {
  const MixedStrategyProfile<Rational> &profile;
  Rational gridSize;
  Rational regret;
};

using SimpdivEvent = std::variant<SimpdivStartEvent, SimpdivRefinementEvent>;
using SimpdivEventCallbackType = std::function<void(const SimpdivEvent &)>;

inline void NullSimpdivEventCallback(const SimpdivEvent &) {}

///
/// Returns the mixed strategy profile in which each player plays their
/// first strategy with probability one.
///
/// This is the default starting point for SimpdivStrategySolve() when no
/// other starting profile is specified.  It is a not-unreasonable default in
/// that it starts with a very coarse grid and, if the game has an
/// equilibrium in pure strategies, or in mixed strategies with small
/// denominators, it will find it quickly.  Starting with a strategy profile
/// with a smaller denominator can lead to a long initial search before
/// reaching a candidate neighborhood for an equilibrium.
///
MixedStrategyProfile<Rational> SimpdivDefaultStart(const Game &p_game);

///
/// This is a simplicial subdivision algorithm with restart, for finding
/// mixed strategy solutions to general finite n-person games.  It is based on
/// van Der Laan, Talman and van Der Heyden, Math of Oper Res, 1987.
///
std::list<MixedStrategyProfile<Rational>> SimpdivStrategySolve(
    const MixedStrategyProfile<Rational> &p_start,
    const Rational &p_maxregret = Rational(1, 10000000), int p_gridResize = 2,
    int p_leashLength = 0,
    StrategyCallbackType<Rational> p_onEquilibrium = NullStrategyCallback<Rational>,
    SimpdivEventCallbackType p_onEvent = NullSimpdivEventCallback,
    const CancelToken &p_cancel = CancelToken());

} // end namespace Gambit::Nash

#endif // GAMBIT_SOLVERS_SIMPDIV_SIMPDIV_H
