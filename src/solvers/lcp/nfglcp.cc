//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/lcp/nfglcp.cc
// Compute Nash equilibria via Lemke-Howson algorithm
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
#include "solvers/linalg/lhtab.h"
#include "solvers/lcp/lcp.h"

namespace Gambit {
namespace Nash {

// Anonymous namespace to encapsulate local utility functions
  
namespace {
template <class T> Matrix<T> Make_A1(const Game &p_game)
{
  int n1 = p_game->GetPlayer(1)->Strategies().size();
  int n2 = p_game->GetPlayer(2)->Strategies().size();
  Matrix<T> A1(1, n1, n1+1, n1+n2);

  PureStrategyProfile profile = p_game->NewPureStrategyProfile();

  Rational min = p_game->GetMinPayoff();
  if (min > Rational(0)) {
    min = Rational(0);
  }
  min -= Rational(1);

  Rational max = p_game->GetMaxPayoff();
  if (max < Rational(0)) {
    max = Rational(0);
  }

  Rational fac(1, max - min);

  for (int i = 1; i <= n1; i++)  {
    profile->SetStrategy(p_game->GetPlayer(1)->Strategies()[i]);
    for (int j = 1; j <= n2; j++)  {
      profile->SetStrategy(p_game->GetPlayer(2)->Strategies()[j]);
      A1(i, n1 + j) = fac * (profile->GetPayoff(1) - min);
    }
  }
  return A1;
}

template <class T> Matrix<T> Make_A2(const Game &p_game)
{
  int n1 = p_game->GetPlayer(1)->Strategies().size();
  int n2 = p_game->GetPlayer(2)->Strategies().size();
  Matrix<T> A2(n1+1, n1+n2, 1, n1);

  PureStrategyProfile profile = p_game->NewPureStrategyProfile();
  
  Rational min = p_game->GetMinPayoff();
  if (min > Rational(0)) {
    min = Rational(0);
  }
  min -= Rational(1);

  Rational max = p_game->GetMaxPayoff();
  if (max < Rational(0)) {
    max = Rational(0);
  }

  Rational fac(1, max - min);

  for (int i = 1; i <= n1; i++)  {
    profile->SetStrategy(p_game->GetPlayer(1)->Strategies()[i]);
    for (int j = 1; j <= n2; j++)  {
      profile->SetStrategy(p_game->GetPlayer(2)->Strategies()[j]);
      A2(n1 + j, i) = fac * (profile->GetPayoff(2) - min);
    }
  }
  return A2;
}

template <class T> Vector<T> Make_b1(const Game &p_game)
{
  Vector<T> b1(1, p_game->GetPlayer(1)->Strategies().size());
  b1 = -(T) 1;
  return b1;
}

template <class T> Vector<T> Make_b2(const Game &p_game)
{
  Vector<T> b2(p_game->GetPlayer(1)->Strategies().size() + 1,
	       p_game->GetPlayer(1)->Strategies().size() +
	       p_game->GetPlayer(2)->Strategies().size());
  b2 = -(T) 1;
  return b2;
}

}  // end anonymous namespace
  

template <class T>
class NashLcpStrategySolver<T>::Solution {
public:
  List<Gambit::linalg::BFS<T> > m_bfsList;
  List<MixedStrategyProfile<T> > m_equilibria;

  bool Contains(const Gambit::linalg::BFS<T> &p_bfs) const
  { return m_bfsList.Contains(p_bfs); }
  void push_back(const Gambit::linalg::BFS<T> &p_bfs)
  { m_bfsList.push_back(p_bfs); }

