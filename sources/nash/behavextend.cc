//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Algorithms for extending behavior profiles to Nash equilibria
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

#include "behavextend.h"
#include "poly/ineqsolv.h"

//
// Design choice: the auxiliary functions here are made static
// rather than members to help hide the gbtPolyMulti-related details of
// the implementation.  Some of these functions might be more
// generally useful, in which case they should be made visible
// somehow.  Also, a namespace would be preferable to using
// static, but static is used for portability.  -- TLT, 5/2001.
//

#ifdef UNUSED

//
// This file is disabled while support interface is in flux
//

//=========================================================================
//                      class algExtendsToNash
//=========================================================================

static void DeviationInfosets(gbtList<gbtGameInfoset> &answer,
			      const gbtEfgSupport & big_supp,
			      const gbtGamePlayer &pl,
			      const gbtGameNode &node,
			      const gbtGameAction &act)
{
  gbtGameNode child  = node->GetChild(act);
  if (child->IsNonterminal()) {
    gbtGameInfoset iset = child->GetInfoset();
    if (iset->GetPlayer() == pl) {
      int insert = 0;
      bool done = false;
      while (!done) {
	insert ++;
	if (insert > answer.Length() ||
	    iset->Precedes(answer[insert]->GetMember(1)))
	  done = true;
      }
      answer.Insert(iset,insert);
    }
    for (int j = 1; j <= iset->NumActions(); j++) {
      DeviationInfosets(answer,big_supp,pl,child, iset->GetAction(j));
    }
  }
}

static gbtList<gbtGameInfoset> DeviationInfosets(const gbtEfgSupport &big_supp,
					      const gbtGamePlayer &pl,
					      const gbtGameInfoset &iset,
					      const gbtGameAction &act)
{
  gbtList<gbtGameInfoset> answer;
  
  for (int i = 1; i <= iset->NumMembers(); i++) {
    DeviationInfosets(answer, big_supp, pl, iset->GetMember(i), act);
  }

  return answer;
}

template <class T>
static gbtPolyMultiList<gbtDouble> 
ActionProbsSumToOneIneqs(const gbtBehavProfile<T> &p_solution,
			 const gbtPolySpace &BehavStratSpace, 
			 const gbtPolyTermOrder &Lex,
			 const gbtEfgSupport &big_supp,
			 const gbtList<gbtList<int> > &var_index) 
{
  gbtPolyMultiList<gbtDouble> answer(&BehavStratSpace, &Lex);

  for (int pl = 1; pl <= p_solution.GetGame().NumPlayers(); pl++) 
    for (gbtGameInfosetIterator infoset(p_solution.GetGame().GetPlayer(pl)); 
	 !infoset.End(); infoset++) {
      if (!big_supp.HasActiveActionAt(*infoset)) {
	int index_base = var_index[pl][(*infoset)->GetId()];
	gbtPolyMulti<gbtDouble> factor(&BehavStratSpace, (gbtDouble)1.0, &Lex);
	for (int k = 1; k < (*infoset)->NumActions(); k++)
	  factor -= gbtPolyMulti<gbtDouble>(&BehavStratSpace, index_base + k, 1, &Lex);
	answer += factor;
      }
    }
  return answer;
}

