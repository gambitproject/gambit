//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/subgame.cc
// Utilities for computing and verifying subgame-perfection
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

#include <stdlib.h>

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

} // end nested anonymous namespace


//
// Some general notes on the strategy for solving by subgames:
//
// * We work with a *copy* of the original game, which is destroyed
//   as we go.
// * Before solving, information set labels on the copy game are
//   set to unique IDs.  These are used to match up information
//   sets in the subgames (which are themselves copies) to the
//   original game.
// * We only carry around DVectors instead of full MixedBehavProfiles,
//   because MixedBehavProfiles allocate space several times the
//   size of the tree to carry around useful quantities.  These
//   quantities are irrelevant for this calculation, so we only
//   store the probabilities, and convert to MixedBehavProfiles
//   at the end of the computation
//

template <class T, class SolverType>
void SolveSubgames(const BehavSupport &p_support,
		   const DVector<T> &p_templateSolution,
		   SolverType p_solver,
		   GameNode n,
		   List<DVector<T> > &solns,
		   List<GameOutcome> &values)
{
  Game efg = p_support.GetGame();
  
  List<DVector<T> > thissolns;
  thissolns.Append(p_templateSolution);
  ((Vector<T> &) thissolns[1]).operator=(T(0));
  
  List<GameNode> subroots;
  for (int i = 1; i <= n->NumChildren(); i++) {
    ChildSubgames(n->GetChild(i), subroots);
  }
  
  List<Array<GameOutcome> > subrootvalues;
  subrootvalues.Append(Array<GameOutcome>(subroots.Length()));
  
  for (int i = 1; i <= subroots.Length(); i++)  {
    //printf("Looking at subgame %d of %d\n", i, subroots.Length());
    List<DVector<T> > subsolns;
    List<GameOutcome> subvalues;
    
    SolveSubgames(p_support, p_templateSolution, p_solver,
		  subroots[i], subsolns, subvalues);
    
    if (subsolns.Length() == 0)  {
      solns = List<DVector<T> >();
      return;
    }

    //printf("Found %d subsolutions for subgame %d of %d\n", 
    //subsolns.Length(), i, subroots.Length());
    
    List<DVector<T> > newsolns;
    List<Array<GameOutcome> > newsubrootvalues;
    
    //printf("Merging with %d existing solutions\n", thissolns.Length());
    for (int soln = 1; soln <= thissolns.Length(); soln++) {
      for (int subsoln = 1; subsoln <= subsolns.Length(); subsoln++) {
	//printf("Merging existing %d with new %d\n", soln, subsoln);
	DVector<T> bp(thissolns[soln]);
	DVector<T> tmp(subsolns[subsoln]);
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
    //printf("Finished solving subgame %d\n", i);
  }
  
  for (int soln = 1; soln <= thissolns.Length(); soln++)   {
    //printf("Analyzing scenario %d of %d\n", soln, thissolns.Length());
    for (int i = 1; i <= subroots.Length(); i++) {
      subroots[i]->SetOutcome(subrootvalues[soln][i]);
    }
    
    Game subgame = CopyGame(efg, n);
    // this prevents double-counting of outcomes at roots of subgames
    // by convention, we will just put the payoffs in the parent subgame
    subgame->GetRoot()->SetOutcome(0);

    BehavSupport subsupport(subgame);
    // Here, we build the support for the subgame
    /*
    for (int pl = 1; pl <= subgame->NumPlayers(); pl++)  {
      GamePlayer subplayer = subgame->GetPlayer(pl);
      GamePlayer player = p_support.GetGame()->GetPlayer(pl);

      for (int iset = 1; iset <= subplayer->NumInfosets(); iset++) {
	GameInfoset subinfoset = subplayer->GetInfoset(iset);

	for (int j = 1; j <= player->NumInfosets(); j++) {
	  GameInfoset infoset = player->GetInfoset(j);
	  if (subinfoset->GetLabel() == infoset->GetLabel()) {
	    for (int act = 1; act <= infoset->NumActions(); act++)  {
	      if (!p_support.Contains(infoset->GetAction(act))) {
		subsupport.RemoveAction(subinfoset->GetAction(act));
	      }
	    }
	    break;
	  }
	}
      }
    }
    */

    List<MixedBehavProfile<T> > sol = (*p_solver)(subsupport);
    
    if (sol.Length() == 0)  {
      solns = List<DVector<T> >();
      //printf("No solutions found\n");
      return;
    }
    
    // Put behavior profile in "total" solution here...
    for (int solno = 1; solno <= sol.Length(); solno++)  {
      solns.Append(thissolns[soln]);
      
      for (int pl = 1; pl <= subgame->NumPlayers(); pl++)  {
	GamePlayer subplayer = subgame->GetPlayer(pl);
	GamePlayer player = p_support.GetGame()->GetPlayer(pl);

	for (int iset = 1; iset <= subplayer->NumInfosets(); iset++) {
	  GameInfoset subinfoset = subplayer->GetInfoset(iset);

	  for (int j = 1; j <= player->NumInfosets(); j++) {
	    if (subinfoset->GetLabel() == player->GetInfoset(j)->GetLabel()) {
	      int id = atoi(subinfoset->GetLabel().c_str());
	      for (int act = 1; act <= subsupport.NumActions(pl, iset); act++) {
		int actno = subsupport.GetAction(pl, iset, act)->GetNumber();
		solns[solns.Length()](pl, id, actno) = sol[solno](pl, iset, act);	  
	      }
	      break;
	    }
	  }
	}
      }
      
      Vector<T> subval(subgame->NumPlayers());
      GameOutcome outcome = n->GetOutcome();
      for (int pl = 1; pl <= subgame->NumPlayers(); pl++)  {
	subval[pl] = sol[solno].GetPayoff(pl);
	if (outcome) {
	  subval[pl] += outcome->GetPayoff<T>(pl);
        }
      }

      GameOutcome ov = efg->NewOutcome();
      for (int pl = 1; pl <= efg->NumPlayers(); pl++) {
	ov->SetPayoff(pl, lexical_cast<std::string>(subval[pl]));
      }
 
      values.Append(ov);
    }
    //printf("Finished with scenario %d of %d; total solutions so far = %d\n",
    //soln, thissolns.Length(), solns.Length());
  }

  n->DeleteTree();
}

template <class T, typename SolverType>
List<MixedBehavProfile<T> > 
SolveBySubgames(const BehavSupport &p_support,
		SolverType p_solver)
{
  Game efg = CopyGame(p_support.GetGame(), p_support.GetGame()->GetRoot());

  for (int pl = 1; pl <= efg->NumPlayers(); pl++) {
    for (int iset = 1; iset <= efg->GetPlayer(pl)->NumInfosets(); iset++) {
      efg->GetPlayer(pl)->GetInfoset(iset)->SetLabel(lexical_cast<std::string>(iset));
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

  List<DVector<T> > vectors;
  List<GameOutcome> values;
  SolveSubgames(support, DVector<T>(support.NumActions()),
		p_solver, efg->GetRoot(), vectors, values);

  List<MixedBehavProfile<T> > solutions;
  for (int i = 1; i <= vectors.Length(); i++) {
    solutions.Append(MixedBehavProfile<T>(p_support));
    for (int j = 1; j <= vectors[i].Length(); j++) {
      solutions[i][j] = vectors[i][j];
    }
  }
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
