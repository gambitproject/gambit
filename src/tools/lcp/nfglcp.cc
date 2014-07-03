//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
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

#include <cstdio>
#include <unistd.h>
#include <iostream>

#include "libgambit/libgambit.h"
#include "nfglcp.h"
#include "lhtab.h"

using namespace Gambit;


template <class T> Matrix<T> Make_A1(const StrategySupport &); 
template <class T> Vector<T> Make_b1(const StrategySupport &);
template <class T> Matrix<T> Make_A2(const StrategySupport &);
template <class T> Vector<T> Make_b2(const StrategySupport &);


template <class T>
class NashLcpStrategySolver<T>::Solution {
public:
  List<BFS<T> > m_bfsList;
  List<MixedStrategyProfile<T> > m_equilibria;

  bool Contains(const BFS<T> &p_bfs) const
  { return m_bfsList.Contains(p_bfs); }
  void push_back(const BFS<T> &p_bfs)
  { m_bfsList.push_back(p_bfs); }

  int EquilibriumCount(void) const { return m_equilibria.size(); }
};
  

//
// Function called when a CBFS is encountered.
// If it is not already in the list p_list, it is added.
// The corresponding equilibrium is computed and output.
// Returns 'true' if the CBFS is new; 'false' if it already appears in the
// list.
//
template <class T> bool
NashLcpStrategySolver<T>::OnBFS(const StrategySupport &p_support,
				LHTableau<T> &p_tableau,
				Solution &p_solution) const
{
  BFS<T> cbfs(p_tableau.GetBFS());
  if (p_solution.Contains(cbfs)) {
    return false;
  }
  p_solution.push_back(cbfs);

  MixedStrategyProfile<T> profile(p_support.NewMixedStrategyProfile<T>());
  int n1 = p_support.NumStrategies(1);
  int n2 = p_support.NumStrategies(2);
  T sum = (T) 0;

  for (int j = 1; j <= n1; j++) {
    if (cbfs.count(j))   sum += cbfs[j];
  }

  if (sum == (T) 0)  {
    // This is the trivial CBFS.
    return false;
  }

  for (int j = 1; j <= n1; j++) {
    if (cbfs.count(j)) {
      profile[p_support.GetStrategy(1, j)] = cbfs[j] / sum;
    }
    else {
      profile[p_support.GetStrategy(1, j)] = (T) 0;
    }
  }

  sum = (T) 0;

  for (int j = 1; j <= n2; j++) {
    if (cbfs.count(n1 + j))  sum += cbfs[n1 + j];
  }

  for (int j = 1; j <= n2; j++) {
    if (cbfs.count(n1 + j)) {
      profile[p_support.GetStrategy(2, j)] = cbfs[n1 + j] / sum;
    }
    else {
      profile[p_support.GetStrategy(2, j)] = (T) 0;
    }
  }
  
  this->m_onEquilibrium->Render(profile);
  p_solution.m_equilibria.push_back(profile);

  if (m_stopAfter > 0 && p_solution.EquilibriumCount() >= m_stopAfter) {
    throw NashEquilibriumLimitReached();
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
NashLcpStrategySolver<T>::AllLemke(const StrategySupport &p_support,
				   int j, LHTableau<T> &B,
				   Solution &p_solution,
				   int depth) const
{
  if (m_maxDepth != 0 && depth > m_maxDepth) {
    return;
  }

  // On the initial depth=0 call, the CBFS we are at is the extraneous
  // solution.
  if (depth > 0 && !OnBFS(p_support, B, p_solution)) {
    return;
  }
  
  for (int i = B.MinCol(); i <= B.MaxCol(); i++) {
    if (i != j)  {
      LHTableau<T> Bcopy(B);
      Bcopy.LemkePath(i);
      AllLemke(p_support, i, Bcopy, p_solution, depth+1);
    }
  }
}

template <class T> List<MixedStrategyProfile<T> > 
NashLcpStrategySolver<T>::Solve(const StrategySupport &p_support) const
{
  Solution solution;

  try {
    Matrix<T> A1 = Make_A1<T>(p_support);
    Vector<T> b1 = Make_b1<T>(p_support);
    Matrix<T> A2 = Make_A2<T>(p_support);
    Vector<T> b2 = Make_b2<T>(p_support);
    LHTableau<T> B(A1, A2, b1, b2);

    if (m_stopAfter != 1) {
      try {
	AllLemke(p_support, 0, B, solution, 0);
      }
      catch (NashEquilibriumLimitReached &) {
	// This pseudo-exception requires no additional action;
	// solution contains details of all equilibria found
      }
    }
    else  {
      B.LemkePath(1);
      OnBFS(p_support, B, solution);
    }
  }
  catch (std::runtime_error &e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return solution.m_equilibria;
}

template class NashLcpStrategySolver<double>;
template class NashLcpStrategySolver<Rational>;



