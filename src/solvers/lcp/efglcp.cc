//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/lcp/efglcp.cc
// Implementation of algorithm to solve extensive forms using linear
// complementarity program from sequence form
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

#include "gambit.h"
#include "solvers/linalg/lemketab.h"
#include "solvers/lcp/lcp.h"

namespace Gambit::Nash {

namespace {

// Holds the correspondence between the sequences (and information sets) of
// a two-player game and the rows/columns of the sequence-form LCP
// tableau, along with the quantities (payoffs, tree-of-sequences
// relationships) needed to populate it.
struct Solution {
  BehaviorSupportProfile support;
  GamePlayer player1, player2;
  GameSequence root1, root2;
  int ns1, ns2, ni1, ni2;
  // The row/column of the tableau assigned to each sequence: player1's
  // occupy 1..ns1, and player2's occupy ns1+1..ns1+ns2.
  std::map<GameSequence, int> index;
  // The row/column of the tableau assigned to the sum-to-one constraint at
  // each information set.  Each player's block of these reserves one
  // extra ("root") index -- see RootIndex1()/RootIndex2() -- for the
  // constraint anchoring the probability of the empty sequence at 1.
  std::map<GameInfoset, int> infosetIndex;
  // The sequences of the actions available at an information set, i.e.
  // the children, in the tree of sequences, of the sequence that leads to
  // it.  This is what expresses the sum-to-one relation among the actions
  // at an information set.
  std::map<GameInfoset, std::vector<GameSequence>> siblings;

  explicit Solution(const Game &p_game)
    : support(p_game), player1(p_game->GetPlayer(1)), player2(p_game->GetPlayer(2)),
      root1(player1->GetSequences().front()), root2(player2->GetSequences().front()),
      ns1(static_cast<int>(support.GetSequences(player1).size())),
      ns2(static_cast<int>(support.GetSequences(player2).size())),
      ni1(static_cast<int>(player1->GetInfosets().size()) + 1),
      ni2(static_cast<int>(player2->GetInfosets().size()) + 1)
  {
    int idx = 1;
    for (auto sequence : support.GetSequences(player1)) {
      index[sequence] = idx++;
    }
    idx = 1;
    for (auto sequence : support.GetSequences(player2)) {
      index[sequence] = ns1 + idx++;
    }

    idx = 2; // index 1 in each player's block is reserved for the root anchor
    for (const auto &infoset : player1->GetInfosets()) {
      infosetIndex[infoset] = ns1 + ns2 + idx++;
    }
    idx = 2;
    for (const auto &infoset : player2->GetInfosets()) {
      infosetIndex[infoset] = ns1 + ns2 + ni1 + idx++;
    }

    for (auto player : {player1, player2}) {
      for (auto sequence : support.GetSequences(player)) {
        if (sequence->GetAction()) {
          siblings[sequence->GetInfoset()].push_back(sequence);
        }
      }
    }
  }

