//#
//# FILE: efgnfg.cc -- efg<->nfg conversion routines
//#
//# $Id$
//#

#include "efg.h"
#include "nfg.h"
#include "nfplayer.h"
#include "nfstrat.h"
#include "mixed.h"
#include "glist.h"
#include "contiter.h"
#include "nfgiter.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__
typedef gArray<int> Correspondence;
#include "glist.imp"
#include "garray.imp"

TEMPLATE class gList<Correspondence *>;
TEMPLATE class gNode<Correspondence *>;
TEMPLATE class gListIter<Correspondence *>;
TEMPLATE gOutput &operator<<(gOutput &, const gList<Correspondence *> &);
TEMPLATE class gArray<gList<Correspondence *> >;
TEMPLATE class gArray<gListIter<Correspondence *> *>;

class Lexicon   {
  public:
    BaseNfg *N;
    gArray<gList<Correspondence *> > strategies;

    Lexicon(const BaseEfg &);
    ~Lexicon();

    void MakeStrategy(EFPlayer *p);
    void MakeReducedStrats(EFPlayer *, Node *, Node *);
};

Lexicon::Lexicon(const BaseEfg &E)
  : N(0), strategies(E.NumPlayers())
{ }

Lexicon::~Lexicon()
{
  for (int i = 1; i <= strategies.Length(); i++)
    while (strategies[i].Length())  delete strategies[i].Remove(1);
}

void Lexicon::MakeStrategy(EFPlayer *p)
{
  Correspondence *c = new Correspondence(p->NumInfosets());
  
  for (int i = 1; i <= p->NumInfosets(); i++)  {
    if (p->InfosetList()[i]->flag == 1)
      (*c)[i] = p->InfosetList()[i]->whichbranch;
    else
      (*c)[i] = 0;
  }
  strategies[p->GetNumber()].Append(c);
}

void Lexicon::MakeReducedStrats(EFPlayer *p, Node *n, Node *nn)
{
  int i;
  Node *m, *mm;

  if (!n->parent)  n->ptr = 0;

  if (n->NumChildren() > 0)  {
    if (n->infoset->player == p)  {
      if (n->infoset->flag == 0)  {
	// we haven't visited this infoset before
	n->infoset->flag = 1;
	for (i = 1; i <= n->NumChildren(); i++)   {
	  Node *m = n->GetChild(i);
	  n->whichbranch = m;
	  n->infoset->whichbranch = i;
	  MakeReducedStrats(p, m, nn);
	}
	n->infoset->flag = 0;
      }
      else  {
	// we have visited this infoset, take same action
	MakeReducedStrats(p, n->children[n->infoset->whichbranch], nn);
      }
    }
    else  {
      n->ptr = NULL;
      if (nn != NULL)
	n->ptr = nn->parent;
      n->whichbranch = n->children[1];
      if (n->infoset)
	n->infoset->whichbranch = 0;
      MakeReducedStrats(p, n->children[1], n->children[1]);
    }
  }
  else if (nn)  {
    for (m = NULL; ; nn = nn->parent->ptr->whichbranch)  {
      m = nn->NextSibling();
      if (m || nn->parent->ptr == NULL)   break;
    }
    if (m)  {
      mm = m->parent->whichbranch;
      m->parent->whichbranch = m;
      MakeReducedStrats(p, m, m);
      m->parent->whichbranch = mm;
    }
    else 
      MakeStrategy(p);
  }
  else
    MakeStrategy(p);
}


#include "tnode.h"

template <class T> Nfg<T> *MakeReducedNfg(Efg<T> &E)
{
  int i;

  Lexicon *L = new Lexicon(E);
  for (i = 1; i <= E.NumPlayers(); i++)  {
    L->MakeReducedStrats(E.PlayerList()[i], E.RootNode(), NULL);
  }
  gArray<int> dim(E.NumPlayers());
  for (i = 1; i <= E.NumPlayers(); i++)
    dim[i] = L->strategies[i].Length();

  L->N = new Nfg<T>(dim);
  L->N->SetTitle(E.GetTitle());
  
  for (i = 1; i <= E.NumPlayers(); i++)   {
    L->N->PlayerList()[i]->SetName(E.PlayerList()[i]->GetName());
    for (int j = 1; j <= L->strategies[i].Length(); j++)   {
      gString name;
      for (int k = 1; k <= L->strategies[i][j]->Length(); k++)
	if ((*L->strategies[i][j])[k] > 0)
	  name += ToString((*L->strategies[i][j])[k]);
        else
	  name += "*";
      L->N->PlayerList()[i]->StrategyList()[j]->name = name;
    }
  }
  
  NFSupport S(*L->N);
  ContIter<T> iter(&S);
  gArray<gArray<int> *> corr(E.NumPlayers());
  gArray<gListIter<Correspondence *> *> corrs(E.NumPlayers());
  for (i = 1; i <= E.NumPlayers(); i++)  {
    corrs[i] = new gListIter<Correspondence *>(L->strategies[i]);
    corr[i] = corrs[i]->GetValue();
  }

  gVector<T> value(E.NumPlayers());

  int pl = E.NumPlayers();
  while (1)  {
    E.Payoff(corr, value);
    for (int j = 1; j <= E.NumPlayers(); j++)
      iter.SetPayoff(j, value[j]);
    
    iter.NextContingency();
    while (pl > 0)   {
      (*corrs[pl])++;
      if (!corrs[pl]->PastEnd())  {
	corr[pl] = corrs[pl]->GetValue();
	break;
      }
      corrs[pl]->GoFirst();
      corr[pl] = corrs[pl]->GetValue();
      pl--;
    }

    if (pl == 0)  break;
    pl = E.NumPlayers();
  }

  for (i = 1; i <= E.NumPlayers(); i++)
    delete corrs[i];

  E.lexicon = L;
  return ((Nfg<T> *) E.lexicon->N);
}

