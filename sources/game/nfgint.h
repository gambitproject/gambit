//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Internal representation structs for normal form
//
// This file is part of Gambit
// Copyright (c) 2003, The Gambit Project
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

#ifndef NFGINT_H
#define NFGINT_H

//
// These are the definitions of the internal representation structures
// for normal form games.  These are intended to be private to the
// game library and should not be accessed by external code.
//

//
// Forward declarations
//
struct gbt_nfg_outcome_rep;
struct gbt_nfg_player_rep;
struct gbt_nfg_infoset_rep;
struct gbt_nfg_strategy_rep;
struct gbt_efg_game_rep;

struct gbt_nfg_outcome_rep {
  int m_id;
  gbt_nfg_game_rep *m_nfg;
  bool m_deleted;
  gbtText m_label;
  gbtBlock<gbtNumber> m_payoffs;
  gbtBlock<double> m_doublePayoffs;
  int m_refCount;

  gbt_nfg_outcome_rep(gbt_nfg_game_rep *, int);
};

struct gbt_nfg_strategy_rep {
  int m_id;
  gbt_nfg_infoset_rep *m_infoset;
  gbtArray<int> *m_behav;
  bool m_deleted;
  gbtText m_label;
  long m_index;
  int m_refCount;

  gbt_nfg_strategy_rep(gbt_nfg_infoset_rep *, int p_id);
  ~gbt_nfg_strategy_rep();
};

struct gbt_nfg_infoset_rep {
  int m_id;
  gbt_nfg_player_rep *m_player;
  bool m_deleted;
  gbtText m_label;
  int m_refCount;
  gbtBlock<gbt_nfg_strategy_rep *> m_actions;

  gbt_nfg_infoset_rep(gbt_nfg_player_rep *, int id, int br);
  ~gbt_nfg_infoset_rep() { }
};

struct gbt_nfg_player_rep {
  int m_id;
  gbt_nfg_game_rep *m_nfg;
  bool m_deleted;
  gbtText m_label;
  gbtBlock<gbt_nfg_infoset_rep *> m_infosets;
  int m_refCount;

  gbt_nfg_player_rep(gbt_nfg_game_rep *, int, int);
};

struct gbt_nfg_game_rep {
  int m_refCount;

  long m_revision;
  long m_outcomeRevision;
  gbtText m_label, m_comment;
  gbtArray<int> m_dimensions;

  gbtBlock<gbt_nfg_player_rep *> m_players;
  gbtBlock<gbt_nfg_outcome_rep *> m_outcomes;

  gbtArray<gbt_nfg_outcome_rep *> m_results;
  gbt_efg_game_rep *m_efg;

  gbt_nfg_game_rep(gbt_efg_game_rep *);
  gbt_nfg_game_rep(const gbtArray<int> &);
  ~gbt_nfg_game_rep();

  void DeleteOutcome(gbt_nfg_outcome_rep *);
};

#endif // NFGINT_H
