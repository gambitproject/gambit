//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/solvers/lp/lp.cc
// Implementation of algorithm to solve efgs via linear programming
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
#include "solvers/lp/lp.h"
#include "solvers/linalg/lpsolve.h"

namespace Gambit::Nash {

namespace {

// Holds the correspondence between the sequences (and information sets) of
// a two-player constant-sum game and the rows/columns of the sequence-form
// LP tableau.  Player 1's sequences occupy rows and
// player 2's occupy columns; each player's information sets occupy a
// block on the *other* axis, expressing the sum-to-one relation among
// that player's own sequences.
struct TableauIndexMap {
  Game game;
  // The row assigned to each of player 1's sequences (1..rowAnchor-1), and
  // the column assigned to each of player 2's sequences (1..colAnchor-1).
  std::map<GameSequence, int> rowIndex, colIndex;
  // The column assigned to the sum-to-one constraint at each of player 1's
  // information sets, and the row assigned to the same for player 2's.
  // Each block reserves its first (colAnchor/rowAnchor) index for the
  // constraint anchoring the probability of the empty sequence at 1.
  std::map<GameInfoset, int> colInfosetIndex, rowInfosetIndex;
  // The number of rows/columns of the tableau, the row/column of each
  // player's information-set block anchor, and the number of rows (all in
  // player 2's block) that hold with equality rather than inequality.
  int rows, cols, rowAnchor, colAnchor, equalityRows;

