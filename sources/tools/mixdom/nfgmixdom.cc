//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute dominated mixed strategies on normal forms
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

#include <iostream>
#include "libgambit/libgambit.h"
#include "lpsolve.h"

template <class T>
bool ComputeMixedDominated(const Gambit::StrategySupport &S, Gambit::StrategySupport &R,
			   int pl, bool strong, T /*junk*/,
			   std::ostream &tracefile)
{
  Gambit::Game nfg = S.GetGame();
  
  gbtArray<bool> dom(S.NumStrats(pl));
  
  T eps;
  gEpsilon(eps);
  
  Gambit::Vector<T> dominator(S.NumStrats(pl));
  
  int st,i,k,n;
  
  if (strong)   {
    T COpt;
    bool ret = false;
    int strats = S.NumStrats(pl);
    int contingencies = 1;
    for(k=1;k<=nfg->NumPlayers();k++)
      if(k!=pl) contingencies*=S.NumStrats(k);
    
    Gambit::Matrix<T> A(1,contingencies+1,1,strats);
    Gambit::Vector<T> B(1,contingencies+1);
    Gambit::Vector<T> C(1,strats);
    
    n = contingencies + 1;
    for (k = 1; k < strats; k++) {
      C[k] = (T) 0;
      A(n, k) = (T) 1;
    }
    A(n, k) = (T) 0;
    B[n] = (T) 1;
    C[k] = (T) 1;
    
    Gambit::StrategyIterator s(S, pl, 1);

    for (n = 1; n <= contingencies; n++) {
      Gambit::PureStrategyProfile profile(s.GetProfile());
      profile.SetStrategy(S.GetStrategy(pl, 1));
      B[n] = -profile.GetPayoff(pl);
      for (k = 2; k <= strats; k++) {
	profile.SetStrategy(S.GetStrategy(pl, k));
	A(n, k - 1) = -profile.GetPayoff(pl);
      }
      A(n, strats) = (T) 1;
      s.NextContingency();
    }
    
    for (k = 1; k <= strats; k++) {
      double s1 = (double)k/(double)(strats);
      // tracefile << '\n' << (gbtRectArray<T> &)A << '\n';
      // tracefile << B << '\n';
      // tracefile << C << '\n';
      LPSolve<T> Tab(A, B, C, 1);
      
      COpt = Tab.OptimumCost();
      tracefile << "\nPlayer = " << pl << " Strat = "<< k;
      // tracefile << " F = " << Tab.IsFeasible();
      // tracefile << " x = " << Tab.OptimumVector();
      // tracefile << " Obj = " << COpt;
      
      dom[k] = false;
      
      if (Tab.IsFeasible() && COpt > eps) {
	tracefile << " Strongly Dominated by ";
	Gambit::Vector<T> xx(Tab.OptimumVector());
	for(i=1,st=1;st<=strats;st++) {
	  if(st==k) dominator[st] = (T)0;
	  else {
	    dominator[st] = xx[i];
	    i++;
	  }
	}
	for (int z = 1; z <= dominator.Length(); z++) {
	  tracefile << dominator[z] << " ";
	}
	ret = true;
	dom[k] = true;
      }
      if (k<strats)
	A.SwitchColumn(k,B);
    }
    // tracefile << "\n";
    
    if (!ret) 
      return false;
    
    for (k = 1; k <= strats; k++)
      if (dom[k])
	R.RemoveStrategy(S.GetStrategy(pl, k));
    
    return true;
  }
  else  {    // look for weak domination
    T C0 = (T) 0, COpt, TmpC;
    bool ret = false;
    int strats = S.NumStrats(pl);
    int contingencies = 1;
    for(k=1;k<=nfg->NumPlayers();k++)
      if(k!=pl) contingencies*=S.NumStrats(k);
    
    Gambit::Matrix<T> A(1,contingencies+1,1,strats-1);
    Gambit::Vector<T> B(1,contingencies+1);
    Gambit::Vector<T> C(1,strats-1);
    
    n=contingencies+1;
    for(k=1;k<strats;k++) {
      C[k] = (T) 0;
      A(n,k)=(T) 1;
    }
    B[n]=(T)1;
    
    Gambit::StrategyIterator s(S, pl, 1);

    for(n=1;n<=contingencies;n++) {
      Gambit::PureStrategyProfile profile(s.GetProfile());
      profile.SetStrategy(S.GetStrategy(pl, 1));
      B[n]=-profile.GetPayoff(pl);
      C0 -= B[n];
      for(k=2;k<=strats;k++) {
	profile.SetStrategy(S.GetStrategy(pl, k));
	A(n,k-1)=-profile.GetPayoff(pl);
	C[k-1]-=A(n,k-1);
      }
      s.NextContingency();
    }
    
    for (k = 1; k <= strats; k++) {
      double s1 = (double)k/(double)(strats);
      // tracefile << '\n' << (gbtRectArray<T> &)A << '\n';
      // tracefile << B << '\n';
      // tracefile << C << '\n';
      LPSolve<T> Tab(A, B, C, 1);
      
      COpt = Tab.OptimumCost();
      tracefile << "\nPlayer = " << pl << " Strat = "<< k;
      // tracefile << " F = " << Tab.IsFeasible();
      // tracefile << " x = " << Tab.OptimumVector();
      // tracefile << " Obj = " << COpt;
      
      dom[k] = false;
      
      if (Tab.IsFeasible() && (COpt >= C0-eps && COpt <=C0+eps))
	tracefile << " Duplicated strategy? ";
      else if (Tab.IsFeasible() && COpt > C0+eps) {
	tracefile << " Weakly Dominated by ";
	Gambit::Vector<T> xx(Tab.OptimumVector());
	for(i=1,st=1;st<=strats;st++) {
	  if(st==k) dominator[st] = (T)0;
	  else {
	    dominator[st] = xx[i];
	    i++;
	  }
	}
	for (int z = 1; z <= dominator.Length(); z++) {
	  tracefile << dominator[z] << ' ';
	}
	ret = true;
	dom[k] = true;
      }
      // else  tracefile << "\n\n";
      if(k<strats) {
	A.SwitchColumn(k,B);
	TmpC=C0; C0=C[k]; C[k]=TmpC;
      }
      
    }
    // tracefile << "\n";
    
    if (!ret) 
      return false;
    
    for (k = 1; k <= strats; k++)
      if (dom[k])
	R.RemoveStrategy(S.GetStrategy(pl, k));
    
    return true;
  }
  
}

