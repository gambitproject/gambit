//
// FILE: efstrat.h -- Strategy-related classes for the extensive form games
//
// $Id$
//

#ifndef EFSTRAT_H
#define EFSTRAT_H

#include "base/base.h"
#include "efg.h"
#include "gpvector.h"
#include "gstatus.h"

class Action;
class EFPlayer;
class EFActionSet;
class Efg;  

class EFSupport {
protected:
  gText m_name;
  const Efg *m_efg;
  gArray<EFActionSet *> m_players;

public:
  EFSupport(const Efg &);
  EFSupport(const EFSupport &);
  virtual ~EFSupport();
  EFSupport &operator=(const EFSupport &);

  bool operator==(const EFSupport &) const;
  bool operator!=(const EFSupport &) const;

  const Efg &Game(void) const { return *m_efg; }

  const gText &GetName(void) const { return m_name; }
  void SetName(const gText &p_name) { m_name = p_name; }

  int NumActions(int pl, int iset) const;
  int NumActions(const Infoset *) const;
  gPVector<int> NumActions(void) const;

  // Checks to see that every infoset in the support has at least one
  // action in it.
  bool HasActiveActionAt(const Infoset *) const;
  bool HasActiveActionsAtAllInfosets(void) const;

  // Returns the position of the action in the support. 
  // Returns zero if the action is not contained in the support
  int Find(const Action *) const;
  int Find(int, int, Action *) const;

  bool ActionIsActive(const int pl, const int iset, const int act) const;
  bool ActionIsActive(Action *) const;
  bool AllActionsInSupportAtInfosetAreActive(const EFSupport &,
					     const Infoset *) const;

  // Find the active actions at an infoset
  const gArray<Action *> &Actions(int pl, int iset) const;
  const gArray<Action *> &Actions(const Infoset *) const;
  gList<Action *> ListOfActions(const Infoset *) const;

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
  gList<Infoset *> ReachableInfosets(const EFPlayer *) const;

  bool AlwaysReaches(const Infoset *) const;
  bool AlwaysReachesFrom(const Infoset *, const Node *) const;
  bool MayReach(const Node *) const;
  bool MayReach(const Infoset *) const;

  bool Dominates(const Action *a, const Action *b,
		 bool strong, bool conditional) const;
  bool IsDominated(const Action *a, 
		   bool strong, bool conditional) const;
  EFSupport *Undominated(bool strong, bool conditional,
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
  EFSupportWithActiveInfo ( const Efg &);
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

  inline  EFSupport UnderlyingSupport() const { return (EFSupport)(*this); }

  bool HasActiveActionsAtActiveInfosets();
  bool HasActiveActionsAtActiveInfosetsAndNoOthers();

  void Dump(gOutput& s) const;
};

gOutput &operator<<(gOutput &f, const EFSupportWithActiveInfo &);


#endif  //# EFSTRAT_H









