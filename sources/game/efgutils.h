//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Utility functions on extensive form games
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
gbtEfgAction LastAction(const efgGame &e, Node *node);

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
bool IsPerfectRecall(const efgGame &, gbtEfgInfoset &, gbtEfgInfoset &);

void RandomEfg(const efgGame &);
efgGame *CompressEfg(const efgGame &, const EFSupport &);

#endif // EFGUTILS_H




