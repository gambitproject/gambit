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

template <class T> class NashLcpBehaviorSolver {
public:
  NashLcpBehaviorSolver(BehaviorCallbackType<T> p_onEquilibrium = NullBehaviorCallback<T>)
    : m_onEquilibrium(p_onEquilibrium)
  {
  }
  ~NashLcpBehaviorSolver() = default;

  std::list<MixedBehaviorProfile<T>> Solve(const Game &) const;

private:
  BehaviorCallbackType<T> m_onEquilibrium;

  class Solution;

  void FillTableau(Matrix<T> &, Solution &) const;
  MixedBehaviorProfile<T> GetProfile(const linalg::LemkeTableau<T> &, const Vector<T> &,
                                     const Solution &) const;
};

// Holds the correspondence between the sequences (and information sets) of
// a two-player game and the rows/columns of the sequence-form LCP
// tableau, along with the quantities (payoffs, tree-of-sequences
// relationships) needed to populate it.
template <class T> class NashLcpBehaviorSolver<T>::Solution {
public:
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
  Rational maxpay;
  T eps;
  Array<linalg::BFS<T>> m_list;
  std::list<MixedBehaviorProfile<T>> m_equilibria;

  explicit Solution(const Game &);

  int RootIndex1() const { return ns1 + ns2 + 1; }
  int RootIndex2() const { return ns1 + ns2 + ni1 + 1; }

  bool AddBFS(const linalg::LemkeTableau<T> &);

  int EquilibriumCount() const { return m_equilibria.size(); }
};

template <class T>
NashLcpBehaviorSolver<T>::Solution::Solution(const Game &p_game)
  : support(p_game), player1(p_game->GetPlayer(1)), player2(p_game->GetPlayer(2)),
    root1(player1->GetSequences().front()), root2(player2->GetSequences().front()),
    ns1(static_cast<int>(support.GetSequences(player1).size())),
    ns2(static_cast<int>(support.GetSequences(player2).size())),
    ni1(static_cast<int>(player1->GetInfosets().size()) + 1),
    ni2(static_cast<int>(player2->GetInfosets().size()) + 1),
    maxpay(p_game->GetMaxPayoff() + Rational(1))
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

template <class T>
bool NashLcpBehaviorSolver<T>::Solution::AddBFS(const linalg::LemkeTableau<T> &tableau)
{
  linalg::BFS<T> cbfs;
  Vector<T> v(tableau.MinRow(), tableau.MaxRow());
  tableau.BasisVector(v);

  for (int i = tableau.MinCol(); i <= tableau.MaxCol(); i++) {
    if (tableau.Member(i)) {
      cbfs.insert(i, v[tableau.Find(i)]);
    }
  }

  if (!contains(m_list, cbfs)) {
    m_list.push_back(cbfs);
    return true;
  }
  else {
    return false;
  }
}

//
// Lemke implements Lemke's algorithm for linear complementarity problems,
// as refined by Eaves for degenerate problems, starting from the primary ray.
//
template <class T>
std::list<MixedBehaviorProfile<T>> NashLcpBehaviorSolver<T>::Solve(const Game &p_game) const
{
  if (p_game->NumPlayers() != 2) {
    throw UndefinedException("Method only valid for two-player games.");
  }
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException(
        "Computing equilibria of games with imperfect recall is not supported.");
  }

  Solution solution(p_game);

  const int ntot = solution.ns1 + solution.ns2 + solution.ni1 + solution.ni2;
  Matrix<T> A(1, ntot, 0, ntot);
  A = static_cast<T>(0);
  FillTableau(A, solution);
  for (int i = A.MinRow(); i <= A.MaxRow(); i++) {
    A(i, 0) = static_cast<T>(-1);
  }
  A(solution.index.at(solution.root1), solution.RootIndex1()) = static_cast<T>(1);
  A(solution.RootIndex1(), solution.index.at(solution.root1)) = static_cast<T>(-1);
  A(solution.index.at(solution.root2), solution.RootIndex2()) = static_cast<T>(1);
  A(solution.RootIndex2(), solution.index.at(solution.root2)) = static_cast<T>(-1);

  Vector<T> b(1, ntot);
  b = static_cast<T>(0);
  b[solution.RootIndex1()] = static_cast<T>(-1);
  b[solution.RootIndex2()] = static_cast<T>(-1);

  linalg::LemkeTableau<T> tab(A, b);
  solution.eps = tab.Epsilon();

  tab.Pivot(solution.RootIndex1(), 0);
  tab.SF_LCPPath(solution.RootIndex1());
  solution.AddBFS(tab);
  Vector<T> sol(tab.MinRow(), tab.MaxRow());
  tab.BasisVector(sol);

  MixedBehaviorProfile<T> profile = GetProfile(tab, sol, solution);
  profile.UndefinedToCentroid();
  solution.m_equilibria.push_back(profile);
  this->m_onEquilibrium(profile);
  return solution.m_equilibria;
}

template <class T>
void NashLcpBehaviorSolver<T>::FillTableau(Matrix<T> &A, Solution &p_solution) const
{
  // Payoff block: for every pair of sequences (one per player), the
  // payoff each player receives when that pair is exactly realised,
  // shifted by a constant large enough to make all entries negative and
  // weighted by chance's probability of that pair actually being realised
  // (which need not be 1 -- see PureSequenceProfile::GetRealizationProbability).
  for (auto profile : p_solution.support.GetSequenceContingencies()) {
    const GameSequence &seq1 = profile.GetSequence(p_solution.player1);
    const GameSequence &seq2 = profile.GetSequence(p_solution.player2);
    const Rational prob = profile.GetRealizationProbability();
    const Rational pay1 = profile.GetPayoff(p_solution.player1) - p_solution.maxpay * prob;
    const Rational pay2 = profile.GetPayoff(p_solution.player2) - p_solution.maxpay * prob;
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
      A(arrivalIdx, infosetIdx) = static_cast<T>(-1);
      A(infosetIdx, arrivalIdx) = static_cast<T>(1);
      for (const auto &child : children) {
        const int childIdx = p_solution.index.at(child);
        A(childIdx, infosetIdx) = static_cast<T>(1);
        A(infosetIdx, childIdx) = static_cast<T>(-1);
      }
    }
  }
}

template <class T>
MixedBehaviorProfile<T> NashLcpBehaviorSolver<T>::GetProfile(const linalg::LemkeTableau<T> &tab,
                                                             const Vector<T> &sol,
                                                             const Solution &p_solution) const
{
  std::map<GameSequence, T> x;
  for (const auto &[sequence, idx] : p_solution.index) {
    T value(0);
    if (tab.Member(idx)) {
      const T candidate = sol[tab.Find(idx)];
      if (candidate > p_solution.eps) {
        value = candidate;
      }
    }
    x[sequence] = value;
  }
  return p_solution.support.ToMixedBehaviorProfile(x);
}

template <class T>
std::list<MixedBehaviorProfile<T>> LcpBehaviorSolve(const Game &p_game,
                                                    BehaviorCallbackType<T> p_onEquilibrium)
{
  return NashLcpBehaviorSolver<T>(p_onEquilibrium).Solve(p_game);
}

template std::list<MixedBehaviorProfile<double>> LcpBehaviorSolve(const Game &,
                                                                  BehaviorCallbackType<double>);
template std::list<MixedBehaviorProfile<Rational>>
LcpBehaviorSolve(const Game &, BehaviorCallbackType<Rational>);

} // end namespace Gambit::Nash
