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
  gbtEfgPlayer player = infoset->GetPlayer();
  int pl = player.GetId();
  bool equal = true;

  if (!conditional) {
    EfgContIter A(*this), B(*this);
    A.Freeze(player.GetId(), infoset->GetNumber()); 
    B.Freeze(player.GetId(), infoset->GetNumber());
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
    if (nodelist.Length() == 0) {
      // This may not be a good idea; I suggest checking for this 
      // prior to entry
      for (int i = 1; i <= infoset->NumMembers(); i++) {
	nodelist.Append(infoset->GetMember(i));
      }
    }
    
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
  gArray<Action *> array(NumActions(a->BelongsTo()));
  for (int act = 1; act <= array.Length(); act++) {
    array[act] = GetAction(a->BelongsTo(), act);
  }
  return SomeElementDominates(*this,array,a,strong,conditional);
}

bool InfosetHasDominatedElement(const EFSupport &S, 
				Infoset *infoset,
				const bool strong,
				const bool conditional,
				const gStatus &/*status*/)
{
  gArray<Action *> actions(S.NumActions(infoset));
  for (int i = 1; i <= actions.Length(); i++) {
    actions[i] = S.GetAction(infoset, i);
  }
  for (int i = 1; i <= S.NumActions(infoset); i++)
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
  gArray<bool> is_dominated(S.NumActions(pl, iset));
  for (int k = 1; k <= S.NumActions(pl, iset); is_dominated[k++] = false);

  for (int i = 1; i <= S.NumActions(pl, iset); i++) {
    for (int j = 1; j <= S.NumActions(pl, iset); j++) {
      if (i != j && !is_dominated[i] && !is_dominated[j]) { 
	if (S.Dominates(S.GetAction(pl, iset, i), S.GetAction(pl, iset, j),
			strong, conditional)) {
	  is_dominated[j] = true;
	  status.Get();
	}
      }
    }
  }
      
  bool action_was_eliminated = false;
  int k = 1;
  while (k <= S.NumActions(pl, iset) && !action_was_eliminated) {
    if (is_dominated[k]) action_was_eliminated = true;
    else k++;
  }
  gArray<Action *> actions(S.NumActions(pl, iset));
  for (int act = 1; act <= actions.Length(); act++) {
    actions[act] = S.GetAction(pl, iset, act);
  }
  while (k <= S.NumActions(pl, iset)) {
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

  for (int iset = 1; iset <= S.GetGame().GetPlayer(pl).NumInfosets();
       iset++, cumiset++) {
    status.SetProgress((double) cumiset /
		       (double) S.GetGame().NumPlayerInfosets());
    status.Get();
    if (ElimDominatedInInfoset(S, T, pl, iset, strong, conditional, status)) 
      action_was_eliminated = true;
  }

  return action_was_eliminated;
}

EFSupport EFSupport::Undominated(bool strong, bool conditional,
				  const gArray<int> &players,
				  gOutput &, // tracefile 
				  gStatus &status) const
{
  EFSupport T(*this);
  int cumiset = 0;

  for (int i = 1; i <= players.Length(); i++)   {
    status.Get();
    status.SetProgress(0, (gText("Eliminating strategies for player ") +
			   ToText(players[i])));
    int pl = players[i];
    ElimDominatedForPlayer(*this, T, pl, cumiset, 
			   strong, conditional, status); 
  }

  return T;
}