static gbtList<gbtEfgSupport> 
DeviationSupports(const gbtEfgSupport & big_supp,
		  const gbtList<gbtGameInfoset> &isetlist,
		  const gbtGamePlayer &/*pl*/,
		  const gbtGameInfoset &/*iset*/,
		  const gbtGameAction &/*act*/)
{
  gbtList<gbtEfgSupport> answer;

  gbtArray<int> active_act_no(isetlist.Length());

  for (int k = 1; k <= active_act_no.Length(); k++)
    active_act_no[k] = 0;
 
  gbtEfgSupport new_supp(big_supp);

  for (int i = 1; i <= isetlist.Length(); i++) {
    for (int j = 1; j < isetlist[i]->NumActions(); j++)
      new_supp.RemoveAction(isetlist[i]->GetAction(j));
    new_supp.AddAction(isetlist[i]->GetAction(1));

    active_act_no[i] = 1;
    for (int k = 1; k < i; k++)
      if (isetlist[k]->Precedes(isetlist[i]->GetMember(1)))
	if (isetlist[k]->GetAction(1)->Precedes(isetlist[i]->GetMember(1))) {
	  new_supp.RemoveAction(isetlist[i]->GetAction(1));
	  active_act_no[i] = 0;
	}
  }
  answer += new_supp;

  int iset_cursor = isetlist.Length();
  while (iset_cursor > 0) {
    if ( active_act_no[iset_cursor] == 0 || 
	 active_act_no[iset_cursor] == isetlist[iset_cursor]->NumActions() )
      iset_cursor--;
    else {
      new_supp.RemoveAction(isetlist[iset_cursor]->
			    GetAction(active_act_no[iset_cursor]));
      active_act_no[iset_cursor]++;
      new_supp.AddAction(isetlist[iset_cursor]->
			 GetAction(active_act_no[iset_cursor]));
      for (int k = iset_cursor + 1; k <= isetlist.Length(); k++) {
	if (active_act_no[k] > 0)
	  new_supp.RemoveAction(isetlist[k]->GetAction(1));
	int h = 1;
	bool active = true;
	while (active && h < k) {
	  if (isetlist[h]->Precedes(isetlist[k]->GetMember(1)))
	    if (active_act_no[h] == 0 || 
		!isetlist[h]->GetAction(active_act_no[h])->Precedes(isetlist[k]->GetMember(1))) {
	      active = false;
	      if (active_act_no[k] > 0) {
		new_supp.RemoveAction(isetlist[k]->GetAction(active_act_no[k]));
		active_act_no[k] = 0;
	      }
	    }
	  h++;
	}
	if (active){
	  new_supp.AddAction(isetlist[k]->GetAction(1));
	  active_act_no[k] = 1;
	}
      }
      answer += new_supp;
    }
  }
  return answer;
}

template <class T> static bool 
NashNodeProbabilityPoly(const gbtBehavProfile<T> &p_solution,
			gbtPolyMulti<gbtDouble> & node_prob,
			const gbtPolySpace &BehavStratSpace, 
			const gbtPolyTermOrder &Lex,
			const gbtEfgSupport &dsupp,
			const gbtList<gbtList<int> > &var_index,
			gbtGameNode tempnode,
			const gbtGamePlayer &/*pl*/,
			const gbtGameInfoset &iset,
			const gbtGameAction &act)
{
  while (tempnode != p_solution.GetGame().GetRoot()) {
    gbtGameAction last_action = tempnode->GetPriorAction();
    gbtGameInfoset last_infoset = last_action->GetInfoset();
    
    if (last_infoset->IsChanceInfoset()) 
      node_prob *= (gbtDouble) last_action->GetChanceProb();
    else 
      if (dsupp.HasActiveActionAt(last_infoset)) {
	if (last_infoset == iset) {
	  if (act != last_action) {
	    return false;
	  }
	}
	else
	  if (dsupp.Contains(last_action)) {
	    if (last_action->GetInfoset()->GetPlayer() !=
		act->GetInfoset()->GetPlayer() ||
		!act->Precedes(tempnode) )
	    node_prob *= (gbtDouble) p_solution.GetActionProb(last_action);
	  }
	  else {
	    return false;
	  }
      }
      else {
	int initial_var_no = 
	  var_index[last_infoset->GetPlayer()->GetId()][last_infoset->GetId()];
	if (last_action->GetId() < last_infoset->NumActions()) {
	  int varno = initial_var_no + last_action->GetId();
	  node_prob *= gbtPolyMulti<gbtDouble>(&BehavStratSpace, varno, 1, &Lex);
	}
	else {
	  gbtPolyMulti<gbtDouble> factor(&BehavStratSpace, (gbtDouble)1.0, &Lex);
	  int k;
	  for (k = 1; k < last_infoset->NumActions(); k++)
	    factor -= gbtPolyMulti<gbtDouble>(&BehavStratSpace,
				     initial_var_no + k, 1, &Lex);
	  node_prob *= factor;
	}
      } 
    tempnode = tempnode->GetParent();
  }
  return true;
}

