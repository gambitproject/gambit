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


template <class T>
NFStrategySet *ComputeMixedDominated(const Nfg<T> &nfg, const NFSupport &S,
				     int pl, bool strong, gOutput &tracefile)
{
  T eps;
  NfgContIter<T> s(&S);
  s.Freeze(pl);
  gEpsilon(eps);

  NFStrategySet *newS = new NFStrategySet(*S.GetNFStrategySet(pl));
  gArray<bool> dom(S.NumStrats(pl));

  if (strong)   {
    T COpt;
    int k,n;
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
      B[n] = -s.Payoff(pl);
      for (k = 2; k <= strats; k++) {
	s.Set(pl, k);
	A(n, k - 1) = -s.Payoff(pl);
      }
      A(n, strats) = (T) 1;
      s.NextContingency();
    }

    for (k = 1; k <= strats; k++)	{
      tracefile << '\n' << (gRectArray<T> &)A << '\n';
      tracefile << B << '\n';
      tracefile << C << '\n';
      LPSolve<T> Tab(A, B, C, 1);

      COpt = Tab.OptimumCost();
      tracefile << "\nPlayer = " << pl << " Strat = "<< k;
      tracefile << " F = " << Tab.IsFeasible();
      tracefile << " x = " << Tab.OptimumVector();
      tracefile << " Obj = " << COpt;

      dom[k] = false;

      if (Tab.IsFeasible() && COpt > eps) {
	tracefile << " Strongly Dominated";
	ret = true;
	dom[k] = true;
      }
      if (k<strats)
	A.SwitchColumn(k,B);
    }
    tracefile << "\n";

    if (!ret)  {
      delete newS;
      return 0;
    }

    for (k = 1; k <= strats; k++)
      if (dom[k])
	newS->RemoveStrategy(S.GetStrategy(pl, k));

    return newS;
  }

  else  {    // look for weak domination
    T C0 = (T) 0, COpt, TmpC;
    int k,n;
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
      B[n]=-s.Payoff(pl);
      C0 -= B[n];
      for(k=2;k<=strats;k++) {
	s.Set(pl,k);
	A(n,k-1)=-s.Payoff(pl);
	C[k-1]-=A(n,k-1);
      }
      s.NextContingency();
    }

    for (k = 1; k <= strats; k++)	{
      tracefile << '\n' << (gRectArray<T> &)A << '\n';
      tracefile << B << '\n';
      tracefile << C << '\n';
      LPSolve<T> Tab(A, B, C, 1);

      COpt = Tab.OptimumCost();
      tracefile << "\nPlayer = " << pl << " Strat = "<< k;
      tracefile << " F = " << Tab.IsFeasible();
      tracefile << " x = " << Tab.OptimumVector();
      tracefile << " Obj = " << COpt;

      dom[k] = false;

      if (Tab.IsFeasible() && (COpt >= C0-eps && COpt <=C0+eps))
	tracefile << " Duplicated strategy?\n\n";
      else if (Tab.IsFeasible() && COpt > C0+eps) {
	tracefile << " Weakly Dominated\n\n";
	ret = true;
	dom[k] = true;
      }
      else  tracefile << "\n\n";
      if(k<strats) {
	A.SwitchColumn(k,B);
	TmpC=C0; C0=C[k]; C[k]=TmpC;
      }

    }
    tracefile << "\n";

    if (!ret)  {
      delete newS;
      return 0;
		}

    for (k = 1; k <= strats; k++)
      if (dom[k])
	newS->RemoveStrategy(S.GetStrategy(pl, k));

    return newS;
  }
}

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__
#include "rational.h"
#include "gstatus.h"
TEMPLATE NFStrategySet *ComputeMixedDominated(const Nfg<double> &,
								const NFSupport &,
								int, bool, gOutput &);
TEMPLATE NFStrategySet *ComputeMixedDominated(const Nfg<gRational> &,
								const NFSupport &,
								int, bool, gOutput &);
#pragma option -Jgx

NFSupport *ComputeMixedDominated(NFSupport &S, bool strong,
				 const gArray<int> &players,
				 gOutput &tracefile, gStatus &status=gstatus)
{
	NFSupport *T = new NFSupport(S);
	bool any = false;

  for (int i = 1; i <= players.Length() && !status.Get(); i++)   {
    int pl = players[i];
    NFStrategySet *SS;
    if (S.BelongsTo().Type() == DOUBLE)
      SS = ComputeMixedDominated((Nfg<double> &) S.BelongsTo(),
				 S, pl, strong, tracefile);
    else
      SS = ComputeMixedDominated((Nfg<gRational> &) S.BelongsTo(),
				 S, pl, strong, tracefile);
	status.SetProgress((double)i/players.Length());
		if (SS)  {
			delete T->GetNFStrategySet(pl);
			T->SetNFStrategySet(pl, SS);
			any = true;
		}
	}

	if (!any || status.Get())  {
		delete T;
		return 0;
	}

	return T;
}






