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
class gbtEfgActionBase;
class gbtEfgInfosetBase;
class gbtEfgNodeBase;
class gbtEfgPlayerBase;
class gbtEfgGameBase;
class gbtNfgGameBase;

class gbtEfgOutcomeBase : public gbtEfgOutcomeRep {
public:
  int m_id;
  gbtEfgGameBase *m_efg;
  gbtText m_label;
  gbtBlock<gbtNumber> m_payoffs;
  gbtBlock<double> m_doublePayoffs;

  gbtEfgOutcomeBase(gbtEfgGameBase *, int);

  int GetId(void) const { return m_id; }
  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }

  gbtArray<gbtNumber> GetPayoff(void) const { return m_payoffs; }
  gbtNumber GetPayoff(const gbtEfgPlayer &p_player) const
  { return m_payoffs[p_player->GetId()]; }
  double GetPayoffDouble(int p_playerId) const 
  { return m_doublePayoffs[p_playerId]; }
  void SetPayoff(const gbtEfgPlayer &p_player, const gbtNumber &p_value)
  { m_payoffs[p_player->GetId()] = p_value; m_doublePayoffs[p_player->GetId()] = p_value; } 

  void DeleteOutcome(void);
};

class gbtEfgStrategyBase : public gbtEfgStrategyRep {
public:
  int m_id;
  gbtText m_label;
  gbtEfgPlayerBase *m_player;
  gbtArray<int> m_actions;

  gbtEfgStrategyBase(int p_id, gbtEfgPlayerBase *p_player,
		     const gbtArray<int> &p_actions)
    : m_id(p_id), m_player(p_player), m_actions(p_actions) { }
  virtual ~gbtEfgStrategyBase() { }

  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }
  int GetId(void) const { return 0; }

  gbtEfgPlayer GetPlayer(void) const;
  gbtEfgAction GetAction(const gbtEfgInfoset &p_infoset) const;
  const gbtArray<int> &GetBehavior(void) const { return m_actions; }
};

class gbtEfgPlayerBase : public gbtEfgPlayerRep {
public:
  int m_id;
  gbtEfgGameBase *m_efg;
  gbtText m_label;
  gbtBlock<gbtEfgInfosetBase *> m_infosets;
  gbtBlock<gbtEfgStrategyBase *> m_strategies;

  gbtEfgPlayerBase(gbtEfgGameBase *, int);
  ~gbtEfgPlayerBase();

  gbtEfgGame GetGame(void) const;
  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }
  int GetId(void) const { return m_id; }

  bool IsChance(void) const { return (m_id == 0); }

  int NumInfosets(void) const { return m_infosets.Length(); }
  gbtEfgInfoset NewInfoset(int p_actions);
  gbtEfgInfoset GetInfoset(int p_index) const;
};

class gbtEfgActionBase : public gbtEfgActionRep {
public:
  int m_id;
  gbtEfgInfosetBase *m_infoset;
  bool m_deleted;
  gbtText m_label;
  int m_refCount;

  gbtEfgActionBase(gbtEfgInfosetBase *, int);
  virtual ~gbtEfgActionBase() { } 

  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }
  int GetId(void) const { return m_id; }
  
  gbtEfgInfoset GetInfoset(void) const;

  gbtNumber GetChanceProb(void) const;
  bool Precedes(gbtEfgNode) const;
  
  void DeleteAction(void);
};

class gbtEfgInfosetBase : public gbtEfgInfosetRep {
public:
  int m_id;
  gbtEfgPlayerBase *m_player;
  bool m_deleted;
  gbtText m_label;
  int m_refCount;
  gbtBlock<gbtEfgActionBase *> m_actions;
  gbtBlock<gbtNumber> m_chanceProbs;
  gbtBlock<gbtEfgNodeBase *> m_members;
  int m_flag, m_whichbranch;

  gbtEfgInfosetBase(gbtEfgPlayerBase *, int id, int br);
  virtual ~gbtEfgInfosetBase();

  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }
  int GetId(void) const { return m_id; }
  gbtEfgGame GetGame(void) const;

  void DeleteInfoset(void);

  bool IsChanceInfoset(void) const;

  gbtEfgPlayer GetPlayer(void) const;
  void SetPlayer(gbtEfgPlayer);
  
  void SetChanceProb(int act, const gbtNumber &value); 
  gbtNumber GetChanceProb(int act) const;

  gbtEfgAction InsertAction(int where);

  gbtEfgAction GetAction(int act) const;
  int NumActions(void) const;

  gbtEfgNode GetMember(int m) const;
  int NumMembers(void) const;

  bool Precedes(gbtEfgNode) const;

  void MergeInfoset(gbtEfgInfoset from);
  void Reveal(gbtEfgPlayer);

  bool GetFlag(void) const;
  void SetFlag(bool);

  int GetWhichBranch(void) const;
  void SetWhichBranch(int);

  void PrintActions(gbtOutput &) const;
};

