//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of extensive form game representation
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

#include "base/base.h"
#include "math/rational.h"

#include "efg.h"
#include "efgutils.h"
#include "efstrat.h"
#include "actiter.h"

// Declarations of internal structures
#include "efgint.h"

//------------------------------------------------------------------------
//       Efg: Constructors, destructor, constructive operators
//------------------------------------------------------------------------

efgGame::efgGame(void)
  : sortisets(true), m_dirty(false), m_revision(0), 
    m_outcome_revision(-1), title("UNTITLED"),
    chance(new gbt_efg_player_rep(this, 0)), afg(0), lexicon(0)
{
  root = new gbt_efg_node_rep(this, 0);

  SortInfosets();
}

efgGame::efgGame(const efgGame &E, gbtEfgNode n /* = null */)
  : sortisets(false), m_dirty(false), m_revision(0), 
    m_outcome_revision(-1), title(E.title), comment(E.comment),
    players(E.players.Length()), outcomes(0, E.outcomes.Last()),
    chance(new gbt_efg_player_rep(this, 0)),
    afg(0), lexicon(0) 
{
  for (int i = 1; i <= players.Length(); i++)  {
    (players[i] = new gbt_efg_player_rep(this, i))->m_label = E.players[i]->m_label;
    for (int j = 1; j <= E.players[i]->m_infosets.Length(); j++)   {
      gbt_efg_infoset_rep *s = new gbt_efg_infoset_rep(players[i], j,
						       E.players[i]->m_infosets[j]->m_actions.Length());
      s->m_label = E.players[i]->m_infosets[j]->m_label;
      for (int k = 1; k <= s->m_actions.Length(); k++) {
	s->m_actions[k]->m_label = E.players[i]->m_infosets[j]->m_actions[k]->m_label;
      }
      players[i]->m_infosets.Append(s);
    }
  }

  for (int i = 1; i <= E.GetChance().NumInfosets(); i++)   {
    gbt_efg_infoset_rep *t = E.chance->m_infosets[i];
    gbt_efg_infoset_rep *s = new gbt_efg_infoset_rep(chance, i, t->m_actions.Length());
    s->m_label = t->m_label;
    for (int act = 1; act <= s->m_chanceProbs.Length(); act++) {
      s->m_chanceProbs[act] = t->m_chanceProbs[act];
      s->m_actions[act]->m_label = t->m_actions[act]->m_label;
    }
    chance->m_infosets.Append(s);
  }

  for (int outc = 1; outc <= E.NumOutcomes(); outc++)  {
    outcomes[outc] = new gbt_efg_outcome_rep(this, outc);
    outcomes[outc]->m_label = E.outcomes[outc]->m_label;
    outcomes[outc]->m_payoffs = E.outcomes[outc]->m_payoffs;
  }

  root = new gbt_efg_node_rep(this, 0);
  CopySubtree(root, (n.rep ? n.rep : E.root));
  
  if (n.rep)   {
    for (int pl = 1; pl <= players.Length(); pl++)  {
      for (int i = 1; i <= players[pl]->m_infosets.Length(); i++)  {
	if (players[pl]->m_infosets[i]->m_members.Length() == 0)
	  delete players[pl]->m_infosets.Remove(i--);
      }
    }
  }

  sortisets = true;
  SortInfosets();
}

#include "lexicon.h"

efgGame::~efgGame()
{
  delete root;
  delete chance;

  for (int i = 1; i <= players.Length(); delete players[i++]);
  for (int i = 1; i <= outcomes.Last(); delete outcomes[i++]);

  if (lexicon)   delete lexicon;
  lexicon = 0;
}

//------------------------------------------------------------------------
//                  Efg: Private member functions
//------------------------------------------------------------------------

void efgGame::DeleteLexicon(void) const
{
  if (lexicon)   delete lexicon;
  lexicon = 0;
}

gbtEfgInfoset efgGame::GetInfosetByIndex(gbtEfgPlayer p, int index) const
{
  for (int i = 1; i <= p.rep->m_infosets.Length(); i++) {
    if (p.rep->m_infosets[i]->m_id == index) {
      return p.rep->m_infosets[i];
    }
  }
  return 0;
}

gbtEfgOutcome efgGame::GetOutcomeByIndex(int index) const
{
  for (int i = 1; i <= outcomes.Last(); i++) {
    if (outcomes[i]->m_id == index)  {
      return outcomes[i];
    }
  }

  return 0;
}

void efgGame::Reindex(void)
{
  int i;

  for (i = 1; i <= players.Length(); i++)  {
    gbt_efg_player_rep *p = players[i];
    for (int j = 1; j <= p->m_infosets.Length(); j++) {
      p->m_infosets[j]->m_id = j;
    }
  }

  for (i = 1; i <= outcomes.Last(); i++) {
    outcomes[i]->m_id = i;
  }
}


void efgGame::NumberNodes(gbt_efg_node_rep *n, int &index)
{
  n->m_id = index++;
  for (int child = 1; child <= n->m_children.Length();
       NumberNodes(n->m_children[child++], index));
} 

