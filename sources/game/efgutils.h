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

#include "game.h"

// returns a glist of all nodes in the extensive form
void Nodes(const gbtGame &, gbtList<gbtGameNode> &list);

void Nodes(const gbtGame &, const gbtGameNode &n, gbtList<gbtGameNode> &list);

// returns the number of nodes in the extensive form
int NumNodes(const gbtGame &);

// returns the number of nodes given a starting node
int CountNodes(const gbtGame &, const gbtGameNode &);

// Returns the terminal nodes in a subtree rooted at 'p_node'
void TerminalNodes(const gbtGameNode &, gbtList<gbtGameNode> &);

// returns the list of nodes which are marked as the roots of subgames
void MarkedSubgameRoots(const gbtGame &, gbtList<gbtGameNode> &list);

// returns the list of nodes which are valid roots of subgames
void LegalSubgameRoots(const gbtGame &, gbtList<gbtGameNode> &list);
void LegalSubgameRoots(const gbtGame &, const gbtGameNode &,
		       gbtList<gbtGameNode> &);
bool HasSubgames(const gbtGame &);
bool HasSubgames(const gbtGame &, const gbtGameNode &n);

bool AllSubgamesMarked(const gbtGame &efg);

// returns the list of nodes which are roots of child subgames
void ChildSubgames(const gbtGame &, const gbtGameNode &, 
		   gbtList<gbtGameNode> &);

gbtGame CompressEfg(const gbtGame &, const gbtEfgSupport &);
// An "in-place" version of the above
void CompressEfgInPlace(gbtGame, const gbtEfgSupport &);

#endif // EFGUTILS_H
