//
// FILE: subsolve.cc -- Implementation of solve-by-subgame algorithms
//
// $Id$
//

#include "gsignal.h"
#include "efg.h"
#include "efgutils.h"
#include "nfg.h"
#include "nfstrat.h"
#include "gwatch.h"
#include "rational.h"

#include "subsolve.h"

void SubgameSolver::FindSubgames(const EFSupport &p_support, Node *n,
				 gList<BehavSolution> &solns,
				 gList<EFOutcome *> &values)
{
  int i;
  Efg &efg = (Efg &) p_support.Game();
  bool marked = AllSubgamesMarked(efg);
  
  gList<BehavProfile<gNumber> > thissolns;
  thissolns.Append(*solution);
  ((gVector<gNumber> &) thissolns[1]).operator=(gNumber(0));
  
  gList<Node *> subroots;
  ChildSubgames(n, subroots);
  
  gList<gArray<EFOutcome *> > subrootvalues;
  subrootvalues.Append(gArray<EFOutcome *>(subroots.Length()));
  
  for (i = 1; i <= subroots.Length(); i++)  {
    gList<BehavSolution> subsolns;
    gList<EFOutcome *> subvalues;
    
    FindSubgames(p_support, subroots[i], subsolns, subvalues);
    
    if (subsolns.Length() == 0)  {
      solns.Flush();
      return;
    }
    
    assert(subvalues.Length() == subsolns.Length());
    
    gList<BehavProfile<gNumber> > newsolns;
    gList<gArray<EFOutcome *> > newsubrootvalues;
    
    for (int soln = 1; soln <= thissolns.Length() &&
	 (max_solns == 0 || newsolns.Length() <= max_solns);
	 soln++)
      for (int subsoln = 1; subsoln <= subsolns.Length() &&
	   (max_solns == 0 || newsolns.Length() <= max_solns); subsoln++)  {
	BehavProfile<gNumber> bp(thissolns[soln]);
	BehavProfile<gNumber> tmp(subsolns[subsoln]);
	for (int j = 1; j <= bp.Length(); j++)
	  bp[j] += tmp[j];
	newsolns.Append(bp);
	
	newsubrootvalues.Append(subrootvalues[soln]);
	newsubrootvalues[newsubrootvalues.Length()][i] = subvalues[subsoln];
      }
    
    thissolns = newsolns;
    subrootvalues = newsubrootvalues;
  }
  
  assert(n->GetSubgameRoot() == n);
  
  // This is here to allow called hook code to figure out which subgame
  // is currently being solved.  The number should correspond to the index
  // of the subgame in the list returned by SubgameRoots().
  
  subgame_number++;
  
  for (int soln = 1; soln <= thissolns.Length(); soln++)   {
    for (i = 1; i <= subroots.Length(); i++)
      subroots[i]->SetOutcome(subrootvalues[soln][i]);
    
    Efg foo(efg, n);
    // this prevents double-counting of outcomes at roots of subgames
    // by convention, we will just put the payoffs in the parent subgame
    foo.RootNode()->SetOutcome(0);
    
    ViewSubgame(subgame_number, foo);
    
    gList<Node *> nodes;
    Nodes(efg, n, nodes);
    
    gList<BehavSolution> sol;

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
	  if (!p_support.Find((*infosets[pl])[index]->Actions()[act])) 
	    subsupport.RemoveAction(p->Infosets()[iset]->Actions()[act]);
	}
      }
    }

    bool interrupted = false;

    try {
      SolveSubgame(foo, subsupport, sol);
      SelectSolutions(subgame_number, foo, sol);
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
      solns.Append(thissolns[soln]);
      
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
	  
	  for (int act = 1; act <= subsupport.NumActions(pl, iset); act++)
	    solns[solns.Length()].Set(((*infosets[pl])[index]->Actions()[act]),
              sol[solno](subsupport.Actions(pl, iset)[act]));
	}

	int j = solns.Length();
	solns[j].SetCreator((EfgAlgType) AlgorithmID());
	
	if (m_isPerfectRecall)
	  solns[j].SetIsNash(triTRUE);
	if (marked && m_isPerfectRecall)
	  solns[j].SetIsSubgamePerfect(triTRUE); 
	if (solns[j].Creator() == EfgAlg_ELIAPSUB
	    && m_isPerfectRecall) {
	  solns[j].SetLiap(solns[j].LiapValue());
	  solns[j].SetIsSequential(triTRUE);      // even if marked = false
	  solns[j].SetIsSubgamePerfect(triTRUE);  // even if marked = false
	}
      }
      
      gVector<gNumber> subval(foo.NumPlayers());
      for (i = 1; i <= foo.NumPlayers(); i++)  {
	subval[i] = sol[solno].Payoff(i);
	if (n->GetOutcome())  {
	  subval[i] += efg.Payoff(n->GetOutcome(), i);
        }
      }

      EFOutcome *ov = efg.NewOutcome();
      for (i = 1; i <= efg.NumPlayers(); i++)
	efg.SetPayoff(ov, i, subval[i]);
 
      values.Append(ov);

      if (interrupted) {
	throw gSignalBreak();
      }
    }
  }

  efg.DeleteTree(n);
}

