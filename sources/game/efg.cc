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
#include "lexicon.h"
#include "nfg.h"

// Declarations of internal structures
#include "efgint.h"

//----------------------------------------------------------------------
//           struct gbt_efg_infoset_rep: Member functions
//----------------------------------------------------------------------

gbt_efg_game_rep::gbt_efg_game_rep(void)
  : m_refCount(1),
    sortisets(true), m_revision(0), 
    m_outcome_revision(-1), title("UNTITLED"),
    chance(new gbt_efg_player_rep(this, 0)), afg(0), lexicon(0)
{
  root = new gbt_efg_node_rep(this, 0);
}

gbt_efg_game_rep::~gbt_efg_game_rep()
{
  delete root;
  delete chance;

  for (int i = 1; i <= players.Length(); delete players[i++]);
  for (int i = 1; i <= outcomes.Last(); delete outcomes[i++]);

  if (lexicon)   delete lexicon;
  lexicon = 0;
}

void gbt_efg_game_rep::DeleteLexicon(void) 
{
  if (lexicon)  {
    delete lexicon;
    lexicon = 0;
  }
}

void gbt_efg_game_rep::NumberNodes(gbt_efg_node_rep *n, int &index)
{
  n->m_id = index++;
  for (int child = 1; child <= n->m_children.Length();
       NumberNodes(n->m_children[child++], index));
} 

