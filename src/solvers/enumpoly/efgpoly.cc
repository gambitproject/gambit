//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/efgpoly.cc
// Enumerates all Nash equilibria of a game, via polynomial equations
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

#include "enumpoly.h"
#include "solvers/nashsupport/nashsupport.h"
#include "gameseq.h"
#include "gpoly.h"
#include "gpolylst.h"
#include "rectangle.h"
#include "quiksolv.h"
#include "behavextend.h"

using namespace Gambit;
using namespace Gambit::Nash;

namespace {

// The set of polynomials is constructed incorporating these techniques:
//
// 1. The sum-to-one equations and sequence form constraints are substituted
//    into the system, resuting in a reduction of the number of variables.
//    This is accomplished by BuildSequenceVariable(), resulting in a
//    mapping from each sequence to an expression - which may be a single
//    variable for the probability of that sequence, or an expression of
//    that probability in terms of the probabilities of other sequences.
// 2. The indifference conditions are implemented by computing the
//    expected payoff polynomial for a player, and then taking the
//    partial derivative of that with respect to each of the player's
//    sequence probabilities (after the substitutions mentioned above)
//    and setting those to zero.

class ProblemData {
public:
  GameSequenceForm sfg;
  VariableSpace Space;
  std::map<GameSequence, int> var;
  std::map<GameSequence, gPoly<double>> variables;

  explicit ProblemData(const BehaviorSupportProfile &p_support);
};

gPoly<double> BuildSequenceVariable(ProblemData &p_data, const GameSequence &p_sequence,
                                    const std::map<GameSequence, int> &var)
{
  if (!p_sequence->action) {
    return {&p_data.Space, 1};
  }
  if (p_sequence->action != p_data.sfg.GetSupport().GetActions(p_sequence->GetInfoset()).back()) {
    return {&p_data.Space, var.at(p_sequence), 1};
  }

  gPoly<double> equation(&p_data.Space);
  for (auto seq : p_data.sfg.GetSequences(p_sequence->player)) {
    if (seq == p_sequence) {
      continue;
    }
    if (int constraint_coef =
            p_data.sfg.GetConstraintEntry(p_sequence->GetInfoset(), seq->action)) {
      equation += BuildSequenceVariable(p_data, seq, var) * double(constraint_coef);
    }
  }
  return equation;
}

ProblemData::ProblemData(const BehaviorSupportProfile &p_support)
  : sfg(p_support),
    Space(sfg.GetSequences().size() - sfg.GetInfosets().size() - sfg.GetPlayers().size())
{
  for (auto sequence : sfg.GetSequences()) {
    if (sequence->action &&
        (sequence->action != p_support.GetActions(sequence->GetInfoset()).back())) {
      var[sequence] = var.size() + 1;
    }
  }

  for (auto sequence : sfg.GetSequences()) {
    variables.emplace(sequence, BuildSequenceVariable(*this, sequence, var));
  }
}

gPoly<double> GetPayoff(ProblemData &p_data, const GamePlayer &p_player)
{
  gPoly<double> equation(&p_data.Space);

  for (auto profile : p_data.sfg.GetContingencies()) {
    auto pay = p_data.sfg.GetPayoff(profile, p_player);
    if (pay != Rational(0)) {
      gPoly<double> term(&p_data.Space, double(pay));
      for (auto player : p_data.sfg.GetPlayers()) {
        term *= p_data.variables.at(profile[player]);
      }
      equation += term;
    }
  }
  return equation;
}

void IndifferenceEquations(ProblemData &p_data, gPolyList<double> &p_equations)
{
  for (auto player : p_data.sfg.GetPlayers()) {
    gPoly<double> payoff = GetPayoff(p_data, player);
    for (auto sequence : p_data.sfg.GetSequences(player)) {
      try {
        p_equations += payoff.PartialDerivative(p_data.var.at(sequence));
      }
      catch (std::out_of_range) {
        // This sequence's variable was already substituted out in terms of
        // the probabilities of other sequences
      }
    }
  }
}

void LastActionProbPositiveInequalities(ProblemData &p_data, gPolyList<double> &p_equations)
{
  for (auto sequence : p_data.sfg.GetSequences()) {
    if (!sequence->action) {
      continue;
    }
    const auto &actions = p_data.sfg.GetSupport().GetActions(sequence->action->GetInfoset());
    if (actions.size() > 1 && sequence->action == actions.back()) {
      p_equations += p_data.variables.at(sequence);
    }
  }
}

std::map<GameSequence, double> ToSequenceProbs(const ProblemData &p_data, const Vector<double> &v)
{
  std::map<GameSequence, double> x;
  for (auto sequence : p_data.sfg.GetSequences()) {
    x[sequence] = p_data.variables.at(sequence).Evaluate(v);
  }
  return x;
}

std::list<MixedBehaviorProfile<double>> SolveSupport(const BehaviorSupportProfile &p_support,
                                                     bool &p_isSingular, int p_stopAfter)
{
  ProblemData data(p_support);
  gPolyList<double> equations(&data.Space);
  IndifferenceEquations(data, equations);
  LastActionProbPositiveInequalities(data, equations);

  // set up the rectangle of search
  Vector<double> bottoms(data.Space.Dmnsn()), tops(data.Space.Dmnsn());
  bottoms = 0;
  tops = 1;

  QuikSolv<double> solver(equations);
  try {
    solver.FindCertainNumberOfRoots({bottoms, tops}, std::numeric_limits<int>::max(), p_stopAfter);
  }
  catch (const SingularMatrixException &) {
    p_isSingular = true;
  }
  catch (const AssertionException &e) {
    // std::cerr << "Assertion warning: " << e.what() << std::endl;
    p_isSingular = true;
  }

  std::list<MixedBehaviorProfile<double>> solutions;
  for (auto root : solver.RootList()) {
    MixedBehaviorProfile<double> sol(data.sfg.ToMixedBehaviorProfile(ToSequenceProbs(data, root)));
    if (ExtendsToNash(sol, BehaviorSupportProfile(sol.GetGame()),
                      BehaviorSupportProfile(sol.GetGame()))) {
      solutions.push_back(sol);
    }
  }
  return solutions;
}

} // end anonymous namespace

namespace Gambit {
namespace Nash {

List<MixedBehaviorProfile<double>>
EnumPolyBehaviorSolve(const Game &p_game, int p_stopAfter, double p_maxregret,
                      EnumPolyMixedBehaviorObserverFunctionType p_onEquilibrium,
                      EnumPolyBehaviorSupportObserverFunctionType p_onSupport)
{
  if (double scale = p_game->GetMaxPayoff() - p_game->GetMinPayoff() != 0.0) {
    p_maxregret *= scale;
  }

  List<MixedBehaviorProfile<double>> ret;
  auto possible_supports = PossibleNashBehaviorSupports(p_game);

  for (auto support : possible_supports->m_supports) {
    p_onSupport("candidate", support);
    bool isSingular = false;
    for (auto solution :
         SolveSupport(support, isSingular, std::max(p_stopAfter - int(ret.size()), 0))) {
      MixedBehaviorProfile<double> fullProfile = solution.ToFullSupport();
      if (fullProfile.GetMaxRegret() < p_maxregret) {
        p_onEquilibrium(fullProfile);
        ret.push_back(fullProfile);
      }
    }
    if (isSingular) {
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
