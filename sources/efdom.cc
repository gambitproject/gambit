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


bool EFSupport::Dominates(const Action *a, const Action *b,
			  bool strong, const bool conditional) const
{
  const Infoset *infoset = a->BelongsTo();
  if (infoset != b->BelongsTo())
    throw efgDominanceException
      ("Dominates(..) needs actions in same infoset.\n");

  const EFSupportWithActiveInfo SAct(*this);
  const EFPlayer *player = infoset->GetPlayer();
  int pl = player->GetNumber();
  bool equal = true;

  if (!conditional) {
    EfgContIter A(*this), B(*this);
    A.Freeze(player->GetNumber(), infoset->GetNumber()); 
    B.Freeze(player->GetNumber(), infoset->GetNumber());
    A.Set(a);
    B.Set(b);

    do  {
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
      
      gList<Infoset *> L;
      L += ReachableInfosets(nodelist[n], a);
      L += ReachableInfosets(nodelist[n], b);
      L.RemoveRedundancies();
      
      EfgConditionalContIter A(*this,L), B(*this,L);
      A.Set(a);
      B.Set(b);
      
      do  {
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
  /*
  return ::Dominates(*this,player->GetNumber(),infoset->GetNumber(),
		   a->GetNumber(),b->GetNumber(),
		   strong, conditional);
  */
}

bool SomeElementDominates(const EFSupport &S, 
			  const gArray<Action *> &array,
			  const Action *a, 
			  const bool strong,
			  const bool conditional)
{
  for (int i = 1; i <= array.Length(); i++)
    if (array[i] != a)
      if (S.Dominates(array[i],a,strong,conditional)) {
	return true;
      }
  return false;
}

bool EFSupport::IsDominated(const Action *a, 
			    bool strong, bool conditional) const
{
  gArray<Action *> array(Actions(a->BelongsTo()));
  return SomeElementDominates(*this,array,a,strong,conditional);
}

bool InfosetHasDominatedElement(const EFSupport &S, 
				const Infoset *i,
				const bool strong,
				const bool conditional,
				const gStatus &/*status*/)
{
  gArray<Action *> actions = S.Actions(i);
  for (int i = 1; i <= actions.Length(); i++)
    if (SomeElementDominates(S,actions,actions[i],
			     strong,conditional))
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
	if (S.Dominates(actions[i], actions[j], strong, conditional)) {
	  is_dominated[j] = true;
	  status.Get();
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
			    const int pl, int &cumiset,
			    const bool strong,
			    const bool conditional,
		                  gStatus &status)
{
  bool action_was_eliminated = false;

  for (int iset = 1; iset <= S.GetGame().Players()[pl]->NumInfosets();
       iset++, cumiset++) {
    status.SetProgress((double) cumiset /
		       (double) S.GetGame().NumPlayerInfosets());
    status.Get();
    if (ElimDominatedInInfoset(S, T, pl, iset, strong, conditional, status)) 
      action_was_eliminated = true;
  }

  return action_was_eliminated;
}

EFSupport *EFSupport::Undominated(bool strong, bool conditional,
				  const gArray<int> &players,
				  gOutput &, // tracefile 
				  gStatus &status) const
{
  EFSupport *T = new EFSupport(*this);
  bool any = false;
  int cumiset;

  try {
    cumiset = 0;
    for (int i = 1; i <= players.Length(); i++)   {
      status.Get();
      int pl = players[i];
      if (ElimDominatedForPlayer(*this, *T, pl, cumiset, 
				 strong, conditional, status)) 
	any = true;
    }

    if (!any)  {
      delete T;
      return 0;
    }
  }
  catch (gSignalBreak &E) {
    delete T;
    throw;
  }

  return T;
}