void gbt_efg_game_rep::SortInfosets(void)
{
  if (!sortisets)  return;

  int pl;

  for (pl = 0; pl <= players.Length(); pl++)  {
    gList<gbtEfgNode> nodes;

    Nodes(gbtEfgGame(this), nodes);

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
  Nodes(gbtEfgGame(this), nodes);

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

//
// Place terminal node 'p_node' in information set 'p_infoset'.
// Assumes node and infoset are valid, node is terminal.
//  
void gbt_efg_game_rep::AppendMove(gbt_efg_node_rep *p_node,
				  gbt_efg_infoset_rep *p_infoset)
{
  p_node->m_infoset = p_infoset;
  p_infoset->m_members.Append(p_node);
  for (int i = p_infoset->m_actions.Length(); i; i--) {
    p_node->m_children.Append(new gbt_efg_node_rep(this, p_node));
  }
  m_revision++;
  DeleteLexicon();
  SortInfosets();
}

//
// Create a new node, a member of 'p_infoset' in the location of 'p_node'
// in the tree, and make the subtree rooted in 'p_node' the first child
// of the new node.  Assumes node and infoset are valid, node is
// nonterminal.
//
void gbt_efg_game_rep::InsertMove(gbt_efg_node_rep *p_node,
				  gbt_efg_infoset_rep *p_infoset)
{
  gbt_efg_node_rep *parent = p_node->m_parent;
  gbt_efg_node_rep *node = new gbt_efg_node_rep(this, parent);
  node->m_infoset = p_infoset;
  p_infoset->m_members.Append(node);
  if (parent) {
    parent->m_children[parent->m_children.Find(p_node)] = node;
  }
  else {
    root = node;
  }
  node->m_children.Append(p_node);
  p_node->m_parent = node;
  for (int i = p_infoset->m_actions.Length() - 1; i; i--) {
    node->m_children.Append(new gbt_efg_node_rep(this, node));
  }
  m_revision++;
  DeleteLexicon();
  SortInfosets();
}


//
// Create a new information set with id 'p_id' for player 'p_player'.
// Assumes player is not null, number of actions positive.
//  
gbt_efg_infoset_rep *gbt_efg_game_rep::NewInfoset(gbt_efg_player_rep *p_player,
						  int p_id, int p_actions)
{
  gbt_efg_infoset_rep *s = new gbt_efg_infoset_rep(p_player, p_id, p_actions);
  p_player->m_infosets.Append(s);
  m_revision++;
  return s;
}


//------------------------------------------------------------------------
//       Efg: Constructors, destructor, constructive operators
//------------------------------------------------------------------------

gbtEfgGame::gbtEfgGame(void)
  : rep(new gbt_efg_game_rep)
{
  rep->SortInfosets();
}

gbtEfgGame gbtEfgGame::Copy(gbtEfgNode n /* = null */) const
{
  gbtEfgGame efg;
  efg.rep->sortisets = false;
  efg.rep->title = rep->title;
  efg.rep->comment = rep->comment;
  efg.rep->players = gBlock<gbt_efg_player_rep *>(rep->players.Length());
  efg.rep->outcomes = gBlock<gbt_efg_outcome_rep *>(rep->outcomes.Length());
  
  for (int i = 1; i <= rep->players.Length(); i++)  {
    (efg.rep->players[i] = new gbt_efg_player_rep(efg.rep, i))->m_label = rep->players[i]->m_label;
    for (int j = 1; j <= rep->players[i]->m_infosets.Length(); j++)   {
      gbt_efg_infoset_rep *s = new gbt_efg_infoset_rep(efg.rep->players[i], j,
						       rep->players[i]->m_infosets[j]->m_actions.Length());
      s->m_label = rep->players[i]->m_infosets[j]->m_label;
      for (int k = 1; k <= s->m_actions.Length(); k++) {
	s->m_actions[k]->m_label = rep->players[i]->m_infosets[j]->m_actions[k]->m_label;
      }
      efg.rep->players[i]->m_infosets.Append(s);
    }
  }

  for (int i = 1; i <= GetChance().NumInfosets(); i++)   {
    gbt_efg_infoset_rep *t = rep->chance->m_infosets[i];
    gbt_efg_infoset_rep *s = new gbt_efg_infoset_rep(efg.rep->chance, i, 
						     t->m_actions.Length());
    s->m_label = t->m_label;
    for (int act = 1; act <= s->m_chanceProbs.Length(); act++) {
      s->m_chanceProbs[act] = t->m_chanceProbs[act];
      s->m_actions[act]->m_label = t->m_actions[act]->m_label;
    }
    efg.rep->chance->m_infosets.Append(s);
  }

  for (int outc = 1; outc <= NumOutcomes(); outc++)  {
    efg.rep->outcomes[outc] = new gbt_efg_outcome_rep(efg.rep, outc);
    efg.rep->outcomes[outc]->m_label = rep->outcomes[outc]->m_label;
    efg.rep->outcomes[outc]->m_payoffs = rep->outcomes[outc]->m_payoffs;
  }

  efg.CopySubtree(efg.rep->root, (n.rep ? n.rep : rep->root));
  
  if (n.rep)   {
    for (int pl = 1; pl <= efg.rep->players.Length(); pl++)  {
      for (int i = 1; i <= efg.rep->players[pl]->m_infosets.Length(); i++)  {
	if (efg.rep->players[pl]->m_infosets[i]->m_members.Length() == 0)
	  delete efg.rep->players[pl]->m_infosets.Remove(i--);
      }
    }
  }

  efg.rep->sortisets = true;
  efg.rep->SortInfosets();

  return efg;
}

gbtEfgGame::gbtEfgGame(const gbtEfgGame &p_efg)
  : rep(p_efg.rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtEfgGame::gbtEfgGame(gbt_efg_game_rep *p_rep)
  : rep(p_rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtEfgGame::~gbtEfgGame()
{
  if (rep && --rep->m_refCount == 0) {
    delete rep;
  }
}

gbtEfgGame &gbtEfgGame::operator=(const gbtEfgGame &p_efg)
{
  if (this != &p_efg) {
    if (rep && --rep->m_refCount == 0) {
      delete rep;
    }

    if ((rep = p_efg.rep) != 0) {
      rep->m_refCount++;
    }
  }
  return *this;
}

bool gbtEfgGame::operator==(const gbtEfgGame &p_efg) const
{
  return rep == p_efg.rep;
}

bool gbtEfgGame::operator!=(const gbtEfgGame &p_efg) const
{
  return rep != p_efg.rep;
}

//------------------------------------------------------------------------
//                  Efg: Private member functions
//------------------------------------------------------------------------

void gbtEfgGame::CopySubtree(gbt_efg_node_rep *n, gbt_efg_node_rep *m)
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
      p = rep->players[m->m_infoset->m_player->m_id];
    }
    else {
      p = rep->chance;
    }

    gbt_efg_infoset_rep *s = p->m_infosets[m->m_infoset->m_id];
    rep->AppendMove(n, s);

    for (int i = 1; i <= n->m_children.Length(); i++)
      CopySubtree(n->m_children[i], m->m_children[i]);
  }
}

//------------------------------------------------------------------------
//               Efg: Title access and manipulation
//------------------------------------------------------------------------

void gbtEfgGame::SetTitle(const gText &s)
{
  rep->title = s; 
  rep->m_revision++;
}

const gText &gbtEfgGame::GetTitle(void) const
{ return rep->title; }

void gbtEfgGame::SetComment(const gText &s)
{
  rep->comment = s;
  rep->m_revision++;
}

const gText &gbtEfgGame::GetComment(void) const
{ return rep->comment; }
  

//------------------------------------------------------------------------
//                    Efg: Writing data files
//------------------------------------------------------------------------

void gbtEfgGame::WriteEfgFile(gOutput &f, gbt_efg_node_rep *n) const
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

void gbtEfgGame::WriteEfgFile(gOutput &p_file, int p_nDecimals) const
{
  int oldPrecision = p_file.GetPrec();
  p_file.SetPrec(p_nDecimals);

  try {
    p_file << "EFG 2 R";
    p_file << " \"" << EscapeQuotes(rep->title) << "\" { ";
    for (int i = 1; i <= rep->players.Length(); i++) {
      p_file << '"' << EscapeQuotes(rep->players[i]->m_label) << "\" ";
    }
    p_file << "}\n";
    p_file << "\"" << EscapeQuotes(rep->comment) << "\"\n\n";

    WriteEfgFile(p_file, rep->root);
    p_file.SetPrec(oldPrecision);
    rep->m_revision++;
  }
  catch (...) {
    p_file.SetPrec(oldPrecision);
    throw;
  }
}


//------------------------------------------------------------------------
//                    Efg: General data access
//------------------------------------------------------------------------

long gbtEfgGame::RevisionNumber(void) const
{ return rep->m_revision; }

int gbtEfgGame::NumPlayers(void) const
{ return rep->players.Length(); }

gbtEfgPlayer gbtEfgGame::NewPlayer(void)
{
  rep->m_revision++;

  gbt_efg_player_rep *ret = new gbt_efg_player_rep(rep,
						   rep->players.Length() + 1);
  rep->players.Append(ret);

  for (int outc = 1; outc <= rep->outcomes.Last();
       rep->outcomes[outc++]->m_payoffs.Append(0));
  for (int outc = 1; outc <= rep->outcomes.Last();
       rep->outcomes[outc++]->m_doublePayoffs.Append(0));
  rep->DeleteLexicon();
  return ret;
}

gbtEfgPlayer gbtEfgGame::GetPlayer(int p_player) const
{
  if (p_player == 0) {
    return rep->chance;
  }
  else {
    return rep->players[p_player];
  }
}

int gbtEfgGame::NumOutcomes(void) const
{ return rep->outcomes.Last(); }

gbtEfgOutcome gbtEfgGame::NewOutcome(void)
{
  rep->m_revision++;
  return NewOutcome(rep->outcomes.Last() + 1);
}

void gbtEfgGame::DeleteOutcome(gbtEfgOutcome &p_outcome)
{
  rep->m_revision++;

  rep->root->DeleteOutcome(p_outcome.rep);
  delete rep->outcomes.Remove(rep->outcomes.Find(p_outcome.rep));
  rep->DeleteLexicon();
}

gbtEfgOutcome gbtEfgGame::GetOutcome(int p_index) const
{
  return rep->outcomes[p_index];
}

bool gbtEfgGame::IsConstSum(void) const
{
  int pl, index;
  gNumber cvalue = (gNumber) 0;

  if (rep->outcomes.Last() == 0)  return true;

  for (pl = 1; pl <= rep->players.Length(); pl++)
    cvalue += rep->outcomes[1]->m_payoffs[pl];

  for (index = 2; index <= rep->outcomes.Last(); index++)  {
    gNumber thisvalue(0);

    for (pl = 1; pl <= rep->players.Length(); pl++)
      thisvalue += rep->outcomes[index]->m_payoffs[pl];

    if (thisvalue > cvalue || thisvalue < cvalue)
      return false;
  }

  return true;
}

gNumber gbtEfgGame::MinPayoff(int pl) const
{
  int index, p, p1, p2;
  gNumber minpay;

  if (NumOutcomes() == 0)  return 0;

  if(pl) { p1=p2=pl;}
  else {p1=1;p2=rep->players.Length();}

  minpay = rep->outcomes[1]->m_payoffs[p1];

  for (index = 1; index <= rep->outcomes.Last(); index++)  {
    for (p = p1; p <= p2; p++)
      if (rep->outcomes[index]->m_payoffs[p] < minpay)
	minpay = rep->outcomes[index]->m_payoffs[p];
  }
  return minpay;
}

gNumber gbtEfgGame::MaxPayoff(int pl) const
{
  int index, p, p1, p2;
  gNumber maxpay;

  if (NumOutcomes() == 0)  return 0;

  if(pl) { p1=p2=pl;}
  else {p1=1;p2=rep->players.Length();}

  maxpay = rep->outcomes[1]->m_payoffs[p1];

  for (index = 1; index <= rep->outcomes.Last(); index++)  {
    for (p = p1; p <= p2; p++)
      if (rep->outcomes[index]->m_payoffs[p] > maxpay)
	maxpay = rep->outcomes[index]->m_payoffs[p];
  }
  return maxpay;
}

gbtEfgNode gbtEfgGame::RootNode(void) const
{ return rep->root; }

gbtEfgOutcome gbtEfgGame::NewOutcome(int index)
{
  rep->m_revision++;
  rep->outcomes.Append(new gbt_efg_outcome_rep(rep, index));
  return rep->outcomes[rep->outcomes.Last()];
} 

//------------------------------------------------------------------------
//                     Efg: Operations on players
//------------------------------------------------------------------------

gbtEfgPlayer gbtEfgGame::GetChance(void) const
{
  return rep->chance;
}

gbtEfgNode gbtEfgGame::DeleteNode(gbtEfgNode n, gbtEfgNode keep)
{
  if (n.IsNull() || keep.IsNull())  {
    throw gbtEfgNullObject();
  }

  if (keep.rep->m_parent != n.rep)   return n;

  if (n.rep->m_gameroot == n.rep) {
    MarkSubgame(keep.rep, keep.rep);
  }

  rep->m_revision++;
  // turn infoset sorting off during tree deletion -- problems will occur
  rep->sortisets = false;

  n.rep->m_children.Remove(n.rep->m_children.Find(keep.rep));
  DeleteTree(n.rep);
  keep.rep->m_parent = n.rep->m_parent;
  if (n.rep->m_parent) {
    n.rep->m_parent->m_children[n.rep->m_parent->m_children.Find(n.rep)] = keep.rep;
  }
  else {
    rep->root = keep.rep;
  }

  delete n.rep;
  rep->DeleteLexicon();

  rep->sortisets = true;

  rep->SortInfosets();
  return keep;
}

gbtEfgInfoset gbtEfgGame::JoinInfoset(gbtEfgInfoset s, gbtEfgNode n)
{
  if (n.IsNull() || s.IsNull())  {
    throw gbtEfgNullObject();
  }

  // can't bridge subgames
  if (s.rep->m_members.Length() > 0 &&
      n.rep->m_gameroot != s.rep->m_members[1]->m_gameroot)
    return 0;
  
  if (!n.rep->m_infoset)   return 0; 
  if (n.rep->m_infoset == s.rep)   return s;
  if (s.rep->m_actions.Length() != n.rep->m_children.Length())  {
    return n.rep->m_infoset;
  }

  rep->m_revision++;

  gbt_efg_infoset_rep *t = n.rep->m_infoset;

  t->m_members.Remove(t->m_members.Find(n.rep));
  s.rep->m_members.Append(n.rep);

  n.rep->m_infoset = s.rep;

  rep->DeleteLexicon();
  rep->SortInfosets();
  return s;
}

gbtEfgInfoset gbtEfgGame::LeaveInfoset(gbtEfgNode n)
{
  if (n.IsNull())  {
    throw gbtEfgNullObject();
  }

  if (!n.rep->m_infoset)   return 0;

  gbt_efg_infoset_rep *s = n.rep->m_infoset;
  if (s->m_members.Length() == 1)   return s;

  rep->m_revision++;

  gbt_efg_player_rep *p = s->m_player;
  s->m_members.Remove(s->m_members.Find(n.rep));
  n.rep->m_infoset = rep->NewInfoset(p, p->m_infosets.Length() + 1,
				     n.rep->m_children.Length());
  n.rep->m_infoset->m_label = s->m_label;
  n.rep->m_infoset->m_members.Append(n.rep);
  for (int i = 1; i <= s->m_actions.Length(); i++) {
    n.rep->m_infoset->m_actions[i]->m_label = s->m_actions[i]->m_label;
  }

  rep->DeleteLexicon();
  rep->SortInfosets();
  return n.rep->m_infoset;
}

gbtEfgInfoset gbtEfgGame::MergeInfoset(gbtEfgInfoset to, gbtEfgInfoset from)
{
  if (to.IsNull() || from.IsNull()) {
    throw gbtEfgNullObject();
  }

  if (to == from ||
      to.rep->m_actions.Length() != from.rep->m_actions.Length())  {
    return from;
  }

  if (to.rep->m_members[1]->m_gameroot != from.rep->m_members[1]->m_gameroot) 
    return from;

  rep->m_revision++;

  to.rep->m_members += from.rep->m_members;
  for (int i = 1; i <= from.rep->m_members.Length(); i++) {
    from.rep->m_members[i]->m_infoset = to.rep;
  }

  from.rep->m_members.Flush();

  rep->DeleteLexicon();
  rep->SortInfosets();
  return to;
}

bool gbtEfgGame::DeleteEmptyInfoset(gbtEfgInfoset s)
{
  if (s.IsNull())  {
    throw gbtEfgNullObject();
  }

  if (s.NumMembers() > 0)   return false;

  rep->m_revision++;
  s.rep->m_player->m_infosets.Remove(s.rep->m_player->m_infosets.Find(s.rep));
  delete s.rep;

  return true;
}

void gbtEfgGame::DeleteEmptyInfosets(void)
{
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    for (int iset = 1; iset <= NumInfosets()[pl]; iset++) {
      if (DeleteEmptyInfoset(rep->players[pl]->m_infosets[iset])) {
        iset--;
      }
    }
  }
} 

