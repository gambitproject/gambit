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

//
// These are the definitions of the internal representation structures
// for extensive form games.  These are intended to be private to the
// game library and should not be accessed by external code.
//

//
// Forward declarations
//
class efgGame;
struct gbt_efg_outcome_rep;
struct gbt_efg_action_rep;
struct gbt_efg_infoset_rep;
struct gbt_efg_player_rep;
class Node;

struct gbt_efg_outcome_rep {
  int m_id;
  efgGame *m_efg;
  bool m_deleted;
  gText m_label;
  gBlock<gNumber> m_payoffs;
  gBlock<double> m_doublePayoffs;
  int m_refCount;

  gbt_efg_outcome_rep(efgGame *, int);
};

struct gbt_efg_player_rep {
  int m_id;
  efgGame *m_efg;
  bool m_deleted;
  gText m_label;
  gBlock<gbt_efg_infoset_rep *> m_infosets;
  int m_refCount;

  gbt_efg_player_rep(efgGame *, int);
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
  gBlock<Node *> m_members;
  int m_flag, m_whichbranch;

  gbt_efg_infoset_rep(gbt_efg_player_rep *, int id, int br);

  void PrintActions(gOutput &) const;
};

#endif  // EFGINT_H
