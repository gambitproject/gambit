//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
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

namespace Gambit::Nash {

template <class T> class GameData {
public:
  int ns1, ns2;
  Rational minpay;
  std::map<GameInfoset, int> infosetOffset;

  explicit GameData(const Game &);

  void FillTableau(Matrix<T> &A, const GameNode &n, const T &prob, int s1, int s2, T payoff);

  void GetBehavior(MixedBehaviorProfile<T> &v, const Array<T> &, const Array<T> &,
                   const GameNode &, int, int);
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

//
// Recursively fills the constraint matrix A for the subtree rooted at 'n'.
//
template <class T>
void GameData<T>::FillTableau(Matrix<T> &A, const GameNode &n, const T &prob, int s1, int s2,
                              T payoff)
{
  const GameOutcome outcome = n->GetOutcome();
  if (outcome) {
    payoff += outcome->GetPayoff<Rational>(n->GetGame()->GetPlayer(1));
  }
  if (n->IsTerminal()) {
    A(s1, s2) += Rational(prob) * (payoff - minpay);
    return;
  }
  const GameInfoset infoset = n->GetInfoset();
  if (n->GetPlayer()->IsChance()) {
    for (const auto &action : infoset->GetActions()) {
      FillTableau(A, n->GetChild(action), prob * static_cast<T>(infoset->GetActionProb(action)),
                  s1, s2, payoff);
    }
  }
  else if (n->GetPlayer()->GetNumber() == 1) {
    const int col = ns2 + infoset->GetNumber() + 1;
    int snew = infosetOffset.at(infoset);
    A(s1, col) = static_cast<T>(1);
    for (const auto &child : n->GetChildren()) {
      A(++snew, col) = static_cast<T>(-1);
      FillTableau(A, child, prob, snew, s2, payoff);
    }
  }
  else {
    const int row = ns1 + infoset->GetNumber() + 1;
    int snew = infosetOffset.at(infoset);
    A(row, s2) = static_cast<T>(-1);
    for (const auto &child : n->GetChildren()) {
      A(row, ++snew) = static_cast<T>(1);
      FillTableau(A, child, prob, s1, snew, payoff);
    }
  }
}

//
// Recursively construct the behavior profile from the sequence form
// solution represented by 'p_primal' (containing player 2's
// sequences) and 'p_dual' (containing player 1's sequences).
//
// Any information sets not reached with positive probability have
// their action probabilities set to zero.
//
template <class T>
void GameData<T>::GetBehavior(MixedBehaviorProfile<T> &v, const Array<T> &p_primal,
                              const Array<T> &p_dual, const GameNode &n, int s1, int s2)
{
  if (n->IsTerminal()) {
    return;
  }
  if (n->GetPlayer()->IsChance()) {
    for (const auto &child : n->GetChildren()) {
      GetBehavior(v, p_primal, p_dual, child, s1, s2);
    }
  }
  else if (n->GetPlayer()->GetNumber() == 2) {
    int snew = infosetOffset.at(n->GetInfoset());
    for (const auto &action : n->GetInfoset()->GetActions()) {
      snew++;
      v[action] =
          (p_primal[s1] > static_cast<T>(0)) ? p_primal[snew] / p_primal[s1] : static_cast<T>(0);
      GetBehavior(v, p_primal, p_dual, n->GetChild(action), snew, s2);
    }
  }
  else {
    int snew = infosetOffset.at(n->GetInfoset());
    for (const auto &action : n->GetInfoset()->GetActions()) {
      snew++;
      v[action] = (p_dual[s2] > static_cast<T>(0)) ? p_dual[snew] / p_dual[s2] : static_cast<T>(0);
      GetBehavior(v, p_primal, p_dual, n->GetChild(action), s1, snew);
    }
  }
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

  data.FillTableau(A, p_game->GetRoot(), static_cast<T>(1), 1, 1, static_cast<T>(0));
  A(1, data.ns2 + 1) = static_cast<T>(-1);
  A(data.ns1 + 1, 1) = static_cast<T>(1);

  b[data.ns1 + 1] = static_cast<T>(1);
  c[data.ns2 + 1] = static_cast<T>(-1);

  Array<T> primal(A.NumColumns()), dual(A.NumRows());
  std::list<MixedBehaviorProfile<T>> solution;
  SolveLP(A, b, c, p_game->GetPlayer(2)->GetInfosets().size() + 1, primal, dual);
  MixedBehaviorProfile<T> profile(p_game);
  data.GetBehavior(profile, primal, dual, p_game->GetRoot(), 1, 1);
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
