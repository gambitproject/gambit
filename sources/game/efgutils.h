//
// FILE: efgutils.h -- Declaration of useful global functions for the 
//                      extensive form
//
// $Id$
//

#ifndef EFGUTILS_H
#define EFGUTILS_H

#include "efg.h"
#include "node.h"

// returns a glist of all nodes in the extensive form
void Nodes (const FullEfg &befg, gList <Node *> &list);

void Nodes (const FullEfg &befg, Node *n, gList <Node *> &list);

// returns the number of nodes in the extensive form
int NumNodes (const FullEfg &befg);

// returns the number of nodes given a starting node
int CountNodes (const FullEfg &e, Node *n);

// returns the action leading up to the node
Action* LastAction(const FullEfg &e, Node *node);

// returns the list of nodes which are marked as the roots of subgames
void MarkedSubgameRoots(const FullEfg &efg, gList<Node *> &list);

// returns the list of nodes which are valid roots of subgames
void LegalSubgameRoots(const FullEfg &efg, gList<Node *> &list);
void LegalSubgameRoots(const FullEfg &efg, Node *, gList<Node *> &);
bool HasSubgames(const FullEfg &efg);
bool HasSubgames(const FullEfg &, Node *n);

bool AllSubgamesMarked(const FullEfg &efg);

// returns the list of nodes which are roots of child subgames
void ChildSubgames(const FullEfg &, Node *, gList<Node *> &);

// determines if a game is perfect recall.  
// if not, returns a pair of infosets violating the definition
bool IsPerfectRecall(const FullEfg &);
bool IsPerfectRecall(const FullEfg &, Infoset *&, Infoset *&);

void RandomEfg(const FullEfg &);
FullEfg *CompressEfg(const FullEfg &, const EFSupport &);

#endif // EFGUTILS_H




