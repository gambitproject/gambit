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
#include "nfg.h"

// Declarations of internal structures
#include "efgint.h"
#include "nfgint.h"

//----------------------------------------------------------------------
//            struct gbt_efg_game_rep: Member functions
//----------------------------------------------------------------------

gbt_efg_game_rep::gbt_efg_game_rep(void)
  : m_refCount(1),
    sortisets(true), m_revision(0), 
    m_outcome_revision(-1), m_label("UNTITLED"),
    chance(new gbt_efg_player_rep(this, 0)), afg(0), m_reducedNfg(0)
{
  root = new gbt_efg_node_rep(this, 0);
  SortInfosets();
}

gbt_efg_game_rep::~gbt_efg_game_rep()
{
  delete root;
  delete chance;

  for (int i = 1; i <= players.Length(); delete players[i++]);
  for (int i = 1; i <= outcomes.Last(); delete outcomes[i++]);
}

void gbt_efg_game_rep::DeleteLexicon(void) 
{
  if (m_reducedNfg) {
    if (--m_reducedNfg->m_refCount == 0) {
      // FIXME: delete the normal form
    }
    m_reducedNfg = 0;
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
    gbtList<gbtEfgNode> nodes;

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
  
  gbtList<gbtEfgNode> nodes;
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
// Create a new node, a member of 'p_infoset' in the location of 'p_node'
// in the tree, and make the subtree rooted in 'p_node' the first child
// of the new node.  Assumes node and infoset are valid.
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
// Delete the move prior to 'p_node' from the tree. 
// 'p_node' is assumed to be valid and assumed to have a parent.
// NOTE: This is an API change in version 0.97.1.1: before this, this
// function deleted a move, keeping a child node.
//
void gbt_efg_game_rep::DeleteMove(gbt_efg_node_rep *p_node)
{
  // turn infoset sorting off during tree deletion -- problems will occur
  sortisets = false;

  gbt_efg_node_rep *parent = p_node->m_parent;
  parent->m_children.Remove(parent->m_children.Find(p_node));
  DeleteTree(parent);
  p_node->m_parent = parent->m_parent;
  if (parent->m_parent) {
    parent->m_parent->m_children[parent->m_parent->m_children.Find(parent)] = p_node;
  }
  else {
    root = p_node;
  }

  if (parent->m_refCount == 0) {
    delete parent;
  }
  else {
    parent->m_deleted = true;
  }

  m_revision++;
  DeleteLexicon();
  sortisets = true;
  SortInfosets();
}

//
// Delete an entire subtree.  Assumes 'p_node' is valid.
//
void gbt_efg_game_rep::DeleteTree(gbt_efg_node_rep *p_node)
{
  while (p_node->m_children.Length() > 0)   {
    gbt_efg_node_rep *child = p_node->m_children.Remove(1);
    DeleteTree(child);
    if (child->m_refCount == 0) {
      delete child;
    }
    else {
      child->m_deleted = true;
    }
  }

  if (p_node->m_infoset)  {
    p_node->m_infoset->m_members.Remove(p_node->m_infoset->m_members.Find(p_node));
    p_node->m_infoset = 0;
  }
  p_node->m_outcome = 0;
  p_node->m_label = "";

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

void gbt_efg_game_rep::DeleteInfoset(gbt_efg_infoset_rep *p_infoset)
{
  gbt_efg_player_rep *player = p_infoset->m_player;
  player->m_infosets.Remove(player->m_infosets.Find(p_infoset));
  if (p_infoset->m_refCount == 0) {
    delete p_infoset;
  }
  else {
    p_infoset->m_deleted = true;
  }

  m_revision++;
  DeleteLexicon();
}

//
// Deletes the outcome from the extensive form.
// Assumes outcome is not null.
//
void gbt_efg_game_rep::DeleteOutcome(gbt_efg_outcome_rep *p_outcome)
{
  // Remove references to the outcome from the tree
  root->DeleteOutcome(p_outcome);

  // Remove the outcome from the list of defined outcomes
  outcomes.Remove(outcomes.Find(p_outcome));

  // If no external references, deallocate the memory;
  // otherwise, mark as "deleted"
  if (p_outcome->m_refCount == 0) {
    delete p_outcome;
  }
  else {
    p_outcome->m_deleted = true;
  }

  // Renumber the remaining outcomes
  for (int outc = 1; outc <= outcomes.Length(); outc++) {
    outcomes[outc]->m_id = outc;
  }

  m_revision++;
  DeleteLexicon();
}

//
// Places node 'p_node' in information set 'p_infoset'.
// Assumes 'p_node' and 'p_infoset' are not null, and the move at
// 'p_node' is compatible with 'p_infoset' (i.e., same number of actions)
//
void gbt_efg_game_rep::JoinInfoset(gbt_efg_infoset_rep *p_infoset,
				   gbt_efg_node_rep *p_node)
{
  if (!p_node->m_infoset ||
      p_node->m_infoset == p_infoset ||
      p_infoset->m_actions.Length() != p_node->m_children.Length())  {
    return;
  }

  gbt_efg_infoset_rep *t = p_node->m_infoset;
  t->m_members.Remove(t->m_members.Find(p_node));
  p_infoset->m_members.Append(p_node);
  p_node->m_infoset = p_infoset;

  m_revision++;
  DeleteLexicon();
  SortInfosets();
}

//
// Breaks 'p_node' out of its information set to a new singleton.
// Assumes 'p_node' is not null.  Returns the new information set.
//
gbt_efg_infoset_rep *gbt_efg_game_rep::LeaveInfoset(gbt_efg_node_rep *p_node)
{
  if (!p_node->m_infoset)  {
    return 0;
  }

  gbt_efg_infoset_rep *infoset = p_node->m_infoset;
  if (infoset->m_members.Length() == 1)  {
    return infoset;
  }

  gbt_efg_player_rep *p = infoset->m_player;
  infoset->m_members.Remove(infoset->m_members.Find(p_node));
  p_node->m_infoset = NewInfoset(p, p->m_infosets.Length() + 1,
				 p_node->m_children.Length());
  p_node->m_infoset->m_label = infoset->m_label;
  p_node->m_infoset->m_members.Append(p_node);
  for (int i = 1; i <= infoset->m_actions.Length(); i++) {
    p_node->m_infoset->m_actions[i]->m_label = infoset->m_actions[i]->m_label;
  }

  m_revision++;
  DeleteLexicon();
  SortInfosets();
  return p_node->m_infoset;
}

//
// Takes all nodes in infoset 'p_from' and places them in 'p_to'.
// Assumes the infosets are not null, and are compatible.
//
void gbt_efg_game_rep::MergeInfoset(gbt_efg_infoset_rep *p_to,
				    gbt_efg_infoset_rep *p_from)
{
  p_to->m_members += p_from->m_members;
  for (int i = 1; i <= p_from->m_members.Length(); i++) {
    p_from->m_members[i]->m_infoset = p_to;
  }

  p_from->m_members.Flush();

  m_revision++;
  DeleteLexicon();
  SortInfosets();
}

//
// Reveals the actions in 'p_where' to the player 'p_who'.
// Assumes information set and player are both non-null.
//
void gbt_efg_game_rep::Reveal(gbt_efg_infoset_rep *p_where,
			      gbt_efg_player_rep *p_who)
{
  if (p_where->m_actions.Length() <= 1)  {
    // only one action; nothing to reveal!
    return;
  }

  root->MarkSubtree(false);  // start with a clean tree
  
  for (int i = 1; i <= p_where->m_actions.Length(); i++) {
    for (int j = 1; j <= p_where->m_members.Length(); j++) { 
      p_where->m_members[j]->m_children[i]->MarkSubtree(true);
    }

    for (int k = 1; k <= p_who->m_infosets.Length(); k++) {
      // iterate over each member of information set 'k'
      // make copy of members to iterate correctly 
      // (since the information set may be changed in the process)
      gbtArray<gbt_efg_node_rep *> members = p_who->m_infosets[k]->m_members;
      gbt_efg_infoset_rep *newiset = 0;
      
      for (int m = 1; m <= members.Length(); m++) {
	gbt_efg_node_rep *n = members[m];
	if (n->m_mark) {
	  // If node is marked, is descendant of action 'i'
	  n->m_mark = false;   // unmark so tree is clean at end
	  if (!newiset) {
	    newiset = LeaveInfoset(n);
	  }
	  else {
	    JoinInfoset(newiset, n);
	  }
	} 
      }
    }
  }

  m_revision++;
  DeleteLexicon();
  SortInfosets();
}

void gbt_efg_game_rep::SetPlayer(gbt_efg_infoset_rep *p_infoset,
				 gbt_efg_player_rep *p_player)
{
  gbt_efg_player_rep *oldPlayer = p_infoset->m_player;
  oldPlayer->m_infosets.Remove(oldPlayer->m_infosets.Find(p_infoset));
  p_infoset->m_player = p_player;
  p_player->m_infosets.Append(p_infoset);

  m_revision++;
  DeleteLexicon();
  SortInfosets();
}

void gbt_efg_game_rep::DeleteAction(gbt_efg_infoset_rep *p_infoset,
				    gbt_efg_action_rep *p_action)
{
  p_infoset->m_actions.Remove(p_action->m_id);
  if (p_infoset->m_player->m_id == 0) {
    p_infoset->m_chanceProbs.Remove(p_action->m_id);
  }
  for (int act = 1; act <= p_infoset->m_actions.Length(); act++) {
    p_infoset->m_actions[act]->m_id = act;
  }

  for (int i = 1; i <= p_infoset->m_members.Length(); i++)   {
    DeleteTree(p_infoset->m_members[i]->m_children[p_action->m_id]);
    gbt_efg_node_rep *node = 
      p_infoset->m_members[i]->m_children.Remove(p_action->m_id);
    if (node->m_refCount == 0) {
      delete node;
    }
    else {
      node->m_deleted = true;
    }
  }

  if (p_action->m_refCount == 0) {
    delete p_action;
  }
  else {
    p_action->m_deleted = true;
  }

  m_revision++;
  DeleteLexicon();
  SortInfosets();
}


void gbt_efg_game_rep::MarkTree(gbt_efg_node_rep *n,
				gbt_efg_node_rep *base)
{
  n->m_ptr = base;
  for (int i = 1; i <= n->m_children.Length(); i++) {
    MarkTree(n->m_children[i], base);
  }
}

bool gbt_efg_game_rep::CheckTree(gbt_efg_node_rep *n,
				 gbt_efg_node_rep *base)
{
  if (n->m_children.Length() == 0)   return true;

  for (int i = 1; i <= n->m_children.Length(); i++) {
    if (!CheckTree(n->m_children[i], base)) {
      return false;
    }
  }

  if (n->m_infoset->m_player->m_id == 0)  {
    return true;
  }

  for (int i = 1; i <= n->m_infoset->m_members.Length(); i++) {
    if (n->m_infoset->m_members[i]->m_ptr != base) {
      return false;
    }
  }

  return true;
}

void gbt_efg_game_rep::MarkSubgame(gbt_efg_node_rep *n, gbt_efg_node_rep *base)
{
  if (n->m_gameroot == n) {
    return;
  }
  n->m_gameroot = base;
  for (int i = 1; i <= n->m_children.Length(); i++) {
    MarkSubgame(n->m_children[i], base);
  }
}


//------------------------------------------------------------------------
//       Efg: Constructors, destructor, constructive operators
//------------------------------------------------------------------------

gbtEfgGame::gbtEfgGame(void)
  : rep(0)
{ }

gbtEfgGame gbtEfgGame::Copy(gbtEfgNode n /* = null */) const
{
  gbtEfgGame efg = NewEfg();
  efg.rep->sortisets = false;
  efg.rep->m_label = rep->m_label;
  efg.rep->comment = rep->comment;
  efg.rep->players = gbtBlock<gbt_efg_player_rep *>(rep->players.Length());
  efg.rep->outcomes = gbtBlock<gbt_efg_outcome_rep *>(rep->outcomes.Length());
  
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

  efg.CopySubtree(efg.rep, efg.rep->root, (n.rep ? n.rep : rep->root));

  if (n.rep)   {
    for (int pl = 1; pl <= efg.rep->players.Length(); pl++)  {
      for (int i = 1; i <= efg.rep->players[pl]->m_infosets.Length(); i++)  {
	if (efg.rep->players[pl]->m_infosets[i]->m_members.Length() == 0) {
	  delete efg.rep->players[pl]->m_infosets.Remove(i--);
	}
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
    //    delete rep;
  }
}

gbtEfgGame &gbtEfgGame::operator=(const gbtEfgGame &p_efg)
{
  if (this != &p_efg) {
    if (rep && --rep->m_refCount == 0) {
      // delete rep;
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

void gbtEfgGame::CopySubtree(gbt_efg_game_rep *p_newEfg,
			     gbt_efg_node_rep *n, gbt_efg_node_rep *m)
{
  if (m->m_infoset)   {
    gbt_efg_player_rep *p;
    if (m->m_infoset->m_player->m_id) {
      p = p_newEfg->players[m->m_infoset->m_player->m_id];
    }
    else {
      p = p_newEfg->chance;
    }

    gbt_efg_infoset_rep *s = p->m_infosets[m->m_infoset->m_id];
    p_newEfg->InsertMove(n, s);

    // This is because of the semantics of InsertMove();
    // the physical node 'n' gets pushed down one level to become the 
    // first child of the newly created node
    gbt_efg_node_rep *newParent = n->m_parent;

    newParent->m_label = m->m_label;
    
    if (m->m_gameroot == m) {
      newParent->m_gameroot = newParent;
    }

    if (m->m_outcome) {
      newParent->m_outcome = p_newEfg->outcomes[m->m_outcome->m_id];
    }

    for (int i = 1; i <= m->m_children.Length(); i++) {
      CopySubtree(p_newEfg, newParent->m_children[i], m->m_children[i]);
    }
  }
  else {
    n->m_label = m->m_label;
    n->m_gameroot = 0;
    if (m->m_outcome) {
      n->m_outcome = p_newEfg->outcomes[m->m_outcome->m_id];
    }
  }
}

//------------------------------------------------------------------------
//               Efg: Title access and manipulation
//------------------------------------------------------------------------

void gbtEfgGame::SetLabel(const gbtText &p_label)
{
  rep->m_label = p_label; 
  rep->m_revision++;
}

const gbtText &gbtEfgGame::GetLabel(void) const
{ return rep->m_label; }

void gbtEfgGame::SetComment(const gbtText &s)
{
  rep->comment = s;
  rep->m_revision++;
}

const gbtText &gbtEfgGame::GetComment(void) const
{ return rep->comment; }
  

//------------------------------------------------------------------------
//                    Efg: Writing data files
//------------------------------------------------------------------------

void gbtEfgGame::WriteEfg(gbtOutput &f, gbt_efg_node_rep *n) const
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

  for (int i = 1; i <= n->m_children.Length(); i++) {
    WriteEfg(f, n->m_children[i]);
  }
}

void gbtEfgGame::WriteEfg(gbtOutput &p_file) const
{
  p_file << "EFG 2 R";
  p_file << " \"" << EscapeQuotes(rep->m_label) << "\" { ";
  for (int i = 1; i <= rep->players.Length(); i++) {
    p_file << '"' << EscapeQuotes(rep->players[i]->m_label) << "\" ";
  }
  p_file << "}\n";
  p_file << "\"" << EscapeQuotes(rep->comment) << "\"\n\n";

  WriteEfg(p_file, rep->root);
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

gbtEfgOutcome gbtEfgGame::GetOutcome(int p_index) const
{
  return rep->outcomes[p_index];
}

gbtEfgSupport gbtEfgGame::NewSupport(void) const
{
  return gbtEfgSupport(*this);
}

bool gbtEfgGame::IsConstSum(void) const
{
  int pl, index;
  gbtNumber cvalue = (gbtNumber) 0;

  if (rep->outcomes.Last() == 0)  return true;

  for (pl = 1; pl <= rep->players.Length(); pl++)
    cvalue += rep->outcomes[1]->m_payoffs[pl];

  for (index = 2; index <= rep->outcomes.Last(); index++)  {
    gbtNumber thisvalue(0);

    for (pl = 1; pl <= rep->players.Length(); pl++)
      thisvalue += rep->outcomes[index]->m_payoffs[pl];

    if (thisvalue > cvalue || thisvalue < cvalue)
      return false;
  }

  return true;
}

bool gbtEfgGame::IsPerfectRecall(void) const
{
  gbtEfgInfoset s1, s2;
  return IsPerfectRecall(s1, s2);
}

bool gbtEfgGame::IsPerfectRecall(gbtEfgInfoset &s1, gbtEfgInfoset &s2) const
{
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    gbtEfgPlayer player = GetPlayer(pl);

    for (int i = 1; i <= player.NumInfosets(); i++) {
      gbtEfgInfoset iset1 = player.GetInfoset(i);
      for (int j = 1; j <= player.NumInfosets(); j++) {
        gbtEfgInfoset iset2 = player.GetInfoset(j);

        bool precedes = false;
        int action = 0;

        for (int m = 1; m <= iset2.NumMembers(); m++) {
          int n;
          for (n = 1; n <= iset1.NumMembers(); n++) {
            if (iset1.GetMember(n).IsPredecessorOf(iset2.GetMember(m))
                && iset1.GetMember(n) != iset2.GetMember(m)) {
              precedes = true;
              for (int act = 1; act <= iset1.NumActions(); act++) {
                if (iset1.GetMember(n).GetChild(act).
                    IsPredecessorOf(iset2.GetMember(m))) {
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

gbtNumber gbtEfgGame::MinPayoff(int pl) const
{
  int index, p, p1, p2;
  gbtNumber minpay;

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

gbtNumber gbtEfgGame::MaxPayoff(int pl) const
{
  int index, p, p1, p2;
  gbtNumber maxpay;

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

gbtEfgNode gbtEfgGame::GetRoot(void) const
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

void gbtEfgGame::DeleteEmptyInfosets(void)
{
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    for (int iset = 1; iset <= NumInfosets()[pl]; iset++) {
      gbt_efg_infoset_rep *infoset = rep->players[pl]->m_infosets[iset];
      if (infoset->m_members.Length() == 0) {
	rep->DeleteInfoset(infoset);
        iset--;
      }
    }
  }
} 

void gbtEfgGame::CopySubtree(gbt_efg_node_rep *src, gbt_efg_node_rep *dest,
			     gbt_efg_node_rep *stop)
{
  if (src == stop) {
    dest->m_outcome = src->m_outcome;
    return;
  }

  if (src->m_children.Length())  {
    rep->InsertMove(dest, src->m_infoset);
    for (int i = 1; i <= src->m_children.Length(); i++) {
      CopySubtree(src->m_children[i], dest->m_parent->m_children[i], stop);
    }
  }

  dest->m_label = src->m_label;
  dest->m_outcome = src->m_outcome;
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

    rep->InsertMove(dest.rep, src.rep->m_infoset);
    for (int i = 1; i <= src.rep->m_children.Length(); i++) {
      CopySubtree(src.rep->m_children[i], dest.rep->m_parent->m_children[i],
		  dest.rep->m_parent);
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
  if (src == dest || dest.rep->m_children.Length() ||
      src.IsPredecessorOf(dest))
    return src;
  if (src.rep->m_gameroot != dest.rep->m_gameroot)  return src;

  rep->m_revision++;

  if (src.rep->m_parent == dest.rep->m_parent) {
    int srcChild = src.rep->m_parent->m_children.Find(src.rep);
    int destChild = src.rep->m_parent->m_children.Find(dest.rep);
    src.rep->m_parent->m_children[srcChild] = dest.rep;
    src.rep->m_parent->m_children[destChild] = src.rep;
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

void gbtEfgGame::SetChanceProb(gbtEfgInfoset infoset,
			    int act, const gbtNumber &value)
{
  if (infoset.IsChanceInfoset()) {
    rep->m_revision++;
    infoset.SetChanceProb(act, value);
  }
}

//---------------------------------------------------------------------
//                     Subgame-related functions
//---------------------------------------------------------------------

bool gbtEfgGame::MarkSubgame(gbtEfgNode n)
{
  if (n.rep->m_gameroot == n.rep) return true;

  if (n.rep->m_gameroot != n.rep && n.IsSubgameRoot())  {
    n.rep->m_gameroot = 0;
    rep->MarkSubgame(n.rep, n.rep);
    return true;
  }

  return false;
}

void gbtEfgGame::UnmarkSubgame(gbtEfgNode n)
{
  if (n.rep->m_gameroot == n.rep && n.rep->m_parent)  {
    n.rep->m_gameroot = 0;
    rep->MarkSubgame(n.rep, n.rep->m_parent->m_gameroot);
  }
}
  

void gbtEfgGame::MarkSubgames(void)
{
  gbtList<gbtEfgNode> subgames;
  LegalSubgameRoots(*this, subgames);

  for (int i = 1; i <= subgames.Length(); i++)  {
    subgames[i].rep->m_gameroot = 0;
    rep->MarkSubgame(subgames[i].rep, subgames[i].rep);
  }
}

void gbtEfgGame::UnmarkSubgames(gbtEfgNode n)
{
  if (n.NumChildren() == 0)   return;

  for (int i = 1; i <= n.NumChildren(); i++)
    UnmarkSubgames(n.GetChild(i));
  
  if (n.rep->m_gameroot == n.rep && n.rep->m_parent)  {
    n.rep->m_gameroot = 0;
    rep->MarkSubgame(n.rep, n.rep->m_parent->m_gameroot);
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

gbtArray<int> gbtEfgGame::NumInfosets(void) const
{
  gbtArray<int> foo(rep->players.Length());
  
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

gbtPVector<int> gbtEfgGame::NumActions(void) const
{
  gbtArray<int> foo(rep->players.Length());
  for (int i = 1; i <= rep->players.Length(); i++) {
    foo[i] = rep->players[i]->m_infosets.Length();
  }

  gbtPVector<int> bar(foo);
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

  gbtPVector<int> nums_actions = NumActions();
  for (int i = 1; i <= NumPlayers(); i++)
    answer += nums_actions[i];
  return answer;
}

gbtPVector<int> gbtEfgGame::NumMembers(void) const
{
  gbtArray<int> foo(rep->players.Length());

  for (int i = 1; i <= rep->players.Length(); i++) {
    foo[i] = rep->players[i]->m_infosets.Length();
  }

  gbtPVector<int> bar(foo);
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

void gbtEfgGame::Payoff(gbt_efg_node_rep *n, gbtNumber prob,
		     const gbtPVector<int> &profile,
		     gbtVector<gbtNumber> &payoff) const
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

void gbtEfgGame::InfosetProbs(gbt_efg_node_rep *n, gbtNumber prob,
			   const gbtPVector<int> &profile,
			   gbtPVector<gbtNumber> &probs) const
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

void gbtEfgGame::Payoff(const gbtPVector<int> &profile, gbtVector<gbtNumber> &payoff) const
{
  ((gbtVector<gbtNumber> &) payoff).operator=((gbtNumber) 0);
  Payoff(rep->root, 1, profile, payoff);
}

void gbtEfgGame::InfosetProbs(const gbtPVector<int> &profile,
			  gbtPVector<gbtNumber> &probs) const
{
  ((gbtVector<gbtNumber> &) probs).operator=((gbtNumber) 0);
  InfosetProbs(rep->root, 1, profile, probs);
}

void gbtEfgGame::Payoff(gbt_efg_node_rep *n, gbtNumber prob,
		     const gbtArray<gbtArray<int> *> &profile,
		     gbtArray<gbtNumber> &payoff) const
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

void gbtEfgGame::Payoff(const gbtArray<gbtArray<int> *> &profile,
		 gbtArray<gbtNumber> &payoff) const
{
  for (int i = 1; i <= payoff.Length(); i++)
    payoff[i] = 0;
  Payoff(rep->root, 1, profile, payoff);
}

bool gbtEfgGame::HasReducedNfg(void) const
{
  return rep->m_reducedNfg;
}

gbtNfgGame gbtEfgGame::AssociatedAfg(void) const
{
  return rep->afg;
}


//-------------------------------------------------------------------------
//                           Global functions 
//-------------------------------------------------------------------------

//
// Returns a new trivial extensive form game
//
gbtEfgGame NewEfg(void)
{
  return gbtEfgGame(new gbt_efg_game_rep);
}
