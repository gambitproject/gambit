//
// FILE: efdom.cc -- Compute dominated strategies on extensive form
//
// $Id$
//

#include "efg.h"
#include "efgciter.h"
#include "rational.h"
#include "gstatus.h"

// With strong set to false, the following routine returns true 
// if action a weakly dominates b.  With strong set to true, true is 
// returned when a yields a strictly greater payoff, regardless of
// others' behavior, which means that the infoset must be reached with
// positive probability in every play of the game.

bool Dominates(const EFSupport &S, int pl, int iset, int a, int b, bool strong,gStatus &status)
{

  EfgContIter A(S), B(S);
    
    A.Freeze(pl, iset);
    A.Set(pl, iset, a);
    B.Freeze(pl, iset);
    B.Set(pl, iset, b);

    if (strong)  {
      do  {
	status.Get();
	gRational ap = A.Payoff(pl);
	gRational bp = B.Payoff(pl);
	if (ap <= bp)  return false;
	A.NextContingency();
      } while (B.NextContingency());
      return true;
    }

    bool equal = true;

    do   {
      status.Get();
      gRational ap = A.Payoff(pl);
      gRational bp = B.Payoff(pl);
      if (ap < bp)   return false;
      else if (ap > bp)  equal = false;
      A.NextContingency();
    } while (B.NextContingency());
    
    return (!equal);
}

// With strong set to false, the following routine returns true 
// if action a weakly dominates b.  This is the same as the routine 
// above, but the procedure is more efficient.  With strong set to true,
// the following returns true when, conditional on any node in the
// infoset having been reached, a yields a strictly higher payoff.

bool ConditionallyDominates(const EFSupport &S, 
			    int pl, 
			    int iset, 
			    int a, int b, 
			    bool strong,
			    gStatus &status)
{
  const Action *aAct = S.Actions(pl,iset)[a];
  const Action *bAct = S.Actions(pl,iset)[b];
  const EFSupportWithActiveNodes SAct(S);

  bool equal = true;

  const Infoset *infoset = S.Game().GetInfosetByIndex(pl,iset);

  gList<const Node *> nodelist = SAct.ReachableNodesInInfoset(infoset);  

  if (nodelist.Length() == 0)
    nodelist = infoset->ListOfMembers();  // This may not be a good idea

  for (int n = 1; n <= nodelist.Length(); n++) {

    gList<const Infoset *> L;
    L += S.ReachableInfosets(nodelist[n],aAct);
    L += S.ReachableInfosets(nodelist[n],bAct);
    L.RemoveRedundancies();

    EfgConditionalContIter A(S,L), B(S,L);
    A.Set(pl, iset, a);
    B.Set(pl, iset, b);
    
    do  {
      status.Get();
      gRational ap = A.Payoff(nodelist[n],pl);  
      gRational bp = B.Payoff(nodelist[n],pl);
      // gRational ap = A.Payoff(pl);  
      // gRational bp = B.Payoff(pl);
      if (strong)
	{ if (ap <= bp)  return false; }
      else
	if (ap < bp)   return false;
	else if (ap > bp)  equal = false;
    } while (A.NextContingency() && B.NextContingency());
  }
    
  if (strong) return true;
  else return (!equal);
}


bool ComputeDominated(EFSupport &S, EFSupport &T,
					int pl, int iset, bool strong,
					gStatus &status)
{
  const gArray<Action *> &actions = S.Actions(pl, iset);

  gArray<int> set(actions.Length());
  int i;
  for (i = 1; i <= set.Length(); i++)
    set[i] = i;

  int min, dis;
  for (min = 0, dis = actions.Length() - 1; min <= dis; )  {
    status.Get();
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

      for (int inc = min + 1; inc <= dis; )  {
	status.Get();
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

	if (min + 1 <= actions.Length())   {
	  status.Get();
	  for (i = min + 1; i <= actions.Length(); i++)
	    T.RemoveAction(actions[set[i]]);
	  return true;
	}
	else
	  return false;
}


EFSupport *ComputeDominated(EFSupport &S, 
			    bool strong,
			    const gArray<int> &players,
			    gOutput & /* tracefile */, 
			    gStatus &status)
{
  EFSupport *T = new EFSupport(S);
  bool any = false;
  unsigned long num_isets=0,cur_iset=0;
  // calc total # of isets
  int i,iset;
  for (i = 1; i <= players.Length(); i++)
    num_isets += S.Game().Players()[players[i]]->NumInfosets();

  for (i = 1; i <= players.Length(); i++)   {
    status.Get();
    int pl = players[i];
    status << "Dominated strategies for player " << pl << ":\n";
    for (iset = 1;
	 iset <= S.Game().Players()[pl]->NumInfosets();
	 iset++)   {
      status.Get();
      status << "Dominated strategies in infoset " << iset << ":\n";
      status.SetProgress((double)cur_iset/(double)num_isets);cur_iset++;
      if (ComputeDominated(S, *T, pl, iset, strong, status))
	any = true;
    }
  }

  if (!any)  {
    delete T;
    return 0;
  }

  return T;
}
