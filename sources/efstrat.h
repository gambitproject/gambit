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
  int Find(const Action *) const;
  bool ActionIsActive(const Action *) const;

  // Find the active actions at an infoset
  const gArray<const Action *> &Actions(int pl, int iset) const;
  const gArray<const Action *> &Actions(const Infoset &) const;
  const gArray<const Action *> &Actions(const Infoset *) const;
        gList<const Action *> ListOfActions(const Infoset *) const;
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
							 const Action *) const;
  const gList<const Infoset *> ReachableInfosets(const Node *) const;
  const gList<const Infoset *> ReachableInfosets(const Node *,
						 const Action *) const;
  bool AlwaysReaches(const Infoset *) const;
  bool AlwaysReachesFrom(const Infoset *, const Node *) const;
  bool MayReach(const Node *) const;
  bool MayReach(const Infoset *) const;

  void Dump(gOutput& s) const;
};

gOutput &operator<<(gOutput &f, const EFSupport &);

class EFSupportWithActiveNodes : public EFSupport {
protected:
  gList<const Node *> reachable_nt_nodes;  // Old implementation, keep to debug
  gBlock<gBlock<gList<const Node *> > > reachable_nonterminal_nodes;

  void generate_nonterminal_nodes(const Node *);
  void delete_this_and_lower_nonterminal_nodes(const Node *);

public:
  EFSupportWithActiveNodes ( const Efg &);
  EFSupportWithActiveNodes ( const EFSupport &);
  EFSupportWithActiveNodes ( const EFSupportWithActiveNodes &);
  virtual ~EFSupportWithActiveNodes();
  EFSupportWithActiveNodes &operator=(const EFSupportWithActiveNodes &);

  bool operator==(const EFSupportWithActiveNodes &) const;
  bool operator!=(const EFSupportWithActiveNodes &) const;

  // Find the reachable nodes at an infoset
  const gList<const Node *> *ReachableNonterminalNodes() const;
  const gList<const Node *> ReachableNodesInInfoset(const Infoset *) const;

  // Action editing functions
  void AddAction(Action *);
  bool RemoveAction(Action *);
};


class EFSupportWithActiveInfo : public EFSupport {
protected:
  gList<gList<bool> >         is_infoset_active;
  gList<gList<gList<bool> > > is_nonterminal_node_active;

  void InitializeActiveListsToAllActive();
  void InitializeActiveLists();

  gList<const Node *> reachable_nt_nodes;  // Old implementation, keep to debug
  gBlock<gBlock<gList<const Node *> > > reachable_nonterminal_nodes;

  void generate_nonterminal_nodes(const Node *);
  void delete_this_and_lower_nonterminal_nodes(const Node *);

public:
  EFSupportWithActiveInfo ( const Efg &);
  EFSupportWithActiveInfo ( const EFSupport &);
  EFSupportWithActiveInfo ( const EFSupportWithActiveInfo &);
  virtual ~EFSupportWithActiveInfo();
  EFSupportWithActiveInfo &operator=(const EFSupportWithActiveInfo &);

  bool operator==(const EFSupportWithActiveInfo &) const;
  bool operator!=(const EFSupportWithActiveInfo &) const;

  // Find the reachable nodes at an infoset
  const gList<const Node *> *ReachableNonterminalNodes() const;
  const gList<const Node *> ReachableNodesInInfoset(const Infoset *) const;

  // Action editing functions
  void AddAction(const Action *);
  bool RemoveAction(const Action *);

  // Information
  bool ActionIsActive(const int pl, const int iset, const int act) const;
  bool ActionIsActive(const Action *) const;
  bool InfosetIsActive(const int pl, const int iset) const;
  bool InfosetIsActive(const Infoset *) const;
  bool NodeIsActive(const int pl, const int iset, const int node) const;
  bool NodeIsActive(const Node *) const;
};


#endif  //# EFSTRAT_H