  explicit TableauIndexMap(const Game &p_game) : game(p_game)
  {
    const GamePlayer player1 = game->GetPlayer(1);
    const GamePlayer player2 = game->GetPlayer(2);

    const int ns1 = static_cast<int>(player1->GetSequences().size());
    const int ns2 = static_cast<int>(player2->GetSequences().size());
    const int ni1 = static_cast<int>(player1->GetInfosets().size()) + 1;
    const int ni2 = static_cast<int>(player2->GetInfosets().size()) + 1;
    rows = ns1 + ni2;
    cols = ns2 + ni1;
    rowAnchor = ns1 + 1;
    colAnchor = ns2 + 1;
    equalityRows = ni2;

    auto sequences1 = player1->GetSequences();
    for (auto [i, sequence] : enumerate(sequences1)) {
      rowIndex[sequence] = static_cast<int>(i) + 1;
    }
    auto sequences2 = player2->GetSequences();
    for (auto [i, sequence] : enumerate(sequences2)) {
      colIndex[sequence] = static_cast<int>(i) + 1;
    }

    // index 1 in each information-set block is reserved for the anchor
    auto infosets1 = player1->GetInfosets();
    for (auto [i, infoset] : enumerate(infosets1)) {
      colInfosetIndex[infoset] = ns2 + static_cast<int>(i) + 2;
    }
    auto infosets2 = player2->GetInfosets();
    for (auto [i, infoset] : enumerate(infosets2)) {
      rowInfosetIndex[infoset] = ns1 + static_cast<int>(i) + 2;
    }
  }
};

template <class T> Matrix<T> ConstructMatrix(const TableauIndexMap &p_indexMap)
{
  Matrix<T> A(1, p_indexMap.rows, 1, p_indexMap.cols);
  A = T{0};

  const Game &game = p_indexMap.game;
  const GamePlayer player1 = game->GetPlayer(1);
  const GamePlayer player2 = game->GetPlayer(2);

  // A constant subtracted from every payoff so that all entries of the
  // payoff block become non-negative, as this LP's standard form requires.
  const Rational payoffShift = game->GetMinPayoff();

  // Payoff block: for every pair of sequences (one per player), player 1's
  // payoff when that pair is exactly realised, shifted by payoffShift and
  // weighted by chance's probability of that pair actually being realised
  // (which need not be 1 -- see PureSequenceProfile::GetRealizationProbability).
  // Player 2's payoff is not separately represented, since the game is
  // constant-sum.
  for (auto profile : game->GetSequenceContingencies()) {
    const GameSequence &seq1 = profile.GetSequence(player1);
    const GameSequence &seq2 = profile.GetSequence(player2);
    const Rational prob = profile.GetRealizationProbability();
    const Rational pay1 = profile.GetPayoff(player1) - payoffShift * prob;
    A(p_indexMap.rowIndex.at(seq1), p_indexMap.colIndex.at(seq2)) = static_cast<T>(pay1);
  }

  // Constraint block for player 1: the sum-to-one relation between the
  // probability of the sequence leading to an information set and the
  // sum of the probabilities of its own actions' sequences, expressed in
  // the column reserved for that information set.
  for (const auto &infoset : player1->GetInfosets()) {
    const int col = p_indexMap.colInfosetIndex.at(infoset);
    const auto children = infoset->GetSequences();
    const int arrivalRow = p_indexMap.rowIndex.at(children.front()->GetParent());
    A(arrivalRow, col) = T{1};
    for (const auto &child : children) {
      A(p_indexMap.rowIndex.at(child), col) = T{-1};
    }
  }

  // Likewise for player 2, expressed in the row reserved for each of
  // their information sets.
  for (const auto &infoset : player2->GetInfosets()) {
    const int row = p_indexMap.rowInfosetIndex.at(infoset);
    const auto children = infoset->GetSequences();
    const int arrivalCol = p_indexMap.colIndex.at(children.front()->GetParent());
    A(row, arrivalCol) = T{-1};
    for (const auto &child : children) {
      A(row, p_indexMap.colIndex.at(child)) = T{1};
    }
  }

  // The two "anchor" entries are the standard sequence-form LP fixtures
  // that link each player's own root sequence to the other player's
  // information-set block, anchoring the probability of the empty
  // sequence at 1.
  const GameSequence root1 = player1->GetSequences().front();
  const GameSequence root2 = player2->GetSequences().front();
  A(p_indexMap.rowIndex.at(root1), p_indexMap.colAnchor) = T{-1};
  A(p_indexMap.rowAnchor, p_indexMap.colIndex.at(root2)) = T{1};

  return A;
}

template <class T> Vector<T> ConstructB(const TableauIndexMap &p_indexMap)
{
  Vector<T> b(1, p_indexMap.rows);
  b = T{0};
  b[p_indexMap.rowAnchor] = T{1};
  return b;
}

template <class T> Vector<T> ConstructC(const TableauIndexMap &p_indexMap)
{
  Vector<T> c(1, p_indexMap.cols);
  c = T{0};
  c[p_indexMap.colAnchor] = T{-1};
  return c;
}

template <class T>
MixedBehaviorProfile<T> GetBehavior(const Array<T> &p_primal, const Array<T> &p_dual,
                                    const TableauIndexMap &p_indexMap)
{
  std::map<GameSequence, T> x;
  for (const auto &[sequence, row] : p_indexMap.rowIndex) {
    x[sequence] = p_dual[row];
  }
  for (const auto &[sequence, col] : p_indexMap.colIndex) {
    x[sequence] = p_primal[col];
  }
  return MixedBehaviorProfile<T>(MixedSequenceProfile<T>(p_indexMap.game, x));
}

} // end anonymous namespace

//
// The routine to actually solve the LP
// This routine takes an LP of the form
//    maximize c x subject to Ax>=b and x>=0,
// except the last 'nequals' constraints in A hold with equality.
// It expects the array p_primal to be the same length as the
// number of columns in A, and the routine returns the primal solution;
// similarly, the array p_dual should have the same length as the
// number of rows in A, and the routine returns the dual solution.
//
// To implement your own custom solver for this problem, simply
// replace this function.
//
template <class T>
void SolveLP(const Matrix<T> &A, const Vector<T> &b, const Vector<T> &c, int nequals,
             Array<T> &p_primal, Array<T> &p_dual, const CancelToken &p_cancel = CancelToken())
{
  const linalg::LPSolve<T> LP(A, b, c, nequals, p_cancel);
  const auto &cbfs = LP.OptimumBFS();

  for (size_t i = 1; i <= A.NumColumns(); i++) {
    p_primal[i] = (cbfs.count(i)) ? cbfs[i] : static_cast<T>(0);
  }
  for (size_t i = 1; i <= A.NumRows(); i++) {
    p_dual[i] = (cbfs.count(-i)) ? cbfs[-i] : static_cast<T>(0);
  }
}

template <class T>
std::list<MixedBehaviorProfile<T>> LpBehaviorSolve(const Game &p_game,
                                                   BehaviorCallbackType<T> p_onEquilibrium,
                                                   const CancelToken &p_cancel)
{
  if (p_game->NumPlayers() != 2) {
    throw UndefinedException("Method only valid for two-player games.");
  }
  if (!p_game->IsConstSum()) {
    throw UndefinedException("Method only valid for constant-sum games.");
  }
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }

