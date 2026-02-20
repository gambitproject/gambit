//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/enumpoly/efgpoly.cc
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

#include "enumpoly.h"
#include "solvers/nashsupport/nashsupport.h"
#include "games/gameseq.h"
#include "polysystem.h"
#include "polysolver.h"
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
  BehaviorSupportProfile m_support;
  std::shared_ptr<VariableSpace> space;
  std::map<GameSequence, int> var;
  std::map<GameSequence, Polynomial<double>> variables;

  explicit ProblemData(const BehaviorSupportProfile &p_support);
};

Polynomial<double> BuildSequenceVariable(ProblemData &p_data, const GameSequence &p_sequence,
                                         const std::map<GameSequence, int> &var)
{
  if (!p_sequence->action) {
    return Polynomial<double>(p_data.space, 1);
  }
  if (p_sequence->action != p_data.m_support.GetActions(p_sequence->GetInfoset()).back()) {
    return Polynomial<double>(p_data.space, var.at(p_sequence), 1);
  }

  Polynomial<double> equation(p_data.space);
  for (auto seq : p_data.m_support.GetSequences(p_sequence->player)) {
    if (seq == p_sequence) {
      continue;
    }
    if (const int constraint_coef =
            p_data.m_support.GetConstraintEntry(p_sequence->GetInfoset(), seq->action)) {
      equation += BuildSequenceVariable(p_data, seq, var) * double(constraint_coef);
    }
  }
  return equation;
}

ProblemData::ProblemData(const BehaviorSupportProfile &p_support)
  : m_support(p_support), space(std::make_shared<VariableSpace>(m_support.GetSequences().size() -
                                                                m_support.GetInfosets().size() -
                                                                m_support.GetPlayers().size()))
{
  for (auto sequence : m_support.GetSequences()) {
    if (sequence->action &&
        (sequence->action != p_support.GetActions(sequence->GetInfoset()).back())) {
      var[sequence] = var.size() + 1;
    }
  }

  for (auto sequence : m_support.GetSequences()) {
    variables.emplace(sequence, BuildSequenceVariable(*this, sequence, var));
  }
}

Polynomial<double> GetPayoff(ProblemData &p_data, const GamePlayer &p_player)
{
  Polynomial<double> equation(p_data.space);

  for (auto profile : p_data.m_support.GetSequenceContingencies()) {
    auto pay = p_data.m_support.GetPayoff(profile, p_player);

    if (pay != Rational(0)) {
      Polynomial<double> term(p_data.space, double(pay));
      for (auto player : p_data.m_support.GetPlayers()) {
        term *= p_data.variables.at(profile[player]);
      }
      equation += term;
    }
  }
  return equation;
}

void IndifferenceEquations(ProblemData &p_data, PolynomialSystem<double> &p_equations)
{
  for (auto player : p_data.m_support.GetPlayers()) {
    const Polynomial<double> payoff = GetPayoff(p_data, player);
    for (auto sequence : p_data.m_support.GetSequences(player)) {
      try {
        p_equations.push_back(payoff.PartialDerivative(p_data.var.at(sequence)));
      }
      catch (std::out_of_range &) {
        // This sequence's variable was already substituted out in terms of
        // the probabilities of other sequences
      }
    }
  }
}

void LastActionProbPositiveInequalities(ProblemData &p_data, PolynomialSystem<double> &p_equations)
{
  for (auto sequence : p_data.m_support.GetSequences()) {
    if (!sequence->action) {
      continue;
    }
    const auto &actions = p_data.m_support.GetActions(sequence->action->GetInfoset());
    if (actions.size() > 1 && sequence->action == actions.back()) {
      p_equations.push_back(p_data.variables.at(sequence));
    }
  }
}

std::map<GameSequence, double> ToSequenceProbs(const ProblemData &p_data, const Vector<double> &v)
{
  std::map<GameSequence, double> x;
  for (auto sequence : p_data.m_support.GetSequences()) {
    x[sequence] = p_data.variables.at(sequence).Evaluate(v);
  }
  return x;
}