void efgGame::SortInfosets(void)
{
  if (!sortisets)  return;

  int pl;

  for (pl = 0; pl <= players.Length(); pl++)  {
    gList<gbtEfgNode> nodes;

    Nodes(*this, nodes);

    gbt_efg_player_rep *player = (pl) ? players[pl] : chance;

    int i, isets = 0;

    // First, move all empty infosets to the back of the list so
    // we don't "lose" them
    int foo = player->m_infosets.Length();
    i = 1;
    while (i < foo)   {
      if (player->m_infosets[i]->m_members.Length() == 0)  {
	gbt_efg_infoset_rep *bar = player->m_infosets[i];
	player->m_infosets[i] = player->m_infosets[foo];
	player->m_infosets[foo--] = bar;
      }
      else
	i++;
    }

    // This will give empty infosets their proper number; the nonempty
    // ones will be renumbered by the next loop
    for (i = 1; i <= player->m_infosets.Length(); i++)
      if (player->m_infosets[i]->m_members.Length() == 0)
	player->m_infosets[i]->m_id = i;
      else
	player->m_infosets[i]->m_id = 0;
  
    for (i = 1; i <= nodes.Length(); i++)  {
      gbtEfgNode n = nodes[i];
      if (n.GetPlayer() == player && n.GetInfoset().GetId() == 0)  {
	n.rep->m_infoset->m_id = ++isets;
	player->m_infosets[isets] = n.rep->m_infoset;
      }
    }  
  }

  // Now, we sort the nodes within the infosets
  
  gList<gbtEfgNode> nodes;
  Nodes(*this, nodes);

  for (pl = 0; pl <= players.Length(); pl++)  {
    gbt_efg_player_rep *player = (pl) ? players[pl] : chance;

    for (int iset = 1; iset <= player->m_infosets.Length(); iset++)  {
      gbt_efg_infoset_rep *s = player->m_infosets[iset];
      for (int i = 1, j = 1; i <= nodes.Length(); i++)  {
	if (nodes[i].rep->m_infoset == s)
	  s->m_members[j++] = nodes[i].rep;
      }
    }
  }

  int nodeindex = 1;
  NumberNodes(root, nodeindex);
}
  
gbt_efg_infoset_rep *efgGame::CreateInfoset(int n, gbtEfgPlayer p, int br)
{
  gbt_efg_infoset_rep *s = new gbt_efg_infoset_rep(p.rep, n, br);
  p.rep->m_infosets.Append(s);
  return s;
}

gbtEfgOutcome efgGame::CreateOutcomeByIndex(int index)
{
  NewOutcome(index);
  return outcomes[outcomes.Last()];
}

void efgGame::CopySubtree(gbt_efg_node_rep *n, gbt_efg_node_rep *m)
{
  n->m_label = m->m_label;

  if (m->m_gameroot == m)
    n->m_gameroot = n;

  if (m->m_outcome) {
    n->m_outcome = m->m_outcome;
  }

  if (m->m_infoset)   {
    gbt_efg_player_rep *p;
    if (m->m_infoset->m_player->m_id) {
      p = players[m->m_infoset->m_player->m_id];
    }
    else {
      p = chance;
    }

    gbt_efg_infoset_rep *s = p->m_infosets[m->m_infoset->m_id];
    AppendNode(n, s);

    for (int i = 1; i <= n->m_children.Length(); i++)
      CopySubtree(n->m_children[i], m->m_children[i]);
  }
}

//------------------------------------------------------------------------
//               Efg: Title access and manipulation
//------------------------------------------------------------------------

void efgGame::SetTitle(const gText &s)
{
  title = s; 
  m_revision++;
  m_dirty = true;
}

const gText &efgGame::GetTitle(void) const
{ return title; }

void efgGame::SetComment(const gText &s)
{
  comment = s;
  m_revision++;
  m_dirty = true;
}

const gText &efgGame::GetComment(void) const
{ return comment; }
  

//------------------------------------------------------------------------
//                    Efg: Writing data files
//------------------------------------------------------------------------

void efgGame::WriteEfgFile(gOutput &f, gbt_efg_node_rep *n) const
{
  if (n->m_children.Length() == 0)   {
    f << "t \"" << EscapeQuotes(n->m_label) << "\" ";
    if (n->m_outcome)  {
      f << n->m_outcome->m_id << " \"" <<
	EscapeQuotes(n->m_outcome->m_label) << "\" ";
      f << "{ ";
      for (int pl = 1; pl <= NumPlayers(); pl++)  {
	f << n->m_outcome->m_payoffs[pl];
	if (pl < NumPlayers())
	  f << ", ";
	else
	  f << " }\n";
      }
    }
    else
      f << "0\n";
  }

  else if (n->m_infoset->m_player->m_id)   {
    f << "p \"" << EscapeQuotes(n->m_label) << "\" " <<
      n->m_infoset->m_player->m_id << ' ';
    f << n->m_infoset->m_id << " \"" <<
      EscapeQuotes(n->m_infoset->m_label) << "\" ";
    n->m_infoset->PrintActions(f);
    f << " ";
    if (n->m_outcome)  {
      f << n->m_outcome->m_id << " \"" <<
	EscapeQuotes(n->m_outcome->m_label) << "\" ";
      f << "{ ";
      for (int pl = 1; pl <= NumPlayers(); pl++)  {
	f << n->m_outcome->m_payoffs[pl];
	if (pl < NumPlayers())
	  f << ", ";
	else
	  f << " }\n";
      }
    }
    else
      f << "0\n";
  }

  else   {    // chance node
    f << "c \"" << n->m_label << "\" ";
    f << n->m_infoset->m_id << " \"" <<
      EscapeQuotes(n->m_infoset->m_label) << "\" ";
    n->m_infoset->PrintActions(f);
    f << " ";
    if (n->m_outcome)  {
      f << n->m_outcome->m_id << " \"" <<
	EscapeQuotes(n->m_outcome->m_label) << "\" ";
      f << "{ ";
      for (int pl = 1; pl <= NumPlayers(); pl++)  {
	f << n->m_outcome->m_payoffs[pl];
        if (pl < NumPlayers()) 
          f << ", ";
        else
          f << " }\n";
      }
    }
    else
      f << "0\n";
  }

  for (int i = 1; i <= n->m_children.Length(); i++)
    WriteEfgFile(f, n->m_children[i]);
}

