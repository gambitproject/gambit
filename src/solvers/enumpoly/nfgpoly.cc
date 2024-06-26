//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/nfgpoly.cc
// Enumerates all Nash equilibria in a normal form game, via solving
// systems of polynomial equations
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

#include <limits>
#include <numeric>

#include "enumpoly.h"
#include "solvers/nashsupport/nashsupport.h"
#include "gpoly.h"
#include "gpolylst.h"
#include "rectangl.h"
#include "quiksolv.h"

using namespace Gambit;

namespace {

// The polynomial representation of each strategy probability, substituting in
// the sum-to-one equation for the probability of the last strategy for each player
std::map<GameStrategy, gPoly<double>> BuildStrategyVariables(const gSpace &space,
                                                             const term_order &lex,
                                                             const StrategySupportProfile &support)
{
  int index = 1;
  std::map<GameStrategy, gPoly<double>> strategy_poly;
  for (auto player : support.GetGame()->GetPlayers()) {
    auto strategies = support.GetStrategies(player);
    gPoly<double> residual(&space, 1, &lex);
    for (auto strategy : strategies) {
      if (strategy != strategies.back()) {
        strategy_poly.try_emplace(strategy, &space, index, 1, &lex);
        residual -= strategy_poly.at(strategy);
        index++;
      }
      else {
        strategy_poly.emplace(strategy, residual);
      }
    }
  }
  return strategy_poly;
}

gPoly<double> IndifferenceEquation(const gSpace &space, const term_order &lex,
                                   const StrategySupportProfile &support,
                                   const std::map<GameStrategy, gPoly<double>> &strategy_poly,
                                   const GameStrategy &s1, const GameStrategy &s2)
{
  gPoly<double> equation(&space, &lex);

  for (StrategyProfileIterator A(support, s1), B(support, s2); !A.AtEnd(); A++, B++) {
    gPoly<double> term(&space, 1, &lex);
    for (auto player : support.GetGame()->GetPlayers()) {
      if (player != s1->GetPlayer()) {
        term *= strategy_poly.at((*A)->GetStrategy(player));
      }
    }
    term *= (*A)->GetPayoff(s1->GetPlayer()) - (*B)->GetPayoff(s1->GetPlayer());
    equation += term;
  }
  return equation;
}

gPolyList<double> ConstructEquations(const gSpace &space, const term_order &lex,
                                     const StrategySupportProfile &support,
                                     const std::map<GameStrategy, gPoly<double>> &strategy_poly)
{
  gPolyList<double> equations(&space, &lex);
  // Indifference equations between pairs of strategies for each player
  for (auto player : support.GetPlayers()) {
    auto strategies = support.GetStrategies(player);
    for (auto s1 = strategies.begin(), s2 = std::next(strategies.begin()); s2 != strategies.end();
         ++s1, ++s2) {
      equations += IndifferenceEquation(space, lex, support, strategy_poly, *s1, *s2);
    }
  }
  // Inequalities for last probability for each player
  for (auto player : support.GetPlayers()) {
    equations += strategy_poly.at(support.GetStrategies(player).back());
  }
  return equations;
}

} // end anonymous namespace

namespace Gambit {
namespace Nash {

std::list<MixedStrategyProfile<double>>
EnumPolyStrategySupportSolve(const StrategySupportProfile &support, bool &is_singular,
                             int p_stopAfter /* = 0 */)
{
  gSpace Space(support.MixedProfileLength() - support.GetGame()->NumPlayers());
  term_order Lex(&Space, lex);

  auto strategy_poly = BuildStrategyVariables(Space, Lex, support);
  gPolyList<double> equations = ConstructEquations(Space, Lex, support, strategy_poly);

  Vector<double> bottoms(Space.Dmnsn()), tops(Space.Dmnsn());
  bottoms = 0;
  tops = 1;
  gRectangle<double> cube(bottoms, tops);
  QuikSolv<double> solver(equations);
  is_singular = false;
  try {
    solver.FindCertainNumberOfRoots(cube, std::numeric_limits<int>::max(), p_stopAfter);
  }
  catch (const SingularMatrixException &) {
    is_singular = true;
  }
  catch (const AssertionException &e) {
    // std::cerr << "Assertion warning: " << e.what() << std::endl;
    is_singular = true;
  }

  std::list<MixedStrategyProfile<double>> solutions;
  for (auto soln : solver.RootList()) {
    solutions.emplace(solutions.end(), support.NewMixedStrategyProfile<double>());
    for (auto mapping : strategy_poly) {
      solutions.back()[mapping.first] = mapping.second.Evaluate(soln);
    }
  }
  return solutions;
}

List<MixedStrategyProfile<double>>
EnumPolyStrategySolve(const Game &p_game,
                      EnumPolyMixedStrategyObserverFunctionType p_onEquilibrium,
                      EnumPolyStrategySupportObserverFunctionType p_onSupport)
{
  List<MixedStrategyProfile<double>> ret;
  auto possible_supports = PossibleNashStrategySupports(p_game);

  for (auto support : possible_supports->m_supports) {
    p_onSupport("candidate", support);
    bool is_singular;
    for (auto solution : EnumPolyStrategySupportSolve(support, is_singular)) {
      MixedStrategyProfile<double> fullProfile = solution.ToFullSupport();
      if (fullProfile.GetLiapValue() < 1.0e-6) {
        p_onEquilibrium(fullProfile);
        ret.push_back(fullProfile);
      }
    }

    if (is_singular) {
      p_onSupport("singular", support);
    }
  }
  return ret;
}

} // namespace Nash
} // namespace Gambit
