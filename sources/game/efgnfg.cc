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

static void MakeStrategy(gbt_nfg_game_rep *p_nfg, gbtEfgPlayerBase *p_player)
{
  gbtArray<int> behav(p_player->NumInfosets());
  gbtText label = "";

  // FIXME: This is a rather lame labeling scheme.
  for (int iset = 1; iset <= p_player->NumInfosets(); iset++)  {
    if (p_player->GetInfoset(iset).GetFlag()) {
      behav[iset] = p_player->GetInfoset(iset).GetWhichBranch();
      label += ToText(behav[iset]);
    }
    else {
      behav[iset] = 0;
      label += "*";
    }
  }

  gbtEfgStrategyBase *strategy = new gbtEfgStrategyBase(p_player->m_strategies.Length() + 1, p_player, behav);
  p_player->m_strategies.Append(strategy);

  gbtNfgPlayerBase *player = p_nfg->m_players[p_player->GetId()];
  gbtNfgActionBase *action = new gbtNfgActionBase(player->m_infosets[1], player->m_infosets[1]->m_actions.Length() + 1);
  action->m_behav = strategy;
  action->m_label = label;
  player->m_infosets[1]->m_actions.Append(action);
}

static void MakeReducedStrats(gbt_nfg_game_rep *p_nfg,
			      gbtEfgPlayerBase *p,
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
	  gbt_efg_node_rep *m = n->m_children[i];
	  n->m_whichbranch = m;
	  n->m_infoset->m_whichbranch = i;
	  MakeReducedStrats(p_nfg, p, m, nn);
	}
	n->m_infoset->m_flag = false;
      }
      else  {
	// we have visited this infoset, take same action
	MakeReducedStrats(p_nfg, p,
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
      MakeReducedStrats(p_nfg, p, n->m_children[1], n->m_children[1]);
    }
  }
  else if (nn)  {
    gbt_efg_node_rep *m;
    for (; ; nn = nn->m_parent->m_ptr->m_whichbranch)  {
      m = nn->GetNextSibling();
      if (m || nn->m_parent->m_ptr == NULL)   break;
    }
    if (m)  {
      gbt_efg_node_rep *mm = m->m_parent->m_whichbranch;
      m->m_parent->m_whichbranch = m;
      MakeReducedStrats(p_nfg, p, m, m);
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

gbtNfgGame gbtEfgGame::GetReducedNfg(void) const
{
  if (rep->m_reducedNfg) {
    return rep->m_reducedNfg;
  }
  
  gbt_nfg_game_rep *nfg = new gbt_nfg_game_rep(rep);
  nfg->m_label = rep->m_label;
  nfg->m_dimensions = gbtArray<int>(NumPlayers());
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    while (rep->players[pl]->m_strategies.Length() > 0) {
      delete rep->players[pl]->m_strategies.Remove(1);
    }
    nfg->m_players.Append(new gbtNfgPlayerBase(nfg, pl, 0));
    nfg->m_players[pl]->m_label = rep->players[pl]->m_label;
    MakeReducedStrats(nfg, rep->players[pl], rep->root, NULL);
    nfg->m_dimensions[pl] = nfg->m_players[pl]->m_infosets[1]->m_actions.Length();
  }
  return (rep->m_reducedNfg = nfg);
}