gbtEfgInfoset gbtEfgGame::SwitchPlayer(gbtEfgInfoset s, gbtEfgPlayer p)
{
  if (s.IsNull() || p.IsNull()) {
    throw gbtEfgNullObject();
  }
  if (s.GetPlayer().IsChance() || p.IsChance()) {
    throw gbtEfgException();
  }
  
  if (s.rep->m_player == p.rep)   return s;

  rep->m_revision++;
  s.rep->m_player->m_infosets.Remove(s.rep->m_player->m_infosets.Find(s.rep));
  s.rep->m_player = p.rep;
  p.rep->m_infosets.Append(s.rep);

  rep->DeleteLexicon();
  rep->SortInfosets();
  return s;
}

void gbtEfgGame::CopySubtree(gbt_efg_node_rep *src, gbt_efg_node_rep *dest,
			  gbt_efg_node_rep *stop)
{
  if (src == stop) {
    dest->m_outcome = src->m_outcome;
    return;
  }

  if (src->m_children.Length())  {
    rep->AppendMove(dest, src->m_infoset);
    for (int i = 1; i <= src->m_children.Length(); i++)
      CopySubtree(src->m_children[i], dest->m_children[i], stop);
  }

  dest->m_label = src->m_label;
  dest->m_outcome = src->m_outcome;
}