template <class T>
void RealizationProbs(const Nfg<T> &N, const MixedProfile<T> &mp,
		      const Efg<T> &E, BehavProfile<T> &bp,
		      int pl, const gArray<int> *const actions, Node *n)
{
  static const T tremble = (T) 0.0;
  T prob;

  for (int i = 1; i <= n->NumChildren(); i++)   {
    if (n->GetPlayer() && !n->GetPlayer()->IsChance())   {
      if (n->GetPlayer()->GetNumber() == pl)  {
	if ((*actions)[n->GetInfoset()->GetNumber()] == i)
	  prob = (T) 1.0 - tremble + tremble / (T) n->NumChildren();
	else
	  prob = tremble / (T) n->NumChildren();
      }
      else
	prob = (T) 1.0 / (T) n->NumChildren();
    }
    else  {   // n.GetPlayer() == 0
      prob = (T) ((ChanceInfoset<T> *) n->GetInfoset())->GetActionProb(i);
    }

    Node *child = n->GetChild(i);
    ((TypedNode<T> *) child)->bval = prob * ((TypedNode<T> *) n)->bval;
    ((TypedNode<T> *) child)->nval += ((TypedNode<T> *) child)->bval;    

    RealizationProbs(N, mp, E, bp, pl, actions, child);
  }    
	
}

template <class T>
void BehaviorStrat(const Efg<T> &E, BehavProfile<T> &bp, int pl, Node *n)
{
  for (int i = 1; i <= n->NumChildren(); i++)   {
    Node *child = n->GetChild(i);
    if (n->GetPlayer() && n->GetPlayer()->GetNumber() == pl)
      if (((TypedNode<T> *) n)->nval > (T) 0.0)  {
	bp(n->GetPlayer()->GetNumber(), n->GetInfoset()->GetNumber(), i) =
	  ((TypedNode<T> *) child)->nval / ((TypedNode<T> *) n)->nval;
      }
    BehaviorStrat(E, bp, pl, child);
  }
}

template <class T> void ClearNodeProbs(TypedNode<T> *n)
{
  n->nval = 0.0;
  for (int i = 1; i <= n->NumChildren(); i++)
    ClearNodeProbs(((TypedNode<T> *) n->GetChild(i)));
}

void BaseEfg::DeleteLexicon(void)
{
  if (lexicon)   delete lexicon;
  lexicon = 0;
}

template <class T>
void MixedToBehav(const Nfg<T> &N, const MixedProfile<T> &mp,
		  const Efg<T> &E, BehavProfile<T> &bp)
{
  if (!E.lexicon || E.lexicon->N !=(BaseEfg *) &N)   return;

  Node *n = E.RootNode();

  for (int pl = 1; pl <= N.NumPlayers(); pl++)   {
    ClearNodeProbs((TypedNode<T> *) n);

    NFStrategySet *S = mp.GetNFSupport().GetNFStrategySet(pl);

    for (int st = 1; st <= S->NumStrats(); st++)  {
      int snum = S->GetStrategy(st)->number;
      if (mp(pl, st) > (T) 0.0)  {
	const gArray<int> *const actions = E.lexicon->strategies[pl][snum];

	((TypedNode<T> *) n)->bval = mp(pl, st);

	RealizationProbs(N, mp, E, bp, pl, actions, E.RootNode());
      }
    }
    
    ((TypedNode<T> *) E.RootNode())->nval = (T) 1.0;
    BehaviorStrat(E, bp, pl, n);
  }
}


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "rational.h"

TEMPLATE void ClearNodeProbs(TypedNode<double> *);
TEMPLATE void ClearNodeProbs(TypedNode<gRational> *);

TEMPLATE void MixedToBehav(const Nfg<double> &,const MixedProfile<double> &,
			   const Efg<double> &, BehavProfile<double> &);
TEMPLATE void MixedToBehav(const Nfg<gRational> &,
			   const MixedProfile<gRational> &,
			   const Efg<gRational> &, BehavProfile<gRational> &);

TEMPLATE void RealizationProbs(const Nfg<double> &N, const MixedProfile<double> &mp,
			       const Efg<double> &E, BehavProfile<double> &bp,
			       int pl, const gArray<int> *const actions, Node *);
TEMPLATE void RealizationProbs(const Nfg<gRational> &N, const MixedProfile<gRational> &mp,
			       const Efg<gRational> &E, BehavProfile<gRational> &bp,
			       int pl, const gArray<int> *const actions, Node *);


TEMPLATE void BehaviorStrat(const Efg<double> &E, BehavProfile<double> &bp, int pl, Node *n);
TEMPLATE void BehaviorStrat(const Efg<gRational> &E, BehavProfile<gRational> &bp, int pl, Node *n);



TEMPLATE Nfg<double> *MakeReducedNfg(Efg<double> &);
TEMPLATE Nfg<gRational> *MakeReducedNfg(Efg<gRational> &);