void efgGame::WriteEfgFile(gOutput &p_file, int p_nDecimals) const
{
  int oldPrecision = p_file.GetPrec();
  p_file.SetPrec(p_nDecimals);

  try {
    p_file << "EFG 2 R";
    p_file << " \"" << EscapeQuotes(title) << "\" { ";
    for (int i = 1; i <= players.Length(); i++) {
      p_file << '"' << EscapeQuotes(players[i]->m_label) << "\" ";
    }
    p_file << "}\n";
    p_file << "\"" << EscapeQuotes(comment) << "\"\n\n";

    WriteEfgFile(p_file, root);
    p_file.SetPrec(oldPrecision);
    m_revision++;
    m_dirty = false;
  }
  catch (...) {
    p_file.SetPrec(oldPrecision);
    throw;
  }
}


//------------------------------------------------------------------------
//                    Efg: General data access
//------------------------------------------------------------------------

int efgGame::NumPlayers(void) const
{ return players.Length(); }

gbtEfgPlayer efgGame::NewPlayer(void)
{
  m_revision++;
  m_dirty = true;

  gbt_efg_player_rep *ret = new gbt_efg_player_rep(this, players.Length() + 1);
  players.Append(ret);

  for (int outc = 1; outc <= outcomes.Last();
       outcomes[outc++]->m_payoffs.Append(0));
  for (int outc = 1; outc <= outcomes.Last();
       outcomes[outc++]->m_doublePayoffs.Append(0));
  DeleteLexicon();
  return ret;
}

gbtEfgPlayer efgGame::GetPlayer(int p_player) const
{
  if (p_player == 0) {
    return chance;
  }
  else {
    return players[p_player];
  }
}

int efgGame::NumOutcomes(void) const
{ return outcomes.Last(); }

gbtEfgOutcome efgGame::NewOutcome(void)
{
  m_revision++;
  m_dirty = true;
  return NewOutcome(outcomes.Last() + 1);
}

void efgGame::DeleteOutcome(gbtEfgOutcome &p_outcome)
{
  m_revision++;
  m_dirty = true;

  root->DeleteOutcome(p_outcome.rep);
  delete outcomes.Remove(outcomes.Find(p_outcome.rep));
  DeleteLexicon();
}

gbtEfgOutcome efgGame::GetOutcome(int p_index) const
{
  return outcomes[p_index];
}

void efgGame::SetPayoff(gbtEfgOutcome p_outcome, int pl, 
			const gNumber &value)
{
  if (p_outcome.IsNull()) {
    return;
  }

  m_revision++;
  m_dirty = true;
  p_outcome.rep->m_payoffs[pl] = value;
  p_outcome.rep->m_doublePayoffs[pl] = (double) value;
}

gNumber efgGame::Payoff(const gbtEfgNode &p_node,
			const gbtEfgPlayer &p_player) const
{
  if (p_player.IsNull()) {
    return gNumber(0);
  }

  return ((p_node.rep->m_outcome) ? 
	  p_node.rep->m_outcome->m_payoffs[p_player.rep->m_id] : gNumber(0));
}

gArray<gNumber> efgGame::Payoff(const gbtEfgOutcome &p_outcome) const
{
  if (p_outcome.IsNull()) {
    gArray<gNumber> ret(players.Length());
    for (int i = 1; i <= ret.Length(); ret[i++] = 0);
    return ret;
  }
  else {
    return p_outcome.rep->m_payoffs;
  }
}

bool efgGame::IsConstSum(void) const
{
  int pl, index;
  gNumber cvalue = (gNumber) 0;

  if (outcomes.Last() == 0)  return true;

  for (pl = 1; pl <= players.Length(); pl++)
    cvalue += outcomes[1]->m_payoffs[pl];

  for (index = 2; index <= outcomes.Last(); index++)  {
    gNumber thisvalue(0);

    for (pl = 1; pl <= players.Length(); pl++)
      thisvalue += outcomes[index]->m_payoffs[pl];

    if (thisvalue > cvalue || thisvalue < cvalue)
      return false;
  }

  return true;
}

gNumber efgGame::MinPayoff(int pl) const
{
  int index, p, p1, p2;
  gNumber minpay;

  if (NumOutcomes() == 0)  return 0;

  if(pl) { p1=p2=pl;}
  else {p1=1;p2=players.Length();}

  minpay = outcomes[1]->m_payoffs[p1];

  for (index = 1; index <= outcomes.Last(); index++)  {
    for (p = p1; p <= p2; p++)
      if (outcomes[index]->m_payoffs[p] < minpay)
	minpay = outcomes[index]->m_payoffs[p];
  }
  return minpay;
}

