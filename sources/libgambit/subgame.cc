//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Utilities for computing and verifying subgame-perfection
//
// This file is part of Gambit
// Copyright (c) 2006, The Gambit Project
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
#include "subgame.h"

namespace Gambit {


// A nested anonymous namespace to privatize these functions 

namespace {

///
/// Returns a copy of the game 'p_efg', including only the subtree rooted
/// at 'p_node'.
///
Game CopyGame(const Game &p_efg, const GameNode &p_node)
{
  std::ostringstream os;
  p_efg->WriteEfgFile(os, p_node);
  std::istringstream is(os.str());
  return ReadGame(is);
}

///
/// Returns a list of the root nodes of all the immediate proper subgames
/// in the subtree rooted at 'p_node'.
/// 
void ChildSubgames(const GameNode &p_node, List<GameNode> &p_list)
{
  if (p_node->IsSubgameRoot()) {
    p_list.Append(p_node);
  }
  else {
    for (int i = 1; i <= p_node->NumChildren(); 
	 ChildSubgames(p_node->GetChild(i++), p_list));
  }
}

///
/// Returns a list of all nodes in the subtree rooted at 'p_subtree'.
///
void Nodes(const GameNode &p_subtree, List<GameNode> &p_list)
{ 
  p_list.Append(p_subtree);
  for (int i = 1; i <= p_subtree->NumChildren(); 
       Nodes(p_subtree->GetChild(i++), p_list));
}

} // end nested anonymous namespace



template <class T, class SolverType>
void SolveSubgames(const BehavSupport &p_support,
		   SolverType p_solver,
		   const Array<Array<GameInfoset > > &p_infosets,
		   GameNode n,
		   List<MixedBehavProfile<T> > &solns,
		   List<GameOutcome> &values)
{
  Game efg = p_support.GetGame();
  
  List<MixedBehavProfile<T> > thissolns;
  thissolns.Append(MixedBehavProfile<T>(p_support));
  ((Vector<T> &) thissolns[1]).operator=(T(0));
  
  List<GameNode> subroots;
  for (int i = 1; i <= n->NumChildren(); i++) {
    ChildSubgames(n->GetChild(i), subroots);
  }
  
  List<Array<GameOutcome> > subrootvalues;
  subrootvalues.Append(Array<GameOutcome>(subroots.Length()));
  
  for (int i = 1; i <= subroots.Length(); i++)  {
    List<MixedBehavProfile<T> > subsolns;
    List<GameOutcome> subvalues;
    
    SolveSubgames(p_support, p_solver, p_infosets,
		  subroots[i], subsolns, subvalues);
    
    if (subsolns.Length() == 0)  {
      solns = List<MixedBehavProfile<T> >();
      return;
    }
    
    List<MixedBehavProfile<T> > newsolns;
    List<Array<GameOutcome> > newsubrootvalues;
    
    for (int soln = 1; soln <= thissolns.Length(); soln++) {
      for (int subsoln = 1; subsoln <= subsolns.Length(); subsoln++) {
	MixedBehavProfile<T> bp(thissolns[soln]);
	MixedBehavProfile<T> tmp(subsolns[subsoln]);
	for (int j = 1; j <= bp.Length(); j++) {
	  bp[j] += tmp[j];
	}
	newsolns.Append(bp);
	
	newsubrootvalues.Append(subrootvalues[soln]);
	newsubrootvalues[newsubrootvalues.Length()][i] = subvalues[subsoln];
      }
    }
    
    thissolns = newsolns;
    subrootvalues = newsubrootvalues;
  }
  
  for (int soln = 1; soln <= thissolns.Length(); soln++)   {
    for (int i = 1; i <= subroots.Length(); i++) {
      subroots[i]->SetOutcome(subrootvalues[soln][i]);
    }
    
    Game foo = CopyGame(efg, n);
    // this prevents double-counting of outcomes at roots of subgames
    // by convention, we will just put the payoffs in the parent subgame
    foo->GetRoot()->SetOutcome(0);

    List<GameNode> nodes;
    Nodes(n, nodes);
    
    BehavSupport subsupport(foo);
    // here, we build the support for the subgame
    for (int pl = 1; pl <= foo->NumPlayers(); pl++)  {
      GamePlayer p = foo->GetPlayer(pl);
      int index;

      for (index = 1; index <= nodes.Length() &&
	   nodes[index]->GetPlayer() != efg->GetPlayer(pl); index++);
	
      if (index > nodes.Length())  continue;

      int base;
	
      for (base = 1; base <= efg->GetPlayer(pl)->NumInfosets(); base++)
	if (efg->GetPlayer(pl)->GetInfoset(base) ==
	    nodes[index]->GetInfoset())  break;
	
      for (int iset = 1; iset <= p->NumInfosets(); iset++)  {
	for (index = 1; index <= p_infosets[pl].Length(); index++)
	  if (p_infosets[pl][index] == efg->GetPlayer(pl)->GetInfoset(iset + base - 1))
	    break;
	  
	for (int act = 1; act <= p->GetInfoset(iset)->NumActions();
	     act++)  {
          if (!p_support.Contains(p_infosets[pl][index]->GetAction(act))) {
            subsupport.RemoveAction(p->GetInfoset(iset)->GetAction(act));
	  }
	}
      }
    }

    List<MixedBehavProfile<T> > sol;

    bool interrupted = false;

    try {
      sol = (*p_solver)(subsupport);
    }
    catch (...) {
      interrupted = true;
      throw;
    }
    
    // put behav profile in "total" solution here...
    if (sol.Length() == 0)  {
      solns = List<MixedBehavProfile<T> >();
      return;
    }
    
    for (int solno = 1; solno <= sol.Length(); solno++)  {
      solns.Append(thissolns[soln]);
      
      for (int pl = 1; pl <= foo->NumPlayers(); pl++)  {
	GamePlayer p = foo->GetPlayer(pl);
	int index;

	for (index = 1; index <= nodes.Length() &&
	     nodes[index]->GetPlayer() != efg->GetPlayer(pl); index++);
	
	if (index > nodes.Length())  continue;

	int base;
	
	for (base = 1; base <= efg->GetPlayer(pl)->NumInfosets(); base++)
	  if (efg->GetPlayer(pl)->GetInfoset(base) ==
	      nodes[index]->GetInfoset())  break;
	
	for (int iset = 1; iset <= p->NumInfosets(); iset++)  {
	  for (index = 1; index <= p_infosets[pl].Length(); index++)
	    if (p_infosets[pl][index] == efg->GetPlayer(pl)->GetInfoset(iset + base - 1))
	      break;
	  
	  for (int act = 1; act <= subsupport.NumActions(pl, iset); act++) {
	    int actno = subsupport.GetAction(pl, iset, act)->GetNumber();
	    solns[solns.Length()](pl, index, actno) = sol[solno](pl, iset, act);	  }
	}
      }
      
      Vector<T> subval(foo->NumPlayers());
      GameOutcome outcome = n->GetOutcome();
      for (int pl = 1; pl <= foo->NumPlayers(); pl++)  {
	subval[pl] = sol[solno].GetPayoff(pl);
	if (outcome) {
	  subval[pl] += outcome->GetPayoff<T>(pl);
        }
      }

      GameOutcome ov = efg->NewOutcome();
      for (int pl = 1; pl <= efg->NumPlayers(); pl++) {
	ov->SetPayoff(pl, ToText(subval[pl]));
      }
 
      values.Append(ov);
    }
  }

  n->DeleteTree();
}

template <class T, typename SolverType>
List<MixedBehavProfile<T> > 
SolveBySubgames(const BehavSupport &p_support,
		SolverType p_solver)
{
  Game efg = CopyGame(p_support.GetGame(), p_support.GetGame()->GetRoot());

  Array<Array<GameInfoset> > infosets(efg->NumPlayers());

  for (int pl = 1; pl <= efg->NumPlayers(); pl++) {
    for (int iset = 1; iset <= efg->GetPlayer(pl)->NumInfosets(); iset++) {
      infosets[pl].Append(efg->GetPlayer(pl)->GetInfoset(iset)); 
    }
  }

  BehavSupport support(efg);

  for (int pl = 1; pl <= efg->NumPlayers(); pl++)  {
    GamePlayer player = p_support.GetGame()->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      GameInfoset infoset = player->GetInfoset(iset);
      for (int act = 1; act <= infoset->NumActions(); act++) {
	if (!p_support.Contains(infoset->GetAction(act))) {
	  support.RemoveAction(efg->GetPlayer(pl)->GetInfoset(iset)->GetAction(act));
	}
      }
    }
  }

  List<MixedBehavProfile<T> > solutions;
  List<GameOutcome> values;
  SolveSubgames(support, p_solver, infosets, efg->GetRoot(), solutions, values);
  return solutions;
}



//
// Explicit instantiations follow
//

template List<MixedBehavProfile<double> > 
SolveBySubgames(const BehavSupport &p_support, DoubleSolver p_solver);

template List<MixedBehavProfile<Rational> > 
SolveBySubgames(const BehavSupport &p_support, RationalSolver p_solver);

} // end namespace Gambit
