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

void gbtEfgNashSubgames::FindSubgames(const gbtEfgSupport &p_support,
				      gbtStatus &p_status,
				      gbtGameNode n,
				      gbtList<BehavSolution> &solns,
				      gbtList<gbtGameOutcome> &values)
{
  int i;
  
  gbtList<gbtBehavProfile<gbtNumber> > thissolns;
  thissolns.Append(solution);
  ((gbtVector<gbtNumber> &) thissolns[1]).operator=(gbtNumber(0));
  
  gbtList<gbtGameNode> subroots;
  ChildSubgames(p_support->GetTree(), n, subroots);
  
  gbtList<gbtArray<gbtGameOutcome> > subrootvalues;
  subrootvalues.Append(gbtArray<gbtGameOutcome>(subroots.Length()));
  
  for (i = 1; i <= subroots.Length(); i++)  {
    gbtList<BehavSolution> subsolns;
    gbtList<gbtGameOutcome> subvalues;
    
    FindSubgames(p_support, p_status, subroots[i], subsolns, subvalues);
    
    if (subsolns.Length() == 0)  {
      solns.Flush();
      return;
    }
    
    assert(subvalues.Length() == subsolns.Length());
    
    gbtList<gbtBehavProfile<gbtNumber> > newsolns;
    gbtList<gbtArray<gbtGameOutcome> > newsubrootvalues;
    
    for (int soln = 1; soln <= thissolns.Length(); soln++) {
      for (int subsoln = 1; subsoln <= subsolns.Length(); subsoln++) {
	gbtBehavProfile<gbtNumber> bp(thissolns[soln]);
	gbtBehavProfile<gbtNumber> tmp(subsolns[subsoln].Profile());
	for (int j = 1; j <= bp->BehavProfileLength(); j++) {
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
    
    gbtGame foo = p_support->GetTree()->Copy(n);
    // this prevents double-counting of outcomes at roots of subgames
    // by convention, we will just put the payoffs in the parent subgame
    foo->GetRoot()->SetOutcome(0);

    gbtList<gbtGameNode> nodes;
    Nodes(p_support->GetTree(), n, nodes);
    
    gbtEfgSupport subsupport = foo->NewEfgSupport();
    // here, we build the support for the subgame
    for (int pl = 1; pl <= foo->NumPlayers(); pl++)  {
      gbtGamePlayer p = foo->GetPlayer(pl);
      int index;

      for (index = 1; index <= nodes.Length() &&
	   nodes[index]->GetPlayer() != p_support->GetPlayer(pl); index++);
	
      if (index > nodes.Length())  continue;

      int base;
	
      for (base = 1; base <= p_support->GetPlayer(pl)->NumInfosets(); base++)
	if (p_support->GetPlayer(pl)->GetInfoset(base) ==
	    nodes[index]->GetInfoset())  break;
	
      assert(base <= p_support->GetPlayer(pl)->NumInfosets());
	
      for (int iset = 1; iset <= p->NumInfosets(); iset++)  {
	for (index = 1; index <= infosets[pl]->Length(); index++)
	  if ((*infosets[pl])[index] == p_support->GetPlayer(pl)->GetInfoset(iset + base - 1))
	    break;
	  
	assert(index <= infosets[pl]->Length());
	for (int act = 1; act <= p->GetInfoset(iset)->NumActions();
	     act++)  {
          if (!p_support->Contains((*infosets[pl])[index]->GetAction(act))) {
            subsupport->RemoveAction(p->GetInfoset(iset)->GetAction(act));
	  }
	}
      }
    }

    gbtList<BehavSolution> sol;

    bool interrupted = false;

    try {
      if (m_efgAlgorithm) {
	sol = m_efgAlgorithm->Solve(subsupport, p_status);
      }
      else if (m_nfgAlgorithm) {
	CompressEfgInPlace(foo, subsupport);
	subsupport = foo->NewEfgSupport();
	gbtGame nfg = foo;
	gbtNfgSupport support(nfg->NewNfgSupport());

	gbtList<MixedSolution> nfgSolutions;

	try {
	  nfgSolutions = m_nfgAlgorithm->Solve(support, p_status);
	}
	catch (gbtSignalBreak &) {
	  throw;
	}

	for (int soln = 1; soln <= nfgSolutions.Length(); soln++) {
	  gbtMixedProfile<gbtNumber> profile(nfgSolutions[soln].Profile());
	  sol.Append((gbtBehavProfile<gbtNumber>) profile);
	}
      }
    }
    catch (gbtSignalBreak &) {
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
      
      for (int pl = 1; pl <= foo->NumPlayers(); pl++)  {
	gbtGamePlayer p = foo->GetPlayer(pl);
	int index;

	for (index = 1; index <= nodes.Length() &&
	     nodes[index]->GetPlayer() != p_support->GetPlayer(pl); index++);
	
	if (index > nodes.Length())  continue;

	int base;
	
	for (base = 1; base <= p_support->GetPlayer(pl)->NumInfosets(); base++)
	  if (p_support->GetPlayer(pl)->GetInfoset(base) ==
	      nodes[index]->GetInfoset())  break;
	
	assert(base <= p_support->GetPlayer(pl)->NumInfosets());
	
	for (int iset = 1; iset <= p->NumInfosets(); iset++)  {
	  for (index = 1; index <= infosets[pl]->Length(); index++)
	    if ((*infosets[pl])[index] == p_support->GetPlayer(pl)->GetInfoset(iset + base - 1))
	      break;
	  
	  assert(index <= infosets[pl]->Length());
	  
	  for (int act = 1; act <= subsupport->NumActions(pl, iset); act++) {
	    int actno = subsupport->GetAction(pl, iset, act)->GetId();
	    solns[solns.Length()].Set(pl, index, actno,
				      sol[solno](subsupport->GetAction(pl, iset, act)));
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

      gbtVector<gbtNumber> subval(foo->NumPlayers());
      for (i = 1; i <= foo->NumPlayers(); i++)  {
	subval[i] = sol[solno].Payoff(i);
	if (!n->GetOutcome().IsNull())  {
	  subval[i] += n->GetOutcome()->GetPayoff(p_support->GetPlayer(i));
        }
      }

      gbtGameOutcome ov = p_support->GetTree()->NewOutcome();
      for (i = 1; i <= p_support->NumPlayers(); i++) {
	ov->SetPayoff(p_support->GetPlayer(i), subval[i]);
      }
 
      values.Append(ov);
    }

    if (interrupted) {
      throw gbtSignalBreak();
    }
  }

  n->DeleteTree();
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

gbtList<BehavSolution> gbtEfgNashSubgames::Solve(const gbtEfgSupport &p_support,
					  gbtStatus &p_status)
{
  solutions.Flush();
  gbtList<gbtGameOutcome> values;

  solution = p_support->NewBehavProfile(gbtNumber(0));
  //  solution = gbtNumber(0);

  gbtGame efg = p_support->GetTree()->Copy(p_support->GetTree()->GetRoot());
  infosets = gbtArray<gbtArray<gbtGameInfoset> *>(efg->NumPlayers());

  for (int i = 1; i <= efg->NumPlayers(); i++) {
    infosets[i] = new gbtArray<gbtGameInfoset>(efg->GetPlayer(i)->NumInfosets());
    for (int j = 1; j <= efg->GetPlayer(i)->NumInfosets(); j++) {
      (*infosets[i])[j] = efg->GetPlayer(i)->GetInfoset(j);
    }
  }

  gbtEfgSupport support = efg->NewEfgSupport();

  for (int pl = 1; pl <= efg->NumPlayers(); pl++)  {
    gbtGamePlayer player = p_support->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      gbtGameInfoset infoset = player->GetInfoset(iset);
      for (int act = 1; act <= infoset->NumActions(); act++) { 
	if (!p_support->Contains(infoset->GetAction(act))) {
	  support->RemoveAction(efg->GetPlayer(pl)->GetInfoset(iset)->GetAction(act));
	}
      }
    }
  }

  m_isPerfectRecall = efg->IsPerfectRecall();

  try {
    FindSubgames(support, p_status, efg->GetRoot(), solutions, values);
  }
  catch (gbtSignalBreak &) { }

  for (int i = 1; i <= efg->NumPlayers(); i++) {
    delete infosets[i];
  }

  return solutions;
}

gbtText gbtEfgNashSubgames::GetAlgorithm(void) const
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

template class gbtArray<gbtArray<gbtGameInfoset> *>;

template bool operator==(const gbtArray<gbtGameOutcome> &,
			 const gbtArray<gbtGameOutcome> &);
template bool operator!=(const gbtArray<gbtGameOutcome> &,
			 const gbtArray<gbtGameOutcome> &);

template gbtOutput &operator<<(gbtOutput &, const gbtArray<gbtGameOutcome> &);

#include "base/glist.imp"

template class gbtList<gbtArray<gbtGameOutcome> >;