//
// MarkSubtree: sets the Node::mark flag on all children of p_node
//
void gbtEfgGame::MarkSubtree(gbt_efg_node_rep *p_node)
{
  p_node->m_mark = true;
  for (int i = 1; i <= p_node->m_children.Length(); i++) {
    MarkSubtree(p_node->m_children[i]);
  }
}

//
// UnmarkSubtree: clears the Node::mark flag on all children of p_node
//
void gbtEfgGame::UnmarkSubtree(gbt_efg_node_rep *p_node)
{
  p_node->m_mark = false;
  for (int i = 1; i <= p_node->m_children.Length(); i++) {
    UnmarkSubtree(p_node->m_children[i]);
  }
}

void gbtEfgGame::Reveal(gbtEfgInfoset where, gbtEfgPlayer who)
{
  if (where.rep->m_actions.Length() <= 1)  {
    // only one action; nothing to reveal!
    return;
  }

  UnmarkSubtree(rep->root);  // start with a clean tree
  
  rep->m_revision++;

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

  rep->DeleteLexicon();
  rep->SortInfosets();
}

gbtEfgNode gbtEfgGame::CopyTree(gbtEfgNode src, gbtEfgNode dest)
{
  if (src.IsNull() || dest.IsNull())  {
    throw gbtEfgNullObject();
  }
  if (src == dest || dest.rep->m_children.Length())   return src;
  if (src.rep->m_gameroot != dest.rep->m_gameroot)  return src;

  if (src.rep->m_children.Length())  {
    rep->m_revision++;

    rep->AppendMove(dest.rep, src.rep->m_infoset);
    for (int i = 1; i <= src.rep->m_children.Length(); i++) {
      CopySubtree(src.rep->m_children[i], dest.rep->m_children[i], dest.rep);
    }

    rep->DeleteLexicon();
    rep->SortInfosets();
  }

  return dest;
}

