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
#include "glist.h"

// returns a glist of all nodes in the extensive form
void Nodes (const BaseEfg &befg, gList <Node *> &list);

void Nodes (const BaseEfg &befg, Node *n, gList <Node *> &list);

// returns a glist of the Terminal Nodes in the extensive form 
void TerminalNodes (const BaseEfg &befg, gList <Node *> &list);

// returns a glist of the NonTerminal Nodes in the extensive form
void NonTerminalNodes (const BaseEfg &befg, gList <Node *> &list);

// returns the number of nodes in the extensive form
int NumNodes (const BaseEfg &befg);

// returns the number of nodes given a starting node
int CountNodes (Node *n);

// returns the action leading up to the node
Action* LastAction( Node* node );

// returns the list of nodes which are the roots of subgames
void SubgameRoots(const BaseEfg &efg, gList<Node *> &list);

// returns the list of nodes which are roots of child subgames
void ChildSubgames(Node *, gList<Node *> &);

#endif // EFGUTILS_H




