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

#ifndef LIBGAMBIT_BEHAVSPT_H
#define LIBGAMBIT_BEHAVSPT_H

#include "game.h"

namespace Gambit {

class BehavSupportPlayer;

class BehavSupport {
protected:
  std::string m_name;
  Game m_efg;
  Array<BehavSupportPlayer *> m_players;

public:
  BehavSupport(const Game &);
  BehavSupport(const BehavSupport &);
  virtual ~BehavSupport();
  BehavSupport &operator=(const BehavSupport &);

  bool operator==(const BehavSupport &) const;
  bool operator!=(const BehavSupport &) const;

  Game GetGame(void) const { return m_efg; }

  const std::string &GetName(void) const { return m_name; }
  void SetName(const std::string &p_name) { m_name = p_name; }

  int NumActions(int pl, int iset) const;
  int NumActions(const GameInfoset &) const;
  PVector<int> NumActions(void) const;
  int NumDegreesOfFreedom(void) const;

  // Checks to see that every infoset in the support has at least one
  // action in it.
  bool HasActiveActionAt(const GameInfoset &) const;
  bool HasActiveActionsAtAllInfosets(void) const;

  // Returns the position of the action in the support. 
  // Returns zero if the action is not contained in the support
  int Find(const GameAction &) const;
  int Find(int, int, GameAction) const;

  bool ActionIsActive(const int pl, const int iset, const int act) const;
  bool ActionIsActive(GameAction) const;
  bool AllActionsInSupportAtInfosetAreActive(const BehavSupport &,
					     const GameInfoset &) const;

  // Find the active actions at an infoset
  const Array<GameAction> &Actions(int pl, int iset) const;
  Array<GameAction> Actions(const GameInfoset &) const;
  gbtList<GameAction> ListOfActions(const GameInfoset &) const;

  // Action editing functions
  virtual void AddAction(const GameAction &);
  virtual bool RemoveAction(const GameAction &);

  // Number of Sequences for the player
  int NumSequences(int pl) const;
  int TotalNumSequences(void) const;

  // Reachable Nodes and Information Sets
  gbtList<GameNode> ReachableNonterminalNodes(const GameNode &) const;
  gbtList<GameNode> ReachableNonterminalNodes(const GameNode &, 
						const GameAction &) const;
  gbtList<GameInfoset> ReachableInfosets(const GameNode &) const;
  gbtList<GameInfoset> ReachableInfosets(const GameNode &, 
					   const GameAction &) const;
  gbtList<GameInfoset> ReachableInfosets(const GamePlayer &) const;

  bool AlwaysReaches(const GameInfoset &) const;
  bool AlwaysReachesFrom(const GameInfoset &, const GameNode &) const;
  bool MayReach(const GameNode &) const;
  bool MayReach(const GameInfoset &) const;

  bool Dominates(const GameAction &a, const GameAction &b,
		 bool strong, bool conditional) const;
  bool IsDominated(const GameAction &a, 
		   bool strong, bool conditional) const;
  BehavSupport Undominated(bool strong, bool conditional,
			 const Array<int> &players,
			std::ostream &) const;
};


// The following class keeps a record of which nodes and infosets are 
// reached by sequences of actions in the support.  This record is
// updated as actions are added and removed.
// BUG - The interface above does not entirely agree with the one
// below vis-a-vis common elements.

class BehavSupportWithActiveInfo : public BehavSupport {
protected:
  Array<gbtList<bool> >         is_infoset_active;
  Array<gbtList<gbtList<bool> > > is_nonterminal_node_active;

  void InitializeActiveListsToAllActive();
  void InitializeActiveListsToAllInactive();
  void InitializeActiveLists();

  void activate(const GameNode &);
  void deactivate(const GameNode &);
  void activate(const GameInfoset &);
  void deactivate(const GameInfoset &);
  bool infoset_has_active_nodes(const int pl, const int iset) const;
  bool infoset_has_active_nodes(const GameInfoset &i) const;
  void activate_this_and_lower_nodes(const GameNode &);
  void deactivate_this_and_lower_nodes(const GameNode &);
  void deactivate_this_and_lower_nodes_returning_deactivated_infosets(
                                                 const GameNode &,
						 gbtList<GameInfoset> *);

public:
  BehavSupportWithActiveInfo ( const Game &);
  BehavSupportWithActiveInfo ( const BehavSupport &);
  BehavSupportWithActiveInfo ( const BehavSupportWithActiveInfo &);
  virtual ~BehavSupportWithActiveInfo();

  // Operators
  BehavSupportWithActiveInfo &operator=(const BehavSupportWithActiveInfo &);
  bool operator==(const BehavSupportWithActiveInfo &) const;
  bool operator!=(const BehavSupportWithActiveInfo &) const;

  // Find the reachable nodes at an infoset
  gbtList<GameNode> ReachableNodesInInfoset(const GameInfoset &) const;
  gbtList<GameNode> ReachableNonterminalNodes() const;

  // Action editing functions
  void AddAction(const GameAction &);
  bool RemoveAction(const GameAction &);
  bool RemoveActionReturningDeletedInfosets(const GameAction &, 
					    gbtList<GameInfoset> *);
  //  void GoToNextSubsupportOf(const BehavSupport &);

  // Information
  bool InfosetIsActive(const int pl, const int iset) const;
  bool InfosetIsActive(const GameInfoset &) const;
  int  NumActiveNodes(const int pl, const int iset) const;
  int  NumActiveNodes(const GameInfoset &) const;
  bool NodeIsActive(const int pl, const int iset, const int node) const;
  bool NodeIsActive(const GameNode &) const;

  bool HasActiveActionsAtActiveInfosets();
  bool HasActiveActionsAtActiveInfosetsAndNoOthers();
};

} // end namespace Gambit

#endif  // LIBGAMBIT_BEHAVSPT_H









