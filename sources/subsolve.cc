//#
//# FILE: subsolve.cc -- Implementation of solve-by-subgame algorithms
//#
//# $Id$
//#

//# This file is a mess as far as Borland instantiations are concerned. The
//# positions of each #pragma option and instantiations are crutical.

#include "efg.h"
#include "efgutils.h"
#include "nfg.h"
#include "nfstrat.h"
#include "gwatch.h"
#include "rational.h"

#ifdef __BORLANDC__
#include "behavsol.h"
class gNode<BehavProfile<double> >;
class gNode<BehavProfile<gRational> >;
class gList<BehavProfile<double> >;
class gList<BehavProfile<gRational> >;
class gListIter<BehavProfile<double> >;
class gListIter<BehavProfile<gRational> >;
class gNode<BehavSolution<double> >;
class gNode<BehavSolution<gRational> >;
class gList<BehavSolution<double> >;
class gList<BehavSolution<gRational> >;
class gListIter<BehavSolution<double> >;
class gListIter<BehavSolution<gRational> >;
#include "mixedsol.h"
class gNode<MixedProfile<double> >;
class gNode<MixedProfile<gRational> >;
class gList<MixedProfile<double> >;
class gList<MixedProfile<gRational> >;
class gListIter<MixedProfile<double> >;
class gListIter<MixedProfile<gRational> >;
class gNode<MixedSolution<double> >;
class gNode<MixedSolution<gRational> >;
class gList<MixedSolution<double> >;
class gList<MixedSolution<gRational> >;
class gListIter<MixedSolution<double> >;
class gListIter<MixedSolution<gRational> >;

gOutput &operator<<(gOutput &, const gArray<int> &);
gOutput &operator<<(gOutput &, const gArray<double> &);
gOutput &operator<<(gOutput &, const gArray<gRational> &);
#endif

#pragma option -Jgd		// must come before subsolve.h since SubgameSolver<double> is defined in it.
#include "subsolve.h"

//-------------------------------------------------------------------------
//                Implementation of base solver algorithm
//-------------------------------------------------------------------------

