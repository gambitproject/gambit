//
// FILE: nfdom.cc -- Compute dominated strategies on normal form
//
// $Id$
//

#include "nfg.h"
#include "nfstrat.h"
#include "nfgciter.h"
#include "rational.h"
#include "gstatus.h"

template <class T>
bool Dominates(const Nfg<T> &N,
	       const NFSupport &S, int pl, int a, int b, bool strong)
{
  NfgContIter A(S), B(S);

  A.Freeze(pl);
  A.Set(pl, a);
  B.Freeze(pl);
  B.Set(pl, b);
  
  if (strong)  {
    do  {
      T ap = N.Payoff(A.GetOutcome(), pl);
      T bp = N.Payoff(B.GetOutcome(), pl);
      if (ap <= bp)  return false;
      A.NextContingency();
    } while (B.NextContingency());
	
    return true;
  }

  bool equal = true;
  
  do   {
    T ap = N.Payoff(A.GetOutcome(), pl);
    T bp = N.Payoff(B.GetOutcome(), pl);
    if (ap < bp)   return false;
    else if (ap > bp)  equal = false;
    A.NextContingency();
  } while (B.NextContingency());

  return (!equal);
}

template <class T>
bool ComputeDominated(const Nfg<T> &N, const NFSupport &S, NFSupport &newS,
		      int pl, bool strong,
		      gOutput &tracefile, gStatus &status)
{
  gArray<int> set(S.NumStrats(pl));
  int i;
  for (i = 1; i <= set.Length(); i++)
    set[i] = i;

  int min, dis;
  double d1,d2;
  d1 = (double)(pl-1)/(double)S.BelongsTo().NumPlayers();
  d2 = (double)pl/(double)S.BelongsTo().NumPlayers();
  for (min = 0, dis = S.NumStrats(pl) - 1; min <= dis && !status.Get(); )  {
    int pp;
    double s1 = (double)min/(double)(dis+1);
    status.SetProgress((1.0-s1)*d1 + s1*d2);
    for (pp = 0;
	 pp < min && !Dominates(N, S, pl, set[pp+1], set[dis+1], strong);
	 pp++);
    if (pp < min)
      dis--;
    else  {
      int foo = set[dis+1];
      set[dis+1] = set[min+1];
      set[min+1] = foo;

      for (int inc = min + 1; inc <= dis; )  {
	if (Dominates(N, S, pl, set[min+1], set[dis+1], strong))  {
	  tracefile << S.Strategies(pl)[set[dis+1]]->number << " dominated by " << S.Strategies(pl)[set[min+1]]->number << '\n';
	  dis--;
	}
	else if (Dominates(N, S, pl, set[dis+1], set[min+1], strong))  {
	  tracefile << S.Strategies(pl)[set[min+1]]->number << " dominated by " << S.Strategies(pl)[set[dis+1]]->number << '\n';
	  foo = set[dis+1];
	  set[dis+1] = set[min+1];
	  set[min+1] = foo;
	  dis--;
	}
	else  {
	  foo = set[dis+1];
	  set[dis+1] = set[inc+1];
	  set[inc+1] = foo;
	  inc++;
	}
      }
      min++;
    }
  }
  
  
  if (min + 1 <= S.NumStrats(pl))   {
    for (i = min + 1; i <= S.NumStrats(pl); i++)
      newS.RemoveStrategy(S.Strategies(pl)[set[i]]);
    
    return true;
  }
  else
    return false;
}


template <class T> 
NFSupport *ComputeDominated(const Nfg<T> &N, NFSupport &S, bool strong,
			    const gArray<int> &players,
			    gOutput &tracefile, gStatus &status)
{
  NFSupport *newS = new NFSupport(S);
  bool any = false;

  for (int i = 1; i <= players.Length() && !status.Get(); i++)   {
    int pl = players[i];
    tracefile << "Dominated strategies for player " << pl << ":\n";
    any |= ComputeDominated(N, S, *newS, pl, strong, tracefile, status);
// status.SetProgress((double)i/players.Length());
  }

  if (!any || status.Get())  {
    delete newS;
    return 0;
  }
  return newS;
}


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__

TEMPLATE
bool Dominates(const Nfg<double> &N,
	       const NFSupport &S, int pl, int a, int b, bool strong);
TEMPLATE
bool Dominates(const Nfg<gRational> &N,
	       const NFSupport &S, int pl, int a, int b, bool strong);

TEMPLATE
bool ComputeDominated(const Nfg<double> &N, const NFSupport &S, NFSupport &newS,
		      int pl, bool strong,
		      gOutput &tracefile, gStatus &status);
TEMPLATE
bool ComputeDominated(const Nfg<gRational> &N, const NFSupport &S, NFSupport &newS,
		      int pl, bool strong,
		      gOutput &tracefile, gStatus &status);


TEMPLATE
NFSupport *ComputeDominated(const Nfg<double> &N, NFSupport &S, bool strong,
			    const gArray<int> &players,
			    gOutput &tracefile, gStatus &status);
TEMPLATE
NFSupport *ComputeDominated(const Nfg<gRational> &N, NFSupport &S, bool strong,
			    const gArray<int> &players,
			    gOutput &tracefile, gStatus &status);


