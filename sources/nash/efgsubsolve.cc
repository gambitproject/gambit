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
#include "efgsubsolve.h"

//-----------------------------------------------------------------------
//              gbtEfgNashSubgames: Private member functions
//-----------------------------------------------------------------------

void gbtEfgNashSubgames::FindSubgames(const EFSupport &p_support,
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
      gbtEfgPlayer p = foo.GetPlayer(pl);
      int index;

      for (index = 1; index <= nodes.Length() &&
	   nodes[index]->GetPlayer() != efg.GetPlayer(pl); index++);
	
      if (index > nodes.Length())  continue;

      int base;
	
      for (base = 1; base <= efg.GetPlayer(pl).NumInfosets(); base++)
	if (efg.GetPlayer(pl).GetInfoset(base) ==
	    nodes[index]->GetInfoset())  break;
	
      assert(base <= efg.GetPlayer(pl).NumInfosets());
	
      for (int iset = 1; iset <= p.NumInfosets(); iset++)  {
	for (index = 1; index <= infosets[pl]->Length(); index++)
	  if ((*infosets[pl])[index] == efg.GetPlayer(pl).GetInfoset(iset + base - 1))
	    break;
	  
	assert(index <= infosets[pl]->Length());
	for (int act = 1; act <= p.GetInfoset(iset)->NumActions();
	     act++)  {
          if (!p_support.Contains((*infosets[pl])[index]->Actions()[act])) {
            subsupport.RemoveAction(p.GetInfoset(iset)->Actions()[act]);
	  }
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
	gbtNfgSupport support(*nfg);

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
	gbtEfgPlayer p = foo.GetPlayer(pl);
	int index;

	for (index = 1; index <= nodes.Length() &&
	     nodes[index]->GetPlayer() != efg.GetPlayer(pl); index++);
	
	if (index > nodes.Length())  continue;

	int base;
	
	for (base = 1; base <= efg.GetPlayer(pl).NumInfosets(); base++)
	  if (efg.GetPlayer(pl).GetInfoset(base) ==
	      nodes[index]->GetInfoset())  break;
	
	assert(base <= efg.GetPlayer(pl).NumInfosets());
	
	for (int iset = 1; iset <= p.NumInfosets(); iset++)  {
	  for (index = 1; index <= infosets[pl]->Length(); index++)
	    if ((*infosets[pl])[index] == efg.GetPlayer(pl).GetInfoset(iset + base - 1))
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
      if (m_efgAlgorithm) {
	solns[j].SetCreator(m_efgAlgorithm->GetAlgorithm());
      }
      else {
	solns[j].SetCreator(m_nfgAlgorithm->GetAlgorithm());
      }

      gVector<gNumber> subval(foo.NumPlayers());
      for (i = 1; i <= foo.NumPlayers(); i++)  {
	subval[i] = sol[solno].Payoff(i);
	if (!efg.GetOutcome(n).IsNull())  {
	  subval[i] += efg.Payoff(efg.GetOutcome(n), efg.GetPlayer(i));
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
//                      gbtEfgNashSubgames: Lifecycle
//-----------------------------------------------------------------------

gbtEfgNashSubgames::~gbtEfgNashSubgames()
{
  if (m_efgAlgorithm) {
    delete m_efgAlgorithm;
  }
  else if (m_nfgAlgorithm) {
    delete m_nfgAlgorithm;
  }
}

//-----------------------------------------------------------------------
//               gbtEfgNashSubgames: Public member functions
//-----------------------------------------------------------------------

gList<BehavSolution> gbtEfgNashSubgames::Solve(const EFSupport &p_support,
					  gStatus &p_status)
{
  gWatch watch;

  solutions.Flush();
  gList<gbtEfgOutcome> values;

  solution = new BehavProfile<gNumber>(p_support);
  ((gVector<gNumber> &) *solution).operator=(gNumber(0));

  efgGame efg((const efgGame &) p_support.GetGame());
  infosets = gArray<gArray<Infoset *> *>(efg.NumPlayers());

  for (int i = 1; i <= efg.NumPlayers(); i++) {
    infosets[i] = new gArray<Infoset *>(efg.GetPlayer(i).NumInfosets());
    for (int j = 1; j <= efg.GetPlayer(i).NumInfosets(); j++) {
      (*infosets[i])[j] = efg.GetPlayer(i).GetInfoset(j);
    }
  }

  EFSupport support(efg);

  for (int pl = 1; pl <= efg.NumPlayers(); pl++)  {
    gbtEfgPlayer player = p_support.GetGame().GetPlayer(pl);
    for (int iset = 1; iset <= player.NumInfosets(); iset++)  {
      Infoset *infoset = player.GetInfoset(iset);
      for (int act = 1; act <= infoset->NumActions(); act++) { 
	if (!p_support.Contains(infoset->Actions()[act])) {
	  support.RemoveAction(efg.GetPlayer(pl).GetInfoset(iset)->Actions()[act]);
	}
      }
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

gText gbtEfgNashSubgames::GetAlgorithm(void) const
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


