//#
//# FILE: nfdom.cc -- Compute dominated strategies on normal form
//#
//# $Id$
//#

#include "nfg.h"
#include "nfstrat.h"
#include "contiter.h"
#include "rational.h"

bool Dominates(const NFSupport &S, int pl, int a, int b, bool strong)
{
  const BaseNfg &N = S.BelongsTo();

  switch (N.Type())   {
    case DOUBLE:  {
      ContIter<double> A(&S), B(&S);

      A.Freeze(pl);
      A.Set(pl, a);
      B.Freeze(pl);
      B.Set(pl, b);
  
      if (strong)  {
	do  {
	  double ap = A.Payoff(pl);
	  double bp = B.Payoff(pl);
	  if (ap <= bp)  return false;
	  A.NextContingency();
	} while (B.NextContingency());
	
	return true;
      }

      bool equal = true;
      
      do   {
	double ap = A.Payoff(pl);
	double bp = B.Payoff(pl);
	if (ap < bp)   return false;
	else if (ap > bp)  equal = false;
	A.NextContingency();
      } while (B.NextContingency());
      
      return (!equal);
    }

    case RATIONAL:  {
      ContIter<gRational> A(&S), B(&S);

      A.Freeze(pl);
      A.Set(pl, a);
      B.Freeze(pl);
      B.Set(pl, b);
  
      if (strong)  {
	do  {
	  gRational ap = A.Payoff(pl);
	  gRational bp = B.Payoff(pl);
	  if (ap <= bp)  return false;
	  A.NextContingency();
	} while (B.NextContingency());
	
	return true;
      }

      bool equal = true;
      
      do   {
	gRational ap = A.Payoff(pl);
	gRational bp = B.Payoff(pl);
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


NFStrategySet *ComputeDominated(NFSupport &S, int pl, bool strong,
				gOutput &tracefile)
{
  NFStrategySet *SS = S.GetNFStrategySet(pl);

  gArray<int> set(SS->NumStrats());
  for (int i = 1; i <= set.Length(); i++)
    set[i] = i;

  for (int min = 0, dis = SS->NumStrats() - 1; min <= dis; )  {
    for (int pp = 0;
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
          tracefile << SS->GetStrategy(set[dis+1])->number << " dominated by " << SS->GetStrategy(set[min+1])->number << '\n'; 
	  dis--;
	}
	else if (Dominates(S, pl, set[dis+1], set[min+1], strong))  {
	  tracefile << SS->GetStrategy(set[min+1])->number << " dominated by " << SS->GetStrategy(set[dis+1])->number << '\n';
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
  
  
  if (min + 1 <= SS->NumStrats())   {
    NFStrategySet *T = new NFStrategySet(*SS);
    for (i = min + 1; i <= SS->NumStrats(); i++)
      T->RemoveStrategy(SS->GetStrategy(set[i]));
    
    return T;
  }
  else
    return 0;
}


NFSupport *ComputeDominated(NFSupport &S, bool strong, 
			    const gArray<int> &players, gOutput &tracefile)
{
  NFSupport *T = new NFSupport(S);
  bool any = false;
  
  for (int i = 1; i <= players.Length(); i++)   {
    int pl = players[i];
    tracefile << "Dominated strategies for player " << pl << ":\n";
    NFStrategySet *SS = ComputeDominated(S, pl, strong, tracefile);
    if (SS)   {
      delete T->GetNFStrategySet(pl);
      T->SetNFStrategySet(pl, SS);
      any = true;
    }
  }
  
  if (!any)  {
    delete T;
    return 0;
  }
  return T;
}