gNumber efgGame::MaxPayoff(int pl) const
{
  int index, p, p1, p2;
  gNumber maxpay;

  if (NumOutcomes() == 0)  return 0;

  if(pl) { p1=p2=pl;}
  else {p1=1;p2=players.Length();}

  maxpay = outcomes[1]->m_payoffs[p1];

  for (index = 1; index <= outcomes.Last(); index++)  {
    for (p = p1; p <= p2; p++)
      if (outcomes[index]->m_payoffs[p] > maxpay)
	maxpay = outcomes[index]->m_payoffs[p];
  }
  return maxpay;
}

gbtEfgNode efgGame::RootNode(void) const
{ return root; }

gbtEfgOutcome efgGame::NewOutcome(int index)
{
  m_revision++;
  m_dirty = true;
  outcomes.Append(new gbt_efg_outcome_rep(this, index));
  return outcomes[outcomes.Last()];
} 

//------------------------------------------------------------------------
//                     Efg: Operations on players
//------------------------------------------------------------------------

gbtEfgPlayer efgGame::GetChance(void) const
{
  return chance;
}

gbtEfgInfoset efgGame::AppendNode(gbtEfgNode n, gbtEfgPlayer p, int count)
{
  if (n.IsNull() || p.IsNull() || count == 0)
    throw Exception();

  m_revision++;
  m_dirty = true;

  if (n.NumChildren() == 0)   {
    n.rep->m_infoset = CreateInfoset(p.rep->m_infosets.Length() + 1,
				     p.rep, count);
    n.rep->m_infoset->m_members.Append(n.rep);
    while (count--) {
      n.rep->m_children.Append(new gbt_efg_node_rep(this, n.rep));
    }
  }

  DeleteLexicon();
  SortInfosets();
  return n.rep->m_infoset;
}  

gbtEfgInfoset efgGame::AppendNode(gbtEfgNode n, gbtEfgInfoset s)
{
  if (n.IsNull() || s.IsNull())   throw Exception();
  
  // Can't bridge subgames...
  if (s.rep->m_members.Length() > 0 &&
      n.rep->m_gameroot != s.rep->m_members[1]->m_gameroot) {
    return 0;
  }

  if (n.rep->m_children.Length() == 0)   {
    m_revision++;
    m_dirty = true;
    n.rep->m_infoset = s.rep;
    s.rep->m_members.Append(n.rep);
    for (int i = 1; i <= s.rep->m_actions.Length(); i++) {
      n.rep->m_children.Append(new gbt_efg_node_rep(this, n.rep));
    }
  }

  DeleteLexicon();
  SortInfosets();
  return s;
}
  
gbtEfgNode efgGame::DeleteNode(gbtEfgNode n, gbtEfgNode keep)
{
  if (n.IsNull() || keep.IsNull())   throw Exception();

  if (keep.rep->m_parent != n.rep)   return n;

  if (n.rep->m_gameroot == n.rep) {
    MarkSubgame(keep.rep, keep.rep);
  }

  m_revision++;
  m_dirty = true;
  // turn infoset sorting off during tree deletion -- problems will occur
  sortisets = false;

  n.rep->m_children.Remove(n.rep->m_children.Find(keep.rep));
  DeleteTree(n.rep);
  keep.rep->m_parent = n.rep->m_parent;
  if (n.rep->m_parent) {
    n.rep->m_parent->m_children[n.rep->m_parent->m_children.Find(n.rep)] = keep.rep;
  }
  else {
    root = keep.rep;
  }

  delete n.rep;
  DeleteLexicon();

  sortisets = true;

  SortInfosets();
  return keep;
}

gbtEfgInfoset efgGame::InsertNode(gbtEfgNode n, gbtEfgPlayer p, int count)
{
  if (n.IsNull() || p.IsNull() || count <= 0)  {
    throw Exception();
  }

  m_revision++;
  m_dirty = true;

  gbt_efg_node_rep *m = new gbt_efg_node_rep(this, n.rep->m_parent);
  m->m_infoset = CreateInfoset(p.rep->m_infosets.Length() + 1, p, count);
  m->m_infoset->m_members.Append(m);
  if (n.rep->m_parent) {
    n.rep->m_parent->m_children[n.rep->m_parent->m_children.Find(n.rep)] = m;
  }
  else {
    root = m;
  }
  m->m_children.Append(n.rep);
  n.rep->m_parent = m;
  while (--count) {
    m->m_children.Append(new gbt_efg_node_rep(this, m));
  }

  DeleteLexicon();
  SortInfosets();
  return m->m_infoset;
}

gbtEfgInfoset efgGame::InsertNode(gbtEfgNode n, gbtEfgInfoset s)
{
  if (n.IsNull() || s.IsNull())  throw Exception();

  // can't bridge subgames
  if (s.rep->m_members.Length() > 0 &&
      n.rep->m_gameroot != s.rep->m_members[1]->m_gameroot)
    return 0;
  
  m_revision++;
  m_dirty = true;

  gbt_efg_node_rep *m = new gbt_efg_node_rep(this, n.rep->m_parent);
  m->m_infoset = s.rep;
  s.rep->m_members.Append(m);
  if (n.rep->m_parent)
    n.rep->m_parent->m_children[n.rep->m_parent->m_children.Find(n.rep)] = m;
  else
    root = m;
  m->m_children.Append(n.rep);
  n.rep->m_parent = m;
  int count = s.rep->m_actions.Length();
  while (--count) {
    m->m_children.Append(new gbt_efg_node_rep(this, m));
  }

  DeleteLexicon();
  SortInfosets();
  return m->m_infoset;
}

