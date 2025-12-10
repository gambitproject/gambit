//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/tools/lp/efglp.cc
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

#include "gambit.h"
#include "solvers/lp/lp.h"
#include "solvers/linalg/lpsolve.h"
#include "games/gameseq.h"

namespace Gambit::Nash {

template <class T> class GameData {
public:
  int ns1, ns2;
  Rational minpay;
  std::map<GameInfoset, int> infosetOffset;

  explicit GameData(const Game &);

  void FillTableau(Matrix<T> &A, const Game &p_game);

  MixedBehaviorProfile<T> GetBehavior(const Array<T> &p_primal, const Array<T> &p_dual,
                                      const Game &p_game);
};

template <class T> GameData<T>::GameData(const Game &p_game) : minpay(p_game->GetMinPayoff())
{
  ns1 = p_game->GetPlayer(1)->NumSequences();
  ns2 = p_game->GetPlayer(2)->NumSequences();
  for (const auto &player : p_game->GetPlayers()) {
    int offset = 1;
    for (const auto &infoset : player->GetInfosets()) {
      infosetOffset[infoset] = offset;
      offset += infoset->GetActions().size();
    }
  }
}

template <class T> void GameData<T>::FillTableau(Matrix<T> &A, const Game &p_game)
{
  auto player1 = p_game->GetPlayer(1);
  auto player2 = p_game->GetPlayer(2);
  auto sequences1 = p_game->GetSequences(player1);
  auto sequences2 = p_game->GetSequences(player2);
  for (auto seq : sequences1) {
    auto parentSeq = seq->parent.lock();
    if (parentSeq) {
      const int col = ns2 + seq->GetInfoset()->GetNumber() + 1;
      const int row = seq->number;
      const int parentRow = parentSeq->number;
      A(parentRow, col) = static_cast<T>(1);
      A(row, col) = static_cast<T>(-1);
    }
  }
  for (auto seq : sequences2) {
    auto parentSeq = seq->parent.lock();
    if (parentSeq) {
      const int row = ns1 + seq->GetInfoset()->GetNumber() + 1;
      const int col = seq->number;
      const int parentCol = parentSeq->number;
      A(row, parentCol) = static_cast<T>(-1);
      A(row, col) = static_cast<T>(1);
    }
  }
  for (auto seq1 : sequences1) {
    for (auto seq2 : sequences2) {
      const int row = seq1->number;
      const int col = seq2->number;
      std::map<GamePlayer, GameSequence> profile;
      profile[player1] = seq1;
      profile[player2] = seq2;
      A(row, col) =
          p_game->GetPayoff(profile, player1) - (minpay * p_game->GetTerminalProb(profile));
    }
  }
}

template <class T>
MixedBehaviorProfile<T> GameData<T>::GetBehavior(const Array<T> &p_primal, const Array<T> &p_dual,
                                                 const Game &p_game)
{
  auto player1 = p_game->GetPlayer(1);
  auto player2 = p_game->GetPlayer(2);
  auto sequences1 = p_game->GetSequences(player1);
  auto sequences2 = p_game->GetSequences(player2);
  Gambit::MixedSequenceProfile<T> msp(p_game);
  for (auto seq : sequences1) {
    const int index = seq->number;
    msp[seq] = p_dual[index];
  }
  for (auto seq : sequences2) {
    const int index = seq->number;
    msp[seq] = p_primal[index];
  }
  return msp.GetMixedBehaviorProfile();
}

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
             Array<T> &p_primal, Array<T> &p_dual)
{
  const linalg::LPSolve<T> LP(A, b, c, nequals);
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
                                                   BehaviorCallbackType<T> p_onEquilibrium)
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

  GameData<T> data(p_game);

  Matrix<T> A(1, data.ns1 + p_game->GetPlayer(2)->GetInfosets().size() + 1, 1,
              data.ns2 + p_game->GetPlayer(1)->GetInfosets().size() + 1);
  Vector<T> b(1, data.ns1 + p_game->GetPlayer(2)->GetInfosets().size() + 1);
  Vector<T> c(1, data.ns2 + p_game->GetPlayer(1)->GetInfosets().size() + 1);

  A = static_cast<T>(0);
  b = static_cast<T>(0);
  c = static_cast<T>(0);

  data.FillTableau(A, p_game);
  A(1, data.ns2 + 1) = static_cast<T>(-1);
  A(data.ns1 + 1, 1) = static_cast<T>(1);

  b[data.ns1 + 1] = static_cast<T>(1);
  c[data.ns2 + 1] = static_cast<T>(-1);

  Array<T> primal(A.NumColumns()), dual(A.NumRows());
  std::list<MixedBehaviorProfile<T>> solution;
  SolveLP(A, b, c, p_game->GetPlayer(2)->GetInfosets().size() + 1, primal, dual);
  MixedBehaviorProfile<T> profile = data.GetBehavior(primal, dual, p_game);
  profile.UndefinedToCentroid();
  p_onEquilibrium(profile, "NE");
  solution.push_back(profile);
  return solution;
}

template std::list<MixedBehaviorProfile<double>> LpBehaviorSolve(const Game &,
                                                                 BehaviorCallbackType<double>);
template std::list<MixedBehaviorProfile<Rational>> LpBehaviorSolve(const Game &,
                                                                   BehaviorCallbackType<Rational>);

template <class T>
std::list<MixedStrategyProfile<T>> LpStrategySolve(const Game &p_game,
                                                   StrategyCallbackType<T> p_onEquilibrium)
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
  SolveLP(A, b, c, 1, primal, dual);

  MixedStrategyProfile<T> eqm(p_game->NewMixedStrategyProfile(static_cast<T>(0)));
  for (int j = 1; j <= m; j++) {
    eqm[p_game->GetPlayer(1)->GetStrategy(j)] = primal[j];
  }
  for (int j = 1; j <= k; j++) {
    eqm[p_game->GetPlayer(2)->GetStrategy(j)] = dual[j];
  }
  p_onEquilibrium(eqm, "NE");
  std::list<MixedStrategyProfile<T>> solution;
  solution.push_back(eqm);
  return solution;
}

template std::list<MixedStrategyProfile<double>> LpStrategySolve(const Game &,
                                                                 StrategyCallbackType<double>);
template std::list<MixedStrategyProfile<Rational>> LpStrategySolve(const Game &,
                                                                   StrategyCallbackType<Rational>);

} // end namespace Gambit::Nash
