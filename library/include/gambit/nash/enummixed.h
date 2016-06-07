//
// This file is part of Gambit
// Copyright (c) 1994-2016, The Gambit Project (http://www.gambit-project.org)
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

#include "gambit/nash.h"

namespace Gambit {
namespace Nash {

//
// Enumerate all mixed-strategy Nash equilibria of a two-player game
// using the lrslib backend.
//
class EnumMixedLrsStrategySolver : public StrategySolver<Rational> {
public:
  EnumMixedLrsStrategySolver(shared_ptr<StrategyProfileRenderer<Rational> > p_onEquilibrium = 0)
    : StrategySolver<Rational>(p_onEquilibrium) { }
  virtual ~EnumMixedLrsStrategySolver() { }

  List<MixedStrategyProfile<Rational> > Solve(const Game &p_game) const;
};

}  // end namespace Gambit::Nash
}  // end namespace Gambit
    
#endif  // GAMBIT_NASH_ENUMMIXED_H