gbtEfgInfoset efgGame::CreateInfoset(gbtEfgPlayer p, int br)
{
  if (p.IsNull() || p.GetGame() != this) {
    throw Exception();
  }
  m_revision++;
  m_dirty = true;
  return CreateInfoset(p.rep->m_infosets.Length() + 1, p.rep, br);
}

gbtEfgInfoset efgGame::JoinInfoset(gbtEfgInfoset s, gbtEfgNode n)
{
  if (n.IsNull() || s.IsNull())  throw Exception();

  // can't bridge subgames
  if (s.rep->m_members.Length() > 0 &&
      n.rep->m_gameroot != s.rep->m_members[1]->m_gameroot)
    return 0;
  
  if (!n.rep->m_infoset)   return 0; 
  if (n.rep->m_infoset == s.rep)   return s;
  if (s.rep->m_actions.Length() != n.rep->m_children.Length())  {
    return n.rep->m_infoset;
  }

  m_revision++;
  m_dirty = true;

  gbt_efg_infoset_rep *t = n.rep->m_infoset;

  t->m_members.Remove(t->m_members.Find(n.rep));
  s.rep->m_members.Append(n.rep);

  n.rep->m_infoset = s.rep;

  DeleteLexicon();
  SortInfosets();
  return s;
}

gbtEfgInfoset efgGame::LeaveInfoset(gbtEfgNode n)
{
  if (n.IsNull())  throw Exception();

  if (!n.rep->m_infoset)   return 0;

  gbt_efg_infoset_rep *s = n.rep->m_infoset;
  if (s->m_members.Length() == 1)   return s;

  m_revision++;
  m_dirty = true;

  gbt_efg_player_rep *p = s->m_player;
  s->m_members.Remove(s->m_members.Find(n.rep));
  n.rep->m_infoset = CreateInfoset(p->m_infosets.Length() + 1, p,
				   n.rep->m_children.Length());
  n.rep->m_infoset->m_label = s->m_label;
  n.rep->m_infoset->m_members.Append(n.rep);
  for (int i = 1; i <= s->m_actions.Length(); i++) {
    n.rep->m_infoset->m_actions[i]->m_label = s->m_actions[i]->m_label;
  }

  DeleteLexicon();
  SortInfosets();
  return n.rep->m_infoset;
}

gbtEfgInfoset efgGame::SplitInfoset(gbtEfgNode n)
{
  if (n.IsNull())  throw Exception();

  if (!n.rep->m_infoset)   return 0;

  gbt_efg_infoset_rep *s = n.rep->m_infoset;
  if (s->m_members.Length() == 1)   return s;

  m_revision++;
  m_dirty = true;

  gbt_efg_player_rep *p = s->m_player;
  gbt_efg_infoset_rep *ns = CreateInfoset(p->m_infosets.Length() + 1, p,
					  n.rep->m_children.Length());
  ns->m_label = s->m_label;
  for (int i = s->m_members.Length(); i > s->m_members.Find(n.rep); i--)   {
    gbt_efg_node_rep *nn = s->m_members.Remove(i);
    ns->m_members.Append(nn);
    nn->m_infoset = ns;
  }
  for (int i = 1; i <= s->m_actions.Length(); i++) {
    ns->m_actions[i]->m_label = s->m_actions[i]->m_label;
    if (p == chance) {
      ns->m_chanceProbs[i] = s->m_chanceProbs[i];
    }
  }
  DeleteLexicon();
  SortInfosets();
  return n.rep->m_infoset;
}

gbtEfgInfoset efgGame::MergeInfoset(gbtEfgInfoset to, gbtEfgInfoset from)
{
  if (to.IsNull() || from.IsNull())  throw Exception();

  if (to == from ||
      to.rep->m_actions.Length() != from.rep->m_actions.Length())  {
    return from;
  }

  if (to.rep->m_members[1]->m_gameroot != from.rep->m_members[1]->m_gameroot) 
    return from;

  m_revision++;
  m_dirty = true;

  to.rep->m_members += from.rep->m_members;
  for (int i = 1; i <= from.rep->m_members.Length(); i++) {
    from.rep->m_members[i]->m_infoset = to.rep;
  }

  from.rep->m_members.Flush();

  DeleteLexicon();
  SortInfosets();
  return to;
}

bool efgGame::DeleteEmptyInfoset(gbtEfgInfoset s)
{
  if (s.IsNull())  throw Exception();

  if (s.NumMembers() > 0)   return false;

  m_revision++;
  m_dirty = true;
  s.rep->m_player->m_infosets.Remove(s.rep->m_player->m_infosets.Find(s.rep));
  delete s.rep;

  return true;
}

void efgGame::DeleteEmptyInfosets(void)
{
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    for (int iset = 1; iset <= NumInfosets()[pl]; iset++) {
      if (DeleteEmptyInfoset(players[pl]->m_infosets[iset])) {
        iset--;
      }
    }
  }
} 

gbtEfgInfoset efgGame::SwitchPlayer(gbtEfgInfoset s, gbtEfgPlayer p)
{
  if (s.IsNull() || p.IsNull()) {
    throw Exception();
  }
  if (s.GetPlayer().IsChance() || p.IsChance()) {
    throw Exception();
  }
  
  if (s.rep->m_player == p.rep)   return s;

  m_revision++;
  m_dirty = true;
  s.rep->m_player->m_infosets.Remove(s.rep->m_player->m_infosets.Find(s.rep));
  s.rep->m_player = p.rep;
  p.rep->m_infosets.Append(s.rep);

  DeleteLexicon();
  SortInfosets();
  return s;
}