template <class T>
static gbtPolyMultiList<gbtDouble> 
NashExpectedPayoffDiffPolys(const gbtBehavProfile<T> &p_solution,
			    const gbtPolySpace &BehavStratSpace, 
			    const gbtPolyTermOrder &Lex,
			    const gbtEfgSupport &little_supp,
			    const gbtEfgSupport &big_supp,
			    const gbtList<gbtList<int> > &var_index) 
{
  gbtPolyMultiList<gbtDouble> answer(&BehavStratSpace, &Lex);

  gbtList<gbtGameNode> terminal_nodes;
  TerminalNodes(p_solution.GetGame().GetRoot(), terminal_nodes);

  for (int pl = 1; pl <= p_solution.GetGame().NumPlayers(); pl++) {
    gbtGamePlayer player = p_solution.GetGame().GetPlayer(pl);
    for (gbtGameInfosetIterator infoset(player); !infoset.End(); infoset++) {
      if (little_supp.MayReach(*infoset)) {
	for (int j = 1; j <= (*infoset)->NumActions(); j++)
	  if (!little_supp.Contains((*infoset)->GetAction(j))) {
	    gbtList<gbtGameInfoset> isetlist = DeviationInfosets(big_supp, 
							      player,
							      *infoset,
							      (*infoset)->GetAction(j));
	    gbtList<gbtEfgSupport> dsupps = DeviationSupports(big_supp, 
							isetlist, 
							player,
							*infoset,
							(*infoset)->GetAction(j));
	    for (int k = 1; k <= dsupps.Length(); k++) {

	    // This will be the utility difference between the
	    // payoff resulting from the profile and deviation to 
	    // the strategy for pl specified by dsupp[k]

	      gbtPolyMulti<gbtDouble> next_poly(&BehavStratSpace, &Lex);

	      for (int n = 1; n <= terminal_nodes.Length(); n++) {
		gbtPolyMulti<gbtDouble> node_prob(&BehavStratSpace, (gbtDouble)1.0, &Lex);
		if (NashNodeProbabilityPoly(p_solution, node_prob,
					    BehavStratSpace,
					    Lex,
					    dsupps[k],
					    var_index,
					    terminal_nodes[n],
					    player,
					    *infoset,
					    (*infoset)->GetAction(j))) {
		  node_prob *= 
		    (gbtDouble) terminal_nodes[n]->GetOutcome()->GetPayoff(player);
		  next_poly += node_prob;
		}
	      }
	      answer += -next_poly + (gbtDouble) p_solution.Payoff(pl);
	    }
	  }
      }
    }
  }
  return answer;
}

template <class T>
static gbtPolyMultiList<gbtDouble> 
ExtendsToNashIneqs(const gbtBehavProfile<T> &p_solution,
		   const gbtPolySpace &BehavStratSpace, 
		   const gbtPolyTermOrder &Lex,
		   const gbtEfgSupport &little_supp,
		   const gbtEfgSupport &big_supp,
		   const gbtList<gbtList<int> > &var_index)
{
  gbtPolyMultiList<gbtDouble> answer(&BehavStratSpace, &Lex);
  answer += ActionProbsSumToOneIneqs(p_solution, BehavStratSpace, 
				     Lex, 
				     big_supp, 
				     var_index);

  answer += NashExpectedPayoffDiffPolys(p_solution, BehavStratSpace, 
				    Lex, 
				    little_supp,
				    big_supp,
				    var_index);
  return answer;
}

template <class T>
bool algExtendsToNash<T>::ExtendsToNash(const gbtBehavProfile<T> &p_solution,
					const gbtEfgSupport &little_supp,
					const gbtEfgSupport &big_supp,
					gbtStatus &m_status)
{
  // This asks whether there is a Nash extension of the BehavSolution to 
  // all information sets at which the behavioral probabilities are not
  // specified.  The assumption is that the support has active actions
  // at infosets at which the behavioral probabilities are defined, and
  // no others.  Also, the BehavSol is assumed to be already a Nash
  // equilibrium for the truncated game obtained by eliminating stuff
  // outside little_supp.
  
  // First we compute the number of variables, and indexing information
  int num_vars(0);
  gbtList<gbtList<int> > var_index;

  for (gbtEfgPlayerIterator player(p_solution.GetGame()); 
       !player.End(); player++) {
    gbtList<int> list_for_pl;

    for (gbtGameInfosetIterator infoset(*player); !infoset.End(); infoset++) {
      list_for_pl += num_vars;
      if (!big_supp.HasActiveActionAt(*infoset)) {
	num_vars += (*infoset)->NumActions() - 1;
      }
    }
    var_index += list_for_pl;
  }

  // We establish the space
  gbtPolySpace BehavStratSpace(num_vars);
  ORD_PTR ptr = &lex;
  gbtPolyTermOrder Lex(&BehavStratSpace, ptr);

  num_vars = BehavStratSpace.Dmnsn();

  gbtPolyMultiList<gbtDouble> inequalities = ExtendsToNashIneqs(p_solution,
						       BehavStratSpace,
						       Lex,
						       little_supp,
						       big_supp,
						       var_index);
  // set up the rectangle of search
  gbtVector<gbtDouble> bottoms(num_vars), tops(num_vars);
  bottoms = (gbtDouble)0;
  tops = (gbtDouble)1;
  gRectangle<gbtDouble> Cube(bottoms, tops); 

  // Set up the test and do it
  gbtPolyIneqSolve<gbtDouble> extension_tester(inequalities,m_status);
  gbtVector<gbtDouble> sample(num_vars);
  bool answer = extension_tester.ASolutionExists(Cube,sample); 
  
  //  assert (answer == m_profile->ExtendsToNash(little_supp, big_supp, m_status));

  return answer;
}