template <class T>
void SubgameSolver<T>::FindSubgames(Node *n, gList<BehavSolution<T> > &solns,
						gList<Outcome *> &values)
{
  int i;
  int failed;
  
  gList<BehavProfile<T> > thissolns;
  thissolns.Append(solution);
  ((gVector<T> &) thissolns[1]).operator=((T) 0);
  
  gList<Node *> subroots;
  ChildSubgames(n, subroots);
  
  gList<gArray<Outcome *> > subrootvalues;
  subrootvalues.Append(gArray<Outcome *>(subroots.Length()));
  
  for (i = 1; i <= subroots.Length(); i++)  {
    gList<BehavSolution<T> > subsolns;
    gList<Outcome *> subvalues;
    
    FindSubgames(subroots[i], subsolns, subvalues);
    
    if (subsolns.Length() == 0)  {
      solns.Flush();
      return;
    }
    
    assert(subvalues.Length() == subsolns.Length());
    
    gList<BehavProfile<T> > newsolns;
    gList<gArray<Outcome *> > newsubrootvalues;
    
    for (int soln = 1; soln <= thissolns.Length() &&
	 (max_solns == 0 || newsolns.Length() <= max_solns);
	 soln++)
      for (int subsoln = 1; subsoln <= subsolns.Length() &&
	   (max_solns == 0 || newsolns.Length() <= max_solns); subsoln++)  {
	BehavProfile<T> bp(thissolns[soln]);
	bp += subsolns[subsoln];
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
    
    Efg<T> foo(efg, n);
    // this prevents double-counting of outcomes at roots of subgames
    // by convention, we will just put the payoffs in the parent subgame
    foo.RootNode()->SetOutcome(0);
    
    ViewSubgame(subgame_number, foo);
    
    gList<BehavSolution<T> > sol;
    failed = SolveSubgame(foo, sol);
    
    SelectSolutions(subgame_number, foo, sol);
    
    // put behav profile in "total" solution here...
    
    if (sol.Length() == 0)  {
      solns.Flush();
      return;
    }
    
    gList<Node *> nodes;
    Nodes(efg, n, nodes);
    
    for (int solno = 1; solno <= sol.Length(); solno++)  {
      solns.Append(thissolns[soln]);
      
      for (int pl = 1; pl <= foo.NumPlayers(); pl++)  {
	EFPlayer *p = foo.PlayerList()[pl];
	int index;
	
	for (index = 1; index <= nodes.Length() &&
	     nodes[index]->GetPlayer() != efg.PlayerList()[pl]; index++);
	
	if (index > nodes.Length())  continue;
	
	int base;
	
	for (base = 1; base <= efg.PlayerList()[pl]->NumInfosets(); base++)
	  if (efg.PlayerList()[pl]->InfosetList()[base] ==
	      nodes[index]->GetInfoset())  break;
	
	assert(base <= efg.PlayerList()[pl]->NumInfosets());
	
	for (int iset = 1; iset <= p->NumInfosets(); iset++)  {
	  for (index = 1; index <= infosets[pl]->Length(); index++)
	    if ((*infosets[pl])[index] == efg.PlayerList()[pl]->InfosetList()[iset + base - 1])
	      break;
	  
	  assert(index <= infosets[pl]->Length());
	  
	  for (int act = 1; act <= p->InfosetList()[iset]->NumActions();
	       act++)
	    solns[solns.Length()](pl, index, act) = sol[solno](pl, iset, act);
	}
	
	solns[solns.Length()].SetCreator(AlgorithmID());
	if(failed == 0)
	  solns[solns.Length()].SetIsNash(T_YES);
      }
      
      gVector<T> subval(foo.NumPlayers());
      for (i = 1; i <= foo.NumPlayers(); i++)  {
	subval[i] = sol[solno].Payoff(i);
	if (n->GetOutcome())
	  subval[i] += ((OutcomeVector<T> &) *n->GetOutcome())[i];
      }

      OutcomeVector<T> *ov = efg.NewOutcome();
      for (i = 1; i <= efg.NumPlayers(); i++)
	(*ov)[i] = subval[i];
 
      values.Append(ov);
    }
  }

  efg.DeleteTree(n);
}

// These are dummies... for specific applications, these can be overriden
// in derived classes to allow interactive access to the solution process

// This is called immediately after the subgame is constructed in the
// solution process.  Mostly to allow viewing of the subgame, but probably
// isn't generally useful.

template <class T>
void SubgameSolver<T>::ViewSubgame(int, const Efg<T> &)
{ }

// This is called in the normal-form solution modules after the normal
// form is constructed.  Note especially that the Nfg is passed
// non-const, so that strategies may be eliminated as seen fit.
// It is assumed that the NFSupport returned is "sensible"

template <class T>
void SubgameSolver<T>::ViewNormal(const Nfg<T> &, NFSupport *&)
{ }
// This is called for each subgame after the solutions have been computed
// The idea is for the called code to possibly allow for viewing or
// selection of "interesting" equilibria for further computation during
// the process.  Again, there is no restriction that one can't
// muck about with the solution list in "bad" ways using this.
// Caveat utor!

template <class T>
void SubgameSolver<T>::SelectSolutions(int, const Efg<T> &,
				       gList<BehavSolution<T> > &)
{ }

template <class T> SubgameSolver<T>::SubgameSolver(const Efg<T> &E, int max)
  : max_solns(max), efg(E), solution(E), infosets(E.NumPlayers())
{
  for (int i = 1; i <= efg.NumPlayers(); i++)
    infosets[i] = new gArray<Infoset *>(efg.PlayerList()[i]->InfosetList());
}

template <class T> SubgameSolver<T>::~SubgameSolver()  
{
  for (int i = 1; i <= efg.NumPlayers(); i++)
    delete infosets[i];
}

template <class T> 
void SubgameSolver<T>::Solve(void)
{
  gWatch watch;

  solutions.Flush();
  subgame_number = 0;

  gList<Outcome *> values;

  ((gVector<T> &) solution).operator=((T) 0);

  FindSubgames(efg.RootNode(), solutions, values);

  time = watch.Elapsed();
}


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgx
class gArray<unsigned char>;

class gNode<gArray<int> >;
class gList<gArray<int> >;
class gListIter<gArray<int> >;

bool operator==(const gArray<Outcome *> &a, const gArray<Outcome *> &b)
{
	if (a.mindex != b.mindex || a.maxdex != b.maxdex)   return false;
	for (int i = a.mindex; i <= a.maxdex; i++)
		if (a[i] != b[i])   return false;
	return true;
}

bool operator!=(const gArray<Outcome *> &a, const gArray<Outcome *> &b)
{
	return !(a == b);
}

bool operator==(const gArray<int> &a, const gArray<int> &b)
{
	if (a.mindex != b.mindex || a.maxdex != b.maxdex)   return false;
	for (int i = a.mindex; i <= a.maxdex; i++)
		if (a[i] != b[i])   return false;
	return true;
}

bool operator!=(const gArray<int> &a, const gArray<int> &b)
{return !(a == b);}

#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__


TEMPLATE class SubgameSolver<double>;
TEMPLATE class SubgameSolver<gRational>;


#include "garray.imp"

TEMPLATE class gArray<gArray<Infoset *> *>;

TEMPLATE bool operator==(const gArray<Outcome *> &, const gArray<Outcome *> &);
TEMPLATE bool operator!=(const gArray<Outcome *> &, const gArray<Outcome *> &);

TEMPLATE gOutput &operator<<(gOutput &, const gArray<Outcome *> &);


#include "glist.imp"

TEMPLATE class gList<gVector<double> >;
TEMPLATE class gNode<gVector<double> >;
TEMPLATE class gList<gVector<gRational> >;
TEMPLATE class gNode<gVector<gRational> >;

TEMPLATE class gList<Outcome *>;
TEMPLATE class gNode<Outcome *>;

TEMPLATE class gList<gArray<Outcome *> >;
TEMPLATE class gNode<gArray<Outcome *> >;

