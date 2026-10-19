//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/nashvariants.h
// Catalog of equilibrium-computation variants
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

#ifndef GAMBIT_GUI_NASHVARIANTS_H
#define GAMBIT_GUI_NASHVARIANTS_H

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "games.h"

namespace Gambit::GUI {

enum class NashGoal { One, Several, All };

enum class NashMethodId {
  EnumMixed,
  EnumPoly,
  EnumPure,
  GNM,
  HP,
  IPA,
  LCP,
  Liap,
  Logit,
  LP,
  Simpdiv
};

enum class VariantRepresentation { Strategic, Behavior };
enum class VariantCoverage { One, Some, AllPure, All };
enum class VariantReliability { Always, NumericalFailurePossible, NoGuarantee };
enum class VariantConcept { Nash, AgentNash };
enum class VariantArithmetic { Exact, FloatingPoint };
enum class VariantScope { AnyGame, TwoPlayer, TwoPlayerConstSum };

/// A method applied in one representation of the game.
struct NashVariant {
  NashMethodId method;
  std::string_view methodName;
  std::string_view name;
  VariantRepresentation representation;
  VariantCoverage coverage;
  VariantReliability reliability;
  VariantConcept solves;
  VariantArithmetic arithmetic;
  VariantScope scope;
  std::string_view caveat;
};

/// Every variant, in alphabetical order by method.
std::span<const NashVariant> NashVariants();

/// The variants of one method, in catalog order.
std::vector<const NashVariant *> NashVariantsOf(NashMethodId p_method);

/// Whether the variant is expected to return what the goal asks for.
bool MeetsGoal(const NashVariant &p_variant, NashGoal p_goal);

/// The reason the variant cannot run on the game, if it cannot.
std::optional<std::string> UnavailableReason(const NashVariant &p_variant, const Game &p_game);

/// The method Gambit recommends for the goal on the game.
NashMethodId RecommendedMethod(NashGoal p_goal, const Game &p_game);

/// The recommended method's variant that runs on the game and meets the goal,
/// preferring the extensive-game variant.
const NashVariant &RecommendedVariant(NashGoal p_goal, const Game &p_game);

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_NASHVARIANTS_H
