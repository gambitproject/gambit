//
// FILE: efdom.cc -- Compute dominated strategies on extensive form
//
// $Id$
//

#include "efg.h"
#include "efgciter.h"
#include "rational.h"

bool Dominates(const EFSupport &S, int pl, int iset, int a, int b, bool strong)
{
  const BaseEfg &E = S.BelongsTo();

  switch (E.Type())   {
    case DOUBLE:  {
      EfgContIter<double> A(S), B(S);

      A.Freeze(pl, iset);
      A.Set(pl, iset, a);
      B.Freeze(pl, iset);
      B.Set(pl, iset, b);
  
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
      EfgContIter<gRational> A(S), B(S);

      A.Freeze(pl, iset);
      A.Set(pl, iset, a);
      B.Freeze(pl, iset);
      B.Set(pl, iset, b);
  
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


bool ComputeDominated(EFSupport &S, EFSupport &T,
		      int pl, int iset, bool strong,
		      gOutput &tracefile)
{
  const gArray<Action *> &actions = S.ActionList(pl, iset);

  gArray<int> set(actions.Length());
  int i;
  for (i = 1; i <= set.Length(); i++)
    set[i] = i;

  int min, dis;
  for (min = 0, dis = actions.Length() - 1; min <= dis; )  {
    int pp;
    for (pp = 0;
	 pp < min && !Dominates(S, pl, iset, set[pp+1], set[dis+1], strong);
	 pp++);
    if (pp < min)
      dis--;
    else  {
      int foo = set[dis+1];
      set[dis+1] = set[min+1];
      set[min+1] = foo;
      
      for (int inc = min + 1; inc <= dis; )  {
	if (Dominates(S, pl, iset, set[min+1], set[dis+1], strong))  {
          tracefile << actions[set[dis+1]]->GetNumber() << " dominated by " << actions[set[min+1]]->GetNumber() << '\n'; 
	  dis--;
	}
	else if (Dominates(S, pl, iset, set[dis+1], set[min+1], strong))  {
	  tracefile << actions[set[min+1]]->GetNumber() << " dominated by " << actions[set[dis+1]]->GetNumber() << '\n';
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
  
  if (min + 1 <= actions.Length())   {
    for (i = min + 1; i <= actions.Length(); i++)
      T.RemoveAction(pl, iset, actions[set[i]]);
    return true;
  }
  else
    return false;
}


EFSupport *ComputeDominated(EFSupport &S, bool strong, 
			    const gArray<int> &players, gOutput &tracefile)
{
  EFSupport *T = new EFSupport(S);
  bool any = false;
  
  for (int i = 1; i <= players.Length(); i++)   {
    int pl = players[i];
    tracefile << "Dominated strategies for player " << pl << ":\n";
    for (int iset = 1; iset <= S.BelongsTo().PlayerList()[pl]->NumInfosets();
	 iset++)   {
      tracefile << "Dominated strategies in infoset " << iset << ":\n";
      if (ComputeDominated(S, *T, pl, iset, strong, tracefile))
	any = true;
    }
  }

  if (!any)  {
    delete T;
    return 0;
  }
  return T;
}
