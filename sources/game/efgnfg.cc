//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Extensive form<->normal form conversions
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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
#include "efg.h"
#include "efstrat.h"
#include "nfg.h"
#include "nfstrat.h"
#include "mixed.h"
#include "nfgiter.h"
#include "nfgciter.h"

#include "lexicon.h"

// For access to internals
#include "efgint.h"
#include "nfgint.h"


Lexicon::Lexicon(const efgGame &E)
  : N(0), strategies(E.NumPlayers())
{ }

Lexicon::~Lexicon()
{
  for (int i = 1; i <= strategies.Length(); i++)
    while (strategies[i].Length())  delete strategies[i].Remove(1);
  if (N)
    N->rep->m_efg = 0;
}

void SetEfg(Nfg *nfg, efgGame *efg)
{
  nfg->rep->m_efg = efg;
}

void Lexicon::MakeLink(efgGame *efg, Nfg *nfg)
{
  nfg->rep->m_efg = efg;
  N = nfg;
}

void Lexicon::MakeStrategy(gbtEfgPlayer p)
{
  lexCorrespondence *c = new lexCorrespondence(p.NumInfosets());
  
  for (int i = 1; i <= p.NumInfosets(); i++)  {
    if (p.GetInfoset(i).GetFlag())
      (*c)[i] = p.GetInfoset(i).GetWhichBranch();
    else
      (*c)[i] = 0;
  }
  strategies[p.GetId()].Append(c);
}

void Lexicon::MakeReducedStrats(const EFSupport &S,
				gbtEfgPlayer p, gbtEfgNode n, gbtEfgNode nn)
{
  int i;
  gbtEfgNode m, mm;

  if (n.GetParent().IsNull())  n.rep->m_ptr = 0;

  if (n.NumChildren() > 0)  {
    if (n.GetInfoset().GetPlayer() == p)  {
      if (!n.GetInfoset().GetFlag())  {
	// we haven't visited this infoset before
	n.GetInfoset().SetFlag(true);
	for (i = 1; i <= n.NumChildren(); i++)   {
	  if (S.Contains(n.GetInfoset().GetAction(i)))  {
	    gbtEfgNode m = n.GetChild(i);
	    n.rep->m_whichbranch = m.rep;
	    n.GetInfoset().SetWhichBranch(i);
	    MakeReducedStrats(S, p, m, nn);
	  }
	}
	n.GetInfoset().SetFlag(false);
      }
      else  {
	// we have visited this infoset, take same action
	MakeReducedStrats(S, p, n.rep->m_children[n.GetInfoset().GetWhichBranch()], nn);
      }
    }
    else  {
      n.rep->m_ptr = NULL;
      if (nn != NULL)
	n.rep->m_ptr = nn.rep->m_parent;
      n.rep->m_whichbranch = n.rep->m_children[1];
      if (n.rep->m_infoset) 
	n.GetInfoset().SetWhichBranch(0);
      MakeReducedStrats(S, p, n.GetChild(1), n.GetChild(1));
    }
  }
  else if (!nn.IsNull())  {
    for (; ; nn = nn.rep->m_parent->m_ptr->m_whichbranch)  {
      m = nn.NextSibling();
      if (!m.IsNull() || nn.rep->m_parent->m_ptr == NULL)   break;
    }
    if (!m.IsNull())  {
      mm = m.rep->m_parent->m_whichbranch;
      m.rep->m_parent->m_whichbranch = m.rep;
      MakeReducedStrats(S, p, m, m);
      m.rep->m_parent->m_whichbranch = mm.rep;
    }
    else
      MakeStrategy(p);
  }
  else
    MakeStrategy(p);
}

//
// The following two functions have been placed in the
// efgGameNamespace namespace for now, to make the friend
// declarations in class efgGame work correctly.
//

Nfg *MakeReducedNfg(const EFSupport &support)
{
  int i;
  efgGame &E = support.GetGame();
  Lexicon *L = new Lexicon(E);
  for (i = 1; i <= E.NumPlayers(); i++) {
    L->MakeReducedStrats(support, E.GetPlayer(i), E.RootNode(), NULL);
  }

  gArray<int> dim(E.NumPlayers());
  for (i = 1; i <= E.NumPlayers(); i++)
    dim[i] = (L->strategies[i].Length()) ? L->strategies[i].Length() : 1;

  L->MakeLink(&E, new Nfg(dim));
  L->N->SetTitle(E.GetTitle());

  for (i = 1; i <= E.NumPlayers(); i++)   {
    L->N->GetPlayer(i).SetLabel(E.GetPlayer(i).GetLabel());
    for (int j = 1; j <= L->strategies[i].Length(); j++)   {
      gText name;
      for (int k = 1; k <= L->strategies[i][j]->Length(); k++)
	if ((*L->strategies[i][j])[k] > 0)
	  name += ToText((*L->strategies[i][j])[k]);
        else
	  name += "*";
      L->N->GetPlayer(i).GetStrategy(j).SetLabel(name);
    }
  }

  gbtNfgSupport S(*L->N);
  NfgContIter iter(S);
  gArray<gArray<int> *> corr(E.NumPlayers());
  gArray<int> corrs(E.NumPlayers());
  for (i = 1; i <= E.NumPlayers(); i++)  {
    corrs[i] = 1;
    corr[i] = L->strategies[i][1];
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

  ((efgGame &) E).rep->lexicon = L;
  SetEfg(((efgGame &) E).rep->lexicon->N, &E);
  return ((efgGame &) E).rep->lexicon->N;
}

Nfg *MakeAfg(const efgGame &E)
{
  Nfg *afg = new Nfg(gArray<int>(E.NumActions()));

  if (!afg)   return 0;

  ((efgGame &) E).rep->afg = afg;
  afg->SetTitle(E.GetTitle() + " (Agent Form)");

  for (int epl = 1, npl = 1; epl <= E.NumPlayers(); epl++)   {
    for (int iset = 1; iset <= E.GetPlayer(epl).NumInfosets(); iset++, npl++)  {
      gbtEfgInfoset s = E.GetPlayer(epl).GetInfoset(iset);
      for (int act = 1; act <= s.NumActions(); act++)  {
	afg->GetPlayer(npl).GetStrategy(act).SetLabel(ToText(act));
      }
    }
  }

  NfgIter iter(*afg);
  int pl = afg->NumPlayers();

  gArray<int> dim(E.NumPlayers());
  for (int i = 1; i <= dim.Length(); i++) {
    dim[i] = E.GetPlayer(i).NumInfosets();
  }
  gPVector<int> profile(dim);
  ((gVector<int> &) profile).operator=(1);

  gVector<gNumber> payoff(E.NumPlayers());
  
  while (1)  {
    E.Payoff(profile, payoff);

    if (iter.GetIndex() >= 0) {
      iter.SetOutcome(afg->NewOutcome());
    }

    for (int epl = 1, npl = 1; epl <= E.NumPlayers(); epl++)
      for (int iset = 1; iset <= E.GetPlayer(epl).NumInfosets(); iset++, npl++)
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

  SetEfg(afg, (efgGame*) &E);

  return afg;
}


#include "base/glist.imp"
#include "base/garray.imp"

template class gList<lexCorrespondence *>;
template gOutput &operator<<(gOutput &, const gList<lexCorrespondence *> &);
template class gArray<gList<lexCorrespondence *> >;






