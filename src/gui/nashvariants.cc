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
    {EnumMixed, "enummixed", "strategic", Strategic, All, Always, Nash, Exact, TwoPlayer,
     "Returns every extreme equilibrium; connected components of equilibria are described by "
     "their extreme points."},
    {EnumPoly, "enumpoly", "sequence form", Behavior, Some, NumericalFailurePossible, Nash,
     FloatingPoint, AnyGame,
     "Finds isolated equilibria only; supports it cannot resolve are skipped. May find "
     "equilibria that the strategic variant does not."},
    {EnumPoly, "enumpoly", "strategic", Strategic, Some, NumericalFailurePossible, Nash,
     FloatingPoint, AnyGame,
     "Finds isolated equilibria only; supports it cannot resolve are skipped. May find "
     "equilibria that the sequence-form variant does not."},
    {EnumPure, "enumpure", "agent form", Behavior, AllPure, Always, AgentNash, Exact, AnyGame,
     "Returns all pure-strategy agent Nash equilibria, which need not be Nash equilibria and "
     "may not exist."},
    {EnumPure, "enumpure", "strategic", Strategic, AllPure, Always, Nash, Exact, AnyGame,
     "Returns all pure-strategy equilibria, which may not exist."},
    {GNM, "gnm", "strategic", Strategic, Some, NumericalFailurePossible, Nash, FloatingPoint,
     AnyGame, "Less thoroughly tested than simplicial subdivision and can fail numerically."},
    {HP, "hp", "strategic", Strategic, Some, NumericalFailurePossible, Nash, FloatingPoint,
     AnyGame, "Returns at most one equilibrium per random prior."},
    {IPA, "ipa", "strategic", Strategic, Some, NoGuarantee, Nash, FloatingPoint, AnyGame,
     "Convergence is not known to be guaranteed."},
    {LCP, "lcp", "Lemke (sequence form)", Behavior, One, Always, Nash, Exact, TwoPlayer,
     "Returns a single equilibrium."},
    {LCP, "lcp", "Lemke-Howson (strategic)", Strategic, Some, Always, Nash, Exact, TwoPlayer,
     "Finds only the accessible equilibria (those reachable by the Lemke-Howson path)."},
    {Liap, "liap", "agent form", Behavior, Some, NoGuarantee, AgentNash, FloatingPoint, AnyGame,
     "Local descent from random starting points. The result is an agent Nash equilibrium, "
     "which need not be a Nash equilibrium."},
    {Liap, "liap", "strategic", Strategic, Some, NoGuarantee, Nash, FloatingPoint, AnyGame,
     "Local descent from random starting points; may converge to a non-equilibrium."},
    {Logit, "logit", "agent form", Behavior, One, NumericalFailurePossible, Nash, FloatingPoint,
     AnyGame, "Traces the agent quantal response equilibrium; its limit is a Nash equilibrium."},
    {Logit, "logit", "strategic", Strategic, One, NumericalFailurePossible, Nash, FloatingPoint,
     AnyGame, "Traces the quantal response equilibrium; its limit is a Nash equilibrium."},
    {LP, "lp", "sequence form", Behavior, One, Always, Nash, Exact, TwoPlayerConstSum,
     "May return a different equilibrium from the strategic variant."},
    {LP, "lp", "strategic", Strategic, One, Always, Nash, Exact, TwoPlayerConstSum,
     "May return a different equilibrium from the sequence-form variant."},
    {Simpdiv, "simpdiv", "strategic", Strategic, Some, Always, Nash, Exact, AnyGame,
     "Returns an approximate equilibrium to within the stated regret."},
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
