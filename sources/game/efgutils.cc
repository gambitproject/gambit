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

#include "math/gdpvect.h"
#include "efgutils.h"

// recursive functions

static void
NDoChild(const gbtGame & e, const gbtGameNode & n, gbtList < gbtGameNode > &list)
{
  list.Append(n);
  for (int i = 1; i <= n->NumChildren(); i++)
    NDoChild(e, n->GetChild(i), list);
}

// Public Functions

int
CountNodes(const gbtGame & e, const gbtGameNode & n)
{
  int num = 1;
  for (int i = 1; i <= n->NumChildren(); i++)
    num += CountNodes(e, n->GetChild(i));
  return num;
}

void
Nodes(const gbtGame & befg, gbtList < gbtGameNode > &list)
{
  list.Flush();
  NDoChild(befg, befg->GetRoot(), list);
}

void
Nodes(const gbtGame & efg, const gbtGameNode &n, gbtList < gbtGameNode > &list)
{
  list.Flush();
  NDoChild(efg, n, list);
}

void
TerminalNodes(const gbtGameNode & p_node, gbtList < gbtGameNode > &p_list)
{
  if (p_node->NumChildren() == 0) {
    p_list.Append(p_node);
  }
  else {
    for (int i = 1; i <= p_node->NumChildren(); i++) {
      TerminalNodes(p_node->GetChild(i), p_list);
    }
  }
}

int
NumNodes(const gbtGame & befg)
{
  return (CountNodes(befg, befg->GetRoot()));
}

