//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Internal representation structs for extensive form
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

#ifndef EFGINT_H
#define EFGINT_H

#include "nfg.h"

//
// These are the definitions of the internal representation structures
// for extensive form games.  These are intended to be private to the
// game library and should not be accessed by external code.
//

//
// Forward declarations
//
class gbtEfgGame;
struct gbt_efg_outcome_rep;
struct gbt_efg_action_rep;
struct gbt_efg_infoset_rep;
struct gbt_efg_node_rep;
struct gbt_efg_player_rep;
struct gbt_efg_game_rep;

struct gbt_efg_outcome_rep {
  int m_id;
  gbt_efg_game_rep *m_efg;
  bool m_deleted;
  gText m_label;
  gBlock<gNumber> m_payoffs;
  gBlock<double> m_doublePayoffs;
  int m_refCount;

  gbt_efg_outcome_rep(gbt_efg_game_rep *, int);
};

struct gbt_efg_player_rep {
  int m_id;
  gbt_efg_game_rep *m_efg;
  bool m_deleted;
  gText m_label;
  gBlock<gbt_efg_infoset_rep *> m_infosets;
  int m_refCount;

  gbt_efg_player_rep(gbt_efg_game_rep *, int);
  ~gbt_efg_player_rep();
};

struct gbt_efg_action_rep {
  int m_id;
  gbt_efg_infoset_rep *m_infoset;
  bool m_deleted;
  gText m_label;
  int m_refCount;

  gbt_efg_action_rep(gbt_efg_infoset_rep *, int);
};

struct gbt_efg_infoset_rep {
  int m_id;
  gbt_efg_player_rep *m_player;
  bool m_deleted;
  gText m_label;
  int m_refCount;
  gBlock<gbt_efg_action_rep *> m_actions;
  gBlock<gNumber> m_chanceProbs;
  gBlock<gbt_efg_node_rep *> m_members;
  int m_flag, m_whichbranch;

  gbt_efg_infoset_rep(gbt_efg_player_rep *, int id, int br);

  void PrintActions(gOutput &) const;
};

struct gbt_efg_node_rep {
  int m_id;
  gbt_efg_game_rep *m_efg;
  bool m_deleted;
  gText m_label;
  int m_refCount;

  bool m_mark;
  gbt_efg_infoset_rep *m_infoset;
  gbt_efg_node_rep *m_parent;
  gbt_efg_outcome_rep *m_outcome;
  gBlock<gbt_efg_node_rep *> m_children;
  gbt_efg_node_rep *m_whichbranch, *m_ptr, *m_gameroot;

  gbt_efg_node_rep(gbt_efg_game_rep *, gbt_efg_node_rep *);
  ~gbt_efg_node_rep();

  void DeleteOutcome(gbt_efg_outcome_rep *outc);
};

struct gbt_efg_game_rep {
  int m_refCount;

  bool sortisets;
  mutable bool m_dirty;
  mutable long m_revision;
  mutable long m_outcome_revision;
  gText title, comment;
  gBlock<gbt_efg_player_rep *> players;
  gBlock<gbt_efg_outcome_rep *> outcomes;
  gbt_efg_node_rep *root;
  gbt_efg_player_rep *chance;
  gbtNfgGame afg;
  mutable Lexicon *lexicon;

  gbt_efg_game_rep(void);
  ~gbt_efg_game_rep();

  void SortInfosets(void);
  void NumberNodes(gbt_efg_node_rep *, int &);
  void DeleteLexicon(void);

  void AppendMove(gbt_efg_node_rep *, gbt_efg_infoset_rep *);
  void InsertMove(gbt_efg_node_rep *, gbt_efg_infoset_rep *);
  gbt_efg_infoset_rep *NewInfoset(gbt_efg_player_rep *,
				  int p_id, int p_actions);
};

#endif  // EFGINT_H




