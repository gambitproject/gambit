//#
//# FILE: subsolve.cc -- Implementation of solve-by-subgame algorithms
//#
//# $Id$
//#  

#include "efg.h"
#include "efgutils.h"
#include "subsolve.h"

#include "gwatch.h"

//-------------------------------------------------------------------------
//                Implementation of base solver algorithm
//-------------------------------------------------------------------------

template <class T>
void SubgameSolver<T>::FindSubgames(Node *n, gList<BehavProfile<T> > &solns)
{
  int i;
  
  solns.Append(solution);
  ((gVector<T> &) solns[1]).operator=((T) 0);

  for (i = 1; i <= n->NumChildren(); i++)  {
    gList<BehavProfile<T> > subsolns;
    FindSubgames(n->GetChild(i), subsolns);

    if (subsolns.Length() == 0)  {
      solns.Flush();
      return;
    }

    solns[1] += subsolns[1];
  }
  
  if (n->GetSubgameRoot() == n)  {
    Efg<T> foo(efg, n);

    gList<BehavProfile<T> > sol;
    SolveSubgame(foo, sol);

    // put behav profile in "total" solution here...

    if (sol.Length() == 0)  {
      solns.Flush();
      return;
    }

    gList<Node *> nodes;
    Nodes(efg, n, nodes);
        
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
	  solns[1](pl, index, act) = sol[1](pl, iset, act);
      }
    }
 
    OutcomeVector<T> *outc = (n->GetOutcome()) ?
        ((OutcomeVector<T> *) n->GetOutcome()) : efg.NewOutcome();

    for (i = 1; i <= foo.NumPlayers(); i++)
      (*outc)[i] += sol[1].Payoff(i);

    efg.DeleteTree(n);
    n->SetOutcome(outc);
  }    
}

template <class T> SubgameSolver<T>::SubgameSolver(const Efg<T> &E)
  : efg(E), solution(E), infosets(E.NumPlayers())
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

  ((gVector<T> &) solution).operator=((T) 0);

  FindSubgames(efg.RootNode(), solutions);

  time = watch.Elapsed();
}

//-------------------------------------------------------------------------
//                      Hooks to specific algorithms
//-------------------------------------------------------------------------

//-------------------
// EFLiap
//-------------------

template <class T>
void EFLiapBySubgame<T>::SolveSubgame(const Efg<T> &E,
				      gList<BehavProfile<T> > &solns)
{
  EFLiapParams EP;

  BehavProfile<T> bp(E);

  EFLiapModule<double> EM(E, EP, bp);
  
  EM.Liap();

  nevals += EM.NumEvals();

  solns = EM.GetSolutions();
}

template <class T>
EFLiapBySubgame<T>::EFLiapBySubgame(const Efg<T> &E, const EFLiapParams &p,
				    const BehavProfile<T> &s)
  : SubgameSolver<T>(E), nevals(0), params(p), start(s)
{ }

template <class T>  EFLiapBySubgame<T>::~EFLiapBySubgame()   { }


//-------------------
// Sequence form
//-------------------

template <class T>
void SeqFormBySubgame<T>::SolveSubgame(const Efg<T> &E,
				       gList<BehavProfile<T> > &solns)
{
  BehavProfile<T> bp(E);

  SeqFormModule<T> M(E, params, bp.GetEFSupport());
  
  M.Lemke();

  npivots += M.NumPivots();

  solns = M.GetSolutions();
}

template <class T>
SeqFormBySubgame<T>::SeqFormBySubgame(const Efg<T> &E, const SeqFormParams &p)
  : SubgameSolver<T>(E), npivots(0), params(p)
{ }

template <class T>  SeqFormBySubgame<T>::~SeqFormBySubgame()   { }
  

#include "mixed.h"

//-------------------
// NFLiap
//-------------------

template <class T>
void NFLiapBySubgame<T>::SolveSubgame(const Efg<T> &E,
				      gList<BehavProfile<T> > &solns)
{
  Nfg<T> *N = MakeReducedNfg((Efg<T> &) E);

  MixedProfile<T> mp(*N);

  NFLiapModule<T> M(*N, params, mp);
  
  M.Liap();

  nevals += M.NumEvals();

  for (int i = 1; i <= M.GetSolutions().Length(); i++)  {
    BehavProfile<T> bp(E);
    MixedToBehav(*N, M.GetSolutions()[i], E, bp);
    solns.Append(bp);
  }

  delete N;
}

