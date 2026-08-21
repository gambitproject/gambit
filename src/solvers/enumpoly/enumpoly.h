//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/enumpoly.h
// Finds Nash equilibria of a game by solving systems of polynomial equations
// by enumerating supports.
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

#ifndef GAMBIT_SOLVERS_ENUMPOLY_ENUMPOLY_H
#define GAMBIT_SOLVERS_ENUMPOLY_ENUMPOLY_H

#include <optional>
#include <variant>

#include "solvers/nash.h"
#include "solvers/nashsupport/nashsupport.h"

namespace Gambit::Nash {

template <class Support> struct EnumPolyCandidateSupportEvent {
  const Support &support;
};

template <class Support> struct EnumPolySingularSupportEvent {
  const Support &support;
};

/// Reported when the search for roots on a support exhausted its rectangle
/// budget before completing.  Distinct from EnumPolySingularSupportEvent:
/// this can happen even when the equilibria being sought are regular.
template <class Support> struct EnumPolyBudgetExceededSupportEvent {
  const Support &support;
};

template <class Support>
using EnumPolyEvent =
    std::variant<EnumPolyCandidateSupportEvent<Support>, EnumPolySingularSupportEvent<Support>,
                 EnumPolyBudgetExceededSupportEvent<Support>>;

template <class Support>
using EnumPolyEventCallbackType = std::function<void(const EnumPolyEvent<Support> &)>;

template <class Support> void NullEnumPolyEventCallback(const EnumPolyEvent<Support> &) {}

/// Default budget on the number of rectangles examined per support before
/// giving up.  Ordinary supports need only a handful, leaving ample headroom
/// for difficult-but-tractable ones while bounding the worst case.
constexpr size_t kDefaultEnumPolyMaxRectangles = 20'000;

std::list<MixedStrategyProfile<double>>
EnumPolyStrategySolve(const Game &p_game, std::optional<size_t> p_stopAfter, double p_maxregret,
                      size_t p_maxRectangles = kDefaultEnumPolyMaxRectangles,
                      StrategyCallbackType<double> p_onEquilibrium = NullStrategyCallback<double>,
                      EnumPolyEventCallbackType<StrategySupportProfile> p_onEvent =
                          NullEnumPolyEventCallback<StrategySupportProfile>,
                      const CancelToken &p_cancel = CancelToken());

std::list<MixedBehaviorProfile<double>>
EnumPolyBehaviorSolve(const Game &, std::optional<size_t> p_stopAfter, double p_maxregret,
                      size_t p_maxRectangles = kDefaultEnumPolyMaxRectangles,
                      BehaviorCallbackType<double> p_onEquilibrium = NullBehaviorCallback<double>,
                      EnumPolyEventCallbackType<BehaviorSupportProfile> p_onEvent =
                          NullEnumPolyEventCallback<BehaviorSupportProfile>,
                      const CancelToken &p_cancel = CancelToken());

} // namespace Gambit::Nash

#endif // GAMBIT_SOLVERS_ENUMPOLY_ENUMPOLY_H
