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

gRectArray<gNumber> *paytable;

bool Dominates(const Nfg &,
	       const NFSupport &S, int pl, int a, int b, bool strong)
{
  NfgContIter A(S), B(S);

  A.Freeze(pl);
  A.Set(pl, a);
  B.Freeze(pl);
  B.Set(pl, b);

  if (strong)  {
    do  {
      gNumber ap = (*paytable)(A.GetOutcome()->GetNumber(), pl);
      gNumber bp = (*paytable)(B.GetOutcome()->GetNumber(), pl);
      if (ap <= bp)  return false;
      A.NextContingency();
    } while (B.NextContingency());
	
    return true;
  }

  bool equal = true;
  
  do   {
    gNumber ap = (*paytable)(A.GetOutcome()->GetNumber(), pl);
    gNumber bp = (*paytable)(B.GetOutcome()->GetNumber(), pl);
    if (ap < bp)   return false;
    else if (ap > bp)  equal = false;
    A.NextContingency();
  } while (B.NextContingency());

  return (!equal);
}

bool ComputeDominated(const Nfg &N, const NFSupport &S, NFSupport &newS,
		      int pl, bool strong,
		      gOutput &tracefile, gStatus &status)
{
  gArray<int> set(S.NumStrats(pl));
  int i;
  for (i = 1; i <= set.Length(); i++)
    set[i] = i;

  int min, dis;
  double d1,d2;
  d1 = (double)(pl-1)/(double)S.Game().NumPlayers();
  d2 = (double)pl/(double)S.Game().NumPlayers();
  for (min = 0, dis = S.NumStrats(pl) - 1; min <= dis; )  {
    status.Get();
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
	  tracefile << S.Strategies(pl)[set[dis+1]]->Number() << " dominated by " << S.Strategies(pl)[set[min+1]]->Number() << '\n';
	  dis--;
	}
	else if (Dominates(N, S, pl, set[dis+1], set[min+1], strong))  {
	  tracefile << S.Strategies(pl)[set[min+1]]->Number() << " dominated by " << S.Strategies(pl)[set[dis+1]]->Number() << '\n';
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


NFSupport *ComputeDominated(const Nfg &N, NFSupport &S, bool strong,
			    const gArray<int> &players,
			    gOutput &tracefile, gStatus &status)
{
  NFSupport *newS = new NFSupport(S);
  bool any = false;

  paytable = new gRectArray<gNumber>(N.NumOutcomes(), N.NumPlayers());
  for (int outc = 1; outc <= N.NumOutcomes(); outc++)  {
    for (int pl = 1; pl <= N.NumPlayers(); pl++)
      (*paytable)(outc, pl) = N.Payoff(N.Outcomes()[outc], pl);
  }

  for (int i = 1; i <= players.Length(); i++)   {
    status.Get();
    int pl = players[i];
    tracefile << "Dominated strategies for player " << pl << ":\n";
    any |= ComputeDominated(N, S, *newS, pl, strong, tracefile, status);
// status.SetProgress((double)i/players.Length());
  }

  delete paytable;

  if (!any)  {
    delete newS;
    return 0;
  }
  return newS;
}