  int EquilibriumCount() const { return m_equilibria.size(); }
};
  
//
// Function called when a CBFS is encountered.
// If it is not already in the list p_list, it is added.
// The corresponding equilibrium is computed and output.
// Returns 'true' if the CBFS is new; 'false' if it already appears in the
// list.
//
template <class T> bool
NashLcpStrategySolver<T>::OnBFS(const Game &p_game,
				linalg::LHTableau<T> &p_tableau,
				Solution &p_solution) const
{
  Gambit::linalg::BFS<T> cbfs(p_tableau.GetBFS());
  if (p_solution.Contains(cbfs)) {
    return false;
  }
  p_solution.push_back(cbfs);

  MixedStrategyProfile<T> profile(p_game->NewMixedStrategyProfile(static_cast<T>(0.0)));
  int n1 = p_game->GetPlayer(1)->Strategies().size();
  int n2 = p_game->GetPlayer(2)->Strategies().size();
  T sum = (T) 0;

  for (int j = 1; j <= n1; j++) {
    if (cbfs.count(j))   sum += cbfs[j];
  }
  if (sum == (T) 0)  {
    // This is the trivial CBFS.
    return false;
  }

  for (int j = 1; j <= n1; j++) {
    GameStrategy strategy = p_game->GetPlayer(1)->Strategies()[j];
    if (cbfs.count(j)) {
      profile[strategy] = cbfs[j] / sum;
    }
    else {
      profile[strategy] = (T) 0;
    }
  }

  sum = (T) 0;
  for (int j = 1; j <= n2; j++) {
    if (cbfs.count(n1 + j))  sum += cbfs[n1 + j];
  }

  for (int j = 1; j <= n2; j++) {
    GameStrategy strategy = p_game->GetPlayer(2)->Strategies()[j];
    if (cbfs.count(n1 + j)) {
      profile[strategy] = cbfs[n1 + j] / sum;
    }
    else {
      profile[strategy] = (T) 0;
    }
  }
  
  this->m_onEquilibrium->Render(profile);
  p_solution.m_equilibria.push_back(profile);

  if (m_stopAfter > 0 && p_solution.EquilibriumCount() >= m_stopAfter) {
    throw EquilibriumLimitReached();
  }

  return true;
}

//
// AllLemke finds all accessible Nash equilibria by recursively 
// calling itself.  p_list maintains the list of basic variables 
// for the equilibria that have already been found.  
// From each new accessible equilibrium, it follows
// all possible paths, adding any new equilibria to the List.  
//
template <class T> void 
NashLcpStrategySolver<T>::AllLemke(const Game &p_game,
				   int j, linalg::LHTableau<T> &B,
				   Solution &p_solution,
				   int depth) const
{
  if (m_maxDepth != 0 && depth > m_maxDepth) {
    return;
  }

  // On the initial depth=0 call, the CBFS we are at is the extraneous
  // solution.
  if (depth > 0 && !OnBFS(p_game, B, p_solution)) {
    return;
  }
  
  for (int i = B.MinCol(); i <= B.MaxCol(); i++) {
    if (i != j)  {
      linalg::LHTableau<T> Bcopy(B);
      Bcopy.LemkePath(i);
      AllLemke(p_game, i, Bcopy, p_solution, depth+1);
    }
  }
}

template <class T> List<MixedStrategyProfile<T> > 
NashLcpStrategySolver<T>::Solve(const Game &p_game) const
{
  if (p_game->NumPlayers() != 2) {
    throw UndefinedException("Method only valid for two-player games.");
  }
  if (!p_game->IsPerfectRecall()) {
    throw UndefinedException("Computing equilibria of games with imperfect recall is not supported.");
  }
  Solution solution;

  try {
    Matrix<T> A1 = Make_A1<T>(p_game);
    Vector<T> b1 = Make_b1<T>(p_game);
    Matrix<T> A2 = Make_A2<T>(p_game);
    Vector<T> b2 = Make_b2<T>(p_game);
    linalg::LHTableau<T> B(A1, A2, b1, b2);

    if (m_stopAfter != 1) {
      AllLemke(p_game, 0, B, solution, 0);
    }
    else  {
      B.LemkePath(1);
      OnBFS(p_game, B, solution);
    }
  }
  catch (EquilibriumLimitReached &) {
    // This pseudo-exception requires no additional action;
    // solution contains details of all equilibria found
  }
  catch (std::runtime_error &e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return solution.m_equilibria;
}

template class NashLcpStrategySolver<double>;
template class NashLcpStrategySolver<Rational>;

}  // end namespace Gambit::Nash
}  // end namespace Gambit