template <class T>
bool IsMixedDominated(const Gambit::StrategySupport &S, Gambit::GameStrategy str,
		      bool strong, T /*junk*/,
		      std::ostream &tracefile)
{
  int pl = str->GetPlayer()->GetNumber();
  Gambit::Game nfg = S.GetGame();
  int whichstrat = str->GetNumber();
  int strats = S.NumStrats(pl);

  T eps;
  gEpsilon(eps);
  
  Gambit::Vector<T> dominator(S.NumStrats(pl));
  
  int st,i,k,n;
  bool ret = false;
  int contingencies = 1;
  for(k=1;k<=nfg->NumPlayers();k++)
    if(k!=pl) contingencies*=S.NumStrats(k);
  
  
  if (strong)   {
    T COpt;
    Gambit::Matrix<T> A(1,contingencies+1,1,strats);
    Gambit::Vector<T> B(1,contingencies+1);
    Gambit::Vector<T> C(1,strats);
    
    n = contingencies + 1;
    for (k = 1; k < strats; k++) {
      C[k] = (T) 0;
      A(n, k) = (T) 1;
    }
    A(n, k) = (T) 0;
    B[n] = (T) 1;
    C[k] = (T) 1;
    
    Gambit::StrategyIterator s(S, pl, whichstrat);

    for (n = 1; n <= contingencies; n++) {
      B[n] = -s.GetPayoff(pl);
      for (k = 1; k <= strats; k++) {
	Gambit::PureStrategyProfile profile(s.GetProfile());
	profile.SetStrategy(S.GetStrategy(pl, k));

	if(k< whichstrat) {
	  A(n, k) = -s.GetPayoff(pl);
	}
	else if (k > whichstrat) {
	  A(n, k - 1) = -s.GetPayoff(pl);
	}
      }
      A(n, strats) = (T) 1;
      s.NextContingency();
    }
    
    // tracefile << '\n' << (gbtRectArray<T> &)A << '\n';
    // tracefile << B << '\n';
    // tracefile << C << '\n';
    LPSolve<T> Tab(A, B, C, 1);
    COpt = Tab.OptimumCost();
    tracefile << "\nPlayer = " << pl << " Strat = "<< whichstrat;
    // tracefile << " F = " << Tab.IsFeasible();
    // tracefile << " x = " << Tab.OptimumVector();
    // tracefile << " Obj = " << COpt;
    
    if (Tab.IsFeasible() && COpt > eps) {
      tracefile << " Strongly Dominated by ";
      Gambit::Vector<T> xx(Tab.OptimumVector());
      for(i=1,st=1;st<=strats;st++) {
	if(st==whichstrat) dominator[st] = (T)0;
	else {
	  dominator[st] = xx[i];
	  i++;
	}
      }
      for (int z = 1; z <= dominator.Length(); z++) {
	tracefile << dominator[z] << ' ';
      }
      ret = true;
    }
    // tracefile << "\n";
    
    if (!ret) 
      return false;
    
    return true;
  }
  else  {    // look for weak domination
    T C0 = (T) 0, COpt;

    Gambit::Matrix<T> A(1,contingencies+1,1,strats-1);
    Gambit::Vector<T> B(1,contingencies+1);
    Gambit::Vector<T> C(1,strats-1);
    
    n=contingencies+1;
    for(k=1;k<strats;k++) {
      C[k] = (T) 0;
      A(n,k)=(T) 1;
    }
    B[n]=(T)1;
    
    Gambit::StrategyIterator s(S, pl, whichstrat);

    for(n=1;n<=contingencies;n++) {
      B[n]=-s.GetPayoff(pl);
      C0 -= B[n];
      for(k=1;k<=strats;k++) {
	Gambit::PureStrategyProfile profile(s.GetProfile());
	profile.SetStrategy(S.GetStrategy(pl, k));

	if(k<whichstrat) {
	  A(n,k)=-s.GetPayoff(pl);
	  C[k]-=A(n,k);
	}
	else if (k > whichstrat) {
	  A(n,k-1)=-s.GetPayoff(pl);
	  C[k-1]-=A(n,k-1);
	}
      }
      s.NextContingency();
    }
    
    // tracefile << '\n' << (gbtRectArray<T> &)A << '\n';
    // tracefile << B << '\n';
    // tracefile << C << '\n';
    LPSolve<T> Tab(A, B, C, 1);
    
    COpt = Tab.OptimumCost();
    tracefile << "\nPlayer = " << pl << " Strat = "<< whichstrat;
    // tracefile << " F = " << Tab.IsFeasible();
    // tracefile << " x = " << Tab.OptimumVector();
    // tracefile << " Obj = " << COpt;
    
    if (Tab.IsFeasible() && (COpt >= C0-eps && COpt <=C0+eps))
      tracefile << " Duplicated strategy? ";
    else if (Tab.IsFeasible() && COpt > C0+eps) {
      tracefile << " Weakly Dominated by ";
      Gambit::Vector<T> xx(Tab.OptimumVector());
      for(i=1,st=1;st<=strats;st++) {
	if(st==whichstrat) dominator[st] = (T)0;
	else {
	  dominator[st] = xx[i];
	  i++;
	}
      }
      for (int z = 1; z <= dominator.Length(); z++) {
	tracefile << dominator[z] << " ";
      }
      ret = true;
    }
    // else  tracefile << "\n\n";
    // tracefile << "\n";
    
    if (!ret) 
      return false;
    
    return true;
  }
  
}

