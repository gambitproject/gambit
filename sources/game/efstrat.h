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

class Action;
class EFActionSet;

class EFSupport {
protected:
  gText m_name;
  efgGame *m_efg;
  gArray<EFActionSet *> m_players;

public:
  EFSupport(const efgGame &);
  EFSupport(const EFSupport &);
  virtual ~EFSupport();
  EFSupport &operator=(const EFSupport &);

  bool operator==(const EFSupport &) const;
  bool operator!=(const EFSupport &) const;

  efgGame &GetGame(void) const { return *m_efg; }

  const gText &GetName(void) const { return m_name; }
  void SetName(const gText &p_name) { m_name = p_name; }

  int NumActions(int pl, int iset) const;
  int NumActions(const Infoset *) const;
  gPVector<int> NumActions(void) const;
  int NumDegreesOfFreedom(void) const;

  // Checks to see that every infoset in the support has at least one
  // action in it.
  bool HasActiveActionAt(const Infoset *) const;
  bool HasActiveActionsAtAllInfosets(void) const;

  bool Contains(const Action *) const;
  bool Contains(int pl, int iset, int act) const;
  int GetIndex(const Action *) const;
  Action *GetAction(Infoset *, int index) const;
  Action *GetAction(int pl, int iset, int index) const;

  // Action editing functions
  virtual void AddAction(const Action *);
  virtual bool RemoveAction(const Action *);

  // Number of Sequences for the player
  int NumSequences(int pl) const;
  int TotalNumSequences(void) const;

  // Reachable Nodes and Information Sets
  gList<Node *> ReachableNonterminalNodes(const Node *) const;
  gList<Node *> ReachableNonterminalNodes(const Node *, const Action *) const;
  gList<Infoset *> ReachableInfosets(const Node *) const;
  gList<Infoset *> ReachableInfosets(const Node *, const Action *) const;
  gList<Infoset *> ReachableInfosets(const gbtEfgPlayer &) const;

  bool AlwaysReaches(const Infoset *) const;
  bool AlwaysReachesFrom(const Infoset *, const Node *) const;
  bool MayReach(const Node *) const;
  bool MayReach(const Infoset *) const;

  bool Dominates(const Action *a, const Action *b,
		 bool strong, bool conditional) const;
  bool IsDominated(const Action *a, 
		   bool strong, bool conditional) const;
  EFSupport Undominated(bool strong, bool conditional,
			 const gArray<int> &players,
			 gOutput &, // tracefile 
			 gStatus &status) const;


  void Dump(gOutput &) const;
};

gOutput &operator<<(gOutput &f, const EFSupport &);


// The following class keeps a record of which nodes and infosets are 
// reached by sequences of actions in the support.  This record is
// updated as actions are added and removed.
// BUG - The interface above does not entirely agree with the one
// below vis-a-vis common elements.

class EFSupportWithActiveInfo : public EFSupport {
protected:
  gArray<gList<bool> >         is_infoset_active;
  gArray<gList<gList<bool> > > is_nonterminal_node_active;

  void InitializeActiveListsToAllActive();
  void InitializeActiveListsToAllInactive();
  void InitializeActiveLists();

  void activate(const Node *);
  void deactivate(const Node *);
  void activate(const Infoset *);
  void deactivate(const Infoset *);
  bool infoset_has_active_nodes(const int pl, const int iset) const;
  bool infoset_has_active_nodes(const Infoset *i) const;
  void activate_this_and_lower_nodes(const Node *);
  void deactivate_this_and_lower_nodes(const Node *);
  void deactivate_this_and_lower_nodes_returning_deactivated_infosets(
                                                 const Node *,
						 gList<Infoset *> *);

public:
  EFSupportWithActiveInfo ( const efgGame &);
  EFSupportWithActiveInfo ( const EFSupport &);
  EFSupportWithActiveInfo ( const EFSupportWithActiveInfo &);
  virtual ~EFSupportWithActiveInfo();

  // Operators
  EFSupportWithActiveInfo &operator=(const EFSupportWithActiveInfo &);
  bool operator==(const EFSupportWithActiveInfo &) const;
  bool operator!=(const EFSupportWithActiveInfo &) const;

  // Find the reachable nodes at an infoset
  const gList<const Node *> ReachableNodesInInfoset(const Infoset *) const;
  const gList<const Node *> ReachableNonterminalNodes() const;

  // Action editing functions
  void AddAction(const Action *);
  bool RemoveAction(const Action *);
  bool RemoveActionReturningDeletedInfosets(const Action *, 
					    gList<Infoset *> *);
  //  void GoToNextSubsupportOf(const EFSupport &);

  // Information
  bool InfosetIsActive(const int pl, const int iset) const;
  bool InfosetIsActive(const Infoset *) const;
  int  NumActiveNodes(const int pl, const int iset) const;
  int  NumActiveNodes(const Infoset *) const;
  bool NodeIsActive(const int pl, const int iset, const int node) const;
  bool NodeIsActive(const Node *) const;

  bool HasActiveActionsAtActiveInfosets();
  bool HasActiveActionsAtActiveInfosetsAndNoOthers();

  void Dump(gOutput& s) const;
};

gOutput &operator<<(gOutput &f, const EFSupportWithActiveInfo &);


#endif  // EFSTRAT_H