void efgGame::CopySubtree(gbt_efg_node_rep *src, gbt_efg_node_rep *dest,
			  gbt_efg_node_rep *stop)
{
  if (src == stop) {
    dest->m_outcome = src->m_outcome;
    return;
  }

  if (src->m_children.Length())  {
    AppendNode(dest, src->m_infoset);
    for (int i = 1; i <= src->m_children.Length(); i++)
      CopySubtree(src->m_children[i], dest->m_children[i], stop);
  }

  dest->m_label = src->m_label;
  dest->m_outcome = src->m_outcome;
}

//
// MarkSubtree: sets the Node::mark flag on all children of p_node
//
void efgGame::MarkSubtree(gbt_efg_node_rep *p_node)
{
  p_node->m_mark = true;
  for (int i = 1; i <= p_node->m_children.Length(); i++) {
    MarkSubtree(p_node->m_children[i]);
  }
}

//
// UnmarkSubtree: clears the Node::mark flag on all children of p_node
//
void efgGame::UnmarkSubtree(gbt_efg_node_rep *p_node)
{
  p_node->m_mark = false;
  for (int i = 1; i <= p_node->m_children.Length(); i++) {
    UnmarkSubtree(p_node->m_children[i]);
  }
}

void efgGame::Reveal(gbtEfgInfoset where, gbtEfgPlayer who)
{
  if (where.rep->m_actions.Length() <= 1)  {
    // only one action; nothing to reveal!
    return;
  }

  UnmarkSubtree(root);  // start with a clean tree
  
  m_revision++;
  m_dirty = true;

  for (int i = 1; i <= where.rep->m_actions.Length(); i++) {
    for (int j = 1; j <= where.rep->m_members.Length(); j++) { 
      MarkSubtree(where.rep->m_members[j]->m_children[i]);
    }

    for (int k = 1; k <= who.rep->m_infosets.Length(); k++) {
      // iterate over each member of information set 'k'
      // make copy of members to iterate correctly 
      // (since the information set may be changed in the process)
      const gArray<gbt_efg_node_rep *> &members = 
	who.rep->m_infosets[k]->m_members;
      gbtEfgInfoset newiset = 0;
      
      for (int m = 1; m <= members.Length(); m++) {
	gbt_efg_node_rep *n = members[m];
	if (n->m_mark) {
	  // If node is marked, is descendant of action 'i'
	  n->m_mark = false;   // unmark so tree is clean at end
	  if (newiset.IsNull()) {
	    newiset = LeaveInfoset(n);
	  }
	  else {
	    JoinInfoset(newiset, n);
	  }
	} 
      }
    }
  }

  Reindex();
}

gbtEfgNode efgGame::CopyTree(gbtEfgNode src, gbtEfgNode dest)
{
  if (src.IsNull() || dest.IsNull())  throw Exception();
  if (src == dest || dest.rep->m_children.Length())   return src;
  if (src.rep->m_gameroot != dest.rep->m_gameroot)  return src;

  if (src.rep->m_children.Length())  {
    m_revision++;
    m_dirty = true;

    AppendNode(dest, src.rep->m_infoset);
    for (int i = 1; i <= src.rep->m_children.Length(); i++) {
      CopySubtree(src.rep->m_children[i], dest.rep->m_children[i], dest.rep);
    }

    DeleteLexicon();
    SortInfosets();
  }

  return dest;
}

gbtEfgNode efgGame::MoveTree(gbtEfgNode src, gbtEfgNode dest)
{
  if (src.IsNull() || dest.IsNull())  throw Exception();
  if (src == dest || dest.rep->m_children.Length() || src.IsPredecessor(dest))
    return src;
  if (src.rep->m_gameroot != dest.rep->m_gameroot)  return src;

  m_revision++;
  m_dirty = true;

  if (src.rep->m_parent == dest.rep->m_parent) {
    int srcChild = src.rep->m_parent->m_children.Find(src.rep);
    int destChild = src.rep->m_parent->m_children.Find(dest.rep);
    src.rep->m_parent->m_children[srcChild] = dest.rep;
    src.rep->m_parent->m_children[destChild] = dest.rep;
  }
  else {
    gbt_efg_node_rep *parent = src.rep->m_parent; 
    parent->m_children[parent->m_children.Find(src.rep)] = dest.rep;
    dest.rep->m_parent->m_children[dest.rep->m_parent->m_children.Find(dest.rep)] = src.rep;
    src.rep->m_parent = dest.rep->m_parent;
    dest.rep->m_parent = parent;
  }

  dest.rep->m_label = "";
  dest.rep->m_outcome = 0;
  
  DeleteLexicon();
  SortInfosets();
  return dest;
}

gbtEfgNode efgGame::DeleteTree(gbtEfgNode n)
{
  if (n.IsNull())  throw Exception();

  m_revision++;
  m_dirty = true;

  while (n.NumChildren() > 0)   {
    DeleteTree(n.rep->m_children[1]);
    delete n.rep->m_children.Remove(1);
  }
  
  if (n.rep->m_infoset)  {
    n.rep->m_infoset->m_members.Remove(n.rep->m_infoset->m_members.Find(n.rep));
    n.rep->m_infoset = 0;
  }
  n.rep->m_outcome = 0;
  n.rep->m_label = "";

  DeleteLexicon();
  SortInfosets();
  return n;
}

