//
// FILE: efstrat.h -- Supports and stuff for the extensive form games
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
    gText name;
    const Efg *befg;
    gArray<EFActionSet *> sets;

public:
  EFSupport ( const Efg &);
  EFSupport ( const EFSupport &);
  virtual ~EFSupport();
  EFSupport &operator=(const EFSupport &);

  bool operator==(const EFSupport &) const;
  bool operator!=(const EFSupport &) const;

  // Return the number of players, infosets, actions in a support 
  int NumActions(int pl, int iset) const;
  int NumActions(const Infoset &) const;
  gPVector<int> NumActions(void) const;

  // Returns the position of the action in the support.  Returns zero
  // if it is not there.
  int Find(Action *) const;
  bool ActionIsActive(Action *) const;

  // Find the active actions at an infoset
  const gArray<Action *> &Actions(int pl, int iset) const;
  const gArray<Action *> &Actions(const Infoset &) const;
  const gArray<Action *> &Actions(const Infoset *) const;
  gList<Action *> ListOfActions(const Infoset *) const;
  const EFActionArray    *ActionArray(const Infoset *) const;

  // Action editing functions
  virtual  void AddAction(Action *);
  virtual  bool RemoveAction(Action *);

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
  gList<gList<bool> >         is_infoset_active;
  gList<gList<gList<bool> > > is_nonterminal_node_active;

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
  void AddAction(Action *);
  bool RemoveAction(Action *);

  // Information
  bool ActionIsActive(const int pl, const int iset, const int act) const;
  bool ActionIsActive(const Action *) const;
  bool InfosetIsActive(const int pl, const int iset) const;
  bool InfosetIsActive(const Infoset *) const;
  int  NumActiveNodes(const int pl, const int iset) const;
  int  NumActiveNodes(const Infoset *) const;
  bool NodeIsActive(const int pl, const int iset, const int node) const;
  bool NodeIsActive(const Node *) const;
};


#endif  //# EFSTRAT_H














