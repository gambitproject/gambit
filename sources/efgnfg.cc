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



#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__
#include "glist.imp"
#include "garray.imp"
#include "lexicon.h"

TEMPLATE class gList<Correspondence *>;
TEMPLATE class gNode<Correspondence *>;
TEMPLATE gOutput &operator<<(gOutput &, const gList<Correspondence *> &);
TEMPLATE class gArray<gList<Correspondence *> >;


Lexicon::Lexicon(const BaseEfg &E)
  : N(0), strategies(E.NumPlayers())
{ }

Lexicon::~Lexicon()
{
  for (int i = 1; i <= strategies.Length(); i++)
    while (strategies[i].Length())  delete strategies[i].Remove(1);
  if (N)
    N->efg = 0;
}

void SetEfg(BaseNfg *nfg, BaseEfg *efg)
{
  nfg->efg = efg;
}

void Lexicon::MakeLink(BaseEfg *efg, BaseNfg *nfg)
{
  nfg->efg = efg;
  N = nfg;
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
	  if (S.Find(n->infoset->GetActionList()[i]))  {
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
    for (m = NULL; ; nn = nn->parent->ptr->whichbranch)  {
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


void BaseEfg::DeleteLexicon(void)
{
  if (lexicon)   delete lexicon;
  lexicon = 0;
}

BaseNfg *AssociatedNfg(BaseEfg *E)
{
  if (E->lexicon)
    return E->lexicon->N;
  else
    return 0;
}

BaseNfg *AssociatedAfg(BaseEfg *E)
{
  return E->afg;
}