class gbtEfgNodeBase : public gbtEfgNodeRep {
public:
  int m_id;
  gbtEfgGameBase *m_efg;
  bool m_deleted;
  gbtText m_label;
  int m_refCount;

  bool m_mark;
  gbtEfgInfosetBase *m_infoset;
  gbtEfgNodeBase *m_parent;
  gbtEfgOutcomeBase *m_outcome;
  gbtBlock<gbtEfgNodeBase *> m_children;
  mutable gbtEfgNodeBase *m_whichbranch, *m_ptr, *m_gameroot;

  gbtEfgNodeBase(gbtEfgGameBase *, gbtEfgNodeBase *);
  virtual ~gbtEfgNodeBase();

  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }
  int GetId(void) const { return m_id; }
  gbtEfgGame GetGame(void) const;

  int NumChildren(void) const;
  gbtEfgNode GetChild(int i) const;
  gbtEfgNode GetChild(const gbtEfgAction &) const; 
  bool IsPredecessorOf(const gbtEfgNode &) const;

  gbtEfgNode GetParent(void) const;
  gbtEfgAction GetPriorAction(void) const; // returns null if root node

  gbtEfgInfoset GetInfoset(void) const;
  int GetMemberId(void) const;

  gbtEfgPlayer GetPlayer(void) const;

  gbtEfgNode GetSubgameRoot(void) const;
  bool IsSubgameRoot(void) const;

  gbtEfgOutcome GetOutcome(void) const;
  void SetOutcome(const gbtEfgOutcome &);

  gbtEfgNode InsertMove(gbtEfgInfoset);
  // Note: Starting in 0.97.1.1, this now deletes the *parent* move
  void DeleteMove(void);
  void DeleteTree(void);

  void JoinInfoset(gbtEfgInfoset);
  gbtEfgInfoset LeaveInfoset(void);

  void DeleteOutcome(gbtEfgOutcomeBase *outc);

  gbtEfgNode GetPriorSibling(void) const;
  gbtEfgNode GetNextSibling(void) const;

  gbtEfgNode GetPriorMember(void) const;
  gbtEfgNode GetNextMember(void) const;

  void MarkSubtree(bool p_mark);
};

class gbtEfgGameBase : public gbtEfgGameRep {
public:
  bool sortisets;
  mutable long m_revision;
  mutable long m_outcome_revision;
  gbtText m_label, comment;
  gbtBlock<gbtEfgPlayerBase *> players;
  gbtBlock<gbtEfgOutcomeBase *> outcomes;
  gbtEfgNodeBase *root;
  gbtEfgPlayerBase *chance;
  mutable gbtNfgGameBase *m_reducedNfg;

  gbtEfgGameBase(void);
  ~gbtEfgGameBase();

  // this is for use with the copy constructor
  void CopySubtree(gbtEfgGameBase *,
		   gbtEfgNodeBase *, gbtEfgNodeBase *);

  void CopySubtree(gbtEfgNodeBase *, gbtEfgNodeBase *,
		   gbtEfgNodeBase *);

  gbtEfgOutcome NewOutcome(int index);

  void WriteEfg(gbtOutput &, gbtEfgNodeBase *) const;

  void Payoff(gbtEfgNodeBase *n, gbtNumber,
	      const gbtPVector<int> &, gbtVector<gbtNumber> &) const;
  void Payoff(gbtEfgNodeBase *n, gbtNumber,
	      const gbtArray<gbtArray<int> > &, gbtArray<gbtNumber> &) const;
  
  void InfosetProbs(gbtEfgNodeBase *n, gbtNumber,
		    const gbtPVector<int> &, gbtPVector<gbtNumber> &) const;
    

  void SortInfosets(void);
  void NumberNodes(gbtEfgNodeBase *, int &);
  void DeleteLexicon(void);

