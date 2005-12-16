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

#include "libgambit.h"

class gbtEfgSupportPlayer;

class gbtEfgSupport {
protected:
  std::string m_name;
  gbtEfgGame *m_efg;
  gbtArray<gbtEfgSupportPlayer *> m_players;

public:
  gbtEfgSupport(const gbtEfgGame &);
  gbtEfgSupport(const gbtEfgSupport &);
  virtual ~gbtEfgSupport();
  gbtEfgSupport &operator=(const gbtEfgSupport &);

  bool operator==(const gbtEfgSupport &) const;
  bool operator!=(const gbtEfgSupport &) const;

  gbtEfgGame &GetGame(void) const { return *m_efg; }

  const std::string &GetName(void) const { return m_name; }
  void SetName(const std::string &p_name) { m_name = p_name; }

  int NumActions(int pl, int iset) const;
  int NumActions(const gbtEfgInfoset &) const;
  gbtPVector<int> NumActions(void) const;
  int NumDegreesOfFreedom(void) const;

  // Checks to see that every infoset in the support has at least one
  // action in it.
  bool HasActiveActionAt(const gbtEfgInfoset &) const;
  bool HasActiveActionsAtAllInfosets(void) const;

  // Returns the position of the action in the support. 
  // Returns zero if the action is not contained in the support
  int Find(const gbtEfgAction &) const;
  int Find(int, int, gbtEfgAction) const;

  bool ActionIsActive(const int pl, const int iset, const int act) const;
  bool ActionIsActive(gbtEfgAction) const;
  bool AllActionsInSupportAtInfosetAreActive(const gbtEfgSupport &,
					     const gbtEfgInfoset &) const;

  // Find the active actions at an infoset
  const gbtArray<gbtEfgAction> &Actions(int pl, int iset) const;
  gbtArray<gbtEfgAction> Actions(const gbtEfgInfoset &) const;
  gbtList<gbtEfgAction> ListOfActions(const gbtEfgInfoset &) const;

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

  bool Dominates(const gbtEfgAction &a, const gbtEfgAction &b,
		 bool strong, bool conditional) const;
  bool IsDominated(const gbtEfgAction &a, 
		   bool strong, bool conditional) const;
  gbtEfgSupport Undominated(bool strong, bool conditional,
			 const gbtArray<int> &players,
			std::ostream &) const;
};


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
  bool infoset_has_active_nodes(const gbtEfgInfoset &i) const;
  void activate_this_and_lower_nodes(const gbtEfgNode &);
  void deactivate_this_and_lower_nodes(const gbtEfgNode &);
  void deactivate_this_and_lower_nodes_returning_deactivated_infosets(
                                                 const gbtEfgNode &,
						 gbtList<gbtEfgInfoset> *);

public:
  gbtEfgSupportWithActiveInfo ( const gbtEfgGame &);
  gbtEfgSupportWithActiveInfo ( const gbtEfgSupport &);
  gbtEfgSupportWithActiveInfo ( const gbtEfgSupportWithActiveInfo &);
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

  inline  gbtEfgSupport UnderlyingSupport() const { return (gbtEfgSupport)(*this); }

  bool HasActiveActionsAtActiveInfosets();
  bool HasActiveActionsAtActiveInfosetsAndNoOthers();
};

#endif  //# EFSTRAT_H









