//
// FILE: efdom.cc -- Compute dominated strategies on extensive form
//
// $Id$
//

#include "efdom.h"
#include "efgciter.h"

efgDominanceException::efgDominanceException(const gText &p_description)
  : m_description(p_description)
{ }

efgDominanceException::~efgDominanceException()
{ }

gText efgDominanceException::Description(void) const
{
  return m_description;
}


bool Dominates(const EFSupport &S, 
	       const int pl, 
	       const int iset, 
	       const int a, const int b, 
	       const bool strong,
	       const bool conditional,
	       const gStatus &status)
{
  const EFSupportWithActiveInfo SAct(S);
  const Infoset *infoset = S.Game().GetInfosetByIndex(pl,iset);
  Action *aAct = S.Actions(pl,iset)[a];
  Action *bAct = S.Actions(pl,iset)[b];

  bool equal = true;

  if (!conditional) {

    EfgContIter A(S), B(S);
    A.Freeze(pl, iset); 
    B.Freeze(pl, iset); 
    A.Set(pl, iset, a); 
    B.Set(pl, iset, b); 
    
    do  {
      status.Get();
      gRational ap = A.Payoff(pl);  
      gRational bp = B.Payoff(pl);

      if (strong)
	{ if (ap <= bp)  return false; }
      else
	if (ap < bp)   return false; 
	else if (ap > bp)  equal = false;
    } while (A.NextContingency() && B.NextContingency());
  }

  else {
    gList<const Node *> nodelist = SAct.ReachableNodesInInfoset(infoset);  
    if (nodelist.Length() == 0)
      nodelist = infoset->ListOfMembers();  // This may not be a good idea;
                                            // I suggest checking for this 
                                            // prior to entry
    
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
	
	if (strong)
	  { if (ap <= bp)  return false; }
	else
	  if (ap < bp)   return false; 
	  else if (ap > bp)  equal = false;
      } while (A.NextContingency() && B.NextContingency());
    }
  }
  
  if (strong) return true;
  else  return (!equal); 
}

// Another window to the computation above.

bool Dominates(const EFSupport &S, 
	       const Action *a, const Action *b,
	       const bool strong,
	       const bool conditional,
	       const gStatus &status)
{
  const Infoset *infoset = a->BelongsTo();
  if (infoset != b->BelongsTo())
    throw efgDominanceException
      ("Dominates(..) needs actions in same infoset.\n");
  const EFPlayer *player = infoset->GetPlayer();

  return Dominates(S,player->GetNumber(),infoset->GetNumber(),
		   a->GetNumber(),b->GetNumber(),
		   strong, conditional, status);
}

bool SomeListElementDominates(const EFSupport &S, 
			      const gList<Action *> &l,
			      const Action *a, 
			      const bool strong,
			      const bool conditional,
			      const gStatus &status)
{
  for (int i = 1; i <= l.Length(); i++)
    if (l[i] != a)
      if (Dominates(S,l[i],a,strong,conditional,status))
	return true;
  return false;
}

bool SomeArrayElementDominates(const EFSupport &S, 
			       const gArray<Action *> &array,
			       const Action *a, 
			       const bool strong,
			       const bool conditional,
			       const gStatus &status)
{
  for (int i = 1; i <= array.Length(); i++)
    if (array[i] != a)
      if (Dominates(S,array[i],a,strong,conditional,status)) {
	return true;
      }
  return false;
}

bool IsDominated(const EFSupport &S, 
		 const Action *a, 
		 const bool strong,
		 const bool conditional,
		 const gStatus &status)
{
  gArray<Action *> array(S.Actions(a->BelongsTo()));
  return SomeArrayElementDominates(S,array,a,strong,conditional,status);
}

bool InfosetHasDominatedElement(const EFSupport &S, 
				const Infoset *i,
				const bool strong,
				const bool conditional,
				const gStatus &/*status*/)
{
  gList<Action *> actions = S.ListOfActions(i);
  for (int i = 1; i <= actions.Length(); i++)
    if (SomeListElementDominates(S,actions,actions[i],
				 strong,conditional,gstatus))
      return true;

  return false;
}

bool ElimDominatedInInfoset(const EFSupport &S, EFSupport &T,
			     const int pl, 
			     const int iset, 
			     const bool strong,
			     const bool conditional,
		                   gStatus &status)
{
  const gArray<Action *> &actions = S.Actions(pl, iset);

  gArray<bool> is_dominated(actions.Length());
  for (int k = 1; k <= actions.Length(); k++)
    is_dominated[k] = false;

  for (int i = 1; i <= actions.Length(); i++)
    for (int j = 1; j <= actions.Length(); j++)
      if (i != j && !is_dominated[i] && !is_dominated[j]) 
	if (Dominates(S, pl, iset, i, j, strong, conditional, status)) {
	  is_dominated[j] = true;
	  status.Get();
	  status << actions[j]->GetNumber() << " dominated by "
		 << actions[i]->GetNumber() << '\n';
	}
      
  bool action_was_eliminated = false;
  int k = 1;
  while (k <= actions.Length() && !action_was_eliminated) {
    if (is_dominated[k]) action_was_eliminated = true;
    else k++;
  }
  while (k <= actions.Length()) {
    if (is_dominated[k]) 
      T.RemoveAction(actions[k]);
    k++;
  }

  return action_was_eliminated;
}

bool ElimDominatedForPlayer(const EFSupport &S, EFSupport &T,
			    const int pl, 
			    const bool strong,
			    const bool conditional,
		                  gStatus &status)
{
  bool action_was_eliminated = false;

  status << "Dominated strategies for player " << pl << ":\n";
  for (int iset = 1; iset <= S.Game().Players()[pl]->NumInfosets(); iset++) {
    status.Get();
    status << "Dominated strategies in infoset " << iset << ":\n";
    if (ElimDominatedInInfoset(S, T, pl, iset, strong, conditional, status)) 
      action_was_eliminated = true;
  }

  return action_was_eliminated;
}

EFSupport *SupportWithoutDominatedOfPlayerList(const EFSupport &S, 
					       const bool strong,
					       const bool conditional,
					       const gArray<int> &players,
					             gOutput &, // tracefile 
					             gStatus &status)
{
  EFSupport *T = new EFSupport(S);
  bool any = false;

  for (int i = 1; i <= players.Length(); i++)   {
    status.Get();
    int pl = players[i];
    status << "Dominated strategies for player " << pl << ":\n";
    if (ElimDominatedForPlayer(S, *T, pl, strong, conditional, status)) 
      any = true;
  }

  if (!any)  {
    delete T;
    return 0;
  }

  return T;
}

/*
void AndyTest(const EFSupport &, gStatus &status);
*/
EFSupport *DominanceTruncatedSupport(const EFSupport &S, 
				     const bool strong,
				     const bool conditional,
				           gOutput & out, // tracefile 
				           gStatus &status)
{
  //  AndyTest(S,status);
  exit(0);

  gBlock<int> players(S.Game().NumPlayers());
  int i;
  for (i = 1; i <= players.Length(); i++)   players[i] = i;

  return SupportWithoutDominatedOfPlayerList(S, 
					    strong, conditional, players, 
					    out, status);
}
