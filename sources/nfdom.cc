//#
//# FILE: nfdom.cc -- Compute dominated strategies on normal form
//#
//# @(#)nfdom.cc	1.9 9/1/96
//#

#include "nfg.h"
#include "nfstrat.h"
#include "nfgciter.h"
#include "rational.h"
#include "gstatus.h"

bool Dominates(const NFSupport &S, int pl, int a, int b, bool strong)
{
  const BaseNfg &N = S.BelongsTo();

  switch (N.Type())   {
    case DOUBLE:  {
      NfgContIter<double> A(S), B(S);

      A.Freeze(pl);
      A.Set(pl, a);
      B.Freeze(pl);
      B.Set(pl, b);
  
      if (strong)  {
	do  {
	  double ap = (*A.Outcome())[pl];
	  double bp = (*B.Outcome())[pl];
	  if (ap <= bp)  return false;
	  A.NextContingency();
	} while (B.NextContingency());
	
	return true;
      }

      bool equal = true;
      
      do   {
	double ap = (*A.Outcome())[pl];
	double bp = (*B.Outcome())[pl];
	if (ap < bp)   return false;
	else if (ap > bp)  equal = false;
	A.NextContingency();
      } while (B.NextContingency());

      return (!equal);
    }

    case RATIONAL:  {
      NfgContIter<gRational> A(S), B(S);

      A.Freeze(pl);
      A.Set(pl, a);
      B.Freeze(pl);
      B.Set(pl, b);
  
      if (strong)  {
	do  {
	  gRational ap = (*A.Outcome())[pl];
	  gRational bp = (*B.Outcome())[pl];
	  if (ap <= bp)  return false;
	  A.NextContingency();
	} while (B.NextContingency());
	
	return true;
      }

      bool equal = true;
      
      do   {
	gRational ap = (*A.Outcome())[pl];
	gRational bp = (*B.Outcome())[pl];
	if (ap < bp)   return false;
	else if (ap > bp)  equal = false;
	A.NextContingency();
      } while (B.NextContingency());
      
      return (!equal);
    }
    default:
		assert(0);
      return false;
  }
}


bool ComputeDominated(const NFSupport &S, NFSupport &T,
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
	 pp < min && !Dominates(S, pl, set[pp+1], set[dis+1], strong);
	 pp++);
    if (pp < min)
      dis--;
    else  {
      int foo = set[dis+1];
      set[dis+1] = set[min+1];
      set[min+1] = foo;

      for (int inc = min + 1; inc <= dis; )  {
	if (Dominates(S, pl, set[min+1], set[dis+1], strong))  {
	  tracefile << S.Strategies(pl)[set[dis+1]]->number << " dominated by " << S.Strategies(pl)[set[min+1]]->number << '\n';
	  dis--;
	}
	else if (Dominates(S, pl, set[dis+1], set[min+1], strong))  {
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
      T.RemoveStrategy(S.Strategies(pl)[set[i]]);
    
    return true;
  }
  else
    return false;
}


NFSupport *ComputeDominated(NFSupport &S, bool strong,
				 const gArray<int> &players,
				 gOutput &tracefile, gStatus &status=gstatus)
{
  NFSupport *T = new NFSupport(S);
  bool any = false;

  for (int i = 1; i <= players.Length() && !status.Get(); i++)   {
    int pl = players[i];
    tracefile << "Dominated strategies for player " << pl << ":\n";
    any |= ComputeDominated(S, *T, pl, strong, tracefile, status);
// status.SetProgress((double)i/players.Length());
  }

  if (!any || status.Get())  {
    delete T;
    return 0;
  }
  return T;
}