gbtEfgAction efgGame::InsertAction(gbtEfgInfoset s)
{
  if (s.IsNull())  throw Exception();

  m_revision++;
  m_dirty = true;
  gbtEfgAction action = s.InsertAction(s.NumActions() + 1);
  for (int i = 1; i <= s.NumMembers(); i++) {
    s.rep->m_members[i]->m_children.Append(new gbt_efg_node_rep(this, s.rep->m_members[i]));
  }
  DeleteLexicon();
  SortInfosets();
  return action;
}

gbtEfgAction efgGame::InsertAction(gbtEfgInfoset s, const gbtEfgAction &a)
{
  if (a.IsNull() || s.IsNull())  throw Exception();

  m_revision++;
  m_dirty = true;

  int where;
  for (where = 1; (where <= s.rep->m_actions.Length() &&
		   s.rep->m_actions[where] != a.rep);
       where++);
  if (where > s.rep->m_actions.Length())   return 0;
  gbtEfgAction action = s.InsertAction(where);
  for (int i = 1; i <= s.NumActions(); i++)
    s.rep->m_members[i]->m_children.Insert(new gbt_efg_node_rep(this, s.rep->m_members[i]), where);

  DeleteLexicon();
  SortInfosets();
  return action;
}

gbtEfgInfoset efgGame::DeleteAction(gbtEfgInfoset s, const gbtEfgAction &a)
{
  if (a.IsNull() || s.IsNull())  throw Exception();

  m_revision++;
  m_dirty = true;
  int where;
  for (where = 1; (where <= s.rep->m_actions.Length() &&
		   s.rep->m_actions[where] != a.rep);
       where++);
  if (where > s.rep->m_actions.Length() || s.rep->m_actions.Length() == 1)
    return s;
  s.RemoveAction(where);
  for (int i = 1; i <= s.rep->m_members.Length(); i++)   {
    DeleteTree(s.rep->m_members[i]->m_children[where]);
    delete s.rep->m_members[i]->m_children.Remove(where);
  }
  DeleteLexicon();
  SortInfosets();
  return s;
}

void efgGame::SetChanceProb(gbtEfgInfoset infoset,
			    int act, const gNumber &value)
{
  if (infoset.IsChanceInfoset()) {
    m_revision++;
    m_dirty = true;
    infoset.SetChanceProb(act, value);
  }
}

//---------------------------------------------------------------------
//                     Subgame-related functions
//---------------------------------------------------------------------

void efgGame::MarkTree(gbt_efg_node_rep *n, gbt_efg_node_rep *base)
{
  n->m_ptr = base;
  for (int i = 1; i <= n->m_children.Length(); i++) {
    MarkTree(n->m_children[i], base);
  }
}

bool efgGame::CheckTree(gbt_efg_node_rep *n, gbt_efg_node_rep *base)
{
  int i;

  if (n->m_children.Length() == 0)   return true;

  for (i = 1; i <= n->m_children.Length(); i++)
    if (!CheckTree(n->m_children[i], base))  return false;

  if (n->m_infoset->m_player->m_id == 0)   return true;

  for (i = 1; i <= n->m_infoset->m_members.Length(); i++)
    if (n->m_infoset->m_members[i]->m_ptr != base)
      return false;

  return true;
}

bool efgGame::IsLegalSubgame(const gbtEfgNode &n)
{
  if (n.NumChildren() == 0)  
    return false;

  MarkTree(n.rep, n.rep);
  return CheckTree(n.rep, n.rep);
}

bool efgGame::MarkSubgame(gbtEfgNode n)
{
  if (n.rep->m_gameroot == n.rep) return true;

  if (n.rep->m_gameroot != n.rep && IsLegalSubgame(n))  {
    n.rep->m_gameroot = 0;
    MarkSubgame(n.rep, n.rep);
    return true;
  }

  return false;
}

void efgGame::UnmarkSubgame(gbtEfgNode n)
{
  if (n.rep->m_gameroot == n.rep && n.rep->m_parent)  {
    n.rep->m_gameroot = 0;
    MarkSubgame(n.rep, n.rep->m_parent->m_gameroot);
  }
}
  

void efgGame::MarkSubgame(gbt_efg_node_rep *n, gbt_efg_node_rep *base)
{
  if (n->m_gameroot == n)  return;
  n->m_gameroot = base;
  for (int i = 1; i <= n->m_children.Length(); i++)
    MarkSubgame(n->m_children[i], base);
}

void efgGame::MarkSubgames(void)
{
  gList<gbtEfgNode> subgames;
  LegalSubgameRoots(*this, subgames);

  for (int i = 1; i <= subgames.Length(); i++)  {
    subgames[i].rep->m_gameroot = 0;
    MarkSubgame(subgames[i].rep, subgames[i].rep);
  }
}

void efgGame::UnmarkSubgames(gbtEfgNode n)
{
  if (n.NumChildren() == 0)   return;

  for (int i = 1; i <= n.NumChildren(); i++)
    UnmarkSubgames(n.GetChild(i));
  
  if (n.rep->m_gameroot == n.rep && n.rep->m_parent)  {
    n.rep->m_gameroot = 0;
    MarkSubgame(n.rep, n.rep->m_parent->m_gameroot);
  }
}


int efgGame::ProfileLength(void) const
{
  int sum = 0;

  for (int i = 1; i <= players.Length(); i++) {
    for (int j = 1; j <= players[i]->m_infosets.Length(); j++) {
      sum += players[i]->m_infosets[j]->m_actions.Length();
    }
  }

  return sum;
}

