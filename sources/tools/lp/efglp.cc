//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of algorithm to solve efgs via linear programming
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

#include "tableau.h"
#include "lpsolve.h"

extern int g_numDecimals;

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const Gambit::MixedBehavProfile<double> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream << "," << p_profile[i];
  }

  p_stream << std::endl;
}

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const Gambit::MixedBehavProfile<Gambit::Rational> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << "," << std::setprecision(g_numDecimals) << p_profile[i];
  }

  p_stream << std::endl;
}

template <class T> class efgLp {
private:
  T maxpay, minpay;
  int ns1,ns2,ni1,ni2;
  Gambit::List<BFS<T> > List;
  Gambit::List<Gambit::GameInfoset> isets1, isets2;

  void FillTableau(const Gambit::BehavSupport &,
		   Gambit::Matrix<T> &, const Gambit::GameNode &, T ,int ,int , int ,int );
  void GetSolutions(const Gambit::BehavSupport &) const;
  int Add_BFS(/*const*/ LPSolve<T> &B);
  
  void GetProfile(const Gambit::BehavSupport &, Gambit::DVector<T> &v, const BFS<T> &sol,
		  const Gambit::GameNode &n, int s1,int s2) const;

public:
  efgLp(void);
  virtual ~efgLp() { }

  void Solve(const Gambit::BehavSupport &);
};


//-------------------------------------------------------------------------
//                      efgLp<T>: Member functions
//-------------------------------------------------------------------------

template <class T>
efgLp<T>::efgLp(void)
{ }

//
// Sets the action probabilities at unreached information sets
// which are left undefined by the sequence form method to
// the centroid.  This helps IsNash and LiapValue work correctly.
//
template <class T>
void UndefinedToCentroid(Gambit::MixedBehavProfile<T> &p_profile)
{
  Gambit::Game efg = p_profile.GetGame();

  for (int pl = 1; pl <= efg->NumPlayers(); pl++) {
    Gambit::GamePlayer player = efg->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      Gambit::GameInfoset infoset = player->GetInfoset(iset);
      
      if (p_profile.GetIsetProb(infoset) > (T) 0) {
	continue;
      }
	  
      T total = (T) 0;
      for (int act = 1; act <= infoset->NumActions(); act++) {
	total += p_profile.GetActionProb(infoset->GetAction(act));
      }

      if (total == (T) 0) {
	for (int act = 1; act <= infoset->NumActions(); act++) {
	  p_profile(pl, iset, act) = (T) 1.0 / (T) infoset->NumActions();
	}
      }
    }
  }
}

template <class T> 
void efgLp<T>::Solve(const Gambit::BehavSupport &p_support)
{
  BFS<T> cbfs((T) 0);
  
  ns1 = p_support.NumSequences(1);
  ns2 = p_support.NumSequences(2);
  ni1 = p_support.GetGame()->GetPlayer(1)->NumInfosets()+1;  
  ni2 = p_support.GetGame()->GetPlayer(2)->NumInfosets()+1; 
  isets1 = p_support.ReachableInfosets(p_support.GetGame()->GetPlayer(1));
  isets2 = p_support.ReachableInfosets(p_support.GetGame()->GetPlayer(2)); 

  if (p_support.GetGame()->NumPlayers() != 2 ||
      !p_support.GetGame()->IsConstSum() ||
      !p_support.GetGame()->IsPerfectRecall()) {
    return;
  }
  
  List = Gambit::List<BFS<T> >();
  
  Gambit::Matrix<T> A(1,ns1+ni2,1,ns2+ni1);
  Gambit::Vector<T> b(1,ns1+ni2);
  Gambit::Vector<T> c(1,ns2+ni1);

  maxpay = p_support.GetGame()->GetMaxPayoff() + Gambit::Rational(1);
  minpay = p_support.GetGame()->GetMinPayoff() - Gambit::Rational(1);

  A = (T)0;
  T prob = (T)1;
  FillTableau(p_support, A, p_support.GetGame()->GetRoot(),prob,1,1,0,0);
  A(1,ns2+1) = -(T)1;
  A(ns1+1,1) = (T)1;

  b = (T)0;
  b[ns1+1] = (T)1;

  c = (T)0;
  c[ns2+1] = -(T)1;

  LPSolve<T> LP(A,b,c,ni2);
  if (!LP.IsAborted()) {
    Add_BFS(LP); 
  }

  GetSolutions(p_support);
}


template <class T> int efgLp<T>::Add_BFS(/*const*/ LPSolve<T> &lp)
{
  BFS<T> cbfs((T) 0);

  // LPSolve<T>::GetAll() does not currently work correctly; for now,
  // LpSolve is restricted to returning only one equilibrium 
  lp.OptBFS(cbfs);
  if (List.Contains(cbfs))  return 0;
  List.Append(cbfs);
  return 1;
}



