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


Lexicon::Lexicon(const gbtEfgGame &p_efg)
  : m_nfg(0), strategies(p_efg.NumPlayers())
{ }

Lexicon::~Lexicon()
{
  for (int i = 1; i <= strategies.Length(); i++)
    while (strategies[i].Length())  delete strategies[i].Remove(1);
  if (m_nfg != 0) {
    m_nfg.rep->m_efg = 0;
  }
}

void SetEfg(gbtNfgGame nfg, gbtEfgGame efg)
{
  nfg.rep->m_efg = efg.rep;
}

void Lexicon::MakeLink(gbtEfgGame efg, gbtNfgGame nfg)
{
  nfg.rep->m_efg = efg.rep;
  m_nfg = nfg;
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

gbtNfgGame MakeReducedNfg(const EFSupport &support)
{
  int i;
  gbtEfgGame efg = support.GetGame();
  Lexicon *L = new Lexicon(efg);
  for (i = 1; i <= efg.NumPlayers(); i++) {
    L->MakeReducedStrats(support, efg.GetPlayer(i), efg.RootNode(), NULL);
  }

  gArray<int> dim(efg.NumPlayers());
  for (i = 1; i <= efg.NumPlayers(); i++)
    dim[i] = (L->strategies[i].Length()) ? L->strategies[i].Length() : 1;

  L->MakeLink(efg, gbtNfgGame(dim));
  L->m_nfg.SetTitle(efg.GetTitle());

  for (i = 1; i <= efg.NumPlayers(); i++)   {
    L->m_nfg.GetPlayer(i).SetLabel(efg.GetPlayer(i).GetLabel());
    for (int j = 1; j <= L->strategies[i].Length(); j++)   {
      gText name;
      for (int k = 1; k <= L->strategies[i][j]->Length(); k++)
	if ((*L->strategies[i][j])[k] > 0)
	  name += ToText((*L->strategies[i][j])[k]);
        else
	  name += "*";
      L->m_nfg.GetPlayer(i).GetStrategy(j).SetLabel(name);
    }
  }

  gbtNfgSupport S(L->m_nfg);
  NfgContIter iter(S);
  gArray<gArray<int> *> corr(efg.NumPlayers());
  gArray<int> corrs(efg.NumPlayers());
  for (i = 1; i <= efg.NumPlayers(); i++)  {
    corrs[i] = 1;
    corr[i] = L->strategies[i][1];
  }

  gArray<gNumber> value(efg.NumPlayers());

  int pl = efg.NumPlayers();
  while (1)  {
    efg.Payoff(corr, value);

    iter.SetOutcome(L->m_nfg.NewOutcome());
    for (int j = 1; j <= efg.NumPlayers(); j++)
      iter.GetOutcome().SetPayoff(L->m_nfg.GetPlayer(j), value[j]);

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
    pl = efg.NumPlayers();
  }

  efg.rep->lexicon = L;
  SetEfg(efg.rep->lexicon->m_nfg, efg);
  return efg.rep->lexicon->m_nfg;
}

gbtNfgGame MakeAfg(const gbtEfgGame &p_efg)
{
  gbtNfgGame afg(gArray<int>(p_efg.NumActions()));

  p_efg.rep->afg = afg;
  afg.SetTitle(p_efg.GetTitle() + " (Agent Form)");

  for (int epl = 1, npl = 1; epl <= p_efg.NumPlayers(); epl++)   {
    for (int iset = 1; iset <= p_efg.GetPlayer(epl).NumInfosets(); iset++, npl++)  {
      gbtEfgInfoset s = p_efg.GetPlayer(epl).GetInfoset(iset);
      for (int act = 1; act <= s.NumActions(); act++)  {
	afg.GetPlayer(npl).GetStrategy(act).SetLabel(ToText(act));
      }
    }
  }

  NfgIter iter(afg);
  int pl = afg.NumPlayers();

  gArray<int> dim(p_efg.NumPlayers());
  for (int i = 1; i <= dim.Length(); i++) {
    dim[i] = p_efg.GetPlayer(i).NumInfosets();
  }
  gPVector<int> profile(dim);
  ((gVector<int> &) profile).operator=(1);

  gVector<gNumber> payoff(p_efg.NumPlayers());
  
  while (1)  {
    p_efg.Payoff(profile, payoff);

    if (iter.GetIndex() >= 0) {
      iter.SetOutcome(afg.NewOutcome());
    }

    for (int epl = 1, npl = 1; epl <= p_efg.NumPlayers(); epl++)
      for (int iset = 1; iset <= p_efg.GetPlayer(epl).NumInfosets(); iset++, npl++)
	iter.GetOutcome().SetPayoff(afg.GetPlayer(npl), payoff[epl]);

    
    while (pl > 0)  {
      if (iter.Next(pl))  {
	profile[pl]++;
	break;
      }
      profile[pl] = 1;
      pl--;
    }

    if (pl == 0)  break;
    pl = afg.NumPlayers();
  }

  SetEfg(afg, p_efg);

  return afg;
}


#include "base/glist.imp"
#include "base/garray.imp"

template class gList<lexCorrespondence *>;
template gOutput &operator<<(gOutput &, const gList<lexCorrespondence *> &);
template class gArray<gList<lexCorrespondence *> >;