gArray<int> efgGame::NumInfosets(void) const
{
  gArray<int> foo(players.Length());
  
  for (int i = 1; i <= foo.Length(); i++) {
    foo[i] = players[i]->m_infosets.Length();
  }

  return foo;
}

int efgGame::NumPlayerInfosets(void) const
{
  int answer(0);
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    answer +=  players[pl]->m_infosets.Length();
  }
  return answer;
}

int efgGame::NumChanceInfosets(void) const
{
  return chance->m_infosets.Length();
}

int efgGame::TotalNumInfosets(void) const
{
  return NumPlayerInfosets() + NumChanceInfosets();
}

gPVector<int> efgGame::NumActions(void) const
{
  gArray<int> foo(players.Length());
  for (int i = 1; i <= players.Length(); i++) {
    foo[i] = players[i]->m_infosets.Length();
  }

  gPVector<int> bar(foo);
  for (int i = 1; i <= players.Length(); i++) {
    for (int j = 1; j <= players[i]->m_infosets.Length(); j++) {
      bar(i, j) = players[i]->m_infosets[j]->m_actions.Length();
    }
  }

  return bar;
}  

int efgGame::NumPlayerActions(void) const
{
  int answer = 0;

  gPVector<int> nums_actions = NumActions();
  for (int i = 1; i <= NumPlayers(); i++)
    answer += nums_actions[i];
  return answer;
}

gPVector<int> efgGame::NumMembers(void) const
{
  gArray<int> foo(players.Length());

  for (int i = 1; i <= players.Length(); i++) {
    foo[i] = players[i]->m_infosets.Length();
  }

  gPVector<int> bar(foo);
  for (int i = 1; i <= players.Length(); i++) {
    for (int j = 1; j <= players[i]->m_infosets.Length(); j++) {
      bar(i, j) = players[i]->m_infosets[j]->m_members.Length();
    }
  }

  return bar;
}

//------------------------------------------------------------------------
//                       Efg: Payoff computation
//------------------------------------------------------------------------

void efgGame::Payoff(gbt_efg_node_rep *n, gNumber prob,
		     const gPVector<int> &profile,
		     gVector<gNumber> &payoff) const
{
  if (n->m_outcome)  {
    for (int i = 1; i <= players.Length(); i++)
      payoff[i] += prob * n->m_outcome->m_payoffs[i];
  }

  if (n->m_infoset && n->m_infoset->m_player->m_id == 0) {
    for (int i = 1; i <= n->m_children.Length(); i++) {
      Payoff(n->m_children[i],
	     prob * n->m_infoset->m_chanceProbs[i],
	     profile, payoff);
    }
  }
  else if (n->m_infoset) {
    Payoff(n->m_children[profile(n->m_infoset->m_player->m_id,
				 n->m_infoset->m_id)],
	   prob, profile, payoff);
  }
}

void efgGame::InfosetProbs(gbt_efg_node_rep *n, gNumber prob,
			   const gPVector<int> &profile,
			   gPVector<gNumber> &probs) const
{
  if (n->m_infoset && n->m_infoset->m_player->m_id == 0) {
    for (int i = 1; i <= n->m_children.Length(); i++) {
      InfosetProbs(n->m_children[i],
		   prob * n->m_infoset->m_chanceProbs[i],
		   profile, probs);
    }
  }
  else if (n->m_infoset)  {
    probs(n->m_infoset->m_player->m_id, n->m_infoset->m_id) += prob;
    InfosetProbs(n->m_children[profile(n->m_infoset->m_player->m_id, 
				       n->m_infoset->m_id)],
		 prob, profile, probs);
  }
}

void efgGame::Payoff(const gPVector<int> &profile, gVector<gNumber> &payoff) const
{
  ((gVector<gNumber> &) payoff).operator=((gNumber) 0);
  Payoff(root, 1, profile, payoff);
}

void efgGame::InfosetProbs(const gPVector<int> &profile,
			  gPVector<gNumber> &probs) const
{
  ((gVector<gNumber> &) probs).operator=((gNumber) 0);
  InfosetProbs(root, 1, profile, probs);
}

void efgGame::Payoff(gbt_efg_node_rep *n, gNumber prob,
		     const gArray<gArray<int> *> &profile,
		     gArray<gNumber> &payoff) const
{
  if (n->m_outcome)   {
    for (int i = 1; i <= players.Length(); i++)
      payoff[i] += prob * n->m_outcome->m_payoffs[i];
  }
  
  if (n->m_infoset && n->m_infoset->m_player->m_id == 0) {
    for (int i = 1; i <= n->m_children.Length(); i++) {
      Payoff(n->m_children[i],
	     prob * n->m_infoset->m_chanceProbs[i],
	     profile, payoff);
    }
  }
  else if (n->m_infoset) {
    Payoff(n->m_children[(*profile[n->m_infoset->m_player->m_id])[n->m_infoset->m_id]],
	   prob, profile, payoff);
  }
}

void efgGame::Payoff(const gArray<gArray<int> *> &profile,
		 gArray<gNumber> &payoff) const
{
  for (int i = 1; i <= payoff.Length(); i++)
    payoff[i] = 0;
  Payoff(root, 1, profile, payoff);
}

Nfg *efgGame::AssociatedNfg(void) const
{
  if (lexicon) {
    return lexicon->N;
  }
  else {
    return 0;
  }
}

Nfg *efgGame::AssociatedAfg(void) const
{
  return afg;
}

