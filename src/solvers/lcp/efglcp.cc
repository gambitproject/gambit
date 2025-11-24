//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
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
#include "games/gameseq.h"

namespace Gambit::Nash {

template <class T> class NashLcpBehaviorSolver {
public:
  NashLcpBehaviorSolver(int p_stopAfter, int p_maxDepth,
                        BehaviorCallbackType<T> p_onEquilibrium = NullBehaviorCallback<T>)
    : m_onEquilibrium(p_onEquilibrium), m_stopAfter(p_stopAfter), m_maxDepth(p_maxDepth)
  {
  }
  ~NashLcpBehaviorSolver() = default;

  std::list<MixedBehaviorProfile<T>> Solve(const Game &) const;

private:
  BehaviorCallbackType<T> m_onEquilibrium;
  int m_stopAfter, m_maxDepth;

  class Solution;

  void FillTableau(Matrix<T> &, const GameNode &, T, int, int, Solution &) const;
  void FillTableauNew(Matrix<T> &A, const Game &p_game, Solution &p_solution) const;
  void AllLemke(const Game &, int dup, linalg::LemkeTableau<T> &B, int depth, Matrix<T> &,
                Solution &) const;
  void GetProfile(const linalg::LemkeTableau<T> &tab, MixedBehaviorProfile<T> &, const Vector<T> &,
                  const GameNode &n, int, int, Solution &) const;
  MixedBehaviorProfile<T> GetProfileNew(const linalg::LemkeTableau<T> &tab, const Vector<T> &sol,
                                        const Game &p_game, Solution &p_solution) const;
};

template <class T> class NashLcpBehaviorSolver<T>::Solution {
public:
  int ns1, ns2, ni1, ni2;
  Rational maxpay;
  std::map<GameInfoset, int> infosetOffset;
  T eps;
  List<linalg::BFS<T>> m_list;
  std::list<MixedBehaviorProfile<T>> m_equilibria;

  explicit Solution(const Game &);

  bool AddBFS(const linalg::LemkeTableau<T> &);

  int EquilibriumCount() const { return m_equilibria.size(); }
};

template <class T>
NashLcpBehaviorSolver<T>::Solution::Solution(const Game &p_game)
  : ns1(p_game->GetPlayer(1)->NumSequences()), ns2(p_game->GetPlayer(2)->NumSequences()),
    ni1(p_game->GetPlayer(1)->GetInfosets().size() + 1),
    ni2(p_game->GetPlayer(2)->GetInfosets().size() + 1),
    maxpay(p_game->GetMaxPayoff() + Rational(1))
{
  for (const auto &player : p_game->GetPlayers()) {
    int offset = 1;
    for (const auto &infoset : player->GetInfosets()) {
      infosetOffset[infoset] = offset;
      offset += infoset->GetActions().size();
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

  const linalg::BFS<T> cbfs;
  Solution solution(p_game);

  const int ntot = solution.ns1 + solution.ns2 + solution.ni1 + solution.ni2;
  Matrix<T> A(1, ntot, 0, ntot);
  A = static_cast<T>(0);

  // BehaviorSupportProfile full_support(p_game);
  // Gambit::GameSequenceForm sequenceForm(full_support);

  // FillTableau(A, p_game->GetRoot(), static_cast<T>(1), 1, 1, solution);
  FillTableauNew(A, p_game, solution);
  for (int i = A.MinRow(); i <= A.MaxRow(); i++) {
    A(i, 0) = static_cast<T>(-1);
  }
  A(1, solution.ns1 + solution.ns2 + 1) = static_cast<T>(1);
  A(solution.ns1 + solution.ns2 + 1, 1) = static_cast<T>(-1);
  A(solution.ns1 + 1, solution.ns1 + solution.ns2 + solution.ni1 + 1) = static_cast<T>(1);
  A(solution.ns1 + solution.ns2 + solution.ni1 + 1, solution.ns1 + 1) = static_cast<T>(-1);

  Vector<T> b(1, ntot);
  b = static_cast<T>(0);
  b[solution.ns1 + solution.ns2 + 1] = static_cast<T>(-1);
  b[solution.ns1 + solution.ns2 + solution.ni1 + 1] = static_cast<T>(-1);

  linalg::LemkeTableau<T> tab(A, b);
  solution.eps = tab.Epsilon();

  try {
    if (m_stopAfter != 1) {
      try {
        AllLemke(p_game, solution.ns1 + solution.ns2 + 1, tab, 0, A, solution);
      }
      catch (EquilibriumLimitReached &) {
        // Handle this silently; equilibria are recorded as found so no action needed
      }
    }
    else {
      tab.Pivot(solution.ns1 + solution.ns2 + 1, 0);
      tab.SF_LCPPath(solution.ns1 + solution.ns2 + 1);
      solution.AddBFS(tab);
      Vector<T> sol(tab.MinRow(), tab.MaxRow());
      tab.BasisVector(sol);
      // MixedBehaviorProfile<T> profile(p_game);
      // GetProfile(tab, profile, sol, p_game->GetRoot(), 1, 1, solution);
      auto profile = GetProfileNew(tab, sol, p_game, solution);
      profile.UndefinedToCentroid();
      solution.m_equilibria.push_back(profile);
      this->m_onEquilibrium(profile, "NE");
    }
  }
  catch (std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return solution.m_equilibria;
}

//
// All_Lemke finds all accessible Nash equilibria by recursively
// calling itself.  List maintains the list of basic variables
// for the equilibria that have already been found.
// From each new accessible equilibrium, it follows
// all possible paths, adding any new equilibria to the List.
//
template <class T>
void NashLcpBehaviorSolver<T>::AllLemke(const Game &p_game, int j, linalg::LemkeTableau<T> &B,
                                        int depth, Matrix<T> &A, Solution &p_solution) const
{
  if (m_maxDepth != 0 && depth > m_maxDepth) {
    return;
  }

  Vector<T> sol(B.MinRow(), B.MaxRow());
  // MixedBehaviorProfile<T> profile(p_game);

  bool newsol = false;
  for (int i = B.MinRow(); i <= B.MaxRow() && !newsol; i++) {
    if (i == j) {
      continue;
    }

    linalg::LemkeTableau<T> BCopy(B);
    // Perturb tableau by a small number
    A(i, 0) = static_cast<T>(-1) / static_cast<T>(1000);
    BCopy.Refactor();

    int missing;
    if (depth == 0) {
      BCopy.Pivot(j, 0);
      missing = -j;
    }
    else {
      missing = BCopy.SF_PivotIn(0);
    }

    newsol = false;

    if (BCopy.SF_LCPPath(-missing) == 1) {
      newsol = p_solution.AddBFS(BCopy);
      BCopy.BasisVector(sol);
      // BehaviorSupportProfile full_support(p_game);
      // Gambit::GameSequenceForm sequenceForm(full_support);
      // GetProfile(BCopy, profile, sol, p_game->GetRoot(), 1, 1, p_solution);
      auto profile = GetProfileNew(BCopy, sol, p_game, p_solution);
      profile.UndefinedToCentroid();
      if (newsol) {
        m_onEquilibrium(profile, "NE");
        p_solution.m_equilibria.push_back(profile);
        if (m_stopAfter > 0 && p_solution.EquilibriumCount() >= m_stopAfter) {
          throw EquilibriumLimitReached();
        }
      }
    }
    else {
      // Dead end
    }

    A(i, 0) = static_cast<T>(-1);
    if (newsol) {
      BCopy.Refactor();
      AllLemke(p_game, i, BCopy, depth + 1, A, p_solution);
    }
  }
}

template <class T>
void NashLcpBehaviorSolver<T>::FillTableau(Matrix<T> &A, const GameNode &n, T prob, int s1, int s2,
                                           Solution &p_solution) const
{
  const int ns1 = p_solution.ns1;
  const int ns2 = p_solution.ns2;
  const int ni1 = p_solution.ni1;

  const GameOutcome outcome = n->GetOutcome();
  if (outcome) {
    A(s1, ns1 + s2) += Rational(prob) * (outcome->GetPayoff<Rational>(n->GetGame()->GetPlayer(1)) -
                                         p_solution.maxpay);
    A(ns1 + s2, s1) += Rational(prob) * (outcome->GetPayoff<Rational>(n->GetGame()->GetPlayer(2)) -
                                         p_solution.maxpay);
  }
  if (n->IsTerminal()) {
    return;
  }
  const GameInfoset infoset = n->GetInfoset();
  if (n->GetPlayer()->IsChance()) {
    for (const auto &action : infoset->GetActions()) {
      FillTableau(A, n->GetChild(action),
                  Rational(prob) * static_cast<Rational>(infoset->GetActionProb(action)), s1, s2,
                  p_solution);
    }
  }
  else if (n->GetPlayer()->GetNumber() == 1) {
    const int infoset_idx = ns1 + ns2 + infoset->GetNumber() + 1;
    A(s1, infoset_idx) = static_cast<T>(-1);
    A(infoset_idx, s1) = static_cast<T>(1);
    int snew = p_solution.infosetOffset.at(infoset);
    for (const auto &child : n->GetChildren()) {
      snew++;
      A(snew, infoset_idx) = static_cast<T>(1);
      A(infoset_idx, snew) = static_cast<T>(-1);
      FillTableau(A, child, prob, snew, s2, p_solution);
    }
  }
  else {
    const int infoset_idx = ns1 + ns2 + ni1 + n->GetInfoset()->GetNumber() + 1;
    A(ns1 + s2, infoset_idx) = static_cast<T>(-1);
    A(infoset_idx, ns1 + s2) = static_cast<T>(1);
    int snew = p_solution.infosetOffset.at(n->GetInfoset());
    for (const auto &child : n->GetChildren()) {
      snew++;
      A(ns1 + snew, infoset_idx) = static_cast<T>(1);
      A(infoset_idx, ns1 + snew) = static_cast<T>(-1);
      FillTableau(A, child, prob, s1, snew, p_solution);
    }
  }
}

template <class T>
void NashLcpBehaviorSolver<T>::FillTableauNew(Matrix<T> &A, const Game &p_game,
                                              Solution &p_solution) const
{
  const int ns1 = p_solution.ns1;
  const int ns2 = p_solution.ns2;
  const int ni1 = p_solution.ni1;
  auto players = p_game->GetPlayers();
  auto it = players.begin();
  auto player1 = *it;
  ++it;
  auto player2 = *it;
  auto sequences1 = p_game->GetSequences(player1);
  auto sequences2 = p_game->GetSequences(player2);
  for (auto seq : sequences1) {
    auto parentSeq = seq->parent.lock();
    if (parentSeq) {
      const int infoset_idx = ns1 + ns2 + seq->GetInfoset()->GetNumber() + 1;
      const int seq_idx = seq->number;
      const int parent_idx = parentSeq->number;
      A(parent_idx, infoset_idx) = static_cast<T>(-1);
      A(infoset_idx, parent_idx) = static_cast<T>(1);
      A(seq_idx, infoset_idx) = static_cast<T>(1);
      A(infoset_idx, seq_idx) = static_cast<T>(-1);
    }
  }
  for (auto seq : sequences2) {
    auto parentSeq = seq->parent.lock();
    if (parentSeq) {
      const int infoset_idx = ns1 + ns2 + ni1 + seq->GetInfoset()->GetNumber() + 1;
      const int seq_idx = seq->number;
      const int parent_idx = parentSeq->number;
      A(ns1 + parent_idx, infoset_idx) = static_cast<T>(-1);
      A(infoset_idx, ns1 + parent_idx) = static_cast<T>(1);
      A(ns1 + seq_idx, infoset_idx) = static_cast<T>(1);
      A(infoset_idx, ns1 + seq_idx) = static_cast<T>(-1);
    }
  }
  for (auto seq1 : sequences1) {
    for (auto seq2 : sequences2) {
      const int s1 = seq1->number;
      const int s2 = seq2->number;
      std::map<GamePlayer, GameSequence> profile;
      profile[player1] = seq1;
      profile[player2] = seq2;
      A(s1, ns1 + s2) = p_game->GetPayoff(profile, player1) - p_solution.maxpay;
      A(ns1 + s2, s1) = p_game->GetPayoff(profile, player2) - p_solution.maxpay;
    }
  }
}

template <class T>
void NashLcpBehaviorSolver<T>::GetProfile(const linalg::LemkeTableau<T> &tab,
                                          MixedBehaviorProfile<T> &v, const Vector<T> &sol,
                                          const GameNode &n, int s1, int s2,
                                          Solution &p_solution) const
{
  const int ns1 = p_solution.ns1;

  if (n->IsTerminal()) {
    return;
  }
  if (n->GetPlayer()->IsChance()) {
    for (const auto &child : n->GetChildren()) {
      GetProfile(tab, v, sol, child, s1, s2, p_solution);
    }
  }
  else if (n->GetPlayer()->GetNumber() == 1) {
    int snew = p_solution.infosetOffset.at(n->GetInfoset());
    for (const auto &action : n->GetInfoset()->GetActions()) {
      snew++;
      v[action] = static_cast<T>(0);
      if (tab.Member(s1)) {
        const int ind = tab.Find(s1);
        if (sol[ind] > p_solution.eps && tab.Member(snew)) {
          const int ind2 = tab.Find(snew);
          if (sol[ind2] > p_solution.eps) {
            v[action] = sol[ind2] / sol[ind];
          }
        }
      }
      GetProfile(tab, v, sol, n->GetChild(action), snew, s2, p_solution);
    }
  }
  else {
    int snew = p_solution.infosetOffset.at(n->GetInfoset());
    for (const auto &action : n->GetInfoset()->GetActions()) {
      snew++;
      v[action] = static_cast<T>(0);
      if (tab.Member(ns1 + s2)) {
        const int ind = tab.Find(ns1 + s2);
        if (sol[ind] > p_solution.eps && tab.Member(ns1 + snew)) {
          const int ind2 = tab.Find(ns1 + snew);
          if (sol[ind2] > p_solution.eps) {
            v[action] = sol[ind2] / sol[ind];
          }
        }
      }
      GetProfile(tab, v, sol, n->GetChild(action), s1, snew, p_solution);
    }
  }
}

template <class T>
MixedBehaviorProfile<T>
NashLcpBehaviorSolver<T>::GetProfileNew(const linalg::LemkeTableau<T> &tab, const Vector<T> &sol,
                                        const Game &p_game, Solution &p_solution) const
{
  const int ns1 = p_solution.ns1;
  auto players = p_game->GetPlayers();
  auto it = players.begin();
  auto player1 = *it;
  ++it;
  auto player2 = *it;
  auto sequences1 = p_game->GetSequences(player1);
  auto sequences2 = p_game->GetSequences(player2);
  Gambit::MixedSequenceProfile<T> msp(p_game);
  for (auto seq : sequences1) {
    int seq_num = seq->number;
    if (tab.Member(seq_num)) {
      int index = tab.Find(seq_num);
      msp[seq] = (sol[index] > p_solution.eps) ? sol[index] : static_cast<T>(0);
    }
    else {
      msp[seq] = static_cast<T>(0);
    }
  }
  for (auto seq : sequences2) {
    int seq_num = seq->number;
    if (tab.Member(ns1 + seq_num)) {
      int index = tab.Find(ns1 + seq_num);
      msp[seq] = (sol[index] > p_solution.eps) ? sol[index] : static_cast<T>(0);
    }
    else {
      msp[seq] = static_cast<T>(0);
    }
  }
  return msp.GetMixedBehaviorProfile();
}

template <class T>
std::list<MixedBehaviorProfile<T>> LcpBehaviorSolve(const Game &p_game, int p_stopAfter,
                                                    int p_maxDepth,
                                                    BehaviorCallbackType<T> p_onEquilibrium)
{
  return NashLcpBehaviorSolver<T>(p_stopAfter, p_maxDepth, p_onEquilibrium).Solve(p_game);
}

template std::list<MixedBehaviorProfile<double>> LcpBehaviorSolve(const Game &, int, int,
                                                                  BehaviorCallbackType<double>);
template std::list<MixedBehaviorProfile<Rational>>
LcpBehaviorSolve(const Game &, int, int, BehaviorCallbackType<Rational>);

} // end namespace Gambit::Nash
