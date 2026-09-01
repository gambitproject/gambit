//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/lcp/efglcp.cc
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

#include "games.h"
#include "solvers/linalg/lemketab.h"
#include "solvers/lcp/lcp.h"

namespace Gambit::Nash {

namespace {

// Holds the correspondence between the sequences (and information sets) of
// a two-player game and the rows/columns of the sequence-form LCP
// tableau.
struct ColumnIndexMap {
  Game game;
  // The row/column of the tableau assigned to each sequence: player1's
  // occupy a contiguous block starting at 1, immediately followed by a
  // contiguous block for player2's.
  std::map<GameSequence, int> index;
  // The row/column of the tableau assigned to the sum-to-one constraint at
  // each information set.  Each player's block of these reserves one
  // extra ("root") index -- rootIndex1 and rootIndex2 below -- for the
  // constraint anchoring the probability of the empty sequence at 1.
  std::map<GameInfoset, int> infosetIndex;
  // The total number of rows/columns of the tableau, and the row/column
  // of each player's root anchor within it (see infosetIndex above).
  int total, rootIndex1, rootIndex2;

  explicit ColumnIndexMap(const Game &p_game) : game(p_game)
  {
    const GamePlayer player1 = game->GetPlayer(1);
    const GamePlayer player2 = game->GetPlayer(2);

    const int ns1 = static_cast<int>(player1->GetSequences().size());
    const int ns2 = static_cast<int>(player2->GetSequences().size());
    const int ni1 = static_cast<int>(player1->GetInfosets().size()) + 1;
    const int ni2 = static_cast<int>(player2->GetInfosets().size()) + 1;
    total = ns1 + ns2 + ni1 + ni2;
    rootIndex1 = ns1 + ns2 + 1;
    rootIndex2 = ns1 + ns2 + ni1 + 1;

    auto sequences1 = player1->GetSequences();
    for (auto [i, sequence] : enumerate(sequences1)) {
      index[sequence] = static_cast<int>(i) + 1;
    }
    auto sequences2 = player2->GetSequences();
    for (auto [i, sequence] : enumerate(sequences2)) {
      index[sequence] = ns1 + static_cast<int>(i) + 1;
    }

    // index 1 in each player's block is reserved for the root anchor
    auto infosets1 = player1->GetInfosets();
    for (auto [i, infoset] : enumerate(infosets1)) {
      infosetIndex[infoset] = ns1 + ns2 + static_cast<int>(i) + 2;
    }
    auto infosets2 = player2->GetInfosets();
    for (auto [i, infoset] : enumerate(infosets2)) {
      infosetIndex[infoset] = ns1 + ns2 + ni1 + static_cast<int>(i) + 2;
    }
  }
};

template <class T> Matrix<T> ConstructMatrix(const ColumnIndexMap &p_indexMap)
{
  Matrix<T> A(1, p_indexMap.total, 0, p_indexMap.total);
  A = T{0};

  const Game &game = p_indexMap.game;
  const GamePlayer player1 = game->GetPlayer(1);
  const GamePlayer player2 = game->GetPlayer(2);

  // A constant large enough that shifting every payoff down by it makes
  // all payoff entries of the matrix negative, as Lemke's algorithm requires.
  const Rational payoffShift = game->GetMaxPayoff() + Rational(1);

  // Payoff block: for every pair of sequences (one per player), the
  // payoff each player receives when that pair is exactly realised,
  // shifted by payoffShift and weighted by chance's probability of that
  // pair actually being realised (which need not be 1 -- see
  // PureSequenceProfile::GetRealizationProbability).
  for (auto profile : game->GetSequenceContingencies()) {
    const GameSequence &seq1 = profile.GetSequence(player1);
    const GameSequence &seq2 = profile.GetSequence(player2);
    const Rational prob = profile.GetRealizationProbability();
    const Rational pay1 = profile.GetPayoff(player1) - payoffShift * prob;
    const Rational pay2 = profile.GetPayoff(player2) - payoffShift * prob;
    A(p_indexMap.index.at(seq1), p_indexMap.index.at(seq2)) = static_cast<T>(pay1);
    A(p_indexMap.index.at(seq2), p_indexMap.index.at(seq1)) = static_cast<T>(pay2);
  }

  // Constraint block: for each information set, the sum-to-one relation
  // between the probability of the sequence leading to it and the sum of
  // the probabilities of its own actions' sequences.
  for (auto player : game->GetPlayers()) {
    for (const auto &infoset : player->GetInfosets()) {
      const int infosetIdx = p_indexMap.infosetIndex.at(infoset);
      const auto children = infoset->GetSequences();
      const int arrivalIdx = p_indexMap.index.at(children.front()->GetParent());
      A(arrivalIdx, infosetIdx) = T{-1};
      A(infosetIdx, arrivalIdx) = T{1};
      for (const auto &child : children) {
        const int childIdx = p_indexMap.index.at(child);
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
  const GameSequence root1 = player1->GetSequences().front();
  const GameSequence root2 = player2->GetSequences().front();
  A(p_indexMap.index.at(root1), p_indexMap.rootIndex1) = T{1};
  A(p_indexMap.rootIndex1, p_indexMap.index.at(root1)) = T{-1};
  A(p_indexMap.index.at(root2), p_indexMap.rootIndex2) = T{1};
  A(p_indexMap.rootIndex2, p_indexMap.index.at(root2)) = T{-1};

  return A;
}

template <class T> Vector<T> ConstructVector(const ColumnIndexMap &p_indexMap)
{
  Vector<T> b(1, p_indexMap.total);
  b = T{0};
  b[p_indexMap.rootIndex1] = T{-1};
  b[p_indexMap.rootIndex2] = T{-1};
  return b;
}

template <class T>
MixedBehaviorProfile<T> GetProfile(const linalg::LemkeTableau<T> &tab, const Vector<T> &sol,
                                   const ColumnIndexMap &p_indexMap)
{
  const T eps = tab.GetZeroTolerance();
  std::map<GameSequence, T> x;
  for (const auto &[sequence, idx] : p_indexMap.index) {
    T value{0};
    if (tab.IsMember(idx)) {
      const T candidate = sol[tab.GetPosition(idx)];
      if (candidate > eps) {
        value = candidate;
      }
    }
    x[sequence] = value;
  }
  return MixedBehaviorProfile<T>(MixedSequenceProfile<T>(p_indexMap.game, x));
}

} // end anonymous namespace

//
// Lemke implements Lemke's algorithm for linear complementarity problems,
// as refined by Eaves for degenerate problems, starting from the primary ray.
//
template <class T>
std::list<MixedBehaviorProfile<T>> LcpBehaviorSolve(const Game &p_game,
                                                    BehaviorCallbackType<T> p_onEquilibrium,
                                                    const CancelToken &p_cancel)
{
  if (p_game->NumPlayers() != 2) {
    throw UndefinedException("Method only valid for two-player games.");
  }
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }

  const ColumnIndexMap columns(p_game);
  linalg::LemkeTableau<T> tab(ConstructMatrix<T>(columns), ConstructVector<T>(columns));

  tab.Pivot(columns.rootIndex1, 0);
  tab.SF_LCPPath(columns.rootIndex1, p_cancel);
  Vector<T> sol(tab.MinRow(), tab.MaxRow());
  tab.GetBasisVector(sol);

  MixedBehaviorProfile<T> profile = GetProfile(tab, sol, columns);
  profile.UndefinedToCentroid();
  p_onEquilibrium(profile);

  std::list<MixedBehaviorProfile<T>> equilibria;
  equilibria.push_back(profile);
  return equilibria;
}

template std::list<MixedBehaviorProfile<double>>
LcpBehaviorSolve(const Game &, BehaviorCallbackType<double>, const CancelToken &);
template std::list<MixedBehaviorProfile<Rational>>
LcpBehaviorSolve(const Game &, BehaviorCallbackType<Rational>, const CancelToken &);

} // end namespace Gambit::Nash
