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
void Nodes (const efgGame &befg, gList <Node *> &list);

void Nodes (const efgGame &befg, Node *n, gList <Node *> &list);

// returns the number of nodes in the extensive form
int NumNodes (const efgGame &befg);

// returns the number of nodes given a starting node
int CountNodes (const efgGame &e, Node *n);

// returns the action leading up to the node
Action* LastAction(const efgGame &e, Node *node);

// returns the list of nodes which are marked as the roots of subgames
void MarkedSubgameRoots(const efgGame &efg, gList<Node *> &list);

// returns the list of nodes which are valid roots of subgames
void LegalSubgameRoots(const efgGame &efg, gList<Node *> &list);
void LegalSubgameRoots(const efgGame &efg, Node *, gList<Node *> &);
bool HasSubgames(const efgGame &efg);
bool HasSubgames(const efgGame &, Node *n);

bool AllSubgamesMarked(const efgGame &efg);

// returns the list of nodes which are roots of child subgames
void ChildSubgames(const efgGame &, Node *, gList<Node *> &);

// determines if a game is perfect recall.  
// if not, returns a pair of infosets violating the definition
bool IsPerfectRecall(const efgGame &);
bool IsPerfectRecall(const efgGame &, Infoset *&, Infoset *&);

void RandomEfg(const efgGame &);
efgGame *CompressEfg(const efgGame &, const EFSupport &);

#endif // EFGUTILS_H




