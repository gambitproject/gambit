//#
//# FILE: subgame.cc -- Implementation of solve-by-subgame algorithms
//#
//# $Id$
//#  

#include "efg.h"
#include "efgutils.h"
#include "subgame.h"

void BaseEfg::MarkTree(Node *n, Node *base)
{
  n->ptr = base;
  for (int i = 1; i <= n->NumChildren(); i++)
    MarkTree(n->GetChild(i), base);
}

bool BaseEfg::CheckTree(Node *n, Node *base)
{
  int i;

  if (n->NumChildren() == 0)   return true;

  for (i = 1; i <= n->NumChildren(); i++)
    if (!CheckTree(n->GetChild(i), base))  return false;

  if (n->GetPlayer()->IsChance())   return true;

  for (i = 1; i <= n->GetInfoset()->NumMembers(); i++)
    if (n->GetInfoset()->GetMember(i)->ptr != base)
      return false;

  return true;
}

bool BaseEfg::Decompose(Node *n)
{
  if (n->NumChildren() == 0 || n->GetPlayer()->IsChance())  
    return false;

  MarkTree(n, n);
  return CheckTree(n, n);
}

template <class T>
void SubgameSolver<T>::FindSubgames(Node *n)
{
  int i;

  for (i = 1; i <= n->NumChildren(); i++)
    FindSubgames(n->GetChild(i));
  
  if (!n->GetParent() || efg.Decompose(n))  {
    Efg<double> foo(efg, n);

    BehavProfile<double> bp(foo);
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
	  solution(pl, index, act) = bp(pl, iset, act);
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

#include "eliap.h"

template <class T>
void SubgameSolver<T>::SolveSubgame(const Efg<T> &E, BehavProfile<T> &bp)
{
  EFLiapParams<double> EP;

  EFLiapModule<double> EM(E, EP, bp);
  
  EM.Liap();

  if (EM.GetSolutions().Length() > 0)
    bp = EM.GetSolutions()[1];
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
const BehavProfile<T> &SubgameSolver<T>::Solve(void)
{
  FindSubgames(efg.RootNode());

  return solution;
}

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__


TEMPLATE class SubgameSolver<double>;

#include "garray.imp"

TEMPLATE class gArray<gArray<Infoset *> *>;