  void InsertMove(gbtEfgNodeBase *, gbtEfgInfosetBase *);
  void DeleteMove(gbtEfgNodeBase *);
  void DeleteTree(gbtEfgNodeBase *);

  gbtEfgInfosetBase *NewInfoset(gbtEfgPlayerBase *,
				  int p_id, int p_actions);
  void DeleteInfoset(gbtEfgInfosetBase *);
  void JoinInfoset(gbtEfgInfosetBase *, gbtEfgNodeBase *); 
  gbtEfgInfosetBase *LeaveInfoset(gbtEfgNodeBase *);
  void MergeInfoset(gbtEfgInfosetBase *, gbtEfgInfosetBase *);
  void Reveal(gbtEfgInfosetBase *, gbtEfgPlayerBase *);
  void SetPlayer(gbtEfgInfosetBase *, gbtEfgPlayerBase *);

  void DeleteAction(gbtEfgInfosetBase *, gbtEfgActionBase *);

  void DeleteOutcome(gbtEfgOutcomeBase *p_outcome);

  // Utility routines for subgames
  void MarkTree(const gbtEfgNodeBase *, const gbtEfgNodeBase *);
  bool CheckTree(const gbtEfgNodeBase *, const gbtEfgNodeBase *);
  void MarkSubgame(gbtEfgNodeBase *, gbtEfgNodeBase *);

  // Formerly the copy constructor
  gbtEfgGame Copy(gbtEfgNode = gbtEfgNode(0)) const;
  
  // TITLE ACCESS AND MANIPULATION
  void SetLabel(const gbtText &s);
  gbtText GetLabel(void) const;
  
  void SetComment(const gbtText &);
  gbtText GetComment(void) const;

  // WRITING DATA FILES
  void WriteEfg(gbtOutput &p_file) const;

  // DATA ACCESS -- GENERAL INFORMATION
  bool IsConstSum(void) const; 
  bool IsPerfectRecall(void) const;
  bool IsPerfectRecall(gbtEfgInfoset &, gbtEfgInfoset &) const;
  long RevisionNumber(void) const;
  gbtNumber MinPayoff(int pl) const;
  gbtNumber MaxPayoff(int pl) const;
 
  // DATA ACCESS -- NODES
  int NumNodes(void) const;
  gbtEfgNode GetRoot(void) const;

  // DATA ACCESS -- PLAYERS
  int NumPlayers(void) const;
  gbtEfgPlayer GetChance(void) const;
  gbtEfgPlayer NewPlayer(void);
  gbtEfgPlayer GetPlayer(int index) const;

  // DATA ACCESS -- OUTCOMES
  int NumOutcomes(void) const;
  gbtEfgOutcome GetOutcome(int p_id) const;
  gbtEfgOutcome NewOutcome(void);

  // DATA ACCESS -- SUPPORTS
  gbtEfgSupport NewSupport(void) const;

  // EDITING OPERATIONS
  void DeleteEmptyInfosets(void);

  gbtEfgNode CopyTree(gbtEfgNode src, gbtEfgNode dest);
  gbtEfgNode MoveTree(gbtEfgNode src, gbtEfgNode dest);

  gbtEfgAction InsertAction(gbtEfgInfoset);
  gbtEfgAction InsertAction(gbtEfgInfoset, const gbtEfgAction &at);

  void SetChanceProb(gbtEfgInfoset, int, const gbtNumber &);

  void MarkSubgames(void);
  bool MarkSubgame(gbtEfgNode);
  void UnmarkSubgame(gbtEfgNode);
  void UnmarkSubgames(gbtEfgNode);

  int ProfileLength(void) const;
  int TotalNumInfosets(void) const;

  gbtArray<int> NumInfosets(void) const;  // Does not include chance infosets
  int NumPlayerInfosets(void) const;
  gbtPVector<int> NumActions(void) const;
  int NumPlayerActions(void) const;
  gbtPVector<int> NumMembers(void) const;
  
  // COMPUTING VALUES OF PROFILES
  void Payoff(const gbtPVector<int> &profile,
		      gbtVector<gbtNumber> &payoff) const;
  void Payoff(const gbtArray<gbtArray<int> > &profile,
		      gbtArray<gbtNumber> &payoff) const;

  void InfosetProbs(const gbtPVector<int> &profile,
			    gbtPVector<gbtNumber> &prob) const;
   
  gbtNfgGame GetReducedNfg(void) const;

};

#endif  // EFGINT_H
