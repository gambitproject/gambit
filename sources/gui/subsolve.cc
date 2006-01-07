//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria of an extensive form game by recursively
// solving subgames
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

#include "libgambit/libgambit.h"
#include "subsolve.h"

void ChildSubgames(gbtEfgNode n, gbtList<gbtEfgNode> &p_list)
{
  if (n->IsSubgameRoot()) {
    p_list.Append(n);
  }
  else {
    for (int i = 1; i <= n->NumChildren(); i++) {
      ChildSubgames(n->GetChild(i), p_list);
    }
  }
}

static void NDoChild(const gbtEfgGame &e, gbtEfgNode n, gbtList<gbtEfgNode> &list)
{ 
  list.Append(n);
  for (int i = 1; i <= n->NumChildren(); i++)
    NDoChild (e, n->GetChild(i), list);
}

static void Nodes (const gbtEfgGame &befg, gbtList <gbtEfgNode> &list)
{
  list = gbtList<gbtEfgNode>();
  NDoChild(befg, befg->GetRoot(), list); 
}

static void Nodes (const gbtEfgGame &efg, gbtEfgNode n, gbtList <gbtEfgNode> &list)
{
  list = gbtList<gbtEfgNode>();
  NDoChild(efg,n, list);
}

//-----------------------------------------------------------------------
//               SubgameSolver: Private member functions
//-----------------------------------------------------------------------

