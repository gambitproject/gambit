//
// FILE: efdom.cc -- Compute dominated strategies on extensive form
//
// $Id$
//

#include "efg.h"
#include "efgciter.h"
#include "rational.h"
#include "gstatus.h"

// The following computes whether action a dominates action b.
// If `conditional' is false, then the computation is with respect
// to whether a dominates b with respect to all possibilities for 
// the game allowed by the support.  The argument `strong' describes
// whether domination is supposed to be strong or weak.  In particular,
// if strong is true and conditional is false, true cannot be
// returned unless all plays (given the support) go through the
// infoset where a and b might be chosen.

bool Dominates(const EFSupport &S, 
			    int pl, 
			    int iset, 
			    int a, int b, 
			    bool strong,
			    bool conditional,
			    gStatus &status)
{
  const Infoset *infoset = S.Game().GetInfosetByIndex(pl,iset);

  if (!conditional) {
    if (!S.MayReach(infoset))
      return false;
    if (S.AlwaysReaches(infoset) || !strong)
      return Dominates(S,pl,iset,a,b,strong,true,status);
    else
      return false;
  }

    //DEBUG
  //    gout << "Got here with conditional being true...\n";

  const EFSupportWithActiveNodes SAct(S);

    //DEBUG
  //    gout << "Got past the construction of SAct...\n";

  const Action *aAct = S.Actions(pl,iset)[a];
  const Action *bAct = S.Actions(pl,iset)[b];

    //DEBUG
  //    gout << "Got to the point of defining nodelist...\n";

  gList<const Node *> nodelist = SAct.ReachableNodesInInfoset(infoset);  
  if (nodelist.Length() == 0)
    nodelist = infoset->ListOfMembers();  // This may not be a good idea;
                                          // I suggest checking for this 
                                          // prior to entry

    //DEBUG
  // gout << "Got past the definition of nodelist...\n";

  bool equal = true;
  for (int n = 1; n <= nodelist.Length(); n++) {

    //DEBUG
    //    gout << "Got to the loop over nodes...\n";

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
	 pp < min && !Dominates(S, pl, iset, set[pp+1], set[dis+1], strong,false,status);
	 pp++);
    if (pp < min)
      dis--;
    else  {
      int foo = set[dis+1];
      set[dis+1] = set[min+1];
      set[min+1] = foo;

      for (int inc = min + 1; inc <= dis; )  {
	status.Get();
	if (Dominates(S, pl, iset, set[min+1], set[dis+1], strong, false,status))  {
		status << actions[set[dis+1]]->GetNumber() << " dominated by "
				<< actions[set[min+1]]->GetNumber() << '\n';
		dis--;
	}
	else if (Dominates(S, pl, iset, set[dis+1], set[min+1], strong,false,status))  {
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