gbtEfgNode gbtEfgGame::MoveTree(gbtEfgNode src, gbtEfgNode dest)
{
  if (src.IsNull() || dest.IsNull())  {
    throw gbtEfgNullObject();
  }
  if (src == dest || dest.rep->m_children.Length() || src.IsPredecessor(dest))
    return src;
  if (src.rep->m_gameroot != dest.rep->m_gameroot)  return src;

  rep->m_revision++;

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
  
  rep->DeleteLexicon();
  rep->SortInfosets();
  return dest;
}

gbtEfgNode gbtEfgGame::DeleteTree(gbtEfgNode n)
{
  if (n.IsNull()) {
    throw gbtEfgNullObject();
  }

  rep->m_revision++;

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

  rep->DeleteLexicon();
  rep->SortInfosets();
  return n;
}

gbtEfgAction gbtEfgGame::InsertAction(gbtEfgInfoset s)
{
  if (s.IsNull()) {
    throw gbtEfgNullObject();
  }

  rep->m_revision++;
  gbtEfgAction action = s.InsertAction(s.NumActions() + 1);
  for (int i = 1; i <= s.NumMembers(); i++) {
    s.rep->m_members[i]->m_children.Append(new gbt_efg_node_rep(rep,
								s.rep->m_members[i]));
  }
  rep->DeleteLexicon();
  rep->SortInfosets();
  return action;
}

