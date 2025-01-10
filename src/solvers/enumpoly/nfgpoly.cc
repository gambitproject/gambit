//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/nfgpoly.cc
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

#include <numeric>

#include "enumpoly.h"
#include "solvers/nashsupport/nashsupport.h"
#include "polysystem.h"
#include "polysolver.h"

using namespace Gambit;

namespace {

// The polynomial representation of each strategy probability, substituting in
// the sum-to-one equation for the probability of the last strategy for each player
std::map<GameStrategy, Polynomial<double>>
BuildStrategyVariables(std::shared_ptr<VariableSpace> space, const StrategySupportProfile &support)
{
  int index = 1;
  std::map<GameStrategy, Polynomial<double>> strategy_poly;
  for (auto player : support.GetGame()->GetPlayers()) {
    auto strategies = support.GetStrategies(player);
    Polynomial<double> residual(space, 1);
    for (auto strategy : strategies) {
      if (strategy != strategies.back()) {
        strategy_poly.try_emplace(strategy, space, index, 1);
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

Polynomial<double>
IndifferenceEquation(std::shared_ptr<VariableSpace> space, const StrategySupportProfile &support,
                     const std::map<GameStrategy, Polynomial<double>> &strategy_poly,
                     const GameStrategy &s1, const GameStrategy &s2)
{
  Polynomial<double> equation(space);

  for (auto iter : StrategyContingencies(support, {s1})) {
    Polynomial<double> term(space, 1);
    for (auto player : support.GetGame()->GetPlayers()) {
      if (player != s1->GetPlayer()) {
        term *= strategy_poly.at(iter->GetStrategy(player));
      }
    }
    term *= iter->GetStrategyValue(s1) - iter->GetStrategyValue(s2);
    equation += term;
  }
  return equation;
}

PolynomialSystem<double>
ConstructEquations(std::shared_ptr<VariableSpace> space, const StrategySupportProfile &support,
                   const std::map<GameStrategy, Polynomial<double>> &strategy_poly)
{
  PolynomialSystem<double> equations(space);
  // Indifference equations between pairs of strategies for each player
  for (auto player : support.GetPlayers()) {
    auto strategies = support.GetStrategies(player);
    for (auto s1 = strategies.begin(), s2 = std::next(strategies.begin()); s2 != strategies.end();
         ++s1, ++s2) {
      equations.push_back(IndifferenceEquation(space, support, strategy_poly, *s1, *s2));
    }
  }
  // Inequalities for last probability for each player
  for (auto player : support.GetPlayers()) {
    equations.push_back(strategy_poly.at(support.GetStrategies(player).back()));
  }
  return equations;
}

} // end anonymous namespace

namespace Gambit {
namespace Nash {

std::list<MixedStrategyProfile<double>>
EnumPolyStrategySupportSolve(const StrategySupportProfile &support, bool &is_singular,
                             int p_stopAfter)
{
  auto space = std::make_shared<VariableSpace>(support.MixedProfileLength() -
                                               support.GetGame()->NumPlayers());

  auto strategy_poly = BuildStrategyVariables(space, support);
  PolynomialSystem<double> equations = ConstructEquations(space, support, strategy_poly);

  Vector<double> bottoms(space->GetDimension()), tops(space->GetDimension());
  bottoms = 0;
  tops = 1;
  PolynomialSystemSolver solver(equations);
  is_singular = false;
  std::list<Vector<double>> roots;
  try {
    roots = solver.FindRoots({bottoms, tops},
                             (p_stopAfter > 0) ? p_stopAfter : std::numeric_limits<int>::max());
  }
  catch (const SingularMatrixException &) {
    is_singular = true;
  }
  catch (const AssertionException &e) {
    // std::cerr << "Assertion warning: " << e.what() << std::endl;
    is_singular = true;
  }

  std::list<MixedStrategyProfile<double>> solutions;
  for (auto soln : roots) {
    solutions.emplace(solutions.end(), support.NewMixedStrategyProfile<double>());
    for (auto mapping : strategy_poly) {
      solutions.back()[mapping.first] = mapping.second.Evaluate(soln);
    }
  }
  return solutions;
}

List<MixedStrategyProfile<double>>
EnumPolyStrategySolve(const Game &p_game, int p_stopAfter, double p_maxregret,
                      EnumPolyMixedStrategyObserverFunctionType p_onEquilibrium,
                      EnumPolyStrategySupportObserverFunctionType p_onSupport)
{
  if (double scale = p_game->GetMaxPayoff() - p_game->GetMinPayoff() != 0.0) {
    p_maxregret *= scale;
  }

  List<MixedStrategyProfile<double>> ret;
  auto possible_supports = PossibleNashStrategySupports(p_game);

  for (auto support : possible_supports->m_supports) {
    p_onSupport("candidate", support);
    bool is_singular;
    for (auto solution : EnumPolyStrategySupportSolve(
             support, is_singular, std::max(p_stopAfter - int(ret.size()), 0))) {
      MixedStrategyProfile<double> fullProfile = solution.ToFullSupport();
      if (fullProfile.GetMaxRegret() < p_maxregret) {
        p_onEquilibrium(fullProfile);
        ret.push_back(fullProfile);
      }
    }

    if (is_singular) {
      p_onSupport("singular", support);
    }
    if (p_stopAfter > 0 && ret.size() >= p_stopAfter) {
      break;
    }
  }
  return ret;
}

} // namespace Nash
} // namespace Gambit
