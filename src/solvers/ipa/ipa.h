//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: library/include/gambit/nash/ipa.h
// Compute Nash equilibria via iterated polymatrix approximation
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

#ifndef GAMBIT_NASH_IPA_H
#define GAMBIT_NASH_IPA_H

#include "games/nash.h"

namespace Gambit {
namespace Nash {

List<MixedStrategyProfile<double>>
IPAStrategySolve(const Game &p_game, StrategyCallbackType p_callback = NullStrategyCallback);

List<MixedStrategyProfile<double>>
IPAStrategySolve(const MixedStrategyProfile<double> &p_pert,
                 StrategyCallbackType p_callback = NullStrategyCallback);

} // namespace Nash
} // namespace Gambit

#endif // GAMBIT_NASH_IPA_H