gbtEfgAction gbtEfgGame::InsertAction(gbtEfgInfoset s, const gbtEfgAction &a)
{
  if (a.IsNull() || s.IsNull()) {
    throw gbtEfgNullObject();
  }

  rep->m_revision++;

  int where;
  for (where = 1; (where <= s.rep->m_actions.Length() &&
		   s.rep->m_actions[where] != a.rep);
       where++);
  if (where > s.rep->m_actions.Length())   return 0;
  gbtEfgAction action = s.InsertAction(where);
  for (int i = 1; i <= s.NumActions(); i++)
    s.rep->m_members[i]->m_children.Insert(new gbt_efg_node_rep(rep, s.rep->m_members[i]), where);

  rep->DeleteLexicon();
  rep->SortInfosets();
  return action;
}

gbtEfgInfoset gbtEfgGame::DeleteAction(gbtEfgInfoset s, const gbtEfgAction &a)
{
  if (a.IsNull() || s.IsNull()) {
    throw gbtEfgNullObject();
  }

  rep->m_revision++;
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
  rep->DeleteLexicon();
  rep->SortInfosets();
  return s;
}

void gbtEfgGame::SetChanceProb(gbtEfgInfoset infoset,
			    int act, const gNumber &value)
{
  if (infoset.IsChanceInfoset()) {
    rep->m_revision++;
    infoset.SetChanceProb(act, value);
  }
}

