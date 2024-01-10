//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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

#include <iostream>
#include "gambit.h"
#include "solvers/linalg/lemketab.h"
#include "solvers/lcp/lcp.h"

namespace Gambit {
namespace Nash {

template <class T> class NashLcpBehaviorSolver<T>::Solution {
public:
  int ns1, ns2, ni1, ni2;
  Rational maxpay;
  std::map<GameInfoset, int> infosetOffset;
  T eps;
  List<linalg::BFS<T> > m_list;
  List<MixedBehaviorProfile<T> > m_equilibria;

  explicit Solution(const Game &);

  bool AddBFS(const linalg::LemkeTableau<T> &);

  int EquilibriumCount() const { return m_equilibria.size(); }
};

template <class T>
NashLcpBehaviorSolver<T>::Solution::Solution(const Game &p_game)
  : ns1(p_game->GetPlayer(1)->NumSequences()),
    ns2(p_game->GetPlayer(2)->NumSequences()),
    ni1(p_game->GetPlayer(1)->NumInfosets() + 1),
    ni2(p_game->GetPlayer(2)->NumInfosets() + 1),
    maxpay(p_game->GetMaxPayoff() + Rational(1))
{
  for (const auto &player : p_game->GetPlayers()) {
    int offset = 1;
    for (const auto &infoset : player->GetInfosets()) {
      infosetOffset[infoset] = offset;
      offset += infoset->NumActions();
    }
  }
}


template <class T> bool 
NashLcpBehaviorSolver<T>::Solution::AddBFS(const linalg::LemkeTableau<T> &tableau)
{
  linalg::BFS<T> cbfs;
  Vector<T> v(tableau.MinRow(), tableau.MaxRow());
  tableau.BasisVector(v);

  for (int i = tableau.MinCol(); i <= tableau.MaxCol(); i++) {
    if (tableau.Member(i)) {
      cbfs.insert(i, v[tableau.Find(i)]);
    }
  }

  if (!m_list.Contains(cbfs)) {
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
template <class T> List<MixedBehaviorProfile<T> > 
NashLcpBehaviorSolver<T>::Solve(const Game &p_game) const
{
  if (p_game->NumPlayers() != 2) {
    throw UndefinedException("Method only valid for two-player games.");
  }
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException("Computing equilibria of games with imperfect recall is not supported.");
  }

  linalg::BFS<T> cbfs;
  Solution solution(p_game);

  int ntot = solution.ns1+solution.ns2+solution.ni1+solution.ni2;
  Matrix<T> A(1,ntot,0,ntot);
  A = static_cast<T>(0);
  FillTableau(A, p_game->GetRoot(), static_cast<T>(1), 1, 1, solution);
  for (int i = A.MinRow(); i <= A.MaxRow(); i++) {
    A(i,0) = static_cast<T>(-1);
  }
  A(1,solution.ns1+solution.ns2+1) = static_cast<T>(1);
  A(solution.ns1+solution.ns2+1,1) = static_cast<T>(-1);
  A(solution.ns1+1,solution.ns1+solution.ns2+solution.ni1+1) = static_cast<T>(1);
  A(solution.ns1+solution.ns2+solution.ni1+1,solution.ns1+1) = static_cast<T>(-1);

  Vector<T> b(1, ntot);
  b = static_cast<T>(0);
  b[solution.ns1+solution.ns2+1] = static_cast<T>(-1);
  b[solution.ns1+solution.ns2+solution.ni1+1] = static_cast<T>(-1);

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
      MixedBehaviorProfile<T> profile(p_game);
      GetProfile(tab, profile, sol, p_game->GetRoot(), 1, 1, solution);
      profile.UndefinedToCentroid();
      solution.m_equilibria.push_back(profile);
      this->m_onEquilibrium->Render(profile);
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
template<class T>
void
NashLcpBehaviorSolver<T>::AllLemke(const Game &p_game,
                                   int j, linalg::LemkeTableau<T> &B, int depth,
                                   Matrix<T> &A,
                                   Solution &p_solution) const
{
  if (m_maxDepth != 0 && depth > m_maxDepth) {
    return;
  }

  Vector<T> sol(B.MinRow(), B.MaxRow());
  MixedBehaviorProfile<T> profile(p_game);

  bool newsol = false;
  for (int i = B.MinRow(); i <= B.MaxRow() && !newsol; i++) {
    if (i == j) continue;

    linalg::LemkeTableau<T> BCopy(B);
    // Perturb tableau by a small number
    A(i,0) = static_cast<T>(-1) / static_cast<T>(1000);
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
      GetProfile(BCopy, profile, sol, p_game->GetRoot(), 1, 1, p_solution);
      profile.UndefinedToCentroid();
      if (newsol) {
        this->m_onEquilibrium->Render(profile);
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
void NashLcpBehaviorSolver<T>::FillTableau(Matrix<T> &A,
                                           const GameNode &n, T prob,
                                           int s1, int s2,
                                           Solution &p_solution) const
{
  int ns1 = p_solution.ns1;
  int ns2 = p_solution.ns2;
  int ni1 = p_solution.ni1;

  GameOutcome outcome = n->GetOutcome();
  if (outcome) {
    A(s1, ns1+s2) += Rational(prob) * (static_cast<Rational>(outcome->GetPayoff(1)) - p_solution.maxpay);
    A(ns1+s2, s1) += Rational(prob) * (static_cast<Rational>(outcome->GetPayoff(2)) - p_solution.maxpay);
  }
  if (n->IsTerminal()) {
    return;
  }
  GameInfoset infoset = n->GetInfoset();
  if (n->GetPlayer()->IsChance()) {
    for (const auto& action : infoset->GetActions()) {
      FillTableau(A, n->GetChild(action),
                  Rational(prob) * static_cast<Rational>(infoset->GetActionProb(action)),
                  s1, s2, p_solution);
    }
  }
  else if (n->GetPlayer()->GetNumber() == 1) {
    int infoset_idx = ns1 + ns2 + infoset->GetNumber() + 1;
    A(s1, infoset_idx) = static_cast<T>(-1);
    A(infoset_idx, s1) = static_cast<T>(1);
    int snew = p_solution.infosetOffset.at(infoset);
    for (const auto& child : n->GetChildren()) {
      snew++;
      A(snew, infoset_idx) = static_cast<T>(1);
      A(infoset_idx, snew) = static_cast<T>(-1);
      FillTableau(A, child, prob, snew, s2, p_solution);
    }
  }
  else {
    int infoset_idx = ns1 + ns2 + ni1 + n->GetInfoset()->GetNumber() + 1;
    A(ns1 + s2, infoset_idx) = static_cast<T>(-1);
    A(infoset_idx, ns1 + s2) = static_cast<T>(1);
    int snew = p_solution.infosetOffset.at(n->GetInfoset());
    for (const auto& child : n->GetChildren()) {
      snew++;
      A(ns1 + snew, infoset_idx) = static_cast<T>(1);
      A(infoset_idx, ns1 + snew) = static_cast<T>(-1);
      FillTableau(A, child, prob, s1, snew, p_solution);
    }
  }
}


template <class T> void
NashLcpBehaviorSolver<T>::GetProfile(const linalg::LemkeTableau<T> &tab,
                                     MixedBehaviorProfile<T> &v,
                                     const Vector<T> &sol,
                                     const GameNode &n, int s1, int s2,
                                     Solution &p_solution) const
{
  int ns1 = p_solution.ns1;

  if (n->IsTerminal()) {
    return;
  }
  if (n->GetPlayer()->IsChance()) {
    for (const auto& child : n->GetChildren()) {
      GetProfile(tab, v, sol, child, s1, s2, p_solution);
    }
  }
  else if (n->GetPlayer()->GetNumber() == 1) {
    int snew = p_solution.infosetOffset.at(n->GetInfoset());
    for (const auto& action : n->GetInfoset()->GetActions()) {
      snew++;
      v(action) = static_cast<T>(0);
      if (tab.Member(s1)) {
        int ind = tab.Find(s1);
        if (sol[ind] > p_solution.eps && tab.Member(snew)) {
          int ind2 = tab.Find(snew);
          if (sol[ind2] > p_solution.eps) {
            v(action) = sol[ind2] / sol[ind];
          }
        }
      }
      GetProfile(tab, v, sol, n->GetChild(action), snew, s2, p_solution);
    }
  }
  else {
    int snew = p_solution.infosetOffset.at(n->GetInfoset());
    for (const auto& action : n->GetInfoset()->GetActions()) {
      snew++;
      v(action) = static_cast<T>(0);
      if (tab.Member(ns1 + s2)) {
        int ind = tab.Find(ns1 + s2);
        if (sol[ind] > p_solution.eps && tab.Member(ns1 + snew)) {
          int ind2 = tab.Find(ns1 + snew);
          if (sol[ind2] > p_solution.eps) {
            v(action) = sol[ind2] / sol[ind];
          }
        }
      }
      GetProfile(tab, v, sol, n->GetChild(action), s1, snew, p_solution);
    }
  }
}

template class NashLcpBehaviorSolver<double>;
template class NashLcpBehaviorSolver<Rational>;

}  // end namespace Gambit::Nash
}  // end namespace Gambit
