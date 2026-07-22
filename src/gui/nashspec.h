//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// Typed specifications for Nash equilibrium computations initiated by the GUI.
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

#ifndef GAMBIT_GUI_NASHSPEC_H
#define GAMBIT_GUI_NASHSPEC_H

#include <variant>

#include "core/rational.h"

namespace Gambit::GUI {

enum class NashRepresentation { Strategic, Behavior };
enum class NashEquilibriumTarget { One, Some, All };

struct EnumPureNashSpec {};
struct EnumMixedNashSpec {};

struct EnumPolyNashSpec {
  int stopAfter{0};
  double maxRegret{1.0e-4};
};

struct GNMNashSpec {
  int perturbations{1};
  double lambdaEnd{-10.0};
  int steps{100};
  int localNewtonInterval{3};
  int localNewtonMaxIterations{10};
};

struct IPANashSpec {
  int perturbations{1};
};

struct LPNashSpec {};

struct LCPNashSpec {
  int stopAfter{0};
  int maxDepth{0};
};

struct LiapNashSpec {
  int startingPoints{10};
  double maxRegret{1.0e-4};
  int maxIterations{1000};
};

struct LogitNashSpec {
  double maxRegret{1.0e-8};
  double omega{1.0};
  double firstStep{0.03};
  double maxAcceleration{1.1};
};

struct SimpdivNashSpec {
  int startingPoints{20};
  int randomDenominator{100};
  int gridResize{2};
  int leashLength{0};
  Rational maxRegret{1, 10000000};
};

using NashMethodSpec =
    std::variant<EnumPureNashSpec, EnumMixedNashSpec, EnumPolyNashSpec, GNMNashSpec, IPANashSpec,
                 LPNashSpec, LCPNashSpec, LiapNashSpec, LogitNashSpec, SimpdivNashSpec>;

struct NashComputationSpec {
  NashRepresentation representation;
  NashEquilibriumTarget target;
  NashMethodSpec method;
  bool selectedByRecommendation{false};
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_NASHSPEC_H
