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

// For access to internals
#include "efgint.h"
#include "nfgint.h"

static void MakeStrategy(gbt_nfg_game_rep *p_nfg, gbtEfgPlayer p_player)
{
  gArray<int> *behav = new gArray<int>(p_player.NumInfosets());
  gText label = "";

  // FIXME: This is a rather lame labeling scheme.
  for (int iset = 1; iset <= p_player.NumInfosets(); iset++)  {
    if (p_player.GetInfoset(iset).GetFlag()) {
      (*behav)[iset] = p_player.GetInfoset(iset).GetWhichBranch();
      label += ToText((*behav)[iset]);
    }
    else {
      (*behav)[iset] = 0;
      label += "*";
    }
  }

  gbt_nfg_player_rep *player = p_nfg->m_players[p_player.GetId()];
  gbt_nfg_strategy_rep *strategy = new gbt_nfg_strategy_rep(player);
  strategy->m_behav = behav;
  strategy->m_label = label;
  player->m_strategies.Append(strategy);
  strategy->m_id = player->m_strategies.Length();
}

static void MakeReducedStrats(gbt_nfg_game_rep *p_nfg,
			      const EFSupport &S,
			      gbt_efg_player_rep *p,
			      gbt_efg_node_rep *n,
			      gbt_efg_node_rep *nn)
{
  if (!n->m_parent)  n->m_ptr = 0;

  if (n->m_children.Length() > 0)  {
    if (n->m_infoset->m_player == p)  {
      if (!n->m_infoset->m_flag)  {
	// we haven't visited this infoset before
	n->m_infoset->m_flag = true;
	for (int i = 1; i <= n->m_children.Length(); i++)   {
	  if (S.Contains(n->m_infoset->m_actions[i]))  {
	    gbt_efg_node_rep *m = n->m_children[i];
	    n->m_whichbranch = m;
	    n->m_infoset->m_whichbranch = i;
	    MakeReducedStrats(p_nfg, S, p, m, nn);
	  }
	}
	n->m_infoset->m_flag = false;
      }
      else  {
	// we have visited this infoset, take same action
	MakeReducedStrats(p_nfg, S, p,
			  n->m_children[n->m_infoset->m_whichbranch],
			  nn);
      }
    }
    else  {
      n->m_ptr = NULL;
      if (nn != NULL) {
	n->m_ptr = nn->m_parent;
      }
      n->m_whichbranch = n->m_children[1];
      if (n->m_infoset) { 
	n->m_infoset->m_whichbranch = 0;
      }
      MakeReducedStrats(p_nfg, S, p, n->m_children[1], n->m_children[1]);
    }
  }
  else if (nn)  {
    gbt_efg_node_rep *m;
    for (; ; nn = nn->m_parent->m_ptr->m_whichbranch)  {
      m = nn->NextSibling();
      if (m || nn->m_parent->m_ptr == NULL)   break;
    }
    if (m)  {
      gbt_efg_node_rep *mm = m->m_parent->m_whichbranch;
      m->m_parent->m_whichbranch = m;
      MakeReducedStrats(p_nfg, S, p, m, m);
      m->m_parent->m_whichbranch = mm;
    }
    else {
      MakeStrategy(p_nfg, p);
    }
  }
  else {
    MakeStrategy(p_nfg, p);
  }
}

gbtNfgGame gbtEfgGame::GetReducedNfg(const EFSupport &p_support) const
{
  if (rep->m_reducedNfg) {
    return rep->m_reducedNfg;
  }
  
  gbt_nfg_game_rep *nfg = new gbt_nfg_game_rep(rep);
  nfg->m_title = rep->title;
  nfg->m_dimensions = gArray<int>(NumPlayers());
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    nfg->m_players.Append(new gbt_nfg_player_rep(nfg, pl, 0));
    nfg->m_players[pl]->m_label = rep->players[pl]->m_label;
    MakeReducedStrats(nfg, p_support, rep->players[pl], rep->root, NULL);
    nfg->m_dimensions[pl] = nfg->m_players[pl]->m_strategies.Length();
  }
  return (rep->m_reducedNfg = nfg);
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

    iter.SetOutcome(afg.NewOutcome());

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

  return afg;
}

