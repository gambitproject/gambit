//
// FILE: efgnfg.cc -- efg<->nfg conversion routines
//
// $Id$
//

#include "efg.h"
#include "nfg.h"
#include "nfplayer.h"
#include "nfstrat.h"
#include "mixed.h"
#include "glist.h"
#include "nfgiter.h"
#include "nfgciter.h"

#include "lexicon.h"

Lexicon::Lexicon(const Efg  &E)
  : N(0), strategies(E.NumPlayers())
{ }

Lexicon::~Lexicon()
{
  for (int i = 1; i <= strategies.Length(); i++)
    while (strategies[i].Length())  delete strategies[i].Remove(1);
  if (N)
    N->efg = 0;
}

void SetEfg(Nfg *nfg, const Efg *efg)
{
  nfg->efg = efg;
}

void Lexicon::MakeLink(const Efg *efg, Nfg *nfg)
{
  nfg->efg = efg;
  N = nfg;
}

void Lexicon::MakeStrategy(EFPlayer *p)
{
  Correspondence *c = new Correspondence(p->NumInfosets());
  
  for (int i = 1; i <= p->NumInfosets(); i++)  {
    if (p->Infosets()[i]->flag == 1)
      (*c)[i] = p->Infosets()[i]->whichbranch;
    else
      (*c)[i] = 0;
  }
  strategies[p->GetNumber()].Append(c);
}

void Lexicon::MakeReducedStrats(const EFSupport &S,
				EFPlayer *p, Node *n, Node *nn)
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
	  if (S.Find(n->infoset->Actions()[i]))  {
	    Node *m = n->GetChild(i);
	    n->whichbranch = m;
	    n->infoset->whichbranch = i;
	    MakeReducedStrats(S, p, m, nn);
	  }
	}
	n->infoset->flag = 0;
      }
      else  {
	// we have visited this infoset, take same action
	MakeReducedStrats(S, p, n->children[n->infoset->whichbranch], nn);
      }
    }
    else  {
      n->ptr = NULL;
      if (nn != NULL)
	n->ptr = nn->parent;
      n->whichbranch = n->children[1];
      if (n->infoset)
	n->infoset->whichbranch = 0;
      MakeReducedStrats(S, p, n->children[1], n->children[1]);
    }
  }
  else if (nn)  {
    for (; ; nn = nn->parent->ptr->whichbranch)  {
      m = nn->NextSibling();
      if (m || nn->parent->ptr == NULL)   break;
    }
    if (m)  {
      mm = m->parent->whichbranch;
      m->parent->whichbranch = m;
      MakeReducedStrats(S, p, m, m);
      m->parent->whichbranch = mm;
    }
    else
      MakeStrategy(p);
  }
  else
    MakeStrategy(p);
}

Nfg *MakeReducedNfg(const EFSupport &support)
{
  int i;
  const Efg &E = support.Game();
  Lexicon *L = new Lexicon(E);
  for (i = 1; i <= E.NumPlayers(); i++)
    L->MakeReducedStrats(support, E.Players()[i], E.RootNode(), NULL);

  gArray<int> dim(E.NumPlayers());
  for (i = 1; i <= E.NumPlayers(); i++)
    dim[i] = (L->strategies[i].Length()) ? L->strategies[i].Length() : 1;

  L->MakeLink(&E, new Nfg(dim));
  L->N->SetTitle(E.GetTitle());

  for (i = 1; i <= E.NumPlayers(); i++)   {
    L->N->Players()[i]->SetName(E.Players()[i]->GetName());
    for (int j = 1; j <= L->strategies[i].Length(); j++)   {
      gText name;
      for (int k = 1; k <= L->strategies[i][j]->Length(); k++)
	if ((*L->strategies[i][j])[k] > 0)
	  name += ToText((*L->strategies[i][j])[k]);
        else
	  name += "*";
      L->N->Players()[i]->Strategies()[j]->SetName(name);
    }
  }

  NFSupport S(*L->N);
  NfgContIter iter(S);
  gArray<gArray<int> *> corr(E.NumPlayers());
  gArray<int> corrs(E.NumPlayers());
  for (i = 1; i <= E.NumPlayers(); i++)  {
    corrs[i] = 1;
    if (L->N->NumStrats(i) > 0)
      corr[i] = L->strategies[i][1];
    else
      corr[i] = new gArray<int>(0);
  }

  gArray<gNumber> value(E.NumPlayers());

  int pl = E.NumPlayers();
  while (1)  {
    E.Payoff(corr, value);

    iter.SetOutcome(L->N->NewOutcome());
    for (int j = 1; j <= E.NumPlayers(); j++)
      L->N->SetPayoff(iter.GetOutcome(), j, value[j]);

    iter.NextContingency();
    while (pl > 0)   {
      corrs[pl]++;
      if (corrs[pl] <= L->strategies[pl].Length())  {
	corr[pl] = L->strategies[pl][corrs[pl]];
	break;
      }
      corrs[pl] = 1;
      corr[pl] = L->strategies[pl][1];
      pl--;
    }

    if (pl == 0)  break;
    pl = E.NumPlayers();
  }

  E.lexicon = L;
  SetEfg(E.lexicon->N, &E);
  return E.lexicon->N;
}

Nfg *MakeAfg(const Efg &E)
{
  Nfg *afg = new Nfg(gArray<int>(E.NumActions()));

  if (!afg)   return 0;

  E.afg = afg;
  afg->SetTitle(E.GetTitle() + " (Agent Form)");

  for (int epl = 1, npl = 1; epl <= E.NumPlayers(); epl++)   {
    for (int iset = 1; iset <= E.Players()[epl]->NumInfosets(); iset++, npl++)  {
      Infoset *s = E.Players()[epl]->Infosets()[iset];
      for (int act = 1; act <= s->NumActions(); act++)  {
	Strategy *st = afg->Strategies(npl)[act];
	st->SetName(ToText(act));
      }
    }
  }

  NfgIter iter(*afg);
  int pl = afg->NumPlayers();

  gArray<int> dim(E.NumPlayers());
  for (int i = 1; i <= dim.Length(); i++)
    dim[i] = E.Players()[i]->NumInfosets();
  gPVector<int> profile(dim);
  ((gVector<int> &) profile).operator=(1);

  gVector<gNumber> payoff(E.NumPlayers());
  
  while (1)  {
    E.Payoff(profile, payoff);

    if (iter.GetIndex() > 0)
      iter.SetOutcome(afg->NewOutcome());

    for (int epl = 1, npl = 1; epl <= E.NumPlayers(); epl++)
      for (int iset = 1; iset <= E.Players()[epl]->NumInfosets(); iset++, npl++)
	afg->SetPayoff(iter.GetOutcome(), npl, payoff[epl]);

    
    while (pl > 0)  {
      if (iter.Next(pl))  {
	profile[pl]++;
	break;
      }
      profile[pl] = 1;
      pl--;
    }

    if (pl == 0)  break;
    pl = afg->NumPlayers();
  }

  SetEfg(afg, &E);

  return afg;
}




#include "glist.imp"
#include "garray.imp"

template class gList<Correspondence *>;
template gOutput &operator<<(gOutput &, const gList<Correspondence *> &);
template class gArray<gList<Correspondence *> >;