template <class T>
bool IsMixedDominated(const Gambit::MixedStrategyProfile<T> &pr, int pl,
		      bool strong, std::ostream &tracefile)
{
  Gambit::StrategySupport S = pr.GetSupport();
  Gambit::Game nfg = pr.GetGame();
  int strats = S.NumStrats(pl);
  Gambit::Vector<T> prob = pr.GetRow(pl);
  assert( prob.Length() == strats);

  T eps,x;
  gEpsilon(eps);
  
  Gambit::Vector<T> dominator(S.NumStrats(pl));
  
  int st,i,k,n;
  bool ret = false;
  int contingencies = 1;
  for(k=1;k<=nfg->NumPlayers();k++)
    if(k!=pl) contingencies*=S.NumStrats(k);
  
  if (strong)   {
    T COpt;
    Gambit::Matrix<T> A(1,contingencies+1,1,strats+1);
    Gambit::Vector<T> B(1,contingencies+1);
    Gambit::Vector<T> C(1,strats+1);
    
    n = contingencies + 1;
    for (k = 1; k <= strats; k++) {
      C[k] = (T) 0;
      A(n, k) = (T) 1;
    }
    A(n, k) = (T) 0;
    B[n] = (T) 1;
    C[k] = (T) 1;
    
    Gambit::StrategyIterator s(S, pl, 1);
    for (n = 1; n <= contingencies; n++) {
      Gambit::PureStrategyProfile profile(s.GetProfile());
      B[n]=(T)0;
      for(int j=1;j<=strats;j++) {
	profile.SetStrategy(S.GetStrategy(pl, j));
	T x1 = profile.GetPayoff(pl);
	T x2 = prob[j];
	x = -x1*x2;
	B[n] += -profile.GetPayoff(pl);
      }
      for (k = 1; k <= strats; k++) {
	profile.SetStrategy(S.GetStrategy(pl, k));
	A(n, k) = -profile.GetPayoff(pl);
      }
      A(n, strats+1) = (T) 1;
      s.NextContingency();
    }

    LPSolve<T> Tab(A, B, C, 1);
    COpt = Tab.OptimumCost();
    tracefile << "\nPlayer = " << pl << " Strat: ";
    for (int z = 1; z <= prob.Length(); z++) {
      tracefile << prob[z] << ' ';
    }
    
    if (Tab.IsFeasible() && COpt > eps) {
      tracefile << " Strongly Dominated by ";
      Gambit::Vector<T> xx(Tab.OptimumVector());
      for(i=1,st=1;st<=strats;st++) {
	dominator[st] = xx[i];
	i++;
      }
      for (int z = 1; z <= dominator.Length(); z++) {
	tracefile << dominator[z] << ' ';
      }
      ret = true;
    }
    // tracefile << "\n";
    
    if (!ret) 
      return false;
    
    return true;
  }
  else  {    // look for weak domination
    T C0 = (T) 0, COpt;

    Gambit::Matrix<T> A(1,contingencies+1,1,strats);
    Gambit::Vector<T> B(1,contingencies+1);
    Gambit::Vector<T> C(1,strats);
    
    n=contingencies+1;
    for(k=1;k<=strats;k++) {
      C[k] = (T) 0;
      A(n,k)=(T) 1;
    }
    B[n]=(T)1;
    
    Gambit::StrategyIterator s(S, pl, 1);

    for(n=1;n<=contingencies;n++) {
      Gambit::PureStrategyProfile profile(s.GetProfile());
      B[n]=(T)0;
      for(int j=1;j<=strats;j++) {
	profile.SetStrategy(S.GetStrategy(pl, j));
	T x1 = profile.GetPayoff(pl);
	T x2 = prob[j];
	x = - x1 * x2;
	B[n]+=x;
	C0 -= x;
      }
      for(k=1;k<=strats;k++) {
	profile.SetStrategy(S.GetStrategy(pl, k));
	x=-profile.GetPayoff(pl);
	A(n,k)=x;
	C[k]-=x;
      }
      s.NextContingency();
    }
    
    // tracefile << '\n' << (gbtRectArray<T> &)A << '\n';
    // tracefile << B << '\n';
    // tracefile << C << '\n';
    LPSolve<T> Tab(A, B, C, 1);
    
    COpt = Tab.OptimumCost();
    tracefile << "\nPlayer = " << pl << " Strat: ";
    for (int z = 1; z <= prob.Length(); z++) {
      tracefile << prob[z] << ' ';
    }
    // tracefile << " F = " << Tab.IsFeasible();
    // tracefile << " x = " << Tab.OptimumVector();
    // tracefile << " Obj = " << COpt;
    
    //if (Tab.IsFeasible() && (COpt >= C0-eps && COpt <=C0+eps))
    //  tracefile << " Duplicated strategy? ";
    // else 
    if (Tab.IsFeasible() && COpt > C0+eps) {
      tracefile << " Weakly Dominated by ";
      Gambit::Vector<T> xx(Tab.OptimumVector());
      for(i=1,st=1;st<=strats;st++) {
	dominator[st] = xx[i];
	i++;
      }
      for (int z = 1; z <= dominator.Length(); z++) {
	tracefile << dominator[z] << ' ';
      }
      ret = true;
    }
    // else  tracefile << "\n\n";
    // tracefile << "\n";
    
    if (!ret) 
      return false;
    
    return true;
  }
}


