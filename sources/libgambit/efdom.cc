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

#include "libgambit.h"
#include "efdom.h"

template <class T> void RemoveRedundancies(gbtList<T> &p_list)
{
  int i = 1; int j = 2;		
  while (i < p_list.Length()) {
    if (p_list[i] == p_list[j])
      p_list.Remove(j);
    else 
      j++;
    if (j > p_list.Length()) { i++; j = i+1; }
  }
}

efgDominanceException::efgDominanceException(const std::string &p_description)
  : m_description(p_description)
{ }

efgDominanceException::~efgDominanceException()
{ }

std::string efgDominanceException::GetDescription(void) const
{
  return m_description;
}


bool gbtEfgSupport::Dominates(const Gambit::GameAction &a, const Gambit::GameAction &b,
			      bool strong, bool conditional) const
{
  Gambit::GameInfoset infoset = a->GetInfoset();
  if (infoset != b->GetInfoset())
    throw efgDominanceException
      ("Dominates(..) needs actions in same infoset.\n");

  const gbtEfgSupportWithActiveInfo SAct(*this);
  Gambit::GamePlayer player = infoset->GetPlayer();
  int pl = player->GetNumber();
  bool equal = true;

  if (!conditional) {
    EfgContIter A(*this), B(*this);
    A.Freeze(player->GetNumber(), infoset->GetNumber()); 
    B.Freeze(player->GetNumber(), infoset->GetNumber());
    A.Set(a);
    B.Set(b);

    do  {
      gbtRational ap = A.Payoff(pl);  
      gbtRational bp = B.Payoff(pl);

      if (strong)
	{ if (ap <= bp)  return false; }
      else
	if (ap < bp)   return false; 
	else if (ap > bp)  equal = false;
    } while (A.NextContingency() && B.NextContingency());
  }

  else {
    gbtList<Gambit::GameNode> nodelist = SAct.ReachableNodesInInfoset(infoset);  
    if (nodelist.Length() == 0) {
      // This may not be a good idea; I suggest checking for this
      // prior to entry
      for (int i = 1; i <= infoset->NumMembers(); i++) {
	nodelist.Append(infoset->GetMember(i));
      }
    }
    
    for (int n = 1; n <= nodelist.Length(); n++) {
      
      gbtList<Gambit::GameInfoset> L;
      L += ReachableInfosets(nodelist[n], a);
      L += ReachableInfosets(nodelist[n], b);
      RemoveRedundancies(L);
      
      EfgConditionalContIter A(*this,L), B(*this,L);
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
			  const gbtArray<Gambit::GameAction> &array,
			  const Gambit::GameAction &a, 
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

bool gbtEfgSupport::IsDominated(const Gambit::GameAction &a, 
				bool strong, bool conditional) const
{
  gbtArray<Gambit::GameAction> array(Actions(a->GetInfoset()));
  return SomeElementDominates(*this,array,a,strong,conditional);
}

bool InfosetHasDominatedElement(const gbtEfgSupport &S, 
				const Gambit::GameInfoset &i,
				bool strong,
				bool conditional)
{
  gbtArray<Gambit::GameAction> actions = S.Actions(i);
  for (int i = 1; i <= actions.Length(); i++)
    if (SomeElementDominates(S,actions,actions[i],
			     strong,conditional))
      return true;

  return false;
}

bool ElimDominatedInInfoset(const gbtEfgSupport &S, gbtEfgSupport &T,
			     const int pl, 
			     const int iset, 
			     const bool strong,
			    const bool conditional)
{
  const gbtArray<Gambit::GameAction> &actions = S.Actions(pl, iset);

  gbtArray<bool> is_dominated(actions.Length());
  for (int k = 1; k <= actions.Length(); k++)
    is_dominated[k] = false;

  for (int i = 1; i <= actions.Length(); i++)
    for (int j = 1; j <= actions.Length(); j++)
      if (i != j && !is_dominated[i] && !is_dominated[j]) 
	if (S.Dominates(actions[i], actions[j], strong, conditional)) {
	  is_dominated[j] = true;
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

bool ElimDominatedForPlayer(const gbtEfgSupport &S, gbtEfgSupport &T,
			    const int pl, int &cumiset,
			    const bool strong,
			    const bool conditional)
{
  bool action_was_eliminated = false;

  for (int iset = 1; iset <= S.GetGame()->GetPlayer(pl)->NumInfosets();
       iset++, cumiset++) {
    if (ElimDominatedInInfoset(S, T, pl, iset, strong, conditional)) 
      action_was_eliminated = true;
  }

  return action_was_eliminated;
}

gbtEfgSupport gbtEfgSupport::Undominated(bool strong, bool conditional,
				  const gbtArray<int> &players,
				 std::ostream &) const
{
  gbtEfgSupport T(*this);
  int cumiset = 0;

  for (int i = 1; i <= players.Length(); i++)   {
    int pl = players[i];
    ElimDominatedForPlayer(*this, T, pl, cumiset, 
			   strong, conditional); 
  }

  return T;
}