//=========================================================================
//                   class algExtendsToAgentNash
//=========================================================================

template <class T>
static bool ANFNodeProbabilityPoly(const gbtBehavProfile<T> &p_solution,
				   gbtPolyMulti<gbtDouble> & node_prob,
				   const gbtPolySpace &BehavStratSpace, 
				   const gbtPolyTermOrder &Lex,
				   const gbtEfgSupport &big_supp,
				   const gbtList<gbtList<int> > &var_index,
				   gbtGameNode tempnode,
				   const int &pl,
				   const int &i,
				   const int &j)
{
  while (tempnode != p_solution.GetGame().GetRoot()) {
    gbtGameAction last_action = tempnode->GetPriorAction();
    gbtGameInfoset last_infoset = last_action->GetInfoset();
    
    if (last_infoset->IsChanceInfoset()) 
      node_prob *= (gbtDouble) last_action->GetChanceProb();
    else 
      if (big_supp.HasActiveActionAt(last_infoset)) {
	if (last_infoset == p_solution.GetGame().GetPlayer(pl)->GetInfoset(i)) {
	  if (j != last_action->GetId()) 
	    return false;
	}
	else
	  if (big_supp.Contains(last_action))
	    node_prob *= (gbtDouble) p_solution.GetActionProb(last_action);
	  else 
	    return false;
      }
      else {
	int initial_var_no = 
	  var_index[last_infoset->GetPlayer()->GetId()][last_infoset->GetId()];
	if (last_action->GetId() < last_infoset->NumActions()) {
	  int varno = initial_var_no + last_action->GetId();
	  node_prob *= gbtPolyMulti<gbtDouble>(&BehavStratSpace, varno, 1, &Lex);
	}
	else {
	  gbtPolyMulti<gbtDouble> factor(&BehavStratSpace, (gbtDouble)1.0, &Lex);
	  int k;
	  for (k = 1; k < last_infoset->NumActions(); k++)
	    factor -= gbtPolyMulti<gbtDouble>(&BehavStratSpace,
				     initial_var_no + k, 1, &Lex);
	  node_prob *= factor;
	}
      } 
    tempnode = tempnode->GetParent();
  }
  return true;
}

template <class T>
static gbtPolyMultiList<gbtDouble> 
ANFExpectedPayoffDiffPolys(const gbtBehavProfile<T> &p_solution,
			   const gbtPolySpace &BehavStratSpace, 
			   const gbtPolyTermOrder &Lex,
			   const gbtEfgSupport &little_supp,
			   const gbtEfgSupport &big_supp,
			   const gbtList<gbtList<int> > &var_index)
{
  gbtPolyMultiList<gbtDouble> answer(&BehavStratSpace, &Lex);

  gbtList<gbtGameNode> terminal_nodes;
  TerminalNodes(p_solution.GetGame().GetRoot(), terminal_nodes);

  for (gbtEfgPlayerIterator player(p_solution.GetGame());
       !player.End(); player++) { 
    for (gbtGameInfosetIterator infoset(*player); !infoset.End(); infoset++) {
      if (little_supp.MayReach(*infoset)) 
	for (int j = 1; j <= (*infoset)->NumActions(); j++)
	  if (!little_supp.Contains((*player)->GetId(),
				    (*infoset)->GetId(), j)) {
	
	    // This will be the utility difference between the
	    // payoff resulting from the profile and deviation to 
	    // action j
	    gbtPolyMulti<gbtDouble> next_poly(&BehavStratSpace, &Lex);

	    for (int n = 1; n <= terminal_nodes.Length(); n++) {
	      gbtPolyMulti<gbtDouble> node_prob(&BehavStratSpace, (gbtDouble)1.0, &Lex);
	      if (ANFNodeProbabilityPoly(p_solution, node_prob,
					 BehavStratSpace,
					 Lex,
					 big_supp,
					 var_index,
					 terminal_nodes[n],
					 (*player)->GetId(),
					 (*infoset)->GetId(), j)) {
		node_prob *= 
		  (gbtDouble) terminal_nodes[n]->GetOutcome()->GetPayoff(*player);
		next_poly += node_prob;
	      }
	    }
	    answer += -next_poly + (gbtDouble) p_solution.Payoff((*player)->GetId());
	  }
    }
  }
  return answer;
}

