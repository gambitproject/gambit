//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/nashvariants.cc
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

#include <algorithm>
#include <array>
#include <stdexcept>

#include "games/gametree.h"

#include "nashvariants.h"

namespace Gambit::GUI {

namespace {

using enum NashMethodId;
using enum VariantRepresentation;
using enum VariantCoverage;
using enum VariantReliability;
using enum VariantConcept;
using enum VariantArithmetic;
using enum VariantScope;

constexpr std::array<NashVariant, 17> s_variants{{
    {.method = EnumMixed,
     .methodName = "enummixed",
     .name = "strategic",
     .representation = Strategic,
     .coverage = All,
     .reliability = Always,
     .solves = Nash,
     .arithmetic = Exact,
     .scope = TwoPlayer},
    {.method = EnumPoly,
     .methodName = "enumpoly",
     .name = "sequence form",
     .representation = Behavior,
     .coverage = Some,
     .reliability = Usually,
     .solves = Nash,
     .arithmetic = FloatingPoint,
     .scope = AnyGame},
    {.method = EnumPoly,
     .methodName = "enumpoly",
     .name = "strategic",
     .representation = Strategic,
     .coverage = Some,
     .reliability = Usually,
     .solves = Nash,
     .arithmetic = FloatingPoint,
     .scope = AnyGame},
    {.method = EnumPure,
     .methodName = "enumpure",
     .name = "agent form",
     .representation = Behavior,
     .coverage = AllPure,
     .reliability = Always,
     .solves = AgentNash,
     .arithmetic = Exact,
     .scope = AnyGame},
    {.method = EnumPure,
     .methodName = "enumpure",
     .name = "strategic",
     .representation = Strategic,
     .coverage = AllPure,
     .reliability = Always,
     .solves = Nash,
     .arithmetic = Exact,
     .scope = AnyGame},
    {.method = GNM,
     .methodName = "gnm",
     .name = "strategic",
     .representation = Strategic,
     .coverage = Some,
     .reliability = Usually,
     .solves = Nash,
     .arithmetic = FloatingPoint,
     .scope = AnyGame},
    {.method = HP,
     .methodName = "hp",
     .name = "strategic",
     .representation = Strategic,
     .coverage = Some,
     .reliability = Usually,
     .solves = Nash,
     .arithmetic = FloatingPoint,
     .scope = AnyGame},
    {.method = IPA,
     .methodName = "ipa",
     .name = "strategic",
     .representation = Strategic,
     .coverage = Some,
     .reliability = NoGuarantee,
     .solves = Nash,
     .arithmetic = FloatingPoint,
     .scope = AnyGame},
    {.method = LCP,
     .methodName = "lcp",
     .name = "Lemke (sequence form)",
     .representation = Behavior,
     .coverage = One,
     .reliability = Always,
     .solves = Nash,
     .arithmetic = Exact,
     .scope = TwoPlayer},
    {.method = LCP,
     .methodName = "lcp",
     .name = "Lemke-Howson (strategic)",
     .representation = Strategic,
     .coverage = Some,
     .reliability = Always,
     .solves = Nash,
     .arithmetic = Exact,
     .scope = TwoPlayer},
    {.method = Liap,
     .methodName = "liap",
     .name = "agent form",
     .representation = Behavior,
     .coverage = Some,
     .reliability = NoGuarantee,
     .solves = AgentNash,
     .arithmetic = FloatingPoint,
     .scope = AnyGame},
    {.method = Liap,
     .methodName = "liap",
     .name = "strategic",
     .representation = Strategic,
     .coverage = Some,
     .reliability = NoGuarantee,
     .solves = Nash,
     .arithmetic = FloatingPoint,
     .scope = AnyGame},
    {.method = Logit,
     .methodName = "logit",
     .name = "agent form",
     .representation = Behavior,
     .coverage = One,
     .reliability = Usually,
     .solves = Nash,
     .arithmetic = FloatingPoint,
     .scope = AnyGame},
    {.method = Logit,
     .methodName = "logit",
     .name = "strategic",
     .representation = Strategic,
     .coverage = One,
     .reliability = Usually,
     .solves = Nash,
     .arithmetic = FloatingPoint,
     .scope = AnyGame},
    {.method = LP,
     .methodName = "lp",
     .name = "sequence form",
     .representation = Behavior,
     .coverage = One,
     .reliability = Always,
     .solves = Nash,
     .arithmetic = Exact,
     .scope = TwoPlayerConstSum},
    {.method = LP,
     .methodName = "lp",
     .name = "strategic",
     .representation = Strategic,
     .coverage = One,
     .reliability = Always,
     .solves = Nash,
     .arithmetic = Exact,
     .scope = TwoPlayerConstSum},
    {.method = Simpdiv,
     .methodName = "simpdiv",
     .name = "strategic",
     .representation = Strategic,
     .coverage = Some,
     .reliability = Always,
     .solves = Nash,
     .arithmetic = Exact,
     .scope = AnyGame},
}};

} // namespace

std::span<const NashVariant> NashVariants() { return s_variants; }

std::vector<const NashVariant *> NashVariantsOf(NashMethodId p_method)
{
  std::vector<const NashVariant *> result;
  for (const auto &variant : s_variants) {
    if (variant.method == p_method) {
      result.push_back(&variant);
    }
  }
  return result;
}

bool MeetsGoal(const NashVariant &p_variant, NashGoal p_goal)
{
  switch (p_goal) {
  case NashGoal::One:
    return p_variant.coverage != AllPure;
  case NashGoal::Several:
    return p_variant.coverage != One;
  case NashGoal::All:
    return p_variant.coverage == All;
  }
  throw std::logic_error("Unknown goal");
}

std::optional<std::string> UnavailableReason(const NashVariant &p_variant, const Game &p_game)
{
  if (p_variant.representation == Behavior && !As<GameTreeRep>(p_game)) {
    return "needs a game tree";
  }
  switch (p_variant.scope) {
  case TwoPlayer:
    if (p_game->NumPlayers() != 2) {
      return "two players only";
    }
    break;
  case TwoPlayerConstSum:
    if (p_game->NumPlayers() != 2 || !p_game->IsConstSum()) {
      return "two-player constant-sum games only";
    }
    break;
  case AnyGame:
    break;
  }
  return std::nullopt;
}

NashMethodId RecommendedMethod(NashGoal p_goal, const Game &p_game)
{
  switch (p_goal) {
  case NashGoal::One:
    return p_game->NumPlayers() == 2 && p_game->IsConstSum() ? LP : Logit;
  case NashGoal::Several:
    return p_game->NumPlayers() == 2 ? LCP : Simpdiv;
  case NashGoal::All:
    return EnumMixed;
  }
  throw std::logic_error("Unknown goal");
}

const NashVariant &RecommendedVariant(NashGoal p_goal, const Game &p_game)
{
  const auto candidates = NashVariantsOf(RecommendedMethod(p_goal, p_game));
  const NashVariant *best = nullptr;
  for (const auto *variant : candidates) {
    if (UnavailableReason(*variant, p_game) || !MeetsGoal(*variant, p_goal)) {
      continue;
    }
    if (!best || variant->representation == Behavior) {
      best = variant;
    }
  }
  if (!best) {
    throw std::logic_error("No variant of the recommended method meets the goal");
  }
  return *best;
}

} // namespace Gambit::GUI