// These are dummies... for specific applications, these can be overriden
// in derived classes to allow interactive access to the solution process

// This is called immediately after the subgame is constructed in the
// solution process.  Mostly to allow viewing of the subgame, but probably
// isn't generally useful.

void SubgameSolver::ViewSubgame(int, const Efg &)
{ }

// This is called in the normal-form solution modules after the normal
// form is constructed.  Note especially that the Nfg is passed
// non-const, so that strategies may be eliminated as seen fit.
// It is assumed that the NFSupport returned is "sensible"

void SubgameSolver::ViewNormal(const Nfg &, NFSupport *&)
{ }
// This is called for each subgame after the solutions have been computed
// The idea is for the called code to possibly allow for viewing or
// selection of "interesting" equilibria for further computation during
// the process.  Again, there is no restriction that one can't
// muck about with the solution list in "bad" ways using this.
// Caveat utor!

void SubgameSolver::SelectSolutions(int, const Efg &,
				       gList<BehavSolution> &)
{ }

SubgameSolver::SubgameSolver(int max)
  : max_solns(max)
{ }

SubgameSolver::~SubgameSolver()  
{ }

gList<BehavSolution> SubgameSolver::Solve(const EFSupport &p_support)
{
  gWatch watch;

  solutions.Flush();
  subgame_number = 0;

  gList<EFOutcome *> values;

  solution = new BehavProfile<gNumber>(p_support);
  ((gVector<gNumber> &) *solution).operator=(gNumber(0));

  Efg efg(p_support.Game());
  infosets = gArray<gArray<Infoset *> *>(efg.NumPlayers());

  for (int i = 1; i <= efg.NumPlayers(); i++)
    infosets[i] = new gArray<Infoset *>(efg.Players()[i]->Infosets());

  EFSupport support(efg);

  for (int pl = 1; pl <= efg.NumPlayers(); pl++)  {
    EFPlayer *player = p_support.Game().Players()[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      Infoset *infoset = player->Infosets()[iset];
      for (int act = 1; act <= infoset->NumActions(); act++) 
	if (!p_support.Find(infoset->Actions()[act]))
	  support.RemoveAction(efg.Players()[pl]->Infosets()[iset]->Actions()[act]);
    }
  }

  m_isPerfectRecall = IsPerfectRecall(efg);

  try {
    FindSubgames(support, efg.RootNode(), solutions, values);
  }
  catch (gSignalBreak &) { }

  for (int i = 1; i <= efg.NumPlayers(); i++)
    delete infosets[i];

  delete solution;

  time = watch.Elapsed();
  return solutions;
}


#include "garray.imp"

template class gArray<gArray<Infoset *> *>;

template bool operator==(const gArray<EFOutcome *> &,
			 const gArray<EFOutcome *> &);
template bool operator!=(const gArray<EFOutcome *> &,
			 const gArray<EFOutcome *> &);

template gOutput &operator<<(gOutput &, const gArray<EFOutcome *> &);


#include "glist.imp"

template class gList<EFOutcome *>;
template class gList<gArray<EFOutcome *> >;


