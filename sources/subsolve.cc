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
void SubgameSolver<T>::FindSubgames(Node *n, BehavProfile<T> &soln)
{
  int i;

  for (i = 1; i <= n->NumChildren(); i++)  {
    BehavProfile<T> subsoln(solution);
    ((gVector<T> &) subsoln).operator=((T) 0);
    FindSubgames(n->GetChild(i), subsoln);

    soln += subsoln;
  }
  
  if (n->GetSubgameRoot() == n)  {
    Efg<T> foo(efg, n);

    BehavProfile<T> bp(foo);
    SolveSubgame(foo, bp);

    // put behav profile in "total" solution here...

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
	  soln(pl, index, act) = bp(pl, iset, act);
      }
    }
 
    OutcomeVector<T> *outc = (n->GetOutcome()) ?
        ((OutcomeVector<T> *) n->GetOutcome()) : efg.NewOutcome();

    for (i = 1; i <= foo.NumPlayers(); i++)
      (*outc)[i] += bp.Payoff(i);

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

  FindSubgames(efg.RootNode(), solution);

  time = watch.Elapsed();

  solutions.Append(solution);
}

//-------------------------------------------------------------------------
//                      Hooks to specific algorithms
//-------------------------------------------------------------------------

//-------------------
// EFLiap
//-------------------

template <class T>
void EFLiapBySubgame<T>::SolveSubgame(const Efg<T> &E, BehavProfile<T> &bp)
{
	EFLiapParams EP;

	EFLiapModule<double> EM(E, EP, bp);
  
  EM.Liap();

  nevals += EM.NumEvals();

  if (EM.GetSolutions().Length() > 0)
    bp = EM.GetSolutions()[1];
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
void SeqFormBySubgame<T>::SolveSubgame(const Efg<T> &E, BehavProfile<T> &bp)
{
  SeqFormModule<T> M(E, params, bp.GetEFSupport());
  
  M.Lemke();

  npivots += M.NumPivots();

  if (M.GetSolutions().Length() > 0)
    bp = M.GetSolutions()[1];
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
void NFLiapBySubgame<T>::SolveSubgame(const Efg<T> &E, BehavProfile<T> &bp)
{
  Nfg<T> *N = MakeReducedNfg((Efg<T> &) E);

  MixedProfile<T> mp(*N);

  NFLiapModule<T> M(*N, params, mp);
  
  M.Liap();

  nevals += M.NumEvals();

  if (M.GetSolutions().Length() > 0)
    MixedToBehav(*N, M.GetSolutions()[1], E, bp);

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
void LemkeBySubgame<T>::SolveSubgame(const Efg<T> &E, BehavProfile<T> &bp)
{
  Nfg<T> *N = MakeReducedNfg((Efg<T> &) E);

  MixedProfile<T> mp(*N);

  LemkeModule<T> M(*N, params, mp.GetNFSupport());
  
  M.Lemke();

  npivots += M.NumPivots();

  if (M.GetSolutions().Length() > 0)
    MixedToBehav(*N, M.GetSolutions()[1], E, bp);

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
void SimpdivBySubgame<T>::SolveSubgame(const Efg<T> &E, BehavProfile<T> &bp)
{
  Nfg<T> *N = MakeReducedNfg((Efg<T> &) E);

  MixedProfile<T> mp(*N);

  SimpdivModule<T> M(*N, params, mp.GetNFSupport());
  
  M.Simpdiv();

  nevals += M.NumEvals();

  if (M.GetSolutions().Length() > 0)
    MixedToBehav(*N, M.GetSolutions()[1], E, bp);

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
void EnumBySubgame<T>::SolveSubgame(const Efg<T> &E, BehavProfile<T> &bp)
{
  Nfg<T> *N = MakeReducedNfg((Efg<T> &) E);

  MixedProfile<T> mp(*N);

  EnumModule<T> M(*N, params, mp.GetNFSupport());
  
  M.Enum();

  npivots += M.NumPivots();

  if (M.GetSolutions().Length() > 0)
    MixedToBehav(*N, M.GetSolutions()[1], E, bp);

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
void PureNashBySubgame<T>::SolveSubgame(const Efg<T> &E, BehavProfile<T> &bp)
{
  Nfg<T> *N = MakeReducedNfg((Efg<T> &) E);

  gList<MixedProfile<T> > solns;
  FindPureNash(*N, solns);

  if (solns.Length() > 0)
    MixedToBehav(*N, solns[1], E, bp);

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
void ZSumBySubgame<T>::SolveSubgame(const Efg<T> &E, BehavProfile<T> &bp)
{
  Nfg<T> *N = MakeReducedNfg((Efg<T> &) E);

  MixedProfile<T> mp(*N);

  ZSumModule<T> M(*N, params, mp.GetNFSupport());
  
  M.ZSum();

  npivots += M.NumPivots();

  gList<MixedProfile<T> > solns;
  M.GetSolutions(solns);

  if (solns.Length() > 0)
    MixedToBehav(*N, solns[1], E, bp);

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





