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

#include "efgutils.h"
#include "efstrat.h"

// recursive functions

static void
NDoChild(const gbtEfgGame & e, const gbtEfgNode & n, gList < gbtEfgNode > &list)
{
  list.Append(n);
  for (int i = 1; i <= n.NumChildren(); i++)
    NDoChild(e, n.GetChild(i), list);
}

static void
MSRDoChild(const gbtEfgGame & e, const gbtEfgNode & n,
           gList < gbtEfgNode > &list)
{
  for (int i = 1; i <= n.NumChildren(); i++)
    MSRDoChild(e, n.GetChild(i), list);
  if (n.GetSubgameRoot() == n)
    list.Append(n);
}

static void
LSRDoChild(const gbtEfgGame & e, const gbtEfgNode & n,
           gList < gbtEfgNode > &list)
{
  for (int i = 1; i <= n.NumChildren(); i++)
    LSRDoChild(e, n.GetChild(i), list);
  if (n.GetGame().IsLegalSubgame(n))
    list.Append(n);
}

static void
CSDoChild(const gbtEfgGame & e, gbtEfgNode n, gList < gbtEfgNode > &list)
{
  if (n.GetSubgameRoot() == n)
    list.Append(n);
  else
    for (int i = 1; i <= n.NumChildren(); i++)
      CSDoChild(e, n.GetChild(i), list);
}

// Public Functions

int
CountNodes(const gbtEfgGame & e, const gbtEfgNode & n)
{
  int num = 1;
  for (int i = 1; i <= n.NumChildren(); i++)
    num += CountNodes(e, n.GetChild(i));
  return num;
}

void
Nodes(const gbtEfgGame & befg, gList < gbtEfgNode > &list)
{
  list.Flush();
  NDoChild(befg, befg.RootNode(), list);
}

void
Nodes(const gbtEfgGame & efg, const gbtEfgNode &n, gList < gbtEfgNode > &list)
{
  list.Flush();
  NDoChild(efg, n, list);
}

void
TerminalNodes(const gbtEfgNode & p_node, gList < gbtEfgNode > &p_list)
{
  if (p_node.NumChildren() == 0) {
    p_list.Append(p_node);
  }
  else {
    for (int i = 1; i <= p_node.NumChildren(); i++) {
      TerminalNodes(p_node.GetChild(i), p_list);
    }
  }
}

void
MarkedSubgameRoots(const gbtEfgGame & efg, gList < gbtEfgNode > &list)
{
  list.Flush();
  MSRDoChild(efg, efg.RootNode(), list);
}

void
LegalSubgameRoots(const gbtEfgGame & efg, gList < gbtEfgNode > &list)
{
  list.Flush();
  LSRDoChild(efg, efg.RootNode(), list);
}

void
LegalSubgameRoots(const gbtEfgGame & efg, const gbtEfgNode & n,
                  gList < gbtEfgNode > &list)
{
  list.Flush();
  LSRDoChild(efg, n, list);
}

bool
HasSubgames(const gbtEfgGame & efg)
{
  gList < gbtEfgNode > list;
  LegalSubgameRoots(efg, list);
  return list.Length() > 1;
}

bool
HasSubgames(const gbtEfgGame & e, gbtEfgNode n)
{
  gList < gbtEfgNode > list;
  LegalSubgameRoots(e, n, list);
  if (n.GetGame().IsLegalSubgame(n))
    return list.Length() > 1;
  return list.Length() > 0;
}

bool
AllSubgamesMarked(const gbtEfgGame & efg)
{
  gList < gbtEfgNode > marked, valid;

  LegalSubgameRoots(efg, valid);
  MarkedSubgameRoots(efg, marked);

  return (marked == valid);
}


void
ChildSubgames(const gbtEfgGame & efg, const gbtEfgNode &n,
	      gList < gbtEfgNode > &list)
{
  list.Flush();
  for (int i = 1; i <= n.NumChildren(); i++)
    CSDoChild(efg, n.GetChild(i), list);
}

int
NumNodes(const gbtEfgGame & befg)
{
  return (CountNodes(befg, befg.RootNode()));
}

gbtEfgAction
LastAction(const gbtEfgGame & e, const gbtEfgNode &node)
{
  gbtEfgNode parent = node.GetParent();
  if (parent == 0)
    return 0;
  for (int i = 1; i <= node.NumChildren(); i++)
    if (parent.GetChild(i) == node)
      return parent.GetInfoset().GetAction(i);
  return 0;
}

bool
IsPerfectRecall(const gbtEfgGame & p_efg)
{
  gbtEfgInfoset s1, s2;
  return IsPerfectRecall(p_efg, s1, s2);
}

bool
IsPerfectRecall(const gbtEfgGame & efg, gbtEfgInfoset & s1, gbtEfgInfoset & s2)
{
  for (int pl = 1; pl <= efg.NumPlayers(); pl++) {
    gbtEfgPlayer player = efg.GetPlayer(pl);

    for (int i = 1; i <= player.NumInfosets(); i++) {
      gbtEfgInfoset iset1 = player.GetInfoset(i);
      for (int j = 1; j <= player.NumInfosets(); j++) {
        gbtEfgInfoset iset2 = player.GetInfoset(j);

        bool precedes = false;
        int action = 0;

        for (int m = 1; m <= iset2.NumMembers(); m++) {
          int n;
          for (n = 1; n <= iset1.NumMembers(); n++) {
            if (iset1.GetMember(n).IsPredecessor(iset2.GetMember(m))
                && iset1.GetMember(n) != iset2.GetMember(m)) {
              precedes = true;
              for (int act = 1; act <= iset1.NumActions(); act++) {
                if (iset1.GetMember(n).GetChild(act).
                    IsPredecessor(iset2.GetMember(m))) {
                  if (action != 0 && action != act) {
                    s1 = iset1;
                    s2 = iset2;
                    return false;
                  }
                  action = act;
                }
              }
              break;
            }
          }

          if (i == j && precedes) {
            s1 = iset1;
            s2 = iset2;
            return false;
          }

          if (n > iset1.NumMembers() && precedes) {
            s1 = iset1;
            s2 = iset2;
            return false;
          }
        }


      }
    }
  }

  return true;
}

gbtEfgGame
CompressEfg(const gbtEfgGame & efg, const EFSupport & S)
{
  gbtEfgGame newefg = efg.Copy();

  for (int pl = 1; pl <= newefg.NumPlayers(); pl++) {
    gbtEfgPlayer player = newefg.GetPlayer(pl);
    for (int iset = 1; iset <= player.NumInfosets(); iset++) {
      gbtEfgInfoset infoset = player.GetInfoset(iset);
      for (int act = infoset.NumActions(); act >= 1; act--) {
        gbtEfgAction oldact =
          efg.GetPlayer(pl).GetInfoset(iset).GetAction(act);
        if (!S.Contains(oldact)) {
          newefg.DeleteAction(infoset, infoset.GetAction(act));
        }
      }
    }
  }

  return newefg;
}


#include "math/rational.h"
// prototype in efg.h

void
RandomEfg(gbtEfgGame & efg)
{
  for (int i = 1; i <= efg.NumPlayers(); i++) {
    for (int j = 1; j <= efg.NumOutcomes(); j++) {
      efg.GetOutcome(j).SetPayoff(efg.GetPlayer(i), gNumber(Uniform()));
    }
  }
}
