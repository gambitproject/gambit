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
//            struct gbtEfgGameBase: Member functions
//----------------------------------------------------------------------

gbtEfgGameBase::gbtEfgGameBase(void)
  : sortisets(true), m_revision(0), 
    m_outcome_revision(-1), m_label("UNTITLED"),
    chance(new gbtEfgPlayerBase(this, 0)), m_reducedNfg(0)
{
  root = new gbtEfgNodeBase(this, 0);
  SortInfosets();
}

gbtEfgGameBase::~gbtEfgGameBase()
{
  delete root;
  delete chance;

  for (int i = 1; i <= players.Length(); delete players[i++]);
  for (int i = 1; i <= outcomes.Last(); delete outcomes[i++]);
}

void gbtEfgGameBase::DeleteLexicon(void) 
{
  // FIXME: delete the normal form
}

void gbtEfgGameBase::NumberNodes(gbtEfgNodeBase *n, int &index)
{
  n->m_id = index++;
  for (int child = 1; child <= n->m_children.Length();
       NumberNodes(n->m_children[child++], index));
} 

void gbtEfgGameBase::SortInfosets(void)
{
  if (!sortisets)  return;

  int pl;

  for (pl = 0; pl <= players.Length(); pl++)  {
    gbtList<gbtEfgNode> nodes;

    Nodes(gbtEfgGame(this), nodes);

    gbtEfgPlayerBase *player = (pl) ? players[pl] : chance;

    int i, isets = 0;

    // First, move all empty infosets to the back of the list so
    // we don't "lose" them
    int foo = player->m_infosets.Length();
    i = 1;
    while (i < foo)   {
      if (player->m_infosets[i]->m_members.Length() == 0)  {
	gbtEfgInfosetBase *bar = player->m_infosets[i];
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
      gbtEfgNodeBase *n = dynamic_cast<gbtEfgNodeBase *>(nodes[i].Get());
      if (n->GetPlayer() == player && n->GetInfoset()->GetId() == 0)  {
	n->m_infoset->m_id = ++isets;
	player->m_infosets[isets] = n->m_infoset;
      }
    }  
  }

  // Now, we sort the nodes within the infosets
  
  gbtList<gbtEfgNode> nodes;
  Nodes(gbtEfgGame(this), nodes);

  for (pl = 0; pl <= players.Length(); pl++)  {
    gbtEfgPlayerBase *player = (pl) ? players[pl] : chance;

    for (int iset = 1; iset <= player->m_infosets.Length(); iset++)  {
      gbtEfgInfosetBase *s = player->m_infosets[iset];
      for (int i = 1, j = 1; i <= nodes.Length(); i++)  {
	gbtEfgNodeBase *n = dynamic_cast<gbtEfgNodeBase *>(nodes[i].Get());
	if (n->m_infoset == s) {
	  s->m_members[j++] = n;
	}
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
void gbtEfgGameBase::InsertMove(gbtEfgNodeBase *p_node,
				  gbtEfgInfosetBase *p_infoset)
{
  gbtEfgNodeBase *parent = p_node->m_parent;
  gbtEfgNodeBase *node = new gbtEfgNodeBase(this, parent);
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
    node->m_children.Append(new gbtEfgNodeBase(this, node));
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
void gbtEfgGameBase::DeleteMove(gbtEfgNodeBase *p_node)
{
  // turn infoset sorting off during tree deletion -- problems will occur
  sortisets = false;

  gbtEfgNodeBase *parent = p_node->m_parent;
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
void gbtEfgGameBase::DeleteTree(gbtEfgNodeBase *p_node)
{
  while (p_node->m_children.Length() > 0)   {
    gbtEfgNodeBase *child = p_node->m_children.Remove(1);
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
gbtEfgInfosetBase *gbtEfgGameBase::NewInfoset(gbtEfgPlayerBase *p_player,
						  int p_id, int p_actions)
{
  gbtEfgInfosetBase *s = new gbtEfgInfosetBase(p_player, p_id, p_actions);
  p_player->m_infosets.Append(s);
  m_revision++;
  return s;
}

void gbtEfgGameBase::DeleteInfoset(gbtEfgInfosetBase *p_infoset)
{
  gbtEfgPlayerBase *player = p_infoset->m_player;
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
void gbtEfgGameBase::DeleteOutcome(gbtEfgOutcomeBase *p_outcome)
{
  // Remove references to the outcome from the tree
  root->DeleteOutcome(p_outcome);

  // Remove the outcome from the list of defined outcomes
  outcomes.Remove(outcomes.Find(p_outcome));

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
void gbtEfgGameBase::JoinInfoset(gbtEfgInfosetBase *p_infoset,
				   gbtEfgNodeBase *p_node)
{
  if (!p_node->m_infoset ||
      p_node->m_infoset == p_infoset ||
      p_infoset->m_actions.Length() != p_node->m_children.Length())  {
    return;
  }

  gbtEfgInfosetBase *t = p_node->m_infoset;
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
gbtEfgInfosetBase *gbtEfgGameBase::LeaveInfoset(gbtEfgNodeBase *p_node)
{
  if (!p_node->m_infoset)  {
    return 0;
  }

  gbtEfgInfosetBase *infoset = p_node->m_infoset;
  if (infoset->m_members.Length() == 1)  {
    return infoset;
  }

  gbtEfgPlayerBase *p = infoset->m_player;
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
void gbtEfgGameBase::MergeInfoset(gbtEfgInfosetBase *p_to,
				    gbtEfgInfosetBase *p_from)
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
void gbtEfgGameBase::Reveal(gbtEfgInfosetBase *p_where,
			      gbtEfgPlayerBase *p_who)
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
      gbtArray<gbtEfgNodeBase *> members = p_who->m_infosets[k]->m_members;
      gbtEfgInfosetBase *newiset = 0;
      
      for (int m = 1; m <= members.Length(); m++) {
	gbtEfgNodeBase *n = members[m];
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

void gbtEfgGameBase::SetPlayer(gbtEfgInfosetBase *p_infoset,
				 gbtEfgPlayerBase *p_player)
{
  gbtEfgPlayerBase *oldPlayer = p_infoset->m_player;
  oldPlayer->m_infosets.Remove(oldPlayer->m_infosets.Find(p_infoset));
  p_infoset->m_player = p_player;
  p_player->m_infosets.Append(p_infoset);

  m_revision++;
  DeleteLexicon();
  SortInfosets();
}

void gbtEfgGameBase::DeleteAction(gbtEfgInfosetBase *p_infoset,
				    gbtEfgActionBase *p_action)
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
    gbtEfgNodeBase *node = 
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


void gbtEfgGameBase::MarkTree(const gbtEfgNodeBase *n,
				const gbtEfgNodeBase *base)
{
  n->m_ptr = const_cast<gbtEfgNodeBase *>(base);
  for (int i = 1; i <= n->m_children.Length(); i++) {
    MarkTree(n->m_children[i], base);
  }
}

bool gbtEfgGameBase::CheckTree(const gbtEfgNodeBase *n,
				 const gbtEfgNodeBase *base)
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

void gbtEfgGameBase::MarkSubgame(gbtEfgNodeBase *n, gbtEfgNodeBase *base)
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

gbtEfgGame gbtEfgGameBase::Copy(gbtEfgNode n /* = null */) const
{
  gbtEfgGameBase *efg = new gbtEfgGameBase();
  efg->sortisets = false;
  efg->m_label = m_label;
  efg->comment = comment;
  efg->players = gbtBlock<gbtEfgPlayerBase *>(players.Length());
  efg->outcomes = gbtBlock<gbtEfgOutcomeBase *>(outcomes.Length());
  
  for (int i = 1; i <= players.Length(); i++)  {
    (efg->players[i] = new gbtEfgPlayerBase(efg, i))->m_label = players[i]->m_label;
    for (int j = 1; j <= players[i]->m_infosets.Length(); j++)   {
      gbtEfgInfosetBase *s = new gbtEfgInfosetBase(efg->players[i], j,
						       players[i]->m_infosets[j]->m_actions.Length());
      s->m_label = players[i]->m_infosets[j]->m_label;
      for (int k = 1; k <= s->m_actions.Length(); k++) {
	s->m_actions[k]->m_label = players[i]->m_infosets[j]->m_actions[k]->m_label;
      }
      efg->players[i]->m_infosets.Append(s);
    }
  }

  for (int i = 1; i <= GetChance()->NumInfosets(); i++)   {
    gbtEfgInfosetBase *t = chance->m_infosets[i];
    gbtEfgInfosetBase *s = new gbtEfgInfosetBase(efg->chance, i, 
						     t->m_actions.Length());
    s->m_label = t->m_label;
    for (int act = 1; act <= s->m_chanceProbs.Length(); act++) {
      s->m_chanceProbs[act] = t->m_chanceProbs[act];
      s->m_actions[act]->m_label = t->m_actions[act]->m_label;
    }
    efg->chance->m_infosets.Append(s);
  }

  for (int outc = 1; outc <= NumOutcomes(); outc++)  {
    efg->outcomes[outc] = new gbtEfgOutcomeBase(efg, outc);
    efg->outcomes[outc]->m_label = outcomes[outc]->m_label;
    efg->outcomes[outc]->m_payoffs = outcomes[outc]->m_payoffs;
  }

  if (n.IsNull()) {
    efg->CopySubtree(efg, efg->root, root);
  }
  else {
    gbtEfgNodeBase *node = dynamic_cast<gbtEfgNodeBase *>(n.Get());
    efg->CopySubtree(efg, efg->root, node);
  }

  if (!n.IsNull())   {
    for (int pl = 1; pl <= efg->players.Length(); pl++)  {
      for (int i = 1; i <= efg->players[pl]->m_infosets.Length(); i++)  {
	if (efg->players[pl]->m_infosets[i]->m_members.Length() == 0) {
	  delete efg->players[pl]->m_infosets.Remove(i--);
	}
      }
    }
  }

  efg->sortisets = true;
  efg->SortInfosets();
  return efg;
}

//------------------------------------------------------------------------
//               gbtEfgGameBase: Private member functions
//------------------------------------------------------------------------

void gbtEfgGameBase::CopySubtree(gbtEfgGameBase *p_newEfg,
			     gbtEfgNodeBase *n, gbtEfgNodeBase *m)
{
  if (m->m_infoset)   {
    gbtEfgPlayerBase *p;
    if (m->m_infoset->m_player->m_id) {
      p = p_newEfg->players[m->m_infoset->m_player->m_id];
    }
    else {
      p = p_newEfg->chance;
    }

    gbtEfgInfosetBase *s = p->m_infosets[m->m_infoset->m_id];
    p_newEfg->InsertMove(n, s);

    // This is because of the semantics of InsertMove();
    // the physical node 'n' gets pushed down one level to become the 
    // first child of the newly created node
    gbtEfgNodeBase *newParent = n->m_parent;

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

void gbtEfgGameBase::SetLabel(const gbtText &p_label)
{
  m_label = p_label; 
  m_revision++;
}

gbtText gbtEfgGameBase::GetLabel(void) const
{ return m_label; }

void gbtEfgGameBase::SetComment(const gbtText &s)
{
  comment = s;
  m_revision++;
}

gbtText gbtEfgGameBase::GetComment(void) const
{ return comment; }
  

//------------------------------------------------------------------------
//                    Efg: Writing data files
//------------------------------------------------------------------------

void gbtEfgGameBase::WriteEfg(gbtOutput &f, gbtEfgNodeBase *n) const
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

void gbtEfgGameBase::WriteEfg(gbtOutput &p_file) const
{
  p_file << "EFG 2 R";
  p_file << " \"" << EscapeQuotes(m_label) << "\" { ";
  for (int i = 1; i <= players.Length(); i++) {
    p_file << '"' << EscapeQuotes(players[i]->m_label) << "\" ";
  }
  p_file << "}\n";
  p_file << "\"" << EscapeQuotes(comment) << "\"\n\n";

  WriteEfg(p_file, root);
}


//------------------------------------------------------------------------
//                    Efg: General data access
//------------------------------------------------------------------------

long gbtEfgGameBase::RevisionNumber(void) const
{ return m_revision; }

int gbtEfgGameBase::NumPlayers(void) const
{ return players.Length(); }

gbtEfgPlayer gbtEfgGameBase::NewPlayer(void)
{
  m_revision++;

  gbtEfgPlayerBase *ret = new gbtEfgPlayerBase(this,
					       players.Length() + 1);
  players.Append(ret);

  for (int outc = 1; outc <= outcomes.Last();
       outcomes[outc++]->m_payoffs.Append(0));
  for (int outc = 1; outc <= outcomes.Last();
       outcomes[outc++]->m_doublePayoffs.Append(0));
  DeleteLexicon();
  return ret;
}

gbtEfgPlayer gbtEfgGameBase::GetPlayer(int p_player) const
{
  if (p_player == 0) {
    return chance;
  }
  else {
    return players[p_player];
  }
}

int gbtEfgGameBase::NumOutcomes(void) const
{ return outcomes.Last(); }

gbtEfgOutcome gbtEfgGameBase::NewOutcome(void)
{
  m_revision++;
  return NewOutcome(outcomes.Last() + 1);
}

gbtEfgOutcome gbtEfgGameBase::GetOutcome(int p_index) const
{
  return outcomes[p_index];
}

gbtEfgSupport gbtEfgGameBase::NewSupport(void) const
{
  return gbtEfgSupport(gbtEfgGame(const_cast<gbtEfgGameBase *>(this)));
}

bool gbtEfgGameBase::IsConstSum(void) const
{
  int pl, index;
  gbtNumber cvalue = (gbtNumber) 0;

  if (outcomes.Last() == 0)  return true;

  for (pl = 1; pl <= players.Length(); pl++)
    cvalue += outcomes[1]->m_payoffs[pl];

  for (index = 2; index <= outcomes.Last(); index++)  {
    gbtNumber thisvalue(0);

    for (pl = 1; pl <= players.Length(); pl++)
      thisvalue += outcomes[index]->m_payoffs[pl];

    if (thisvalue > cvalue || thisvalue < cvalue)
      return false;
  }

  return true;
}

bool gbtEfgGameBase::IsPerfectRecall(void) const
{
  gbtEfgInfoset s1, s2;
  return IsPerfectRecall(s1, s2);
}

bool gbtEfgGameBase::IsPerfectRecall(gbtEfgInfoset &s1, gbtEfgInfoset &s2) const
{
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    gbtEfgPlayer player = GetPlayer(pl);

    for (int i = 1; i <= player->NumInfosets(); i++) {
      gbtEfgInfoset iset1 = player->GetInfoset(i);
      for (int j = 1; j <= player->NumInfosets(); j++) {
        gbtEfgInfoset iset2 = player->GetInfoset(j);

        bool precedes = false;
        int action = 0;

        for (int m = 1; m <= iset2->NumMembers(); m++) {
          int n;
          for (n = 1; n <= iset1->NumMembers(); n++) {
            if (iset1->GetMember(n)->IsPredecessorOf(iset2->GetMember(m))
                && iset1->GetMember(n) != iset2->GetMember(m)) {
              precedes = true;
              for (int act = 1; act <= iset1->NumActions(); act++) {
                if (iset1->GetMember(n)->GetChild(act)->
                    IsPredecessorOf(iset2->GetMember(m))) {
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

          if (n > iset1->NumMembers() && precedes) {
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

gbtNumber gbtEfgGameBase::MinPayoff(int pl) const
{
  int index, p, p1, p2;
  gbtNumber minpay;

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

gbtNumber gbtEfgGameBase::MaxPayoff(int pl) const
{
  int index, p, p1, p2;
  gbtNumber maxpay;

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

gbtEfgNode gbtEfgGameBase::GetRoot(void) const
{ return root; }

int gbtEfgGameBase::NumNodes(void) const
{ return ::NumNodes(gbtEfgGame(const_cast<gbtEfgGameBase *>(this))); }

gbtEfgOutcome gbtEfgGameBase::NewOutcome(int index)
{
  m_revision++;
  outcomes.Append(new gbtEfgOutcomeBase(this, index));
  return outcomes[outcomes.Last()];
} 

//------------------------------------------------------------------------
//                     Efg: Operations on players
//------------------------------------------------------------------------

gbtEfgPlayer gbtEfgGameBase::GetChance(void) const
{
  return chance;
}

void gbtEfgGameBase::DeleteEmptyInfosets(void)
{
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    for (int iset = 1; iset <= NumInfosets()[pl]; iset++) {
      gbtEfgInfosetBase *infoset = players[pl]->m_infosets[iset];
      if (infoset->m_members.Length() == 0) {
	DeleteInfoset(infoset);
        iset--;
      }
    }
  }
} 

void gbtEfgGameBase::CopySubtree(gbtEfgNodeBase *src, gbtEfgNodeBase *dest,
			     gbtEfgNodeBase *stop)
{
  if (src == stop) {
    dest->m_outcome = src->m_outcome;
    return;
  }

  if (src->m_children.Length())  {
    InsertMove(dest, src->m_infoset);
    for (int i = 1; i <= src->m_children.Length(); i++) {
      CopySubtree(src->m_children[i], dest->m_parent->m_children[i], stop);
    }
  }

  dest->m_label = src->m_label;
  dest->m_outcome = src->m_outcome;
}

gbtEfgNode gbtEfgGameBase::CopyTree(gbtEfgNode p_src, gbtEfgNode p_dest)
{
  if (p_src.IsNull() || p_dest.IsNull())  {
    throw gbtEfgNullObject();
  }
  if (p_src == p_dest || p_dest->NumChildren() > 0)   return p_src;

  gbtEfgNodeBase *src = dynamic_cast<gbtEfgNodeBase *>(p_src.Get());
  gbtEfgNodeBase *dest = dynamic_cast<gbtEfgNodeBase *>(p_dest.Get());
  if (src->m_gameroot != dest->m_gameroot)  return src;

  if (src->m_children.Length())  {
    m_revision++;

    InsertMove(dest, src->m_infoset);
    for (int i = 1; i <= src->m_children.Length(); i++) {
      CopySubtree(src->m_children[i], dest->m_parent->m_children[i],
		  dest->m_parent);
    }

    DeleteLexicon();
    SortInfosets();
  }

  return dest;
}

gbtEfgNode gbtEfgGameBase::MoveTree(gbtEfgNode p_src, gbtEfgNode p_dest)
{
  if (p_src.IsNull() || p_dest.IsNull())  {
    throw gbtEfgNullObject();
  }
  gbtEfgNodeBase *src = dynamic_cast<gbtEfgNodeBase *>(p_src.Get());
  gbtEfgNodeBase *dest = dynamic_cast<gbtEfgNodeBase *>(p_dest.Get());

  if (src == dest || dest->m_children.Length() ||
      src->IsPredecessorOf(dest))
    return src;
  if (src->m_gameroot != dest->m_gameroot)  return src;

  m_revision++;

  if (src->m_parent == dest->m_parent) {
    int srcChild = src->m_parent->m_children.Find(src);
    int destChild = src->m_parent->m_children.Find(dest);
    src->m_parent->m_children[srcChild] = dest;
    src->m_parent->m_children[destChild] = src;
  }
  else {
    gbtEfgNodeBase *parent = src->m_parent; 
    parent->m_children[parent->m_children.Find(src)] = dest;
    dest->m_parent->m_children[dest->m_parent->m_children.Find(dest)] = src;
    src->m_parent = dest->m_parent;
    dest->m_parent = parent;
  }

  dest->m_label = "";
  dest->m_outcome = 0;
  
  DeleteLexicon();
  SortInfosets();
  return dest;
}

gbtEfgAction gbtEfgGameBase::InsertAction(gbtEfgInfoset s)
{
  if (s.IsNull()) {
    throw gbtEfgNullObject();
  }

  m_revision++;
  gbtEfgAction action = s->InsertAction(s->NumActions() + 1);
  gbtEfgInfosetBase *infoset = dynamic_cast<gbtEfgInfosetBase *>(s.Get());
  for (int i = 1; i <= s->NumMembers(); i++) {
    infoset->m_members[i]->m_children.Append(new gbtEfgNodeBase(this,
								infoset->m_members[i]));
  }
  DeleteLexicon();
  SortInfosets();
  return action;
}

gbtEfgAction gbtEfgGameBase::InsertAction(gbtEfgInfoset s, const gbtEfgAction &a)
{
  if (a.IsNull() || s.IsNull()) {
    throw gbtEfgNullObject();
  }

  m_revision++;

  gbtEfgInfosetBase *infoset = dynamic_cast<gbtEfgInfosetBase *>(s.Get());
  gbtEfgActionBase *action = dynamic_cast<gbtEfgActionBase *>(a.Get());
  int where;
  for (where = 1; (where <= infoset->m_actions.Length() &&
		   infoset->m_actions[where] != action);
       where++);
  if (where > infoset->m_actions.Length())   return 0;
  gbtEfgAction newAction = s->InsertAction(where);
  for (int i = 1; i <= s->NumActions(); i++)
    infoset->m_members[i]->m_children.Insert(new gbtEfgNodeBase(this, infoset->m_members[i]), where);

  DeleteLexicon();
  SortInfosets();
  return newAction;
}

void gbtEfgGameBase::SetChanceProb(gbtEfgInfoset infoset,
			       int act, const gbtNumber &value)
{
  if (infoset->IsChanceInfoset()) {
    m_revision++;
    infoset->SetChanceProb(act, value);
  }
}

//---------------------------------------------------------------------
//                     Subgame-related functions
//---------------------------------------------------------------------

bool gbtEfgGameBase::MarkSubgame(gbtEfgNode p_node)
{
  gbtEfgNodeBase *n = dynamic_cast<gbtEfgNodeBase *>(p_node.Get());

  if (n->m_gameroot == n) return true;

  if (n->m_gameroot != n && n->IsSubgameRoot())  {
    n->m_gameroot = 0;
    MarkSubgame(n, n);
    return true;
  }

  return false;
}

void gbtEfgGameBase::UnmarkSubgame(gbtEfgNode p_node)
{
  gbtEfgNodeBase *n = dynamic_cast<gbtEfgNodeBase *>(p_node.Get());

  if (n->m_gameroot == n && n->m_parent)  {
    n->m_gameroot = 0;
    MarkSubgame(n, n->m_parent->m_gameroot);
  }
}
  

void gbtEfgGameBase::MarkSubgames(void)
{
  gbtList<gbtEfgNode> subgames;
  LegalSubgameRoots(gbtEfgGame(this), subgames);

  for (int i = 1; i <= subgames.Length(); i++)  {
    gbtEfgNodeBase *n = dynamic_cast<gbtEfgNodeBase *>(subgames[i].Get());
    n->m_gameroot = 0;
    MarkSubgame(n, n);
  }
}

void gbtEfgGameBase::UnmarkSubgames(gbtEfgNode p_node)
{
  if (p_node->NumChildren() == 0)   return;

  for (int i = 1; i <= p_node->NumChildren(); i++)
    UnmarkSubgames(p_node->GetChild(i));
  
  gbtEfgNodeBase *n = dynamic_cast<gbtEfgNodeBase *>(p_node.Get());
  if (n->m_gameroot == n && n->m_parent)  {
    n->m_gameroot = 0;
    MarkSubgame(n, n->m_parent->m_gameroot);
  }
}


int gbtEfgGameBase::ProfileLength(void) const
{
  int sum = 0;

  for (int i = 1; i <= players.Length(); i++) {
    for (int j = 1; j <= players[i]->m_infosets.Length(); j++) {
      sum += players[i]->m_infosets[j]->m_actions.Length();
    }
  }

  return sum;
}

gbtArray<int> gbtEfgGameBase::NumInfosets(void) const
{
  gbtArray<int> foo(players.Length());
  
  for (int i = 1; i <= foo.Length(); i++) {
    foo[i] = players[i]->m_infosets.Length();
  }

  return foo;
}

int gbtEfgGameBase::NumPlayerInfosets(void) const
{
  int answer(0);
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    answer += players[pl]->m_infosets.Length();
  }
  return answer;
}

int gbtEfgGameBase::TotalNumInfosets(void) const
{
  return NumPlayerInfosets() + chance->m_infosets.Length();
}

gbtPVector<int> gbtEfgGameBase::NumActions(void) const
{
  gbtArray<int> foo(players.Length());
  for (int i = 1; i <= players.Length(); i++) {
    foo[i] = players[i]->m_infosets.Length();
  }

  gbtPVector<int> bar(foo);
  for (int i = 1; i <= players.Length(); i++) {
    for (int j = 1; j <= players[i]->m_infosets.Length(); j++) {
      bar(i, j) = players[i]->m_infosets[j]->m_actions.Length();
    }
  }

  return bar;
}  

int gbtEfgGameBase::NumPlayerActions(void) const
{
  int answer = 0;

  gbtPVector<int> nums_actions = NumActions();
  for (int i = 1; i <= NumPlayers(); i++)
    answer += nums_actions[i];
  return answer;
}

gbtPVector<int> gbtEfgGameBase::NumMembers(void) const
{
  gbtArray<int> foo(players.Length());

  for (int i = 1; i <= players.Length(); i++) {
    foo[i] = players[i]->m_infosets.Length();
  }

  gbtPVector<int> bar(foo);
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

void gbtEfgGameBase::Payoff(gbtEfgNodeBase *n, gbtNumber prob,
			const gbtPVector<int> &profile,
			gbtVector<gbtNumber> &payoff) const
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

void gbtEfgGameBase::InfosetProbs(gbtEfgNodeBase *n, gbtNumber prob,
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

void gbtEfgGameBase::Payoff(const gbtPVector<int> &profile, gbtVector<gbtNumber> &payoff) const
{
  ((gbtVector<gbtNumber> &) payoff).operator=((gbtNumber) 0);
  Payoff(root, 1, profile, payoff);
}

void gbtEfgGameBase::InfosetProbs(const gbtPVector<int> &profile,
			  gbtPVector<gbtNumber> &probs) const
{
  ((gbtVector<gbtNumber> &) probs).operator=((gbtNumber) 0);
  InfosetProbs(root, 1, profile, probs);
}

void gbtEfgGameBase::Payoff(gbtEfgNodeBase *n, gbtNumber prob,
			const gbtArray<gbtArray<int> > &profile,
			gbtArray<gbtNumber> &payoff) const
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
    Payoff(n->m_children[profile[n->m_infoset->m_player->m_id][n->m_infoset->m_id]],
	   prob, profile, payoff);
  }
}

void gbtEfgGameBase::Payoff(const gbtArray<gbtArray<int> > &profile,
			gbtArray<gbtNumber> &payoff) const
{
  for (int i = 1; i <= payoff.Length(); i++)
    payoff[i] = 0;
  Payoff(root, 1, profile, payoff);
}

//-------------------------------------------------------------------------
//                           Global functions 
//-------------------------------------------------------------------------

//
// Returns a new trivial extensive form game
//
gbtEfgGame NewEfg(void)
{
  return gbtEfgGame(new gbtEfgGameBase);
}