void SubgameSolver::FindSubgames(const gbtEfgSupport &p_support,
				 gbtEfgNode n,
				 gbtList<gbtBehavProfile<gbtNumber> > &solns,
				 gbtList<gbtEfgOutcome> &values)
{
  int i;
  gbtEfgGame efg = p_support.GetGame();
  
  gbtList<gbtBehavProfile<gbtNumber> > thissolns;
  thissolns.Append(*solution);
  ((gbtVector<gbtNumber> &) thissolns[1]).operator=(gbtNumber(0));
  
  gbtList<gbtEfgNode> subroots;
  ChildSubgames(n, subroots);
  
  gbtList<Gambit::Array<gbtEfgOutcome> > subrootvalues;
  subrootvalues.Append(Gambit::Array<gbtEfgOutcome>(subroots.Length()));
  
  for (i = 1; i <= subroots.Length(); i++)  {
    gbtList<gbtBehavProfile<gbtNumber> > subsolns;
    gbtList<gbtEfgOutcome> subvalues;
    
    FindSubgames(p_support, subroots[i], subsolns, subvalues);
    
    if (subsolns.Length() == 0)  {
      solns = gbtList<gbtBehavProfile<gbtNumber> >();
      return;
    }
    
    assert(subvalues.Length() == subsolns.Length());
    
    gbtList<gbtBehavProfile<gbtNumber> > newsolns;
    gbtList<Gambit::Array<gbtEfgOutcome> > newsubrootvalues;
    
    for (int soln = 1; soln <= thissolns.Length(); soln++) {
      for (int subsoln = 1; subsoln <= subsolns.Length(); subsoln++) {
	gbtBehavProfile<gbtNumber> bp(thissolns[soln]);
	gbtBehavProfile<gbtNumber> tmp(subsolns[subsoln]);
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
    for (i = 1; i <= subroots.Length(); i++) {
      subroots[i]->SetOutcome(subrootvalues[soln][i]);
    }
    
    gbtEfgGame foo(efg, n);
    // this prevents double-counting of outcomes at roots of subgames
    // by convention, we will just put the payoffs in the parent subgame
    foo.GetRoot()->SetOutcome(0);

    gbtList<gbtEfgNode> nodes;
    Nodes(efg, n, nodes);
    
    gbtEfgSupport subsupport(foo);
    // here, we build the support for the subgame
    for (int pl = 1; pl <= foo.NumPlayers(); pl++)  {
      gbtEfgPlayer p = foo.GetPlayer(pl);
      int index;

      for (index = 1; index <= nodes.Length() &&
	   nodes[index]->GetPlayer() != efg.GetPlayer(pl); index++);
	
      if (index > nodes.Length())  continue;

      int base;
	
      for (base = 1; base <= efg.GetPlayer(pl)->NumInfosets(); base++)
	if (efg.GetPlayer(pl)->GetInfoset(base) ==
	    nodes[index]->GetInfoset())  break;
	
      assert(base <= efg.GetPlayer(pl)->NumInfosets());
	
      for (int iset = 1; iset <= p->NumInfosets(); iset++)  {
	for (index = 1; index <= infosets[pl]->Length(); index++)
	  if ((*infosets[pl])[index] == efg.GetPlayer(pl)->GetInfoset(iset + base - 1))
	    break;
	  
	assert(index <= infosets[pl]->Length());
	for (int act = 1; act <= p->GetInfoset(iset)->NumActions();
	     act++)  {
          if (!p_support.Find(pl, index, (*infosets[pl])[index]->GetAction(act)))
            subsupport.RemoveAction(p->GetInfoset(iset)->GetAction(act));
	}
      }
    }

    gbtList<gbtBehavProfile<gbtNumber> > sol;

    bool interrupted = false;

    try {
      if (m_efgAlgorithm) {
	sol = m_efgAlgorithm->Solve(subsupport);
      }
      else if (m_nfgAlgorithm) {
	gbtNfgGame *nfg = subsupport.GetGame().MakeReducedNfg();
	gbtNfgSupport support(nfg);

	gbtList<gbtMixedProfile<gbtNumber> > nfgSolutions;

	try {
	  nfgSolutions = m_nfgAlgorithm->Solve(support);
	}
	catch (...) {
	  delete nfg;
	  throw;
	}

	for (int soln = 1; soln <= nfgSolutions.Length(); soln++) {
	  gbtMixedProfile<gbtNumber> profile(nfgSolutions[soln]);
	  sol.Append(gbtBehavProfile<gbtNumber>(profile));
	}

	delete nfg;
      }
    }
    catch (...) {
      interrupted = true;
      throw;
    }
    
    // put behav profile in "total" solution here...
    if (sol.Length() == 0)  {
      solns = gbtList<gbtBehavProfile<gbtNumber> >();
      return;
    }
    
    for (int solno = 1; solno <= sol.Length(); solno++)  {
      int ii = solns.Append(thissolns[soln]);
      
      for (int pl = 1; pl <= foo.NumPlayers(); pl++)  {
	gbtEfgPlayer p = foo.GetPlayer(pl);
	int index;

	for (index = 1; index <= nodes.Length() &&
	     nodes[index]->GetPlayer() != efg.GetPlayer(pl); index++);
	
	if (index > nodes.Length())  continue;

	int base;
	
	for (base = 1; base <= efg.GetPlayer(pl)->NumInfosets(); base++)
	  if (efg.GetPlayer(pl)->GetInfoset(base) ==
	      nodes[index]->GetInfoset())  break;
	
	assert(base <= efg.GetPlayer(pl)->NumInfosets());
	
	for (int iset = 1; iset <= p->NumInfosets(); iset++)  {
	  for (index = 1; index <= infosets[pl]->Length(); index++)
	    if ((*infosets[pl])[index] == efg.GetPlayer(pl)->GetInfoset(iset + base - 1))
	      break;
	  
	  assert(index <= infosets[pl]->Length());
	  
	  for (int act = 1; act <= subsupport.NumActions(pl, iset); act++) {
	    int actno = subsupport.Actions(pl, iset)[act]->GetNumber();
	    solns[solns.Length()](pl, index, actno) = sol[solno](pl, iset, act);	  }
	}
      }
      
      int j = solns.Length();

      gbtVector<gbtNumber> subval(foo.NumPlayers());
      for (i = 1; i <= foo.NumPlayers(); i++)  {
	subval[i] = sol[solno].Payoff(i);
	if (n->GetOutcome())  {
	  subval[i] += n->GetOutcome()->GetPayoff(i);
        }
      }

      gbtEfgOutcome ov = efg.NewOutcome();
      for (i = 1; i <= efg.NumPlayers(); i++)
	ov->SetPayoff(i, ToText(subval[i]));
 
      values.Append(ov);
    }
  }

  efg.DeleteTree(n);
}

//-----------------------------------------------------------------------
//                      SubgameSolver: Lifecycle
//-----------------------------------------------------------------------

SubgameSolver::~SubgameSolver()
{
  if (m_efgAlgorithm) {
    delete m_efgAlgorithm;
  }
  else if (m_nfgAlgorithm) {
    delete m_nfgAlgorithm;
  }
}

//-----------------------------------------------------------------------
//               SubgameSolver: Public member functions
//-----------------------------------------------------------------------

gbtList<gbtBehavProfile<gbtNumber> > SubgameSolver::Solve(const gbtEfgSupport &p_support)
{
  solutions = gbtList<gbtBehavProfile<gbtNumber> >();
  gbtList<gbtEfgOutcome> values;

  solution = new gbtBehavProfile<gbtNumber>(p_support);
  ((gbtVector<gbtNumber> &) *solution).operator=(gbtNumber(0));

  gbtEfgGame efg((const gbtEfgGame &) p_support.GetGame());
  infosets = Gambit::Array<Gambit::Array<gbtEfgInfoset> *>(efg.NumPlayers());

  for (int i = 1; i <= efg.NumPlayers(); i++) {
    infosets[i] = new Gambit::Array<gbtEfgInfoset>;
    for (int iset = 1; iset <= efg.GetPlayer(i)->NumInfosets(); iset++) {
      infosets[i]->Append(efg.GetPlayer(i)->GetInfoset(iset)); 
    }
  }

  gbtEfgSupport support(efg);

  for (int pl = 1; pl <= efg.NumPlayers(); pl++)  {
    gbtEfgPlayer player = p_support.GetGame().GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      gbtEfgInfoset infoset = player->GetInfoset(iset);
      for (int act = 1; act <= infoset->NumActions(); act++) 
	if (!p_support.Find(infoset->GetAction(act)))
	  support.RemoveAction(efg.GetPlayer(pl)->GetInfoset(iset)->GetAction(act));
    }
  }

  m_isPerfectRecall = efg.IsPerfectRecall();

  try {
    FindSubgames(support, efg.GetRoot(), solutions, values);
  }
  catch (...) { }

  for (int i = 1; i <= efg.NumPlayers(); i++) {
    delete infosets[i];
  }

  delete solution;

  return solutions;
}

