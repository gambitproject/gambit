//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to supports for extensive forms
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

#ifndef EFSTRAT_H
#define EFSTRAT_H

#include "base/base.h"
#include "base/gstatus.h"
#include "math/gpvector.h"
#include "efg.h"

class gbtEfgActionSet;

class gbtEfgSupport {
protected:
  gbtText m_label;
  gbtEfgGame m_efg;
  gbtArray<gbtEfgActionSet *> m_players;

public:
  gbtEfgSupport(const gbtEfgGame &);
  gbtEfgSupport(const gbtEfgSupport &);
  virtual ~gbtEfgSupport();
  gbtEfgSupport &operator=(const gbtEfgSupport &);

  bool operator==(const gbtEfgSupport &) const;
  bool operator!=(const gbtEfgSupport &) const;

  gbtEfgGame GetGame(void) const { return m_efg; }

  const gbtText &GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }

  int NumActions(int pl, int iset) const;
  int NumActions(const gbtEfgInfoset &) const;
  gbtPVector<int> NumActions(void) const;
  int NumDegreesOfFreedom(void) const;

  // Checks to see that every infoset in the support has at least one
  // action in it.
  bool HasActiveActionAt(const gbtEfgInfoset &) const;
  bool HasActiveActionsAtAllInfosets(void) const;

  bool Contains(const gbtEfgAction &) const;
  bool Contains(int pl, int iset, int act) const;
  int GetIndex(const gbtEfgAction &) const;
  gbtEfgAction GetAction(const gbtEfgInfoset &, int index) const;
  gbtEfgAction GetAction(int pl, int iset, int index) const;

  // Action editing functions
  virtual void AddAction(const gbtEfgAction &);
  virtual bool RemoveAction(const gbtEfgAction &);

  // Number of Sequences for the player
  int NumSequences(int pl) const;
  int TotalNumSequences(void) const;

  // Reachable Nodes and Information Sets
  gbtList<gbtEfgNode> ReachableNonterminalNodes(const gbtEfgNode &) const;
  gbtList<gbtEfgNode> ReachableNonterminalNodes(const gbtEfgNode &,
					      const gbtEfgAction &) const;
  gbtList<gbtEfgInfoset> ReachableInfosets(const gbtEfgNode &) const;
  gbtList<gbtEfgInfoset> ReachableInfosets(const gbtEfgNode &,
					 const gbtEfgAction &) const;
  gbtList<gbtEfgInfoset> ReachableInfosets(const gbtEfgPlayer &) const;

  bool AlwaysReaches(const gbtEfgInfoset &) const;
  bool AlwaysReachesFrom(const gbtEfgInfoset &, const gbtEfgNode &) const;
  bool MayReach(const gbtEfgNode &) const;
  bool MayReach(const gbtEfgInfoset &) const;

  bool Dominates(const gbtEfgAction &, const gbtEfgAction &,
		 bool strong, bool conditional) const;
  bool IsDominated(const gbtEfgAction &,
		   bool strong, bool conditional) const;
  gbtEfgSupport Undominated(bool strong, bool conditional,
			 const gbtArray<int> &players,
			 gbtOutput &, // tracefile 
			 gbtStatus &status) const;


  void Dump(gbtOutput &) const;
};

gbtOutput &operator<<(gbtOutput &f, const gbtEfgSupport &);


// The following class keeps a record of which nodes and infosets are 
// reached by sequences of actions in the support.  This record is
// updated as actions are added and removed.
// BUG - The interface above does not entirely agree with the one
// below vis-a-vis common elements.

class gbtEfgSupportWithActiveInfo : public gbtEfgSupport {
protected:
  gbtArray<gbtList<bool> >         is_infoset_active;
  gbtArray<gbtList<gbtList<bool> > > is_nonterminal_node_active;

  void InitializeActiveListsToAllActive();
  void InitializeActiveListsToAllInactive();
  void InitializeActiveLists();

  void activate(const gbtEfgNode &);
  void deactivate(const gbtEfgNode &);
  void activate(const gbtEfgInfoset &);
  void deactivate(const gbtEfgInfoset &);
  bool infoset_has_active_nodes(const int pl, const int iset) const;
  bool infoset_has_active_nodes(const gbtEfgInfoset &) const;
  void activate_this_and_lower_nodes(const gbtEfgNode &);
  void deactivate_this_and_lower_nodes(const gbtEfgNode &);
  void deactivate_this_and_lower_nodes_returning_deactivated_infosets(
                                                 const gbtEfgNode &,
						 gbtList<gbtEfgInfoset> *);

public:
  gbtEfgSupportWithActiveInfo(const gbtEfgGame &);
  gbtEfgSupportWithActiveInfo(const gbtEfgSupport &);
  gbtEfgSupportWithActiveInfo(const gbtEfgSupportWithActiveInfo &);
  virtual ~gbtEfgSupportWithActiveInfo();

  // Operators
  gbtEfgSupportWithActiveInfo &operator=(const gbtEfgSupportWithActiveInfo &);
  bool operator==(const gbtEfgSupportWithActiveInfo &) const;
  bool operator!=(const gbtEfgSupportWithActiveInfo &) const;

  // Find the reachable nodes at an infoset
  gbtList<gbtEfgNode> ReachableNodesInInfoset(const gbtEfgInfoset &) const;
  gbtList<gbtEfgNode> ReachableNonterminalNodes() const;

  // Action editing functions
  void AddAction(const gbtEfgAction &);
  bool RemoveAction(const gbtEfgAction &);
  bool RemoveActionReturningDeletedInfosets(const gbtEfgAction &,
					    gbtList<gbtEfgInfoset> *);
  //  void GoToNextSubsupportOf(const gbtEfgSupport &);

  // Information
  bool InfosetIsActive(const int pl, const int iset) const;
  bool InfosetIsActive(const gbtEfgInfoset &) const;
  int  NumActiveNodes(const int pl, const int iset) const;
  int  NumActiveNodes(const gbtEfgInfoset &) const;
  bool NodeIsActive(const int pl, const int iset, const int node) const;
  bool NodeIsActive(const gbtEfgNode &) const;

  bool HasActiveActionsAtActiveInfosets();
  bool HasActiveActionsAtActiveInfosetsAndNoOthers();

  void Dump(gbtOutput& s) const;
};

gbtOutput &operator<<(gbtOutput &f, const gbtEfgSupportWithActiveInfo &);


#endif  // EFSTRAT_H