template <class T>
static gbtPolyMultiList<gbtDouble> 
ExtendsToANFNashIneqs(const gbtBehavProfile<T> &p_solution,
		      const gbtPolySpace &BehavStratSpace, 
		      const gbtPolyTermOrder &Lex,
		      const gbtEfgSupport &little_supp,
		      const gbtEfgSupport &big_supp,
		      const gbtList<gbtList<int> > &var_index)
{
  gbtPolyMultiList<gbtDouble> answer(&BehavStratSpace, &Lex);
  answer += ActionProbsSumToOneIneqs(p_solution, BehavStratSpace, 
				     Lex, 
				     big_supp, 
				     var_index);
  answer += ANFExpectedPayoffDiffPolys(p_solution, BehavStratSpace, 
				       Lex, 
				       little_supp,
				       big_supp,
				       var_index);
  return answer;
}

template <class T> bool
algExtendsToAgentNash<T>::ExtendsToAgentNash(const gbtBehavProfile<T> &p_solution,
					     const gbtEfgSupport &little_supp,
					     const gbtEfgSupport &big_supp,
					     gbtStatus &p_status)
{
  // This asks whether there is an ANF Nash extension of the BehavSolution to 
  // all information sets at which the behavioral probabilities are not
  // specified.  The assumption is that the support has active actions
  // at infosets at which the behavioral probabilities are defined, and
  // no others.
  
  // First we compute the number of variables, and indexing information
  int num_vars(0);
  gbtList<gbtList<int> > var_index;

  for (gbtEfgPlayerIterator player(p_solution.GetGame());
       !player.End(); player++) {
    gbtList<int> list_for_pl;

    for (gbtGameInfosetIterator infoset(*player); !infoset.End(); infoset++) {
      list_for_pl += num_vars;
      if (!big_supp.HasActiveActionAt(*infoset)) {
	num_vars += (*infoset)->NumActions() - 1;
      }
    }
    var_index += list_for_pl;
  }

  // We establish the space
  gbtPolySpace BehavStratSpace(num_vars);
  ORD_PTR ptr = &lex;
  gbtPolyTermOrder Lex(&BehavStratSpace, ptr);

  num_vars = BehavStratSpace.Dmnsn();
  gbtPolyMultiList<gbtDouble> inequalities = ExtendsToANFNashIneqs(p_solution,
							  BehavStratSpace,
							  Lex,
							  little_supp,
							  big_supp,
							  var_index);

  // set up the rectangle of search
  gbtVector<gbtDouble> bottoms(num_vars), tops(num_vars);
  bottoms = (gbtDouble)0;
  tops = (gbtDouble)1;
  gRectangle<gbtDouble> Cube(bottoms, tops); 

  // Set up the test and do it
  gbtPolyIneqSolve<gbtDouble> extension_tester(inequalities, p_status);
  gbtVector<gbtDouble> sample(num_vars);

  // Temporarily, we check the old set up vs. the new
  bool ANFanswer = extension_tester.ASolutionExists(Cube,sample); 
  //  assert (ANFanswer == m_profile->ExtendsToANFNash(little_supp,
  //						   big_supp,
  //						   m_status));

  /* 
  bool NASHanswer = m_profile->ExtendsToNash(Support(),Support(),m_status);

  //DEBUG
  if (ANFanswer && !NASHanswer)
    gout << 
      "The following should be extendable to an ANF Nash, but not to a Nash:\n"
	 << *m_profile << "\n\n";
  if (NASHanswer && !ANFanswer)
    gout << 
      "ERROR: said to be extendable to a Nash, but not to an ANF Nash:\n"
	 << *m_profile << "\n\n";
	  */
  return ANFanswer;
}


//====================================================================
//                     Template instantiations
//====================================================================

#include "math/rational.h"
#include "math/double.h"

template class algExtendsToNash<double>;
template class algExtendsToNash<gbtDouble>;
template class algExtendsToNash<gbtRational>;

template class algExtendsToAgentNash<double>;
template class algExtendsToAgentNash<gbtDouble>;
template class algExtendsToAgentNash<gbtRational>;

#endif  // UNUSED