//---------------------------------------------------------------------
//                     Subgame-related functions
//---------------------------------------------------------------------

void gbtEfgGame::MarkTree(gbt_efg_node_rep *n, gbt_efg_node_rep *base)
{
  n->m_ptr = base;
  for (int i = 1; i <= n->m_children.Length(); i++) {
    MarkTree(n->m_children[i], base);
  }
}

bool gbtEfgGame::CheckTree(gbt_efg_node_rep *n, gbt_efg_node_rep *base)
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

bool gbtEfgGame::IsLegalSubgame(const gbtEfgNode &n)
{
  if (n.NumChildren() == 0)  
    return false;

  MarkTree(n.rep, n.rep);
  return CheckTree(n.rep, n.rep);
}

bool gbtEfgGame::MarkSubgame(gbtEfgNode n)
{
  if (n.rep->m_gameroot == n.rep) return true;

  if (n.rep->m_gameroot != n.rep && IsLegalSubgame(n))  {
    n.rep->m_gameroot = 0;
    MarkSubgame(n.rep, n.rep);
    return true;
  }

  return false;
}

void gbtEfgGame::UnmarkSubgame(gbtEfgNode n)
{
  if (n.rep->m_gameroot == n.rep && n.rep->m_parent)  {
    n.rep->m_gameroot = 0;
    MarkSubgame(n.rep, n.rep->m_parent->m_gameroot);
  }
}
  

void gbtEfgGame::MarkSubgame(gbt_efg_node_rep *n, gbt_efg_node_rep *base)
{
  if (n->m_gameroot == n)  return;
  n->m_gameroot = base;
  for (int i = 1; i <= n->m_children.Length(); i++)
    MarkSubgame(n->m_children[i], base);
}

void gbtEfgGame::MarkSubgames(void)
{
  gList<gbtEfgNode> subgames;
  LegalSubgameRoots(*this, subgames);

  for (int i = 1; i <= subgames.Length(); i++)  {
    subgames[i].rep->m_gameroot = 0;
    MarkSubgame(subgames[i].rep, subgames[i].rep);
  }
}

void gbtEfgGame::UnmarkSubgames(gbtEfgNode n)
{
  if (n.NumChildren() == 0)   return;

  for (int i = 1; i <= n.NumChildren(); i++)
    UnmarkSubgames(n.GetChild(i));
  
  if (n.rep->m_gameroot == n.rep && n.rep->m_parent)  {
    n.rep->m_gameroot = 0;
    MarkSubgame(n.rep, n.rep->m_parent->m_gameroot);
  }
}


int gbtEfgGame::ProfileLength(void) const
{
  int sum = 0;

  for (int i = 1; i <= rep->players.Length(); i++) {
    for (int j = 1; j <= rep->players[i]->m_infosets.Length(); j++) {
      sum += rep->players[i]->m_infosets[j]->m_actions.Length();
    }
  }

  return sum;
}