template <class T> void efgLp<T>::GetProfile(const Gambit::BehavSupport &p_support,
					     Gambit::DVector<T> &v,
					     const BFS<T> &sol,
					     const Gambit::GameNode &n,
					     int s1,int s2) const
{
  
  int i,pl,inf,snew;
  T eps = (T)0;
//  eps = tab->Epsilon();
  if(n->GetInfoset()) {
    if(n->GetPlayer()->IsChance()) {
      for(i=1;i<=n->NumChildren();i++)
	GetProfile(p_support,v,sol,n->GetChild(i),s1,s2);
    }
    pl = n->GetPlayer()->GetNumber();
    if(pl==2) {
    inf= isets2.Find(n->GetInfoset());
      snew=1;
      for(i=1;i<inf;i++)
	snew+=p_support.NumActions(isets2[i]->GetPlayer()->GetNumber(), isets2[i]->GetNumber()); 
      for(i=1;i<=p_support.NumActions(n->GetInfoset()->GetPlayer()->GetNumber(), n->GetInfoset()->GetNumber());i++) {
	v(pl,inf,i) = (T)0;
	if(sol.IsDefined(s1)) {
	  if(sol(s1)>eps) {
	    if(sol.IsDefined(snew+i)) {
	      if(sol(snew+i)>eps)
		v(pl,inf,i) = sol(snew+i)/sol(s1);
	    }
	  } 
	} 
	GetProfile(p_support,v,sol,n->GetChild(p_support.GetAction(n->GetInfoset()->GetPlayer()->GetNumber(), n->GetInfoset()->GetNumber(), i)->GetNumber()),snew+i,s2);
      }
    }
    if(pl==1) {
    inf= isets1.Find(n->GetInfoset());
      snew=1;
      for(i=1;i<inf;i++)
	snew+=p_support.NumActions(isets1[i]->GetPlayer()->GetNumber(), isets1[i]->GetNumber()); 
      for(i=1;i<=p_support.NumActions(n->GetInfoset()->GetPlayer()->GetNumber(), n->GetInfoset()->GetNumber());i++) {
	v(pl,inf,i) = (T)0;
	if(sol.IsDefined(-s2)) {
	  if(sol(-s2)>eps) {
	    if(sol.IsDefined(-(snew+i))) {
	      if(sol(-(snew+i))>eps)
		v(pl,inf,i) = sol(-(snew+i))/sol(-s2);
	    }
	  } 
	} 
	GetProfile(p_support,v,sol,n->GetChild(p_support.GetAction(n->GetInfoset()->GetPlayer()->GetNumber(), n->GetInfoset()->GetNumber(), i)->GetNumber()),s1,snew+i);
      }
    }
  }
}


template <class T>
void efgLp<T>::FillTableau(const Gambit::BehavSupport &p_support,
			   Gambit::Matrix<T> &A, const Gambit::GameNode &n, T prob,
			   int s1, int s2, int i1, int i2)
{
  int i,snew;
  Gambit::GameOutcome outcome = n->GetOutcome();
  if (outcome) {
    A(s1,s2) = Gambit::Rational(A(s1,s2)) +
      Gambit::Rational(prob) * outcome->GetPayoff<Gambit::Rational>(1) - Gambit::Rational(minpay);
  }
  if(n->GetInfoset()) {
    if(n->GetPlayer()->IsChance()) {
      Gambit::GameInfoset infoset = n->GetInfoset();
      for(i=1;i<=n->NumChildren();i++)
	FillTableau(p_support, A, n->GetChild(i),
		    Gambit::Rational(prob) * infoset->GetActionProb<Gambit::Rational>(i),
		    s1,s2,i1,i2);
    }
    int pl = n->GetPlayer()->GetNumber();
    if(pl==1) {
      i1=isets1.Find(n->GetInfoset());
      snew=1;
      for(i=1;i<i1;i++)
	snew+=p_support.NumActions(isets1[i]->GetPlayer()->GetNumber(), isets1[i]->GetNumber());
      A(s1,ns2+i1+1) = (T) +1;
      for(i=1;i<=p_support.NumActions(n->GetInfoset()->GetPlayer()->GetNumber(), n->GetInfoset()->GetNumber());i++) {
	A(snew+i,ns2+i1+1) = (T) -1;
	FillTableau(p_support, A, n->GetChild(p_support.GetAction(n->GetInfoset()->GetPlayer()->GetNumber(), n->GetInfoset()->GetNumber(), i)->GetNumber()),prob,snew+i,s2,i1,i2);
      }
    }
    if(pl==2) {
      i2=isets2.Find(n->GetInfoset());
      snew=1;
      for(i=1;i<i2;i++)
	snew+=p_support.NumActions(isets2[i]->GetPlayer()->GetNumber(), isets2[i]->GetNumber());
      A(ns1+i2+1,s2) = (T) -1;
      for(i=1;i<=p_support.NumActions(n->GetInfoset()->GetPlayer()->GetNumber(), n->GetInfoset()->GetNumber());i++) {
	A(ns1+i2+1,snew+i) = (T) +1;
	FillTableau(p_support, A, n->GetChild(p_support.GetAction(n->GetInfoset()->GetPlayer()->GetNumber(), n->GetInfoset()->GetNumber(), i)->GetNumber()),prob,s1,snew+i,i1,i2);
      }
    }
  }
}

template <class T>
void efgLp<T>::GetSolutions(const Gambit::BehavSupport &p_support) const
{
  for (int i = 1; i <= List.Length(); i++)    {
    Gambit::MixedBehavProfile<T> profile(p_support);
    GetProfile(p_support,
	       profile.GetDPVector(), List[i],
	       p_support.GetGame()->GetRoot(), 1, 1);
    UndefinedToCentroid(profile);
    PrintProfile(std::cout, "NE", profile);
  }

}

template <class T>
void SolveExtensive(const Gambit::Game &p_game)
{
  efgLp<T> algorithm;
  algorithm.Solve(p_game);
}

template void SolveExtensive<double>(const Gambit::Game &);
template void SolveExtensive<Gambit::Rational>(const Gambit::Game &);
