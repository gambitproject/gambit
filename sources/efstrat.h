//
// FILE: efstrat.h -- Strategy-related classes for the extensive form games
//
// $Id$
//

#ifndef EFSTRAT_H
#define EFSTRAT_H

#include "gtext.h"
#include "gblock.h"
#include "efg.h"
#include "gpvector.h"

class Action;
class EFPlayer;
class EFActionArray;
class EFActionSet;
class Efg;  


class EFSupport {
protected:
  gText m_name;
  const Efg *befg;
  gArray<EFActionSet *> sets;

public:
  EFSupport ( const Efg &);
  EFSupport ( const EFSupport &);
  virtual ~EFSupport();
  EFSupport &operator=(const EFSupport &);

  bool operator==(const EFSupport &) const;
  bool operator!=(const EFSupport &) const;

  const gText &GetName(void) const { return m_name; }
  void SetName(const gText &p_name) { m_name = p_name; }

  // Return the number of players, infosets, actions in a support 
  int NumActions(int pl, int iset) const;
  int NumActions(const Infoset &) const;
  int NumActions(const Infoset *) const;
  gPVector<int> NumActions(void) const;

  // Returns the position of the action in the support.  Returns zero
  // if it is not there.
  int Find(Action *) const;
  int Find(int, int, Action *) const;
  bool ActionIsActive(const int pl, const int iset, const int act) const;
  bool ActionIsActive(Action *) const;
  bool AllActionsInSupportAtInfosetAreActive(const EFSupport &,
					     const Infoset *) const;

  // Find the active actions at an infoset
  const gArray<Action *> &Actions(int pl, int iset) const;
  const gArray<Action *> &Actions(const Infoset &) const;
  const gArray<Action *> &Actions(const Infoset *) const;
  gList<Action *> ListOfActions(const Infoset *) const;
  const EFActionArray    *ActionArray(const Infoset *) const;

  // Action editing functions
  virtual  void AddAction(const Action *);
  virtual  bool RemoveAction(const Action *);

  // Returns the Efg associated with this Support.
  const Efg &Game(void) const;
  const Node *RootNode(void) const;

  // Checks to see that every infoset in the support has at least one
  // action in it.
  bool IsValid(void) const;

  // Number of Sequences for the player
  int NumSequences(int pl) const;
  int TotalNumSequences() const;

  // Reachable Nodes and Information Sets
  const gList<const Node *>    ReachableNonterminalNodes(const Node *) const;
  const gList<const Node *>    ReachableNonterminalNodes(const Node *,
							 Action *) const;
  const gList<const Infoset *> ReachableInfosets(const Node *) const;
  const gList<const Infoset *> ReachableInfosets(const Node *,
						 Action *) const;
  const gList<const Infoset *> ReachableInfosets(const EFPlayer *) const;

  bool AlwaysReaches(const Infoset *) const;
  bool AlwaysReachesFrom(const Infoset *, const Node *) const;
  bool MayReach(const Node *) const;
  bool MayReach(const Infoset *) const;

  void Dump(gOutput& s) const;
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









