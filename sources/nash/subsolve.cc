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

#include "base/base.h"
#include "subsolve.h"

//-----------------------------------------------------------------------
//               SubgameSolver: Private member functions
//-----------------------------------------------------------------------

void SubgameSolver::FindSubgames(const EFSupport &p_support,
				 gStatus &p_status,
				 Node *n,
				 gList<BehavSolution> &solns,
				 gList<gbtEfgOutcome> &values)
{
  int i;
  efgGame &efg = p_support.GetGame();
  
  gList<BehavProfile<gNumber> > thissolns;
  thissolns.Append(*solution);
  ((gVector<gNumber> &) thissolns[1]).operator=(gNumber(0));
  
  gList<Node *> subroots;
  ChildSubgames(efg, n, subroots);
  
  gList<gArray<gbtEfgOutcome> > subrootvalues;
  subrootvalues.Append(gArray<gbtEfgOutcome>(subroots.Length()));
  
  for (i = 1; i <= subroots.Length(); i++)  {
    gList<BehavSolution> subsolns;
    gList<gbtEfgOutcome> subvalues;
    
    FindSubgames(p_support, p_status, subroots[i], subsolns, subvalues);
    
    if (subsolns.Length() == 0)  {
      solns.Flush();
      return;
    }
    
    assert(subvalues.Length() == subsolns.Length());
    
    gList<BehavProfile<gNumber> > newsolns;
    gList<gArray<gbtEfgOutcome> > newsubrootvalues;
    
    for (int soln = 1; soln <= thissolns.Length(); soln++) {
      for (int subsoln = 1; subsoln <= subsolns.Length(); subsoln++) {
	BehavProfile<gNumber> bp(thissolns[soln]);
	BehavProfile<gNumber> tmp(*subsolns[subsoln].Profile());
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
      efg.SetOutcome(subroots[i], subrootvalues[soln][i]);
    }
    
    efgGame foo(efg, n);
    // this prevents double-counting of outcomes at roots of subgames
    // by convention, we will just put the payoffs in the parent subgame
    foo.SetOutcome(foo.RootNode(), 0);

    gList<Node *> nodes;
    Nodes(efg, n, nodes);
    
    EFSupport subsupport(foo);
    // here, we build the support for the subgame
    for (int pl = 1; pl <= foo.NumPlayers(); pl++)  {
      EFPlayer *p = foo.Players()[pl];
      int index;

      for (index = 1; index <= nodes.Length() &&
	   nodes[index]->GetPlayer() != efg.Players()[pl]; index++);
	
      if (index > nodes.Length())  continue;

      int base;
	
      for (base = 1; base <= efg.Players()[pl]->NumInfosets(); base++)
	if (efg.Players()[pl]->Infosets()[base] ==
	    nodes[index]->GetInfoset())  break;
	
      assert(base <= efg.Players()[pl]->NumInfosets());
	
      for (int iset = 1; iset <= p->NumInfosets(); iset++)  {
	for (index = 1; index <= infosets[pl]->Length(); index++)
	  if ((*infosets[pl])[index] == efg.Players()[pl]->Infosets()[iset + base - 1])
	    break;
	  
	assert(index <= infosets[pl]->Length());
	for (int act = 1; act <= p->Infosets()[iset]->NumActions();
	     act++)  {
          if (!p_support.Find(pl, index, (*infosets[pl])[index]->Actions()[act]))
            subsupport.RemoveAction(p->Infosets()[iset]->Actions()[act]);
	}
      }
    }

    gList<BehavSolution> sol;

    bool interrupted = false;

    try {
      if (m_efgAlgorithm) {
	sol = m_efgAlgorithm->Solve(subsupport, p_status);
      }
      else if (m_nfgAlgorithm) {
	Nfg *nfg = MakeReducedNfg(subsupport);
	NFSupport support(*nfg);

	gList<MixedSolution> nfgSolutions;

	try {
	  nfgSolutions = m_nfgAlgorithm->Solve(support, p_status);
	}
	catch (gSignalBreak &) {
	  delete nfg;
	  throw;
	}

	for (int soln = 1; soln <= nfgSolutions.Length(); soln++) {
	  MixedProfile<gNumber> profile(*nfgSolutions[soln].Profile());
	  sol.Append(BehavProfile<gNumber>(profile));
	}

	delete nfg;
      }
      //      SolveSubgame(foo, subsupport, sol, p_status);
      //      SelectSolutions(subgame_number, foo, sol);
    }
    catch (gSignalBreak &) {
      interrupted = true;
    }
    
    // put behav profile in "total" solution here...
    if (sol.Length() == 0)  {
      solns.Flush();
      return;
    }
    
    for (int solno = 1; solno <= sol.Length(); solno++)  {
      int ii = solns.Append(thissolns[soln]);
      solns[ii].SetEpsilon(sol[solno].Epsilon());
      
      for (int pl = 1; pl <= foo.NumPlayers(); pl++)  {
	EFPlayer *p = foo.Players()[pl];
	int index;

	for (index = 1; index <= nodes.Length() &&
	     nodes[index]->GetPlayer() != efg.Players()[pl]; index++);
	
	if (index > nodes.Length())  continue;

	int base;
	
	for (base = 1; base <= efg.Players()[pl]->NumInfosets(); base++)
	  if (efg.Players()[pl]->Infosets()[base] ==
	      nodes[index]->GetInfoset())  break;
	
	assert(base <= efg.Players()[pl]->NumInfosets());
	
	for (int iset = 1; iset <= p->NumInfosets(); iset++)  {
	  for (index = 1; index <= infosets[pl]->Length(); index++)
	    if ((*infosets[pl])[index] == efg.Players()[pl]->Infosets()[iset + base - 1])
	      break;
	  
	  assert(index <= infosets[pl]->Length());
	  
	  for (int act = 1; act <= subsupport.NumActions(pl, iset); act++) {
	    int actno = subsupport.Actions(pl, iset)[act]->GetNumber();
	    solns[solns.Length()].Set(pl, index, actno,
				      sol[solno](subsupport.Actions(pl, iset)[act]));
	  }
	}
      }
      
      int j = solns.Length();
      //      solns[j].SetCreator((EfgAlgType) AlgorithmID());

      gVector<gNumber> subval(foo.NumPlayers());
      for (i = 1; i <= foo.NumPlayers(); i++)  {
	subval[i] = sol[solno].Payoff(i);
	if (!efg.GetOutcome(n).IsNull())  {
	  subval[i] += efg.Payoff(efg.GetOutcome(n), efg.Players()[i]);
        }
      }

      gbtEfgOutcome ov = efg.NewOutcome();
      for (i = 1; i <= efg.NumPlayers(); i++) {
	efg.SetPayoff(ov, i, subval[i]);
      }
 
      values.Append(ov);
    }

    if (interrupted) {
      throw gSignalBreak();
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

gList<BehavSolution> SubgameSolver::Solve(const EFSupport &p_support,
					  gStatus &p_status)
{
  gWatch watch;

  solutions.Flush();
  gList<gbtEfgOutcome> values;

  solution = new BehavProfile<gNumber>(p_support);
  ((gVector<gNumber> &) *solution).operator=(gNumber(0));

  efgGame efg((const efgGame &) p_support.GetGame());
  infosets = gArray<gArray<Infoset *> *>(efg.NumPlayers());

  for (int i = 1; i <= efg.NumPlayers(); i++)
    infosets[i] = new gArray<Infoset *>(efg.Players()[i]->Infosets());

  EFSupport support(efg);

  for (int pl = 1; pl <= efg.NumPlayers(); pl++)  {
    EFPlayer *player = p_support.GetGame().Players()[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      Infoset *infoset = player->Infosets()[iset];
      for (int act = 1; act <= infoset->NumActions(); act++) 
	if (!p_support.Find(infoset->Actions()[act]))
	  support.RemoveAction(efg.Players()[pl]->Infosets()[iset]->Actions()[act]);
    }
  }

  m_isPerfectRecall = IsPerfectRecall(efg);

  try {
    FindSubgames(support, p_status, efg.RootNode(), solutions, values);
  }
  catch (gSignalBreak &) { }

  for (int i = 1; i <= efg.NumPlayers(); i++) {
    delete infosets[i];
  }

  delete solution;

  time = watch.Elapsed();
  return solutions;
}

gText SubgameSolver::GetAlgorithm(void) const
{
  if (m_efgAlgorithm) {
    return m_efgAlgorithm->GetAlgorithm();
  }
  else if (m_nfgAlgorithm) {
    return m_nfgAlgorithm->GetAlgorithm();
  }
  else {
    return "";
  }
}


#include "base/garray.imp"

template class gArray<gArray<Infoset *> *>;

template bool operator==(const gArray<gbtEfgOutcome> &,
			 const gArray<gbtEfgOutcome> &);
template bool operator!=(const gArray<gbtEfgOutcome> &,
			 const gArray<gbtEfgOutcome> &);

template gOutput &operator<<(gOutput &, const gArray<gbtEfgOutcome> &);

#include "base/glist.imp"

template class gList<gArray<gbtEfgOutcome> >;


