//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute dominated actions in extensive form games
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "efdom.h"
#include "game/efgciter.h"
#include "gamebase.h"

gbtEfgDominanceException::gbtEfgDominanceException(const gbtText &p_description)
  : m_description(p_description)
{ }

gbtEfgDominanceException::~gbtEfgDominanceException()
{ }

gbtText gbtEfgDominanceException::Description(void) const
{
  return m_description;
}


bool gbtEfgSupportBase::Dominates(const gbtGameAction &a, const gbtGameAction &b,
				  bool strong, const bool conditional) const
{
  gbtGameInfoset infoset = a->GetInfoset();
  if (infoset != b->GetInfoset())
    throw gbtEfgDominanceException
      ("Dominates(..) needs actions in same infoset.\n");

  gbtEfgSupport SAct(Copy());
  gbtGamePlayer player = infoset->GetPlayer();
  int pl = player->GetId();
  bool equal = true;

  if (!conditional) {
    gbtEfgContIterator A(Copy()), B(Copy());
    A.Freeze(player->GetId(), infoset->GetId()); 
    B.Freeze(player->GetId(), infoset->GetId());
    A.Set(a);
    B.Set(b);

    do  {
      gbtRational ap = A.GetPayoff(pl);  
      gbtRational bp = B.GetPayoff(pl);

      if (strong)
	{ if (ap <= bp)  return false; }
      else
	if (ap < bp)   return false; 
	else if (ap > bp)  equal = false;
    } while (A.NextContingency() && B.NextContingency());
  }

  else {
    gbtList<gbtGameNode> nodelist = SAct->ReachableNodesInInfoset(infoset);  
    if (nodelist.Length() == 0) {
      // This may not be a good idea; I suggest checking for this 
      // prior to entry
      for (int i = 1; i <= infoset->NumMembers(); i++) {
	nodelist.Append(infoset->GetMember(i));
      }
    }
    
    for (int n = 1; n <= nodelist.Length(); n++) {
      gbtList<gbtGameInfoset> L;
      L += ReachableInfosets(nodelist[n], a);
      L += ReachableInfosets(nodelist[n], b);
      L.RemoveRedundancies();
      
      gbtEfgConditionalContIterator A(Copy(), L), B(Copy(), L);
      A.Set(a);
      B.Set(b);
      
      do  {
	gbtRational ap = A.Payoff(nodelist[n],pl);  
	gbtRational bp = B.Payoff(nodelist[n],pl);
	
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

bool SomeElementDominates(const gbtEfgSupport &S, 
			  const gbtArray<gbtGameAction> &array,
			  const gbtGameAction &a,
			  bool strong, bool conditional)
{
  for (int i = 1; i <= array.Length(); i++)
    if (array[i] != a)
      if (S->Dominates(array[i],a,strong,conditional)) {
	return true;
      }
  return false;
}

bool gbtEfgSupportBase::IsDominated(const gbtGameAction &a, 
				    bool strong, bool conditional) const
{
  gbtArray<gbtGameAction> array(NumActions(a->GetInfoset()));
  for (int act = 1; act <= array.Length(); act++) {
    array[act] = GetAction(a->GetInfoset(), act);
  }
  return SomeElementDominates(Copy(),array,a,strong,conditional);
}

bool InfosetHasDominatedElement(const gbtEfgSupport &S, 
				gbtGameInfoset infoset,
				bool strong, bool conditional,
				gbtStatus &/*status*/)
{
  gbtArray<gbtGameAction> actions(S->NumActions(infoset));
  for (int i = 1; i <= actions.Length(); i++) {
    actions[i] = S->GetAction(infoset, i);
  }
  for (int i = 1; i <= S->NumActions(infoset); i++)
    if (SomeElementDominates(S,actions,actions[i],
			     strong,conditional))
      return true;

  return false;
}

bool ElimDominatedInInfoset(const gbtEfgSupport &S, gbtEfgSupport &T,
			     const int pl, 
			     const int iset, 
			     const bool strong,
			     const bool conditional,
		                   gbtStatus &status)
{
  gbtArray<bool> is_dominated(S->NumActions(pl, iset));
  for (int k = 1; k <= S->NumActions(pl, iset); is_dominated[k++] = false);

  for (int i = 1; i <= S->NumActions(pl, iset); i++) {
    for (int j = 1; j <= S->NumActions(pl, iset); j++) {
      if (i != j && !is_dominated[i] && !is_dominated[j]) { 
	if (S->Dominates(S->GetAction(pl, iset, i), S->GetAction(pl, iset, j),
			 strong, conditional)) {
	  is_dominated[j] = true;
	  status.Get();
	}
      }
    }
  }
      
  bool action_was_eliminated = false;
  int k = 1;
  while (k <= S->NumActions(pl, iset) && !action_was_eliminated) {
    if (is_dominated[k]) action_was_eliminated = true;
    else k++;
  }
  gbtArray<gbtGameAction> actions(S->NumActions(pl, iset));
  for (int act = 1; act <= actions.Length(); act++) {
    actions[act] = S->GetAction(pl, iset, act);
  }
  while (k <= S->NumActions(pl, iset)) {
    if (is_dominated[k]) 
      T->RemoveAction(actions[k]);
    k++;
  }

  return action_was_eliminated;
}

bool ElimDominatedForPlayer(const gbtEfgSupport &S, gbtEfgSupport &T,
			    const int pl, int &cumiset,
			    const bool strong,
			    const bool conditional,
		                  gbtStatus &status)
{
  bool action_was_eliminated = false;

  for (int iset = 1; iset <= S->GetPlayer(pl)->NumInfosets();
       iset++, cumiset++) {
    status.SetProgress((double) cumiset /
		       (double) S->NumPlayerInfosets());
    status.Get();
    if (ElimDominatedInInfoset(S, T, pl, iset, strong, conditional, status)) 
      action_was_eliminated = true;
  }

  return action_was_eliminated;
}

gbtEfgSupport
gbtEfgSupportBase::Undominated(bool strong, bool conditional,
			       const gbtArray<int> &players,
			       gbtOutput &, // tracefile 
			       gbtStatus &status) const
{
  gbtEfgSupport T(Copy());
  int cumiset = 0;

  for (int i = 1; i <= players.Length(); i++)   {
    status.Get();
    status.SetProgress(0, (gbtText("Eliminating strategies for player ") +
			   ToText(players[i])));
    int pl = players[i];
    ElimDominatedForPlayer(Copy(), T, pl, cumiset, 
			   strong, conditional, status); 
  }

  return T;
}
