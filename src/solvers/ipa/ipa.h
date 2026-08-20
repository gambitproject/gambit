//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/ipa/ipa.h
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

#ifndef GAMBIT_SOLVERS_IPA_IPA_H
#define GAMBIT_SOLVERS_IPA_IPA_H

#include <variant>

#include "solvers/nash.h"
#include "solvers/gtracer/gtracer.h"

namespace Gambit::Nash {

/// @brief Reports the state of the approximating strategy profile at the end of an
///        iteration of IPA
struct IPAStepEvent {
  const MixedStrategyProfile<double> &profile;
  int iteration;
  double zDiff;
  double sDiff;
};

/// @brief Reports why a call to IPA's iteration terminated, once at the end of the call.
///        Because IPA may internally restart the iteration from where it left off if it
///        has not yet converged, this may be raised more than once in the course of solving.
struct IPATerminationEvent {
  gametracer::IPATerminationReason reason;
  std::string message;
};

using IPAEvent = std::variant<IPAStepEvent, IPATerminationEvent>;
using IPAEventCallbackType = std::function<void(const IPAEvent &)>;

inline void NullIPAEventCallback(const IPAEvent &) {}

std::list<MixedStrategyProfile<double>>
IPAStrategySolve(const Game &p_game,
                 StrategyCallbackType<double> p_onEquilibrium = NullStrategyCallback<double>,
                 IPAEventCallbackType p_onEvent = NullIPAEventCallback,
                 const CancelToken &p_cancel = CancelToken());

std::list<MixedStrategyProfile<double>>
IPAStrategySolve(const MixedStrategyProfile<double> &p_pert,
                 StrategyCallbackType<double> p_onEquilibrium = NullStrategyCallback<double>,
                 IPAEventCallbackType p_onEvent = NullIPAEventCallback,
                 const CancelToken &p_cancel = CancelToken());

} // namespace Gambit::Nash

#endif // GAMBIT_SOLVERS_IPA_IPA_H
