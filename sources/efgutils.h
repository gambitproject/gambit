//
//  File: efgutils.h -- Declaration of useful global functions for the 
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


#endif // EFGUTILS_H




