//
// FILE: efdom.cc -- Compute dominated strategies on extensive form
//
// $Id$
//

#include "efg.h"
#include "efgciter.h"
#include "rational.h"
#include "gstatus.h"

bool Dominates(const EFSupport &S, int pl, int iset, int a, int b, bool strong,gStatus &status)
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
	} while (B.NextContingency() && !status.Get());

	return true;
			}

			bool equal = true;

			do   {
	double ap = A.Payoff(pl);
	double bp = B.Payoff(pl);
	if (ap < bp)   return false;
	else if (ap > bp)  equal = false;
	A.NextContingency();
			} while (B.NextContingency() && !status.Get());

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
			} while (B.NextContingency() && !status.Get());

			return true;
		}

		bool equal = true;

		do   {
			gRational ap = A.Payoff(pl);
			gRational bp = B.Payoff(pl);
			if (ap < bp)   return false;
			else if (ap > bp)  equal = false;
			A.NextContingency();
		} while (B.NextContingency() && !status.Get());

		return (!equal);
	}
		default:
			assert(0);
			return false;
	}
}


bool ComputeDominated(EFSupport &S, EFSupport &T,
					int pl, int iset, bool strong,
					gStatus &status)
{
	const gArray<Action *> &actions = S.ActionList(pl, iset);

  gArray<int> set(actions.Length());
  int i;
  for (i = 1; i <= set.Length(); i++)
    set[i] = i;

  int min, dis;
  for (min = 0, dis = actions.Length() - 1; min <= dis && !status.Get(); )  {
    int pp;
    for (pp = 0;
	 pp < min && !Dominates(S, pl, iset, set[pp+1], set[dis+1], strong,status);
	 pp++);
    if (pp < min)
      dis--;
    else  {
      int foo = set[dis+1];
      set[dis+1] = set[min+1];
      set[min+1] = foo;

			for (int inc = min + 1; inc <= dis && !status.Get(); )  {
	if (Dominates(S, pl, iset, set[min+1], set[dis+1], strong,status))  {
		status << actions[set[dis+1]]->GetNumber() << " dominated by "
				<< actions[set[min+1]]->GetNumber() << '\n';
		dis--;
	}
	else if (Dominates(S, pl, iset, set[dis+1], set[min+1], strong,status))  {
		status << actions[set[min+1]]->GetNumber() << " dominated by "
			<< actions[set[dis+1]]->GetNumber() << '\n';
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

	if (min + 1 <= actions.Length() && !status.Get())   {
		for (i = min + 1; i <= actions.Length(); i++)
			T.RemoveAction(pl, iset, actions[set[i]]);
		return true;
	}
	else
		return false;
}


EFSupport *ComputeDominated(EFSupport &S, bool strong,
					const gArray<int> &players,
					gOutput & /* tracefile */, gStatus &status)
{
  EFSupport *T = new EFSupport(S);
  bool any = false;
  unsigned long num_isets=0,cur_iset=0;
  // calc total # of isets
  int i,iset;
  for (i = 1; i <= players.Length(); i++)
    num_isets += S.BelongsTo().PlayerList()[players[i]]->NumInfosets();

  for (i = 1; i <= players.Length() && !status.Get(); i++)   {
    int pl = players[i];
    status << "Dominated strategies for player " << pl << ":\n";
    for (iset = 1;
	 iset <= S.BelongsTo().PlayerList()[pl]->NumInfosets() && !status.Get();
	 iset++)   {
      status << "Dominated strategies in infoset " << iset << ":\n";
      status.SetProgress((double)cur_iset/(double)num_isets);cur_iset++;
      if (ComputeDominated(S, *T, pl, iset, strong, status))
	any = true;
    }
  }

  if (!any || status.Get())  {
    delete T;
    if (status.Get()) status.Reset();
    return 0;
  }

  return T;
}
