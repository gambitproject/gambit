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

bool ComputeMixedDominated(const Nfg &nfg,
			   const NFSupport &S, NFSupport &R,
         const gArray<gNumber> &values,
			   int pl, bool strong, gOutput &tracefile,
			   gStatus &status)
{
  gRational eps;
  NfgContIter s(S);
  s.Freeze(pl);
  gEpsilon(eps);
  double d1,d2;
  d1 = (double)(pl-1)/(double)S.Game().NumPlayers();
  d2 = (double)pl/(double)S.Game().NumPlayers();

  gArray<bool> dom(S.NumStrats(pl));
  gVector<gRational> dominator(S.NumStrats(pl));

  int st,i,k,n;

  if (strong)   {
    gRational COpt;
    bool ret = false;
    int strats = S.NumStrats(pl);
    int contingencies = 1;
    for(k=1;k<=nfg.NumPlayers();k++)
      if(k!=pl) contingencies*=S.NumStrats(k);

    gMatrix<gRational> A(1,contingencies+1,1,strats);
    gVector<gRational> B(1,contingencies+1);
    gVector<gRational> C(1,strats);

    n = contingencies + 1;
    for (k = 1; k < strats; k++) {
      C[k] = (gRational) 0;
      A(n, k) = (gRational) 1;
    }
    A(n, k) = (gRational) 0;
    B[n] = (gRational) 1;
    C[k] = (gRational) 1;

    s.First();
    for (n = 1; n <= contingencies; n++) {
      s.Set(pl, 1);
      B[n] = -nfg.Payoff(s.GetOutcome(), pl).Evaluate(values);
      for (k = 2; k <= strats; k++) {
	s.Set(pl, k);
	A(n, k - 1) = -nfg.Payoff(s.GetOutcome(), pl).Evaluate(values);
      }
      A(n, strats) = (gRational) 1;
      s.NextContingency();
    }

    for (k = 1; k <= strats && !status.Get(); k++)	{
      double s1 = (double)k/(double)(strats);
      status.SetProgress((1.0-s1)*d1 + s1*d2);
      // tracefile << '\n' << (gRectArray<T> &)A << '\n';
      // tracefile << B << '\n';
      // tracefile << C << '\n';
      LPSolve<gRational> Tab(A, B, C, 1);

      COpt = Tab.OptimumCost();
      tracefile << "\nPlayer = " << pl << " Strat = "<< k;
      // tracefile << " F = " << Tab.IsFeasible();
      // tracefile << " x = " << Tab.OptimumVector();
      // tracefile << " Obj = " << COpt;

      dom[k] = false;

      if (Tab.IsFeasible() && COpt > eps) {
	tracefile << " Strongly Dominated by ";
	gVector<gRational> xx(Tab.OptimumVector());
	for(i=1,st=1;st<=strats;st++) {
	  if(st==k) dominator[st] = (gRational)0;
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
    gRational C0 = (gRational) 0, COpt, TmpC;
    bool ret = false;
    int strats = S.NumStrats(pl);
    int contingencies = 1;
    for(k=1;k<=nfg.NumPlayers();k++)
      if(k!=pl) contingencies*=S.NumStrats(k);

    gMatrix<gRational> A(1,contingencies+1,1,strats-1);
    gVector<gRational> B(1,contingencies+1);
    gVector<gRational> C(1,strats-1);

    n=contingencies+1;
    for(k=1;k<strats;k++) {
      C[k] = (gRational) 0;
      A(n,k)=(gRational) 1;
    }
    B[n]=(gRational)1;

    s.First();
    for(n=1;n<=contingencies;n++) {
      s.Set(pl, 1);
      B[n]=-nfg.Payoff(s.GetOutcome(), pl).Evaluate(values);
      C0 -= B[n];
      for(k=2;k<=strats;k++) {
	s.Set(pl,k);
	A(n,k-1)=-nfg.Payoff(s.GetOutcome(), pl).Evaluate(values);
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
      LPSolve<gRational> Tab(A, B, C, 1);

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
	gVector<gRational> xx(Tab.OptimumVector());
	for(i=1,st=1;st<=strats;st++) {
	  if(st==k) dominator[st] = (gRational)0;
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

NFSupport *ComputeMixedDominated(const Nfg &N, NFSupport &S,
         const gArray<gNumber> &values, bool strong,
				 const gArray<int> &players,
				 gOutput &tracefile, gStatus &status)
{
  NFSupport *newS = new NFSupport(S);
  bool any = false;
  
  for (int i = 1; i <= players.Length() && !status.Get(); i++)   {
    int pl = players[i];

    any |= ComputeMixedDominated(N, S, *newS, values, pl, strong, tracefile, status);
  }
  tracefile << "\n";
  if (!any || status.Get())  {
    delete newS;
    return 0;
  }
  
  return newS;
}







