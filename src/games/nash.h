//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/libgambit/nash.h
// Framework for computing (sub)sets of Nash equilibria.
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

#ifndef LIBGAMBIT_NASH_H
#define LIBGAMBIT_NASH_H

#include <functional>
#include "gambit.h"

namespace Gambit::Nash {

template <class T>
using StrategyCallbackType =
    std::function<void(const MixedStrategyProfile<T> &, const std::string &)>;

/// @brief A fallback callback function for mixed strategy profiles that does nothing
template <class T> void NullStrategyCallback(const MixedStrategyProfile<T> &, const std::string &)
{
}

template <class T>
using BehaviorCallbackType =
    std::function<void(const MixedBehaviorProfile<T> &, const std::string &)>;

/// @brief A fallback callback function for mixed behavior profiles that does nothing
template <class T> void NullBehaviorCallback(const MixedBehaviorProfile<T> &, const std::string &)
{
}

template <class T>
std::list<MixedBehaviorProfile<T>>
ToMixedBehaviorProfile(const std::list<MixedStrategyProfile<T>> &p_list)
{
  std::list<MixedBehaviorProfile<T>> ret;
  for (const auto &profile : p_list) {
    ret.push_back(MixedBehaviorProfile<T>(profile));
  }
  return ret;
}

template <class T>
using BehaviorSolverType = std::function<std::list<MixedBehaviorProfile<T>>(const Game &)>;

//
// Exception raised when maximum number of equilibria to compute
// has been reached.  A convenience for unraveling a potentially
// deep recursion.
//
class EquilibriumLimitReached : public std::runtime_error {
public:
  EquilibriumLimitReached() : std::runtime_error("Reached target number of equilibria") {}
  ~EquilibriumLimitReached() noexcept override = default;
};

} // namespace Gambit::Nash

#endif // LIBGAMBIT_NASH_H
