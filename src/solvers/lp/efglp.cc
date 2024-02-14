//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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
#include "solvers/linalg/lpsolve.h"
#include "efglp.h"

using namespace Gambit;

template <class T> class NashLpBehavSolver<T>::GameData {
public:
  int ns1, ns2, ni1, ni2;
  Rational minpay;
  std::map<GameInfoset, int> infosetOffset;

  explicit GameData(const Game &);

  void FillTableau(Matrix<T> &A, const GameNode &n, const T &prob, int s1, int s2);

  void GetBehavior(MixedBehaviorProfile<T> &v, const Array<T> &, const Array<T> &,
                   const GameNode &, int, int);
};

template <class T> NashLpBehavSolver<T>::GameData::GameData(const Game &p_game)
{
  ns1 = p_game->GetPlayer(1)->NumSequences();
  ns2 = p_game->GetPlayer(2)->NumSequences();
  ni1 = p_game->GetPlayer(1)->NumInfosets() + 1;
  ni2 = p_game->GetPlayer(2)->NumInfosets() + 1;
  for (const auto &player : p_game->GetPlayers()) {
    int offset = 1;
    for (const auto &infoset : player->GetInfosets()) {
      infosetOffset[infoset] = offset;
      offset += infoset->NumActions();
    }
  }
  minpay = p_game->GetMinPayoff();
}

//
// Recursively fills the constraint matrix A for the subtree rooted at 'n'.
//
template <class T>
void NashLpBehavSolver<T>::GameData::FillTableau(Matrix<T> &A, const GameNode &n, const T &prob,
                                                 int s1, int s2)
{
  GameOutcome outcome = n->GetOutcome();
  if (outcome) {
    A(s1, s2) += Rational(prob) * (static_cast<Rational>(outcome->GetPayoff(1)) - minpay);
  }
  if (n->IsTerminal()) {
    return;
  }
  GameInfoset infoset = n->GetInfoset();
  if (n->GetPlayer()->IsChance()) {
    for (const auto &action : infoset->GetActions()) {
      FillTableau(A, n->GetChild(action), prob * static_cast<T>(infoset->GetActionProb(action)),
                  s1, s2);
    }
  }
  else if (n->GetPlayer()->GetNumber() == 1) {
    int col = ns2 + infoset->GetNumber() + 1;
    int snew = infosetOffset.at(infoset);
    A(s1, col) = static_cast<T>(1);
    for (const auto &child : n->GetChildren()) {
      A(++snew, col) = static_cast<T>(-1);
      FillTableau(A, child, prob, snew, s2);
    }
  }
  else {
    int row = ns1 + infoset->GetNumber() + 1;
    int snew = infosetOffset.at(infoset);
    A(row, s2) = static_cast<T>(-1);
    for (const auto &child : n->GetChildren()) {
      A(row, ++snew) = static_cast<T>(1);
      FillTableau(A, child, prob, s1, snew);
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
bool NashLpBehavSolver<T>::SolveLP(const Matrix<T> &A, const Vector<T> &b, const Vector<T> &c,
                                   int nequals, Array<T> &p_primal, Array<T> &p_dual) const
{
  linalg::LPSolve<T> LP(A, b, c, nequals);
  const auto &cbfs(LP.OptimumBFS());

  for (int i = 1; i <= A.NumColumns(); i++) {
    p_primal[i] = (cbfs.count(i)) ? cbfs[i] : static_cast<T>(0);
  }
  for (int i = 1; i <= A.NumRows(); i++) {
    p_dual[i] = (cbfs.count(-i)) ? cbfs[-i] : static_cast<T>(0);
  }
  return true;
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
void NashLpBehavSolver<T>::GameData::GetBehavior(MixedBehaviorProfile<T> &v,
                                                 const Array<T> &p_primal, const Array<T> &p_dual,
                                                 const GameNode &n, int s1, int s2)
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
      v[action] = (p_primal[s1] > (T)0) ? p_primal[snew] / p_primal[s1] : (T)0;
      GetBehavior(v, p_primal, p_dual, n->GetChild(action), snew, s2);
    }
  }
  else {
    int snew = infosetOffset.at(n->GetInfoset());
    for (const auto &action : n->GetInfoset()->GetActions()) {
      snew++;
      v[action] = (p_dual[s2] > (T)0) ? p_dual[snew] / p_dual[s2] : (T)0;
      GetBehavior(v, p_primal, p_dual, n->GetChild(action), s1, snew);
    }
  }
}

//
// Compute and print one equilibrium by solving a linear program based
// on the sequence form representation of the game.
//
template <class T>
List<MixedBehaviorProfile<T>> NashLpBehavSolver<T>::Solve(const Game &p_game) const
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

  Gambit::linalg::BFS<T> cbfs;

  GameData data = GameData(p_game);

  Matrix<T> A(1, data.ns1 + data.ni2, 1, data.ns2 + data.ni1);
  Vector<T> b(1, data.ns1 + data.ni2);
  Vector<T> c(1, data.ns2 + data.ni1);

  A = (T)0;
  b = (T)0;
  c = (T)0;

  data.FillTableau(A, p_game->GetRoot(), (T)1, 1, 1);
  A(1, data.ns2 + 1) = (T)-1;
  A(data.ns1 + 1, 1) = (T)1;

  b[data.ns1 + 1] = (T)1;
  c[data.ns2 + 1] = (T)-1;

  Array<T> primal(A.NumColumns()), dual(A.NumRows());
  List<MixedBehaviorProfile<T>> solution;
  if (SolveLP(A, b, c, data.ni2, primal, dual)) {
    MixedBehaviorProfile<T> profile(p_game);
    data.GetBehavior(profile, primal, dual, p_game->GetRoot(), 1, 1);
    profile.UndefinedToCentroid();
    this->m_onEquilibrium->Render(profile);
    solution.push_back(profile);
  }
  return solution;
}

template class NashLpBehavSolver<double>;
template class NashLpBehavSolver<Rational>;
