//#
//# FILE: subsolve.cc -- Implementation of solve-by-subgame algorithms
//#
//# $Id$
//#

#include "efg.h"
#include "efgutils.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE

bool operator==(const gArray<int> &a, const gArray<int> &b)
{
	if (a.mindex != b.mindex || a.maxdex != b.maxdex)   return false;
	for (int i = a.mindex; i <= a.maxdex; i++)
		if (a[i] != b[i])   return false;
	return true;
}

bool operator!=(const gArray<int> &a, const gArray<int> &b)
{return !(a == b);}

#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__
#include "glist.imp"

TEMPLATE class gList<gVector<double> >;
TEMPLATE class gNode<gVector<double> >;
TEMPLATE class gList<gVector<gRational> >;
TEMPLATE class gNode<gVector<gRational> >;

TEMPLATE class gList<Outcome *>;
TEMPLATE class gNode<Outcome *>;

TEMPLATE class gList<gArray<Outcome *> >;
TEMPLATE class gNode<gArray<Outcome *> >;

#pragma option -Jgx

#include "nfg.h"
#include "nfstrat.h"
#include "subsolve.h"

#include "gwatch.h"

//-------------------------------------------------------------------------
//                Implementation of base solver algorithm
//-------------------------------------------------------------------------

template <class T>
void SubgameSolver<T>::FindSubgames(Node *n, gList<BehavSolution<T> > &solns,
				    gList<Outcome *> &values)
{
  int i;

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
    SolveSubgame(foo, sol);

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

//-------------------------------------------------------------------------
//                      Hooks to specific algorithms
//-------------------------------------------------------------------------

//-------------------
// EFLiap
//-------------------

void EFLiapBySubgame::SolveSubgame(const Efg<double> &E,
				   gList<BehavSolution<double> > &solns)
{
  BehavProfile<double> bp(E);

  long this_nevals, this_niters;

  Liap(E, params, bp, solns, this_nevals, this_niters);

  nevals += this_nevals;
}

EFLiapBySubgame::EFLiapBySubgame(const Efg<double> &E, const EFLiapParams &p,
				 const BehavProfile<double> &s, int max)
  : SubgameSolver<double>(E, max), nevals(0), params(p), start(s)
{ }

EFLiapBySubgame::~EFLiapBySubgame()   { }


//-------------------
// Sequence form
//-------------------

template <class T>
void SeqFormBySubgame<T>::SolveSubgame(const Efg<T> &E,
				       gList<BehavSolution<T> > &solns)
{
  BehavProfile<T> bp(E);

  SeqFormModule<T> M(E, params, bp.GetEFSupport());
  
  M.Lemke();

  npivots += M.NumPivots();

  solns = M.GetSolutions();
}

template <class T>
SeqFormBySubgame<T>::SeqFormBySubgame(const Efg<T> &E, const SeqFormParams &p,
				      int max)
  : SubgameSolver<T>(E, max), npivots(0), params(p)
{ }

template <class T>  SeqFormBySubgame<T>::~SeqFormBySubgame()   { }
  

#include "mixed.h"

//-------------------
// NFLiap
//-------------------

void NFLiapBySubgame::SolveSubgame(const Efg<double> &E,
				   gList<BehavSolution<double> > &solns)
{
  Nfg<double> *N = MakeReducedNfg((Efg<double> &) E);

  NFSupport *S=new NFSupport(*N);

  ViewNormal(*N, S);

  MixedProfile<double> mp(*N, *S);
  
  long this_nevals, this_niters;

  gList<MixedSolution<double> > subsolns;
  Liap(*N, params, mp, subsolns, this_nevals, this_niters);

  nevals += this_nevals;

  for (int i = 1; i <= subsolns.Length(); i++)  {
    BehavProfile<double> bp(E);
    MixedToBehav(*N, subsolns[i], E, bp);
    solns.Append(bp);
  }

  delete N;
}

NFLiapBySubgame::NFLiapBySubgame(const Efg<double> &E, const NFLiapParams &p,
				 const BehavProfile<double> &s, int max)
  : SubgameSolver<double>(E, max), nevals(0), params(p), start(s)
{ }

NFLiapBySubgame::~NFLiapBySubgame()   { }


//-------------------
// Lemke-Howson
//-------------------

template <class T>
void LemkeBySubgame<T>::SolveSubgame(const Efg<T> &E, 
				     gList<BehavSolution<T> > &solns)
{
  Nfg<T> *N = MakeReducedNfg((Efg<T> &) E);

  NFSupport *S=new NFSupport(*N);

  ViewNormal(*N, S);

  	MixedProfile<T> mp(*N, *S);

  LemkeModule<T> M(*N, params, mp.GetNFSupport());
  
  M.Lemke();

  npivots += M.NumPivots();

  for (int i = 1; i <= M.GetSolutions().Length(); i++)  {
    BehavProfile<T> bp(E);
    MixedToBehav(*N, M.GetSolutions()[i], E, bp);
		solns.Append(bp);
  }

  delete N;
}

template <class T>
LemkeBySubgame<T>::LemkeBySubgame(const Efg<T> &E, const LemkeParams &p,
				  int max)
  : SubgameSolver<T>(E, max), npivots(0), params(p)
{ }

template <class T> LemkeBySubgame<T>::~LemkeBySubgame()   { }


//-------------------
// Simpdiv
//-------------------

template <class T>
void SimpdivBySubgame<T>::SolveSubgame(const Efg<T> &E,
				       gList<BehavSolution<T> > &solns)
{
  Nfg<T> *N = MakeReducedNfg((Efg<T> &) E);

  NFSupport *S=new NFSupport(*N);

  ViewNormal(*N, S);

  	MixedProfile<T> mp(*N, *S);

  SimpdivModule<T> M(*N, params, mp.GetNFSupport());
  
  M.Simpdiv();

  nevals += M.NumEvals();

  for (int i = 1; i <= M.GetSolutions().Length(); i++)  {
    BehavProfile<T> bp(E);
    MixedToBehav(*N, M.GetSolutions()[i], E, bp);
    solns.Append(bp);
  }

  delete N;
}

template <class T>
SimpdivBySubgame<T>::SimpdivBySubgame(const Efg<T> &E, const SimpdivParams &p,
				      int max)
  : SubgameSolver<T>(E, max), params(p)
{ }

template <class T> SimpdivBySubgame<T>::~SimpdivBySubgame()   { }


//-------------------
// Enum
//-------------------

template <class T>
void EnumBySubgame<T>::SolveSubgame(const Efg<T> &E,
				    gList<BehavSolution<T> > &solns)
{
  Nfg<T> *N = MakeReducedNfg((Efg<T> &) E);

	NFSupport *S=new NFSupport(*N);

	ViewNormal(*N, S);

	MixedProfile<T> mp(*N, *S);

	EnumModule<T> M(*N, params, mp.GetNFSupport());

	M.Enum();

	npivots += M.NumPivots();

	for (int i = 1; i <= M.GetSolutions().Length(); i++)  {
		BehavProfile<T> bp(E);
		MixedToBehav(*N, M.GetSolutions()[i], E, bp);
		solns.Append(bp);
	}

	delete N;
}

template <class T>
EnumBySubgame<T>::EnumBySubgame(const Efg<T> &E, const EnumParams &p, int max)
  : SubgameSolver<T>(E, max), npivots(0), params(p)
{ }

template <class T> EnumBySubgame<T>::~EnumBySubgame()   { }


//-------------------
// PureNash
//-------------------

template <class T>
void PureNashBySubgame<T>::SolveSubgame(const Efg<T> &E,
					gList<BehavSolution<T> > &solns)
{
  Nfg<T> *N = MakeReducedNfg((Efg<T> &) E);

  NFSupport *S=new NFSupport(*N);

  ViewNormal(*N, S);

  gList<MixedSolution<T> > sol;
  FindPureNash(*N, sol);

  for (int i = 1; i <= sol.Length(); i++)  {
		BehavProfile<T> bp(E);
    MixedToBehav(*N, sol[i], E, bp);
    solns.Append(bp);
  }

  delete N;
}

template <class T>
PureNashBySubgame<T>::PureNashBySubgame(const Efg<T> &E, int max)
  : SubgameSolver<T>(E, max)
{ }

template <class T> PureNashBySubgame<T>::~PureNashBySubgame()   { }


//-------------------
// ZSum
//-------------------

template <class T>
void ZSumBySubgame<T>::SolveSubgame(const Efg<T> &E,
				    gList<BehavSolution<T> > &solns)
{
  Nfg<T> *N = MakeReducedNfg((Efg<T> &) E);

	NFSupport *S=new NFSupport(*N);

	ViewNormal(*N, S);

		MixedProfile<T> mp(*N, *S);

	ZSumModule<T> M(*N, params, mp.GetNFSupport());

	M.ZSum();

  npivots += M.NumPivots();

	gList<MixedSolution<T> > sol;
  M.GetSolutions(sol);

  for (int i = 1; i <= sol.Length(); i++)  {
    BehavProfile<T> bp(E);
    MixedToBehav(*N, sol[i], E, bp);
    solns.Append(bp);
  }

  delete N;
}

template <class T>
ZSumBySubgame<T>::ZSumBySubgame(const Efg<T> &E, const ZSumParams &p,
				int max)
  : SubgameSolver<T>(E, max), npivots(0), params(p)
{ }

template <class T> ZSumBySubgame<T>::~ZSumBySubgame()   { }


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
class gArray<unsigned char>;
class gArray<BFS<double> >;
class gArray<BFS<gRational> >;
class gList<LUupdate<double> >;
class gList<LUupdate<gRational> >;
class gList<BehavProfile<double> >;
class gList<BehavProfile<gRational> >;
class gListIter<BehavProfile<double> >;
class gListIter<BehavProfile<gRational> >;
class gNode<BehavProfile<double> >;
class gNode<BehavProfile<gRational> >;
class gListIter<MixedProfile<double> >;
class gListIter<MixedProfile<gRational> >;
class gNode<MixedProfile<double> >;
class gNode<MixedProfile<gRational> >;

class gNode<gArray<int> >;
class gList<gArray<int> >;
class gListIter<gArray<int> >;

class gNode<BFS<double> >;
class gNode<BFS<gRational> >;
class gList<BFS<double> >;
class gList<BFS<gRational> >;
class gListIter<BFS<double> >;
class gListIter<BFS<gRational> >;


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

gOutput &operator<<(gOutput &, const gArray<int> &);
gOutput &operator<<(gOutput &, const gArray<double> &);
gOutput &operator<<(gOutput &, const gArray<gRational> &);
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "rational.h"

TEMPLATE class SubgameSolver<double>;
TEMPLATE class SubgameSolver<gRational>;

TEMPLATE class SeqFormBySubgame<double>;
TEMPLATE class SeqFormBySubgame<gRational>;

TEMPLATE class LemkeBySubgame<double>;
TEMPLATE class LemkeBySubgame<gRational>;

TEMPLATE class SimpdivBySubgame<double>;
TEMPLATE class SimpdivBySubgame<gRational>;

TEMPLATE class EnumBySubgame<double>;
TEMPLATE class EnumBySubgame<gRational>;

TEMPLATE class PureNashBySubgame<double>;
TEMPLATE class PureNashBySubgame<gRational>;

TEMPLATE class ZSumBySubgame<double>;
TEMPLATE class ZSumBySubgame<gRational>;

#include "garray.imp"

TEMPLATE class gArray<gArray<Infoset *> *>;

TEMPLATE bool operator==(const gArray<Outcome *> &, const gArray<Outcome *> &);
TEMPLATE bool operator!=(const gArray<Outcome *> &, const gArray<Outcome *> &);

TEMPLATE gOutput &operator<<(gOutput &, const gArray<Outcome *> &);