bool IsMixedDominated(const Gambit::StrategySupport &S, Gambit::GameStrategy *str,
		      bool strong, bool rational,
		      std::ostream &tracefile)
{
  if (rational) {
    return IsMixedDominated(S, str, strong, (Gambit::Rational)0, tracefile);
  }
  else {
    return IsMixedDominated(S, str, strong, (double)0, tracefile);
  }
}


// Note: junk is dummy arg so the functions can be templated. 
// There is probably a cleaner way to do this.  

template bool 
ComputeMixedDominated(const Gambit::StrategySupport &S, Gambit::StrategySupport &R,int pl, bool strong, 
		      Gambit::Rational junk, std::ostream &tracefile);
template bool 
ComputeMixedDominated(const Gambit::StrategySupport &S, Gambit::StrategySupport &R,int pl, bool strong, 
		      double junk, std::ostream &tracefile);

template bool
IsMixedDominated(const Gambit::StrategySupport &S, Gambit::GameStrategy str,
		 bool strong, Gambit::Rational junk, std::ostream &tracefile);
template bool
IsMixedDominated(const Gambit::StrategySupport &S, Gambit::GameStrategy str,
		 bool strong, double junk, std::ostream &tracefile);

template bool 
IsMixedDominated(const Gambit::MixedStrategyProfile<Gambit::Rational> &pr, int pl,
		 bool strong, std::ostream &tracefile);

template bool 
IsMixedDominated(const Gambit::MixedStrategyProfile<double> &pr, int pl,
		 bool strong, std::ostream &tracefile);


int main(int, char *argv[])
{
  // This is just a placeholder for now
  return 0;
}
