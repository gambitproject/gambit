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
class gbtNfgPlayerBase;
struct gbt_nfg_infoset_rep;
class gbtNfgActionBase;
class gbtEfgStrategyBase;
class gbtEfgGameBase;
class gbtNfgGameBase;

class gbtNfgOutcomeBase : public gbtNfgOutcomeRep {
public:
  int m_id;
  gbtNfgGameBase *m_nfg;
  gbtText m_label;
  gbtBlock<gbtNumber> m_payoffs;
  gbtBlock<double> m_doublePayoffs;

  gbtNfgOutcomeBase(gbtNfgGameBase *, int);

  int GetId(void) const { return m_id; }
  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }

  gbtArray<gbtNumber> GetPayoff(void) const { return m_payoffs; }
  gbtNumber GetPayoff(const gbtNfgPlayer &p_player) const
  { return m_payoffs[p_player->GetId()]; }
  double GetPayoffDouble(int p_playerId) const 
  { return m_doublePayoffs[p_playerId]; }
  void SetPayoff(const gbtNfgPlayer &p_player, const gbtNumber &p_value)
  { m_payoffs[p_player->GetId()] = p_value; m_doublePayoffs[p_player->GetId()] = p_value; } 

  void DeleteOutcome(void);
};

struct gbt_nfg_infoset_rep {
  int m_id;
  gbtNfgPlayerBase *m_player;
  bool m_deleted;
  gbtText m_label;
  int m_refCount;
  gbtBlock<gbtNfgActionBase *> m_actions;

  gbt_nfg_infoset_rep(gbtNfgPlayerBase *, int id, int br);
  ~gbt_nfg_infoset_rep() { }
};

class gbtNfgActionBase : public gbtNfgActionRep {
public:
  int m_id;
  gbt_nfg_infoset_rep *m_infoset;
  gbtEfgStrategyBase *m_behav;
  bool m_deleted;
  gbtText m_label;
  long m_index;
  int m_refCount;

  gbtNfgActionBase(gbt_nfg_infoset_rep *, int p_id);
  virtual ~gbtNfgActionBase();

  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }
  int GetId(void) const { return m_id; }

  //  virtual gbtNfgInfoset GetInfoset(void) const = 0;

  gbtNfgPlayer GetPlayer(void) const;
  long GetIndex(void) const { return m_index; }

  gbtEfgStrategy GetBehavior(void) const;
};

class gbtNfgPlayerBase : public gbtNfgPlayerRep {
public:
  int m_id;
  gbtNfgGameBase *m_nfg;
  bool m_deleted;
  gbtText m_label;
  gbtBlock<gbt_nfg_infoset_rep *> m_infosets;
  int m_refCount;

  gbtNfgPlayerBase(gbtNfgGameBase *, int, int);
  ~gbtNfgPlayerBase() { }

  gbtNfgGame GetGame(void) const;
  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }
  int GetId(void) const { return m_id; }

  bool IsChance(void) const { return false; }

  int NumInfosets(void) const { return 1; }
  //  gbtNfgInfoset NewInfoset(int p_actions) 
  // { throw gbtGameUndefinedOperation(); }
  // gbtNfgInfoset GetInfoset(int p_index) const { return m_infosets[p_index]; }

  int NumStrategies(void) const { return m_infosets[1]->m_actions.Length(); }
  gbtNfgAction GetStrategy(int p_index) const
  { return m_infosets[1]->m_actions[p_index]; }
};

class gbtNfgGameBase : public gbtNfgGameRep {
public:
  long m_revision;
  mutable long m_outcomeRevision;
  gbtText m_label, m_comment;
  gbtArray<int> m_dimensions;

  gbtBlock<gbtNfgPlayerBase *> m_players;
  gbtBlock<gbtNfgOutcomeBase *> m_outcomes;

  gbtArray<gbtNfgOutcomeBase *> m_results;
  gbtEfgGameBase *m_efg;

  gbtNfgGameBase(gbtEfgGameBase *);
  gbtNfgGameBase(const gbtArray<int> &);
  ~gbtNfgGameBase();

  void IndexStrategies(void);
  void BreakLink(void);

  // GENERAL DATA ACCESS AND MANIPULATION  
  void SetLabel(const gbtText &s);
  gbtText GetLabel(void) const;

  void SetComment(const gbtText &);
  gbtText GetComment(void) const;

  bool IsConstSum(void) const;
  long RevisionNumber(void) const;

  void WriteNfg(gbtOutput &p_file) const;

  // PLAYERS AND STRATEGIES
  int NumPlayers(void) const;
  gbtNfgPlayer GetPlayer(int i) const;

  int NumStrats(int pl) const;
  const gbtArray<int> &NumStrats(void) const; 
  int ProfileLength(void) const;

  // OUTCOMES
  gbtNfgOutcome NewOutcome(void);
  gbtNfgOutcome GetOutcome(int p_id) const;
  int NumOutcomes(void) const;

  void SetOutcomeIndex(int index, const gbtNfgOutcome &outcome);
  gbtNfgOutcome GetOutcomeIndex(int index) const;

  void InitPayoffs(void) const;

  // SUPPORTS
  gbtNfgSupport NewSupport(void) const;

  gbtEfgGame AssociatedEfg(void) const;
  bool HasAssociatedEfg(void) const;

  void DeleteOutcome(gbtNfgOutcomeBase *);
};

#endif // NFGINT_H