  const TableauIndexMap indexMap(p_game);
  const Matrix<T> A = ConstructMatrix<T>(indexMap);
  const Vector<T> b = ConstructB<T>(indexMap);
  const Vector<T> c = ConstructC<T>(indexMap);

  Array<T> primal(A.NumColumns()), dual(A.NumRows());
  SolveLP(A, b, c, indexMap.equalityRows, primal, dual, p_cancel);

  MixedBehaviorProfile<T> profile = GetBehavior(primal, dual, indexMap);
  profile.UndefinedToCentroid();
  p_onEquilibrium(profile);

  std::list<MixedBehaviorProfile<T>> solution;
  solution.push_back(profile);
  return solution;
}

template std::list<MixedBehaviorProfile<double>>
LpBehaviorSolve(const Game &, BehaviorCallbackType<double>, const CancelToken &);
template std::list<MixedBehaviorProfile<Rational>>
LpBehaviorSolve(const Game &, BehaviorCallbackType<Rational>, const CancelToken &);

template <class T>
std::list<MixedStrategyProfile<T>> LpStrategySolve(const Game &p_game,
                                                   StrategyCallbackType<T> p_onEquilibrium,
                                                   const CancelToken &p_cancel)
{
  if (p_game->NumPlayers() != 2) {
    throw UndefinedException("Method only valid for two-player games.");
  }
  if (!p_game->IsConstSum()) {
    throw UndefinedException("Method only valid for constant-sum games.");
  }
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }

  const int m = p_game->GetPlayer(1)->GetStrategies().size();
  const int k = p_game->GetPlayer(2)->GetStrategies().size();

  Matrix<T> A(1, k + 1, 1, m + 1);
  Vector<T> b(1, k + 1);
  Vector<T> c(1, m + 1);
  const PureStrategyProfile profile = p_game->NewPureStrategyProfile();

  const Rational minpay = p_game->GetMinPayoff() - Rational(1);

  for (int i = 1; i <= k; i++) {
    profile->SetStrategy(p_game->GetPlayer(2)->GetStrategy(i));
    for (int j = 1; j <= m; j++) {
      profile->SetStrategy(p_game->GetPlayer(1)->GetStrategy(j));
      A(i, j) = minpay - profile->GetPayoff(p_game->GetPlayer(1));
    }
    A(i, m + 1) = static_cast<T>(1);
  }
  for (int j = 1; j <= m; j++) {
    A(k + 1, j) = static_cast<T>(1);
  }
  A(k + 1, m + 1) = static_cast<T>(0);

  b = static_cast<T>(0);
  b[k + 1] = static_cast<T>(1);
  c = static_cast<T>(0);
  c[m + 1] = static_cast<T>(1);

  Array<T> primal(A.NumColumns()), dual(A.NumRows());
  SolveLP(A, b, c, 1, primal, dual, p_cancel);

  MixedStrategyProfile<T> eqm(p_game->NewMixedStrategyProfile(static_cast<T>(0)));
  for (int j = 1; j <= m; j++) {
    eqm[p_game->GetPlayer(1)->GetStrategy(j)] = primal[j];
  }
  for (int j = 1; j <= k; j++) {
    eqm[p_game->GetPlayer(2)->GetStrategy(j)] = dual[j];
  }
  p_onEquilibrium(eqm);
  std::list<MixedStrategyProfile<T>> solution;
  solution.push_back(eqm);
  return solution;
}

template std::list<MixedStrategyProfile<double>>
LpStrategySolve(const Game &, StrategyCallbackType<double>, const CancelToken &);
template std::list<MixedStrategyProfile<Rational>>
LpStrategySolve(const Game &, StrategyCallbackType<Rational>, const CancelToken &);

} // end namespace Gambit::Nash
