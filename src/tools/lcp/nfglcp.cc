//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
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
#include "lhtab.h"

using namespace Gambit;

extern int g_numDecimals, g_stopAfter, g_maxDepth;
extern bool g_printDetail;

namespace {
//
// Pseudo-exception raised when maximum number of equilibria to compute
// has been reached.  A convenience for unraveling a potentially
// deep recursion.
//
// FIXME: There is an identical twin of this in efglcp.cc.  This should be
// refactored into a more generally-useful and generally-visible location.
//
class EquilibriumLimitReachedNfg : public Exception {
public:
  virtual ~EquilibriumLimitReachedNfg() throw() { }
  const char *what(void) const throw() { return "Reached target number of equilibria"; }
};

} // end anonymous namespace


void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const MixedStrategyProfile<double> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.MixedProfileLength(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << "," << std::setprecision(g_numDecimals) << p_profile[i];
  }

  p_stream << std::endl;
}

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const MixedStrategyProfile<Rational> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.MixedProfileLength(); i++) {
    p_stream << "," << p_profile[i];
  }

  p_stream << std::endl;
}

template <class T>
void PrintProfileDetail(std::ostream &p_stream,
			const MixedStrategyProfile<T> &p_profile)
{
  char buffer[256];

  for (int pl = 1; pl <= p_profile.GetGame()->NumPlayers(); pl++) {
    GamePlayer player = p_profile.GetGame()->GetPlayer(pl);
    p_stream << "Strategy profile for player " << pl << ":\n";
    
    p_stream << "Strategy   Prob          Value\n";
    p_stream << "-------    -----------   -----------\n";

    for (int st = 1; st <= player->NumStrategies(); st++) {
      GameStrategy strategy = player->GetStrategy(st);

      if (strategy->GetLabel() != "") {
	sprintf(buffer, "%7s    ", strategy->GetLabel().c_str());
      }
      else {
	sprintf(buffer, "%7d   ", st);
      }
      p_stream << buffer;
	
      sprintf(buffer, "%11s   ", lexical_cast<std::string>(p_profile[strategy], g_numDecimals).c_str());
      p_stream << buffer;

      sprintf(buffer, "%11s   ", lexical_cast<std::string>(p_profile.GetPayoff(strategy), g_numDecimals).c_str());
      p_stream << buffer;

      p_stream << "\n";
    }
  }
}


template <class T> Matrix<T> Make_A1(const StrategySupport &); 
template <class T> Vector<T> Make_b1(const StrategySupport &);
template <class T> Matrix<T> Make_A2(const StrategySupport &);
template <class T> Vector<T> Make_b2(const StrategySupport &);


//
// Function called when a CBFS is encountered.
// If it is not already in the list p_list, it is added.
// The corresponding equilibrium is computed and output.
// Returns 'true' if the CBFS is new; 'false' if it already appears in the
// list.
//
template <class T>
bool OnBFS(const StrategySupport &p_support,
	   List<BFS<T> > &p_list, LHTableau<T> &p_tableau)
{
  BFS<T> cbfs(p_tableau.GetBFS());
  if (p_list.Contains(cbfs)) {
    return false;
  }

  p_list.Append(cbfs);

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
  
  PrintProfile(std::cout, "NE", profile);
  if (g_printDetail) {
    PrintProfileDetail(std::cout, profile);
  }

  if (g_stopAfter > 0 && p_list.Length() >= g_stopAfter) {
    throw EquilibriumLimitReachedNfg();
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
template <class T> void AllLemke(const StrategySupport &p_support,
				 int j, LHTableau<T> &B,
				 List<BFS<T> > &p_list,
				 int depth)
{
  if (g_maxDepth != 0 && depth > g_maxDepth) {
    return;
  }

  // On the initial depth=0 call, the CBFS we are at is the extraneous
  // solution.
  if (depth > 0 && !OnBFS(p_support, p_list, B)) {
    return;
  }
  
  for (int i = B.MinCol(); i <= B.MaxCol(); i++) {
    if (i != j)  {
      LHTableau<T> Bcopy(B);
      Bcopy.LemkePath(i);
      AllLemke(p_support, i, Bcopy, p_list, depth+1);
    }
  }
}

template <class T>
void SolveStrategic(const Game &p_game)
{
  StrategySupport support(p_game);
  List<BFS<T> > bfsList;

  try {
    Matrix<T> A1 = Make_A1<T>(support);
    Vector<T> b1 = Make_b1<T>(support);
    Matrix<T> A2 = Make_A2<T>(support);
    Vector<T> b2 = Make_b2<T>(support);
    LHTableau<T> B(A1, A2, b1, b2);

    if (g_stopAfter != 1) {
      try {
	AllLemke(support, 0, B, bfsList, 0);
      }
      catch (EquilibriumLimitReachedNfg &) {
	// This pseudo-exception requires no additional action;
	// bfsList will contain the list of equilibria found
      }
    }
    else  {
      B.LemkePath(1);
      OnBFS(support, bfsList, B);
    }

    return;
  }
  catch (...) {
    // for now, we won't give *any* solutions -- but we should list
    // any solutions found!
    throw;
  }
}

template void SolveStrategic<double>(const Game &);
template void SolveStrategic<Rational>(const Game &);