template <class T>
NFLiapBySubgame<T>::NFLiapBySubgame(const Efg<T> &E, const NFLiapParams &p,
				    const BehavProfile<T> &s)

  : SubgameSolver<T>(E), nevals(0), params(p), start(s)
{ }

template <class T> NFLiapBySubgame<T>::~NFLiapBySubgame()   { }


//-------------------
// Lemke-Howson
//-------------------

template <class T>
void LemkeBySubgame<T>::SolveSubgame(const Efg<T> &E, 
				     gList<BehavProfile<T> > &solns)
{
  Nfg<T> *N = MakeReducedNfg((Efg<T> &) E);

  MixedProfile<T> mp(*N);

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
LemkeBySubgame<T>::LemkeBySubgame(const Efg<T> &E, const LemkeParams &p)
  : SubgameSolver<T>(E), npivots(0), params(p)
{ }

template <class T> LemkeBySubgame<T>::~LemkeBySubgame()   { }


//-------------------
// Simpdiv
//-------------------

template <class T>
void SimpdivBySubgame<T>::SolveSubgame(const Efg<T> &E,
				       gList<BehavProfile<T> > &solns)
{
  Nfg<T> *N = MakeReducedNfg((Efg<T> &) E);

  MixedProfile<T> mp(*N);

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
SimpdivBySubgame<T>::SimpdivBySubgame(const Efg<T> &E, const SimpdivParams &p)
  : SubgameSolver<T>(E), params(p)
{ }

template <class T> SimpdivBySubgame<T>::~SimpdivBySubgame()   { }


//-------------------
// Enum
//-------------------

template <class T>
void EnumBySubgame<T>::SolveSubgame(const Efg<T> &E,
				    gList<BehavProfile<T> > &solns)
{
  Nfg<T> *N = MakeReducedNfg((Efg<T> &) E);

  MixedProfile<T> mp(*N);

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
EnumBySubgame<T>::EnumBySubgame(const Efg<T> &E, const EnumParams &p)
  : SubgameSolver<T>(E), npivots(0),params(p)
{ }

template <class T> EnumBySubgame<T>::~EnumBySubgame()   { }


//-------------------
// PureNash
//-------------------

template <class T>
void PureNashBySubgame<T>::SolveSubgame(const Efg<T> &E,
					gList<BehavProfile<T> > &solns)
{
  Nfg<T> *N = MakeReducedNfg((Efg<T> &) E);

  gList<MixedProfile<T> > sol;
  FindPureNash(*N, sol);

  for (int i = 1; i <= sol.Length(); i++)  {
    BehavProfile<T> bp(E);
    MixedToBehav(*N, sol[i], E, bp);
    solns.Append(bp);
  }

  delete N;
}

template <class T>
PureNashBySubgame<T>::PureNashBySubgame(const Efg<T> &E)
  : SubgameSolver<T>(E)
{ }

template <class T> PureNashBySubgame<T>::~PureNashBySubgame()   { }


//-------------------
// ZSum
//-------------------

template <class T>
void ZSumBySubgame<T>::SolveSubgame(const Efg<T> &E,
				    gList<BehavProfile<T> > &solns)
{
  Nfg<T> *N = MakeReducedNfg((Efg<T> &) E);

  MixedProfile<T> mp(*N);

  ZSumModule<T> M(*N, params, mp.GetNFSupport());
  
  M.ZSum();

  npivots += M.NumPivots();

  gList<MixedProfile<T> > sol;
  M.GetSolutions(sol);

  for (int i = 1; i <= sol.Length(); i++)  {
    BehavProfile<T> bp(E);
    MixedToBehav(*N, sol[i], E, bp);
    solns.Append(bp);
  }

  delete N;
}

template <class T>
ZSumBySubgame<T>::ZSumBySubgame(const Efg<T> &E, const ZSumParams &p)
  : SubgameSolver<T>(E), npivots(0), params(p)
{ }

template <class T> ZSumBySubgame<T>::~ZSumBySubgame()   { }


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
class gArray<unsigned char>;
class gArray<BFS<double> >;
class gArray<BFS<gRational> >;
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "rational.h"

TEMPLATE class SubgameSolver<double>;
TEMPLATE class SubgameSolver<gRational>;

TEMPLATE class EFLiapBySubgame<double>;

TEMPLATE class SeqFormBySubgame<double>;
TEMPLATE class SeqFormBySubgame<gRational>;

TEMPLATE class NFLiapBySubgame<double>;

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





