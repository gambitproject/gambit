//
// FILE: nfdommix.cc -- Elimination of dominated strategies in nfg
//
// $Id$
//

#include "gambitio.h"
#include "nfg.h"
#include "nfgiter.h"
#include "nfgciter.h"
#include "nfstrat.h"
#include "lpsolve.h"
#include "gstatus.h"

template <class T>
bool ComputeMixedDominated(const Nfg<T> &nfg,
			   const NFSupport &S, NFSupport &R,
			   int pl, bool strong, gOutput &tracefile,
			   gStatus &status)
{
  T eps;
  NfgContIter s(S);
  s.Freeze(pl);
  gEpsilon(eps);
  double d1,d2;
  d1 = (double)(pl-1)/(double)S.BelongsTo().NumPlayers();
  d2 = (double)pl/(double)S.BelongsTo().NumPlayers();

  gArray<bool> dom(S.NumStrats(pl));
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

    for (k = 1; k <= strats && !status.Get(); k++)	{
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

    for (k = 1; k <= strats && !status.Get(); k++)	{
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
	tracefile << " Duplicated strategy?\n\n";
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

template <class T> 
NFSupport *ComputeMixedDominated(const Nfg<T> &N, NFSupport &S, bool strong,
				 const gArray<int> &players,
				 gOutput &tracefile, gStatus &status)
{
  NFSupport *newS = new NFSupport(S);
  bool any = false;
  
  for (int i = 1; i <= players.Length() && !status.Get(); i++)   {
    int pl = players[i];

    any |= ComputeMixedDominated(N, S, *newS, pl, strong, tracefile, status);
  }
  tracefile << "\n";
  if (!any || status.Get())  {
    delete newS;
    return 0;
  }
  
  return newS;
}


#include "rational.h"

template bool ComputeMixedDominated(const Nfg<double> &,
				    const NFSupport &, NFSupport &,
				    int, bool, gOutput &,
				    gStatus &);
template bool ComputeMixedDominated(const Nfg<gRational> &,
				    const NFSupport &, NFSupport &,
				    int, bool, gOutput &,
				    gStatus &);

template
NFSupport *ComputeMixedDominated(const Nfg<double> &N, NFSupport &S, bool strong,
				 const gArray<int> &players,
				 gOutput &tracefile, gStatus &status);
template
NFSupport *ComputeMixedDominated(const Nfg<gRational> &N, NFSupport &S, bool strong,
				 const gArray<int> &players,
				 gOutput &tracefile, gStatus &status);






