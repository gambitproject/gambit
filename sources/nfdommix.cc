//
// FILE: nfdommix.cc -- Elimination of dominated strategies in nfg
//
// $Id$
//

#include "gstream.h"
#include "nfg.h"
#include "nfgiter.h"
#include "nfgciter.h"
#include "nfstrat.h"
#include "lpsolve.h"
#include "gstatus.h"

// Note: junk is dummy arg so the function can be templated. 
// There is probably a cleaner way to do this.  
template <class T>
bool ComputeMixedDominated(const NFSupport &S, NFSupport &R,
			   int pl, bool strong, T junk,
			   gOutput &tracefile, gStatus &status)
{
  Nfg nfg = S.Game();
  NfgContIter s(S);

  s.Freeze(pl);
  double d1,d2;
  d1 = (double)(pl-1)/(double)S.Game().NumPlayers();
  d2 = (double)pl/(double)S.Game().NumPlayers();
  
  gArray<bool> dom(S.NumStrats(pl));
  
  T eps;
  gEpsilon(eps);
  
  gVector<T> dominator(S.NumStrats(pl));
  
  int st,i,k,n;
  
  if (strong)   {
    T COpt;
    bool ret = false;
    int strats = S.NumStrats(pl);
    int contingencies = 1;
    for(k=1;k<=nfg.NumPlayers();k++)
      if(k!=pl) contingencies*=S.NumStrats(k);
    
    gMatrix<T> A(1,contingencies+1,1,strats);
    gVector<T> B(1,contingencies+1);
    gVector<T> C(1,strats);
    
    n = contingencies + 1;
    for (k = 1; k < strats; k++) {
      C[k] = (T) 0;
      A(n, k) = (T) 1;
    }
    A(n, k) = (T) 0;
    B[n] = (T) 1;
    C[k] = (T) 1;
    
    s.First();
    for (n = 1; n <= contingencies; n++) {
      s.Set(pl, 1);
      B[n] = -nfg.Payoff(s.GetOutcome(), pl);
      for (k = 2; k <= strats; k++) {
	s.Set(pl, k);
	A(n, k - 1) = -nfg.Payoff(s.GetOutcome(), pl);
      }
      A(n, strats) = (T) 1;
      s.NextContingency();
    }
    
    for (k = 1; k <= strats; k++) {
      status.Get();
      double s1 = (double)k/(double)(strats);
      status.SetProgress((1.0-s1)*d1 + s1*d2);
      // tracefile << '\n' << (gRectArray<T> &)A << '\n';
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
	gVector<T> xx(Tab.OptimumVector());
	for(i=1,st=1;st<=strats;st++) {
	  if(st==k) dominator[st] = (T)0;
	  else {
	    dominator[st] = xx[i];
	    i++;
	  }
	}
	tracefile << dominator;
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
	R.RemoveStrategy(S.Strategies(pl)[k]);
    
    return true;
  }
  else  {    // look for weak domination
    T C0 = (T) 0, COpt, TmpC;
    bool ret = false;
    int strats = S.NumStrats(pl);
    int contingencies = 1;
    for(k=1;k<=nfg.NumPlayers();k++)
      if(k!=pl) contingencies*=S.NumStrats(k);
    
    gMatrix<T> A(1,contingencies+1,1,strats-1);
    gVector<T> B(1,contingencies+1);
    gVector<T> C(1,strats-1);
    
    n=contingencies+1;
    for(k=1;k<strats;k++) {
      C[k] = (T) 0;
      A(n,k)=(T) 1;
    }
    B[n]=(T)1;
    
    s.First();
    for(n=1;n<=contingencies;n++) {
      s.Set(pl, 1);
      B[n]=-nfg.Payoff(s.GetOutcome(), pl);
      C0 -= B[n];
      for(k=2;k<=strats;k++) {
	s.Set(pl,k);
	A(n,k-1)=-nfg.Payoff(s.GetOutcome(), pl);
	C[k-1]-=A(n,k-1);
      }
      s.NextContingency();
    }
    
    for (k = 1; k <= strats; k++) {
      status.Get();
      double s1 = (double)k/(double)(strats);
      status.SetProgress((1.0-s1)*d1 + s1*d2);
      // tracefile << '\n' << (gRectArray<T> &)A << '\n';
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
	gVector<T> xx(Tab.OptimumVector());
	for(i=1,st=1;st<=strats;st++) {
	  if(st==k) dominator[st] = (T)0;
	  else {
	    dominator[st] = xx[i];
	    i++;
	  }
	}
	tracefile << dominator;
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
	R.RemoveStrategy(S.Strategies(pl)[k]);
    
    return true;
  }
  
}

// Note: junk is dummy arg so the function can be templated. 
// There is probably a cleaner way to do this.  
template <class T>
bool IsMixedDominated(const NFSupport &S,Strategy *str,
			   bool strong, T junk,
			   gOutput &tracefile)
{
  int pl = str->Player()->GetNumber();
  Nfg nfg = str->Player()->Game();
  int whichstrat = str->Number();
  int strats = S.NumStrats(pl);
  bool dom;

  NfgContIter s(S);
  s.Freeze(pl);
  
  
  T eps;
  gEpsilon(eps);
  
  gVector<T> dominator(S.NumStrats(pl));
  
  int st,i,k,n;
  bool ret = false;
  int contingencies = 1;
  for(k=1;k<=nfg.NumPlayers();k++)
    if(k!=pl) contingencies*=S.NumStrats(k);
  
  
  if (strong)   {
    T COpt;
    gMatrix<T> A(1,contingencies+1,1,strats);
    gVector<T> B(1,contingencies+1);
    gVector<T> C(1,strats);
    
    n = contingencies + 1;
    for (k = 1; k < strats; k++) {
      C[k] = (T) 0;
      A(n, k) = (T) 1;
    }
    A(n, k) = (T) 0;
    B[n] = (T) 1;
    C[k] = (T) 1;
    
    s.First();
    for (n = 1; n <= contingencies; n++) {
      s.Set(pl, whichstrat);
      B[n] = -nfg.Payoff(s.GetOutcome(), pl);
      for (k = 1; k <= strats; k++) 
	if(k< whichstrat) {
	  s.Set(pl, k);
	  A(n, k) = -nfg.Payoff(s.GetOutcome(), pl);
	}
	else if (k > whichstrat) {
	  s.Set(pl, k);
	  A(n, k - 1) = -nfg.Payoff(s.GetOutcome(), pl);
	}
      A(n, strats) = (T) 1;
      s.NextContingency();
    }
    
    // tracefile << '\n' << (gRectArray<T> &)A << '\n';
    // tracefile << B << '\n';
    // tracefile << C << '\n';
    LPSolve<T> Tab(A, B, C, 1);
    COpt = Tab.OptimumCost();
    tracefile << "\nPlayer = " << pl << " Strat = "<< whichstrat;
    // tracefile << " F = " << Tab.IsFeasible();
    // tracefile << " x = " << Tab.OptimumVector();
    // tracefile << " Obj = " << COpt;
    
    dom = false;
    
    if (Tab.IsFeasible() && COpt > eps) {
      tracefile << " Strongly Dominated by ";
      gVector<T> xx(Tab.OptimumVector());
      for(i=1,st=1;st<=strats;st++) {
	if(st==whichstrat) dominator[st] = (T)0;
	else {
	  dominator[st] = xx[i];
	  i++;
	}
      }
      tracefile << dominator;
      ret = true;
      dom = true;
    }
    // tracefile << "\n";
    
    if (!ret) 
      return false;
    
    return true;
  }
  else  {    // look for weak domination
    T C0 = (T) 0, COpt;

    gMatrix<T> A(1,contingencies+1,1,strats-1);
    gVector<T> B(1,contingencies+1);
    gVector<T> C(1,strats-1);
    
    n=contingencies+1;
    for(k=1;k<strats;k++) {
      C[k] = (T) 0;
      A(n,k)=(T) 1;
    }
    B[n]=(T)1;
    
    s.First();
    for(n=1;n<=contingencies;n++) {
      s.Set(pl, whichstrat);
      B[n]=-nfg.Payoff(s.GetOutcome(), pl);
      C0 -= B[n];
      for(k=1;k<=strats;k++) 
	if(k<whichstrat) {
	  s.Set(pl,k);
	  A(n,k)=-nfg.Payoff(s.GetOutcome(), pl);
	  C[k]-=A(n,k);
	}
	else if (k > whichstrat) {
	  s.Set(pl,k);
	  A(n,k-1)=-nfg.Payoff(s.GetOutcome(), pl);
	  C[k-1]-=A(n,k-1);
	}
      s.NextContingency();
    }
    
    // tracefile << '\n' << (gRectArray<T> &)A << '\n';
    // tracefile << B << '\n';
    // tracefile << C << '\n';
    LPSolve<T> Tab(A, B, C, 1);
    
    COpt = Tab.OptimumCost();
    tracefile << "\nPlayer = " << pl << " Strat = "<< whichstrat;
    // tracefile << " F = " << Tab.IsFeasible();
    // tracefile << " x = " << Tab.OptimumVector();
    // tracefile << " Obj = " << COpt;
    
    dom = false;
    
    if (Tab.IsFeasible() && (COpt >= C0-eps && COpt <=C0+eps))
      tracefile << " Duplicated strategy? ";
    else if (Tab.IsFeasible() && COpt > C0+eps) {
      tracefile << " Weakly Dominated by ";
      gVector<T> xx(Tab.OptimumVector());
      for(i=1,st=1;st<=strats;st++) {
	if(st==k) dominator[st] = (T)0;
	else {
	  dominator[st] = xx[i];
	  i++;
	}
      }
      tracefile << dominator;
      ret = true;
      dom = true;
    }
    // else  tracefile << "\n\n";
    // tracefile << "\n";
    
    if (!ret) 
      return false;
    
    return true;
  }
  
}

NFSupport *ComputeMixedDominated(NFSupport &S,
				 bool strong, gPrecision precision,
				 const gArray<int> &players,
				 gOutput &tracefile, gStatus &status)
{
  NFSupport *newS = new NFSupport(S);
  bool any = false;
  
  if(precision == precRATIONAL) {
    
    for (int i = 1; i <= players.Length(); i++)   {
      status.Get();
      int pl = players[i];
      
      any |= ComputeMixedDominated<gRational>(S, *newS, pl, strong,
				  (gRational)0, tracefile, status);
    }
  }
  else if (precision == precDOUBLE) {
    for (int i = 1; i <= players.Length(); i++)   {
      status.Get();
      int pl = players[i];
      
      any |= ComputeMixedDominated<double>(S, *newS, pl, strong,
				  (double)0, tracefile, status);
    }
  }


  tracefile << "\n";
  if (!any)  {
    delete newS;
    return 0;
  }
  
  return newS;
}

bool IsMixedDominated(const NFSupport &S,Strategy *str,
			   bool strong, gPrecision precision,
			   gOutput &tracefile)
{
  bool ret = false;
  if(precision == precRATIONAL) 
    ret =  IsMixedDominated<gRational>(S, str, strong, (gRational)0, tracefile);
  else if (precision == precDOUBLE) 
    ret =  IsMixedDominated<double>(S, str, strong, (double)0, tracefile);
  
  return ret;
}

template bool 
ComputeMixedDominated(const NFSupport &S, NFSupport &R,int pl, bool strong, 
		      gRational junk, gOutput &tracefile, gStatus &status);
template bool 
ComputeMixedDominated(const NFSupport &S, NFSupport &R,int pl, bool strong, 
		      double junk, gOutput &tracefile, gStatus &status);

template bool
IsMixedDominated(const NFSupport &S,Strategy *str,
		 bool strong, gRational junk, gOutput &tracefile);
template bool
IsMixedDominated(const NFSupport &S,Strategy *str,
		 bool strong, double junk, gOutput &tracefile);