gArray<int> gbtEfgGame::NumInfosets(void) const
{
  gArray<int> foo(rep->players.Length());
  
  for (int i = 1; i <= foo.Length(); i++) {
    foo[i] = rep->players[i]->m_infosets.Length();
  }

  return foo;
}

int gbtEfgGame::NumPlayerInfosets(void) const
{
  int answer(0);
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    answer += rep->players[pl]->m_infosets.Length();
  }
  return answer;
}

int gbtEfgGame::TotalNumInfosets(void) const
{
  return NumPlayerInfosets() + rep->chance->m_infosets.Length();
}

gPVector<int> gbtEfgGame::NumActions(void) const
{
  gArray<int> foo(rep->players.Length());
  for (int i = 1; i <= rep->players.Length(); i++) {
    foo[i] = rep->players[i]->m_infosets.Length();
  }

  gPVector<int> bar(foo);
  for (int i = 1; i <= rep->players.Length(); i++) {
    for (int j = 1; j <= rep->players[i]->m_infosets.Length(); j++) {
      bar(i, j) = rep->players[i]->m_infosets[j]->m_actions.Length();
    }
  }

  return bar;
}  

int gbtEfgGame::NumPlayerActions(void) const
{
  int answer = 0;

  gPVector<int> nums_actions = NumActions();
  for (int i = 1; i <= NumPlayers(); i++)
    answer += nums_actions[i];
  return answer;
}

gPVector<int> gbtEfgGame::NumMembers(void) const
{
  gArray<int> foo(rep->players.Length());

  for (int i = 1; i <= rep->players.Length(); i++) {
    foo[i] = rep->players[i]->m_infosets.Length();
  }

  gPVector<int> bar(foo);
  for (int i = 1; i <= rep->players.Length(); i++) {
    for (int j = 1; j <= rep->players[i]->m_infosets.Length(); j++) {
      bar(i, j) = rep->players[i]->m_infosets[j]->m_members.Length();
    }
  }

  return bar;
}

//------------------------------------------------------------------------
//                       Efg: Payoff computation
//------------------------------------------------------------------------

void gbtEfgGame::Payoff(gbt_efg_node_rep *n, gNumber prob,
		     const gPVector<int> &profile,
		     gVector<gNumber> &payoff) const
{
  if (n->m_outcome)  {
    for (int i = 1; i <= rep->players.Length(); i++)
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

void gbtEfgGame::InfosetProbs(gbt_efg_node_rep *n, gNumber prob,
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

void gbtEfgGame::Payoff(const gPVector<int> &profile, gVector<gNumber> &payoff) const
{
  ((gVector<gNumber> &) payoff).operator=((gNumber) 0);
  Payoff(rep->root, 1, profile, payoff);
}

void gbtEfgGame::InfosetProbs(const gPVector<int> &profile,
			  gPVector<gNumber> &probs) const
{
  ((gVector<gNumber> &) probs).operator=((gNumber) 0);
  InfosetProbs(rep->root, 1, profile, probs);
}

void gbtEfgGame::Payoff(gbt_efg_node_rep *n, gNumber prob,
		     const gArray<gArray<int> *> &profile,
		     gArray<gNumber> &payoff) const
{
  if (n->m_outcome)   {
    for (int i = 1; i <= rep->players.Length(); i++)
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

void gbtEfgGame::Payoff(const gArray<gArray<int> *> &profile,
		 gArray<gNumber> &payoff) const
{
  for (int i = 1; i <= payoff.Length(); i++)
    payoff[i] = 0;
  Payoff(rep->root, 1, profile, payoff);
}

gbtNfgGame gbtEfgGame::AssociatedNfg(void) const
{
  if (rep->lexicon) {
    return rep->lexicon->m_nfg;
  }
  else {
    return 0;
  }
}

bool gbtEfgGame::HasAssociatedNfg(void) const
{
  return rep->lexicon;
}

gbtNfgGame gbtEfgGame::AssociatedAfg(void) const
{
  return rep->afg;
}

Lexicon *gbtEfgGame::GetLexicon(void) const
{
  return rep->lexicon;
}
