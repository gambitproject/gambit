//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria via Lemke-Howson algorithm
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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
#include "lhtab.h"

using namespace Gambit;

extern int g_numDecimals;
extern bool g_printDetail;

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const MixedStrategyProfile<double> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream << "," << p_profile[i];
  }

  p_stream << std::endl;
}

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const MixedStrategyProfile<Rational> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << "," << std::setprecision(g_numDecimals) << p_profile[i];
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
	
      sprintf(buffer, "%11s   ", ToText(p_profile[strategy], g_numDecimals).c_str());
      p_stream << buffer;

      sprintf(buffer, "%11s   ", ToText(p_profile.GetStrategyValue(strategy), g_numDecimals).c_str());
      p_stream << buffer;

      p_stream << "\n";
    }
  }
}


template <class T> Matrix<T> Make_A1(const StrategySupport &, 
				      const T &);
template <class T> Vector<T> Make_b1(const StrategySupport &, 
				      const T &);
template <class T> Matrix<T> Make_A2(const StrategySupport &,
				      const T &);
template <class T> Vector<T> Make_b2(const StrategySupport &,
				      const T &);


template <class T> class nfgLcp {
private:
  int m_stopAfter, m_maxDepth;

  int AddBfs(LHTableau<T> &, List<BFS<T> > &);
  void AddSolutions(const StrategySupport &,
		    const List<BFS<T> > &, const T &);
  void AllLemke(const StrategySupport &, int, LHTableau<T> &B, List<BFS<T> > &,
		int depth);
  
public:
  nfgLcp(void);
  virtual ~nfgLcp() { }
  
  int StopAfter(void) const { return m_stopAfter; }
  void SetStopAfter(int p_stopAfter) { m_stopAfter = p_stopAfter; }

  int MaxDepth(void) const { return m_maxDepth; }
  void SetMaxDepth(int p_maxDepth) { m_maxDepth = p_maxDepth; }

  void Solve(const StrategySupport &);
};

//---------------------------------------------------------------------------
//                        nfgLcp: member functions
//---------------------------------------------------------------------------

template <class T> 
void nfgLcp<T>::Solve(const StrategySupport &p_support)
{
  BFS<T> cbfs((T) 0);

  if (p_support.GetGame()->NumPlayers() != 2) {
    return;
  }

  List<BFS<T> > bfsList;

  try {
    Matrix<T> A1 = Make_A1(p_support, (T) 0);
    Vector<T> b1 = Make_b1(p_support, (T) 0);
    Matrix<T> A2 = Make_A2(p_support, (T) 0);
    Vector<T> b2 = Make_b2(p_support, (T) 0);
    LHTableau<T> B(A1,A2,b1,b2);
    if (m_stopAfter != 1) {
      AllLemke(p_support,0,B,bfsList,m_maxDepth);
    }
    else  {
      B.LemkePath(1);
      AddBfs(B, bfsList);
    }

    return AddSolutions(p_support, bfsList, B.Epsilon());
  }
  catch (...) {
    // for now, we won't give *any* solutions -- but we should list
    // any solutions found!
    throw;
  }
  // any other exceptions will propagate out.
}

template <class T> int nfgLcp<T>::AddBfs(LHTableau<T> &p_tableau,
					      List<BFS<T> > &p_list)
{
  BFS<T> cbfs((T) 0);
  cbfs = p_tableau.GetBFS();
  if ((m_stopAfter > 0 && p_list.Length() > m_stopAfter) ||
      p_list.Contains(cbfs)) {  
    return 0;
  }
  p_list.Append(cbfs);
  return 1;
}

//
// AllLemke finds all accessible Nash equilibria by recursively 
// calling itself.  List maintains the list of basic variables 
// for the equilibria that have already been found.  
// From each new accessible equilibrium, it follows
// all possible paths, adding any new equilibria to the List.  
//
template <class T> void nfgLcp<T>::AllLemke(const StrategySupport &p_support,
					    int j, LHTableau<T> &B,
					    List<BFS<T> > &p_list,
					    int depth)
{
  if (m_maxDepth != 0 && depth > m_maxDepth) {
    return;
  }

  if (!AddBfs(B, p_list)) {
    return;
  }
  
  for (int i = B.MinCol(); 
       i <= B.MaxCol() &&
       (m_stopAfter==0 || (p_list.Length()-1) < m_stopAfter);
       i++) {
    if (i != j)  {
      LHTableau<T> Bcopy(B);
      Bcopy.LemkePath(i);
      AllLemke(p_support,i,Bcopy, p_list, depth+1);
    }
  }
  return;
}

template <class T>
void nfgLcp<T>::AddSolutions(const StrategySupport &p_support,
			     const List<BFS<T> > &p_list,
			     const T &epsilon)
{
  int i,j;
  int n1 = p_support.NumStrategies(1);
  int n2 = p_support.NumStrategies(2);

  for (i = 1; i <= p_list.Length(); i++)    {
    MixedStrategyProfile<T> profile(p_support);
    T sum = (T) 0;

    for (j = 1; j <= n1; j++)
      if (p_list[i].IsDefined(j))   sum += p_list[i](j);

    if (sum == (T) 0)  continue;

    for (j = 1; j <= n1; j++) {
      if (p_list[i].IsDefined(j)) {
	profile[p_support.GetStrategy(1, j)] = p_list[i](j) / sum;
      }
      else {
	profile[p_support.GetStrategy(1, j)] = (T) 0;
      }
    }

    sum = (T) 0;

    for (j = 1; j <= n2; j++)
      if (p_list[i].IsDefined(n1 + j))  
	sum += p_list[i](n1 + j);

    if (sum == (T) 0)  continue;

    for (j = 1; j <= n2; j++) {
      if (p_list[i].IsDefined(n1 + j)) {
	profile[p_support.GetStrategy(2, j)] = p_list[i](n1 + j) / sum;
      }
      else {
	profile[p_support.GetStrategy(2, j)] = (T) 0;
      }
    }
    PrintProfile(std::cout, "NE", profile);
    if (g_printDetail) {
      PrintProfileDetail(std::cout, profile);
    }
  }
}

//-------------------------------------------------------------------------
//                    nfgLcp<T>: Member functions
//-------------------------------------------------------------------------

template <class T>
nfgLcp<T>::nfgLcp(void)
  : m_stopAfter(0), m_maxDepth(0)
{ }


template <class T>
void SolveStrategic(const Game &p_game)
{
  nfgLcp<T> algorithm;
  algorithm.Solve(p_game);
}

template void SolveStrategic<double>(const Game &);
template void SolveStrategic<Rational>(const Game &);