std::list<MixedBehaviorProfile<double>> SolveSupport(const BehaviorSupportProfile &p_support,
                                                     bool &p_isSingular, int p_stopAfter)
{
  ProblemData data(p_support);
  PolynomialSystem<double> equations(data.space);
  IndifferenceEquations(data, equations);
  // LastActionProbPositiveInequalities(data, equations); // Buggy: Forces x=0. Keep commented out.

  // BYPASS: If the support is pure (0 variables), bypass the numerical solver
  if (data.space->GetDimension() == 0) {
    std::list<MixedBehaviorProfile<double>> solutions;
    Vector<double> empty_root(0);
    const MixedBehaviorProfile<double> sol(
        data.m_support.ToMixedBehaviorProfile(ToSequenceProbs(data, empty_root)));
    if (ExtendsToNash(sol, BehaviorSupportProfile(sol.GetGame()),
                      BehaviorSupportProfile(sol.GetGame()))) {
      solutions.push_back(sol);
    }
    return solutions;
  }

  // set up the rectangle of search
  Vector<double> bottoms(data.space->GetDimension()), tops(data.space->GetDimension());
  bottoms = 0;
  tops = 1;

  PolynomialSystemSolver solver(equations);
  std::list<Vector<double>> roots;
  try {
    roots = solver.FindRoots({bottoms, tops},
                             (p_stopAfter > 0) ? p_stopAfter : std::numeric_limits<int>::max());
  }
  catch (const SingularMatrixException &) {
    p_isSingular = true;
  }
  catch (const std::domain_error &) {
    p_isSingular = true;
  }

  std::list<MixedBehaviorProfile<double>> solutions;
  for (auto root : roots) {
    const MixedBehaviorProfile<double> sol(
        data.m_support.ToMixedBehaviorProfile(ToSequenceProbs(data, root)));
    if (ExtendsToNash(sol, BehaviorSupportProfile(sol.GetGame()),
                      BehaviorSupportProfile(sol.GetGame()))) {
      solutions.push_back(sol);
    }
  }
  return solutions;
}

} // end anonymous namespace

namespace Gambit::Nash {

std::list<MixedBehaviorProfile<double>>
EnumPolyBehaviorSolve(const Game &p_game, int p_stopAfter, double p_maxregret,
                      EnumPolyMixedBehaviorObserverFunctionType p_onEquilibrium,
                      EnumPolyBehaviorSupportObserverFunctionType p_onSupport)
{
  const double scale = p_game->GetMaxPayoff() - p_game->GetMinPayoff();
  if (scale != 0.0) {
    p_maxregret *= scale;
  }

  std::list<MixedBehaviorProfile<double>> ret;
  auto possible_supports = PossibleNashBehaviorSupports(p_game);

  for (auto support : possible_supports->m_supports) {
    p_onSupport("candidate", support);
    bool isSingular = false;

    for (auto solution :
         SolveSupport(support, isSingular, std::max(p_stopAfter - int(ret.size()), 0))) {

      // Removed 'const' so we can normalize the missing unreached information sets
      MixedBehaviorProfile<double> fullProfile = solution.ToFullSupport();

      // FIX: Normalize unreached information sets that were pruned from the support.
      // This prevents GetAgentMaxRegret() from corrupting the math due to 0-sum probabilities.
      for (auto player : fullProfile.GetGame()->GetPlayers()) {
        if (player->IsChance()) continue;
        for (auto infoset : player->GetInfosets()) {
          double sum = 0;
          for (auto action : infoset->GetActions()) {
            sum += fullProfile.GetActionProb(action);
          }
          if (sum < 1e-6) {
            double uniform = 1.0 / infoset->GetActions().size();
            for (auto action : infoset->GetActions()) {
              fullProfile[action] = uniform;
            }
          }
        }
      }

      double current_regret = fullProfile.GetAgentMaxRegret();

      if (current_regret < p_maxregret) {
        p_onEquilibrium(fullProfile);
        ret.push_back(fullProfile);
      }
    }
    if (isSingular) {
      p_onSupport("singular", support);
    }
    if (p_stopAfter > 0 && static_cast<int>(ret.size()) >= p_stopAfter) {
      break;
    }
  }

  return ret;
}

} // namespace Gambit::Nash