  int Total() const { return ns1 + ns2 + ni1 + ni2; }
  int RootIndex1() const { return ns1 + ns2 + 1; }
  int RootIndex2() const { return ns1 + ns2 + ni1 + 1; }
};

template <class T> Matrix<T> ConstructMatrix(const Solution &p_solution)
{
  Matrix<T> A(1, p_solution.Total(), 0, p_solution.Total());
  A = T{0};

  // A constant large enough that shifting every payoff down by it makes
  // all payoff entries of the matrix negative, as Lemke's algorithm requires.
  const Rational payoffShift = p_solution.support.GetGame()->GetMaxPayoff() + Rational(1);

  // Payoff block: for every pair of sequences (one per player), the
  // payoff each player receives when that pair is exactly realised,
  // shifted by payoffShift and weighted by chance's probability of that
  // pair actually being realised (which need not be 1 -- see
  // PureSequenceProfile::GetRealizationProbability).
  for (auto profile : p_solution.support.GetSequenceContingencies()) {
    const GameSequence &seq1 = profile.GetSequence(p_solution.player1);
    const GameSequence &seq2 = profile.GetSequence(p_solution.player2);
    const Rational prob = profile.GetRealizationProbability();
    const Rational pay1 = profile.GetPayoff(p_solution.player1) - payoffShift * prob;
    const Rational pay2 = profile.GetPayoff(p_solution.player2) - payoffShift * prob;
    A(p_solution.index.at(seq1), p_solution.index.at(seq2)) = static_cast<T>(pay1);
    A(p_solution.index.at(seq2), p_solution.index.at(seq1)) = static_cast<T>(pay2);
  }

  // Constraint block: for each information set, the sum-to-one relation
  // between the probability of the sequence leading to it and the sum of
  // the probabilities of its own actions' sequences.
  for (auto player : {p_solution.player1, p_solution.player2}) {
    for (const auto &infoset : player->GetInfosets()) {
      const int infosetIdx = p_solution.infosetIndex.at(infoset);
      const auto &children = p_solution.siblings.at(infoset);
      const int arrivalIdx = p_solution.index.at(children.front()->GetParent());
      A(arrivalIdx, infosetIdx) = T{-1};
      A(infosetIdx, arrivalIdx) = T{1};
      for (const auto &child : children) {
        const int childIdx = p_solution.index.at(child);
        A(childIdx, infosetIdx) = T{1};
        A(infosetIdx, childIdx) = T{-1};
      }
    }
  }

  // Column 0 and the two "root anchor" entries are the standard
  // sequence-form LCP fixtures that anchor the probability of each
  // player's empty sequence at 1.
  for (int i = A.MinRow(); i <= A.MaxRow(); i++) {
    A(i, 0) = T{-1};
  }
  A(p_solution.index.at(p_solution.root1), p_solution.RootIndex1()) = T{1};
  A(p_solution.RootIndex1(), p_solution.index.at(p_solution.root1)) = T{-1};
  A(p_solution.index.at(p_solution.root2), p_solution.RootIndex2()) = T{1};
  A(p_solution.RootIndex2(), p_solution.index.at(p_solution.root2)) = T{-1};

  return A;
}

template <class T> Vector<T> ConstructVector(const Solution &p_solution)
{
  Vector<T> b(1, p_solution.Total());
  b = T{0};
  b[p_solution.RootIndex1()] = T{-1};
  b[p_solution.RootIndex2()] = T{-1};
  return b;
}

template <class T>
MixedBehaviorProfile<T> GetProfile(const linalg::LemkeTableau<T> &tab, const Vector<T> &sol,
                                   const Solution &p_solution)
{
  const T eps = tab.Epsilon();
  std::map<GameSequence, T> x;
  for (const auto &[sequence, idx] : p_solution.index) {
    T value{0};
    if (tab.Member(idx)) {
      const T candidate = sol[tab.Find(idx)];
      if (candidate > eps) {
        value = candidate;
      }
    }
    x[sequence] = value;
  }
  return p_solution.support.ToMixedBehaviorProfile(x);
}

} // end anonymous namespace

//
// Lemke implements Lemke's algorithm for linear complementarity problems,
// as refined by Eaves for degenerate problems, starting from the primary ray.
//
template <class T>
std::list<MixedBehaviorProfile<T>> LcpBehaviorSolve(const Game &p_game,
                                                    BehaviorCallbackType<T> p_onEquilibrium)
{
  if (p_game->NumPlayers() != 2) {
    throw UndefinedException("Method only valid for two-player games.");
  }
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }

  Solution solution(p_game);
  linalg::LemkeTableau<T> tab(ConstructMatrix<T>(solution), ConstructVector<T>(solution));

  tab.Pivot(solution.RootIndex1(), 0);
  tab.SF_LCPPath(solution.RootIndex1());
  Vector<T> sol(tab.MinRow(), tab.MaxRow());
  tab.BasisVector(sol);

  MixedBehaviorProfile<T> profile = GetProfile(tab, sol, solution);
  profile.UndefinedToCentroid();
  p_onEquilibrium(profile);

  std::list<MixedBehaviorProfile<T>> equilibria;
  equilibria.push_back(profile);
  return equilibria;
}

template std::list<MixedBehaviorProfile<double>> LcpBehaviorSolve(const Game &,
                                                                  BehaviorCallbackType<double>);
template std::list<MixedBehaviorProfile<Rational>>
LcpBehaviorSolve(const Game &, BehaviorCallbackType<Rational>);

} // end namespace Gambit::Nash
