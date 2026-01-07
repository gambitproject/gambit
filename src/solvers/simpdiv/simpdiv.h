//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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

namespace Gambit::Nash {

///
/// This is a simplicial subdivision algorithm with restart, for finding
/// mixed strategy solutions to general finite n-person games.  It is based on
/// van Der Laan, Talman and van Der Heyden, Math of Oper Res, 1987.
///
std::list<MixedStrategyProfile<Rational>> SimpdivStrategySolve(
    const MixedStrategyProfile<Rational> &p_start,
    const Rational &p_maxregret = Rational(1, 1000000), int p_gridResize = 2,
    int p_leashLength = 0,
    StrategyCallbackType<Rational> p_onEquilibrium = NullStrategyCallback<Rational>);

} // end namespace Gambit::Nash

#endif // GAMBIT_NASH_SIMPDIV_H
