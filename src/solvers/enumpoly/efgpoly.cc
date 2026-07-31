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
#include "polysystem.h"
#include "polysolver.h"
#include "indexproduct.h"

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
  // The sequences of the actions available at an information set, i.e. the
  // children, in the tree of sequences, of the sequence that leads to it.
  // (Grouping by information set rather than directly by parent sequence
  // matters: the same parent sequence can lead to different information
  // sets, e.g. depending on an intervening chance move or another player's
  // action.)  This is what expresses the sum-to-one relation among the
  // actions at an information set.
  std::map<GameInfoset, std::vector<GameSequence>> siblings;

  explicit ProblemData(const BehaviorSupportProfile &p_support);
};

// A sequence's variable is eliminated in favour of the other sequences at
// its information set exactly when it is the last (in the support's
// ordering) action at that information set.
bool IsEliminated(const BehaviorSupportProfile &p_support, const GameSequence &p_sequence)
{
  return p_sequence->GetAction() == p_support.GetActions(p_sequence->GetInfoset()).back();
}

Polynomial<double> BuildSequenceVariable(ProblemData &p_data, const GameSequence &p_sequence)
{
  if (!p_sequence->GetAction()) {
    return Polynomial<double>(p_data.space, 1);
  }
  if (!IsEliminated(p_data.m_support, p_sequence)) {
    return Polynomial<double>(p_data.space, p_data.var.at(p_sequence), 1);
  }

  // The last action at an information set is eliminated using the
  // sum-to-one relation among the sequences at that information set: its
  // probability is that of its parent sequence, less the probabilities of
  // its sibling sequences (the other actions at the same information set).
  Polynomial<double> equation = BuildSequenceVariable(p_data, p_sequence->GetParent());
  for (const auto &sibling : p_data.siblings.at(p_sequence->GetInfoset())) {
    if (sibling != p_sequence) {
      equation -= BuildSequenceVariable(p_data, sibling);
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
    if (sequence->GetAction()) {
      siblings[sequence->GetInfoset()].push_back(sequence);
      if (!IsEliminated(m_support, sequence)) {
        var[sequence] = var.size() + 1;
      }
    }
  }

  for (auto sequence : m_support.GetSequences()) {
    variables.emplace(sequence, BuildSequenceVariable(*this, sequence));
  }
}

Polynomial<double> GetPayoff(ProblemData &p_data, const GamePlayer &p_player)
{
  Polynomial<double> equation(p_data.space);

  for (auto profile : p_data.m_support.GetSequenceContingencies()) {
    auto pay = profile.GetPayoff(p_player);
    if (pay != Rational(0)) {
      Polynomial<double> term(p_data.space, pay);
      for (auto player : p_data.m_support.GetPlayers()) {
        term *= p_data.variables.at(profile.GetSequence(player));
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
      if (auto it = p_data.var.find(sequence); it != p_data.var.end()) {
        p_equations.push_back(payoff.PartialDerivative(it->second));
      }
      // Sequences with no entry in p_data.var have had their variable
      // substituted out in terms of the probabilities of other sequences.
    }
  }
}

void LastActionProbPositiveInequalities(ProblemData &p_data, PolynomialSystem<double> &p_equations)
{
  for (auto sequence : p_data.m_support.GetSequences()) {
    if (!sequence->GetAction()) {
      continue;
    }
    if (p_data.m_support.GetActions(sequence->GetInfoset()).size() > 1 &&
        IsEliminated(p_data.m_support, sequence)) {
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

std::optional<MixedBehaviorProfile<double>>
FindNashExtension(const MixedBehaviorProfile<double> &p_baseProfile, double p_maxRegret)
{
  const Game &game = p_baseProfile.GetGame();
  std::list<GameInfoset> extensionInfosets;
  for (const auto &infoset : game->GetInfosets()) {
    if (!p_baseProfile.IsDefinedAt(infoset)) {
      extensionInfosets.push_back(infoset);
    }
  }
  Array<int> firstIndex(extensionInfosets.size());
  std::fill(firstIndex.begin(), firstIndex.end(), 1);
  Array<int> lastIndex(extensionInfosets.size());
  std::transform(extensionInfosets.begin(), extensionInfosets.end(), lastIndex.begin(),
                 [](const auto &infoset) { return infoset->GetActions().size(); });
  CartesianIndexProduct indices(firstIndex, lastIndex);
  for (const auto &index : indices) {
    auto extension = p_baseProfile.ToFullSupport();
    for (auto [i, infoset] : enumerate(extensionInfosets)) {
      extension[infoset->GetAction(index[i + 1])] = 1.0;
    }
    if (extension.GetMaxRegret() < p_maxRegret) {
      return extension;
    }
  }
  return std::nullopt;
}

std::list<MixedBehaviorProfile<double>> SolveSupport(const BehaviorSupportProfile &p_support,
                                                     bool &p_isSingular, int p_stopAfter,
                                                     double p_maxRegret)
{
  ProblemData data(p_support);
  PolynomialSystem<double> equations(data.space);
  IndifferenceEquations(data, equations);
  LastActionProbPositiveInequalities(data, equations);

  // set up the rectangle of search
  Vector<double> bottoms(data.space->GetDimension()), tops(data.space->GetDimension());
  bottoms = 1e-12;
  tops = 1 - 1e-12;

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
  for (const auto &root : roots) {
    const MixedBehaviorProfile<double> sol(
        data.m_support.ToMixedBehaviorProfile(ToSequenceProbs(data, root)));
    auto extended = FindNashExtension(sol, p_maxRegret);
    if (extended.has_value()) {
      solutions.push_back(extended.value());
    }
  }
  return solutions;
}

} // end anonymous namespace

namespace Gambit::Nash {

std::list<MixedBehaviorProfile<double>>
EnumPolyBehaviorSolve(const Game &p_game, int p_stopAfter, double p_maxregret,
                      BehaviorCallbackType<double> p_onEquilibrium,
                      EnumPolyEventCallbackType<BehaviorSupportProfile> p_onEvent)
{
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }

  const double scale = p_game->GetMaxPayoff() - p_game->GetMinPayoff();
  if (scale != 0.0) {
    p_maxregret *= scale;
  }

  std::list<MixedBehaviorProfile<double>> ret;
  auto possible_supports = PossibleNashBehaviorSupports(p_game);

  for (auto support : possible_supports->m_supports) {
    p_onEvent(EnumPolyCandidateSupportEvent<BehaviorSupportProfile>{support});
    bool isSingular = false;
    for (const auto &solution :
         SolveSupport(support, isSingular, std::max(p_stopAfter - static_cast<int>(ret.size()), 0),
                      p_maxregret)) {
      p_onEquilibrium(solution);
      ret.push_back(solution);
    }
    if (isSingular) {
      p_onEvent(EnumPolySingularSupportEvent<BehaviorSupportProfile>{support});
    }
    if (p_stopAfter > 0 && static_cast<int>(ret.size()) >= p_stopAfter) {
      break;
    }
  }
  return ret;
}

} // namespace Gambit::Nash
