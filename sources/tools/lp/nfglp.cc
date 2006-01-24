//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of algorithm to compute mixed strategy equilibria
// of constant sum normal form games via linear programming
//
// This file is part of Gambit
// Copyright (c) 2006, The Gambit Project
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

#include <unistd.h>
#include <iostream>
#include "libgambit/libgambit.h"
#include "lpsolve.h"

extern int g_numDecimals;

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const Gambit::MixedStrategyProfile<double> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream << "," << p_profile[i];
  }

  p_stream << std::endl;
}

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const Gambit::MixedStrategyProfile<Gambit::Rational> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << "," << std::setprecision(g_numDecimals) << p_profile[i];
  }

  p_stream << std::endl;
}

template <class T> class nfgLp {
private:
  int Add_BFS(const Gambit::StrategySupport &, /*const*/ LPSolve<T> &B,
	      Gambit::List<BFS<T> > &);
  void GetSolutions(const Gambit::StrategySupport &, const Gambit::List<BFS<T> > &,
		    const T &) const;

public:
  nfgLp(void);
  virtual ~nfgLp() { }

  void Solve(const Gambit::StrategySupport &);
};

//-------------------------------------------------------------------------
//                      nfgLp<T>: Member functions
//-------------------------------------------------------------------------

template <class T> nfgLp<T>::nfgLp(void)
{ }

template <class T> 
void nfgLp<T>::Solve(const Gambit::StrategySupport &p_support)
{
  BFS<T> cbfs((T) 0);
  
  if (p_support.GetGame()->NumPlayers() != 2 || 
      !p_support.GetGame()->IsConstSum()) {
    return; 
  }
  
  Gambit::List<BFS<T> > bfsList;
  
  int i,j,m,k;
  
  m = p_support.NumStrategies(1);
  k = p_support.NumStrategies(2);
  Gambit::Matrix<T> A(1,k+1,1,m+1);
  Gambit::Vector<T> b(1,k+1);
  Gambit::Vector<T> c(1,m+1);
  Gambit::PureStrategyProfile profile(p_support.GetGame());

  T minpay = p_support.GetGame()->GetMinPayoff() - Gambit::Rational(1);

  for (i = 1; i <= k; i++)  {
    profile.SetStrategy(p_support.GetStrategy(2, i));
    for (j = 1; j <= m; j++)  {
      profile.SetStrategy(p_support.GetStrategy(1, j));
      A(i, j) = Gambit::Rational(minpay) - profile.GetPayoff<Gambit::Rational>(1);
    }
    A(i,m+1) = (T)1;
  }
  for (j = 1;j<=m;j++)  {
    A(k+1,j)= (T)1;
  }
  A(k+1,m+1) = (T)0;

  b = (T)0;
  b[k+1] = (T)1;
  c = (T)0;
  c[m+1] = (T)1;

  LPSolve<T> LP(A,b,c,1);

  if (!LP.IsAborted()) {
    Add_BFS(p_support, LP, bfsList); 
  }

  GetSolutions(p_support, bfsList, LP.Epsilon());
}

template <class T> int nfgLp<T>::Add_BFS(const Gambit::StrategySupport &p_support,
					 /*const*/ LPSolve<T> &lp,
					 Gambit::List<BFS<T> > &p_list)
{
  BFS<T> cbfs((T) 0);

  // LPSolve<T>::GetAll() does not currently work correctly; for now,
  // LpSolve is restricted to returning only one equilibrium
  lp.OptBFS(cbfs);
  cbfs.Remove(p_support.GetGame()->GetPlayer(1)->NumStrategies()+1);
  cbfs.Remove(-p_support.GetGame()->GetPlayer(2)->NumStrategies()-1);
  if (p_list.Contains(cbfs))  return 0;
  p_list.Append(cbfs);
  return 1;
}

template <class T>
void nfgLp<T>::GetSolutions(const Gambit::StrategySupport &p_support,
			    const Gambit::List<BFS<T> > &p_list,
			    const T &p_epsilon) const
{
  int n1 = p_support.NumStrategies(1);
  int n2 = p_support.NumStrategies(2);

  for (int i = 1; i <= p_list.Length(); i++)    {
    Gambit::MixedStrategyProfile<T> profile(p_support);
    int j;
    for (j = 1; j <= n1; j++) 
      if (p_list[i].IsDefined(j))   
	profile(1, j) = p_list[i](j);
      else  profile(1, j) = (T) 0;

    for (j = 1; j <= n2; j++)
      if (p_list[i].IsDefined(-j))
	profile(2, j) = p_list[i](-j);
      else
	profile(2, j) = (T) 0;

    PrintProfile(std::cout, "NE", profile);
  }
}

template <class T>
void SolveStrategic(const Gambit::Game &p_game)
{
  nfgLp<T> algorithm;
  algorithm.Solve(p_game);
}

template void SolveStrategic<double>(const Gambit::Game &);
template void SolveStrategic<Gambit::Rational>(const Gambit::Game &);
