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
struct gbt_efg_action_rep;
struct gbt_efg_infoset_rep;
struct gbt_efg_node_rep;
struct gbt_efg_player_rep;
struct gbt_efg_game_rep;

class gbtEfgOutcomeBase : public gbtEfgOutcomeRep {
public:
  int m_id;
  gbt_efg_game_rep *m_efg;
  gbtText m_label;
  gbtBlock<gbtNumber> m_payoffs;
  gbtBlock<double> m_doublePayoffs;

  gbtEfgOutcomeBase(gbt_efg_game_rep *, int);

  int GetId(void) const { return m_id; }
  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }

  gbtArray<gbtNumber> GetPayoff(void) const { return m_payoffs; }
  gbtNumber GetPayoff(const gbtEfgPlayer &p_player) const
  { return m_payoffs[p_player.GetId()]; }
  double GetPayoffDouble(int p_playerId) const 
  { return m_doublePayoffs[p_playerId]; }
  void SetPayoff(const gbtEfgPlayer &p_player, const gbtNumber &p_value)
  { m_payoffs[p_player.GetId()] = p_value; m_doublePayoffs[p_player.GetId()] = p_value; } 

  void DeleteOutcome(void);
};

struct gbt_efg_player_rep {
  int m_id;
  gbt_efg_game_rep *m_efg;
  bool m_deleted;
  gbtText m_label;
  gbtBlock<gbt_efg_infoset_rep *> m_infosets;
  int m_refCount;

  gbt_efg_player_rep(gbt_efg_game_rep *, int);
  ~gbt_efg_player_rep();
};

struct gbt_efg_action_rep {
  int m_id;
  gbt_efg_infoset_rep *m_infoset;
  bool m_deleted;
  gbtText m_label;
  int m_refCount;

  gbt_efg_action_rep(gbt_efg_infoset_rep *, int);
};

struct gbt_efg_infoset_rep {
  int m_id;
  gbt_efg_player_rep *m_player;
  bool m_deleted;
  gbtText m_label;
  int m_refCount;
  gbtBlock<gbt_efg_action_rep *> m_actions;
  gbtBlock<gbtNumber> m_chanceProbs;
  gbtBlock<gbt_efg_node_rep *> m_members;
  int m_flag, m_whichbranch;

  gbt_efg_infoset_rep(gbt_efg_player_rep *, int id, int br);
  ~gbt_efg_infoset_rep();

  void PrintActions(gbtOutput &) const;
};

struct gbt_efg_node_rep {
  int m_id;
  gbt_efg_game_rep *m_efg;
  bool m_deleted;
  gbtText m_label;
  int m_refCount;

  bool m_mark;
  gbt_efg_infoset_rep *m_infoset;
  gbt_efg_node_rep *m_parent;
  gbtEfgOutcomeBase *m_outcome;
  gbtBlock<gbt_efg_node_rep *> m_children;
  gbt_efg_node_rep *m_whichbranch, *m_ptr, *m_gameroot;

  gbt_efg_node_rep(gbt_efg_game_rep *, gbt_efg_node_rep *);
  ~gbt_efg_node_rep();

  void DeleteOutcome(gbtEfgOutcomeBase *outc);

  gbt_efg_node_rep *GetPriorSibling(void);
  gbt_efg_node_rep *GetNextSibling(void);

  gbt_efg_node_rep *GetPriorMember(void);
  gbt_efg_node_rep *GetNextMember(void);

  void MarkSubtree(bool p_mark);
};

struct gbt_efg_game_rep {
  int m_refCount;

  bool sortisets;
  mutable long m_revision;
  mutable long m_outcome_revision;
  gbtText m_label, comment;
  gbtBlock<gbt_efg_player_rep *> players;
  gbtBlock<gbtEfgOutcomeBase *> outcomes;
  gbt_efg_node_rep *root;
  gbt_efg_player_rep *chance;
  gbtNfgGame afg;
  gbt_nfg_game_rep *m_reducedNfg;

  gbt_efg_game_rep(void);
  ~gbt_efg_game_rep();

  void SortInfosets(void);
  void NumberNodes(gbt_efg_node_rep *, int &);
  void DeleteLexicon(void);

  void InsertMove(gbt_efg_node_rep *, gbt_efg_infoset_rep *);
  void DeleteMove(gbt_efg_node_rep *);
  void DeleteTree(gbt_efg_node_rep *);

  gbt_efg_infoset_rep *NewInfoset(gbt_efg_player_rep *,
				  int p_id, int p_actions);
  void DeleteInfoset(gbt_efg_infoset_rep *);
  void JoinInfoset(gbt_efg_infoset_rep *, gbt_efg_node_rep *); 
  gbt_efg_infoset_rep *LeaveInfoset(gbt_efg_node_rep *);
  void MergeInfoset(gbt_efg_infoset_rep *, gbt_efg_infoset_rep *);
  void Reveal(gbt_efg_infoset_rep *, gbt_efg_player_rep *);
  void SetPlayer(gbt_efg_infoset_rep *, gbt_efg_player_rep *);

  void DeleteAction(gbt_efg_infoset_rep *, gbt_efg_action_rep *);

  void DeleteOutcome(gbtEfgOutcomeBase *p_outcome);

  // Utility routines for subgames
  void MarkTree(gbt_efg_node_rep *, gbt_efg_node_rep *);
  bool CheckTree(gbt_efg_node_rep *, gbt_efg_node_rep *);
  void MarkSubgame(gbt_efg_node_rep *, gbt_efg_node_rep *);
};

#endif  // EFGINT_H
