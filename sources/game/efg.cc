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

#ifdef __GNUG__
#pragma implementation "infoset.h"
#pragma implementation "efplayer.h"
#pragma implementation "node.h"
#endif   // __GNUG__

#include "efg.h"
#include "efgutils.h"
#include "efstrat.h"
#include "actiter.h"

//----------------------------------------------------------------------
//                gbt_efg_outcome_rep: Declaration
//----------------------------------------------------------------------

struct gbt_efg_outcome_rep {
  int m_id;
  efgGame *m_efg;
  bool m_deleted;
  gText m_label;
  gBlock<gNumber> m_payoffs;
  gBlock<double> m_doublePayoffs;
  int m_refCount;

  gbt_efg_outcome_rep(efgGame *, int);
};

gbt_efg_outcome_rep::gbt_efg_outcome_rep(efgGame *p_efg, int p_id)
  : m_id(p_id), m_efg(p_efg), m_deleted(false), 
    m_payoffs(p_efg->NumPlayers()), m_doublePayoffs(p_efg->NumPlayers()),
    m_refCount(1)
{
  for (int i = 1; i <= m_payoffs.Length(); i++) {
    m_payoffs[i] = 0;
    m_doublePayoffs[i] = 0.0;
  }
}

gbtEfgOutcome::gbtEfgOutcome(void)
  : rep(0)
{ }

gbtEfgOutcome::gbtEfgOutcome(gbt_efg_outcome_rep *p_rep)
  : rep(p_rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtEfgOutcome::gbtEfgOutcome(const gbtEfgOutcome &p_outcome)
  : rep(p_outcome.rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtEfgOutcome::~gbtEfgOutcome()
{
  if (rep) {
    if (--rep->m_refCount == 0) {
      delete rep;
    }
  }
}

gbtEfgOutcome &gbtEfgOutcome::operator=(const gbtEfgOutcome &p_outcome)
{
  if (this == &p_outcome) {
    return *this;
  }

  if (rep && --rep->m_refCount == 0) {
    delete rep;
  }

  if ((rep = p_outcome.rep) != 0) {
    rep->m_refCount++;
  }
  return *this;
}

bool gbtEfgOutcome::operator==(const gbtEfgOutcome &p_outcome) const
{
  return (rep == p_outcome.rep);
} 

bool gbtEfgOutcome::operator!=(const gbtEfgOutcome &p_outcome) const
{
  return (rep != p_outcome.rep);
} 

bool gbtEfgOutcome::IsNull(void) const
{
  return (rep == 0);
}

efgGame *gbtEfgOutcome::GetGame(void) const
{
  return (rep) ? rep->m_efg : 0;
}

gText gbtEfgOutcome::GetLabel(void) const
{
  if (rep) {
    return rep->m_label;
  }
  else {
    return "";
  }
}

gOutput &operator<<(gOutput &p_stream, const gbtEfgOutcome &)
{ 
  return p_stream;
}

//----------------------------------------------------------------------
//                   gbt_efg_player_rep: Declaration
//----------------------------------------------------------------------

struct gbt_efg_player_rep {
  int m_id;
  efgGame *m_efg;
  bool m_deleted;
  gText m_label;
  gBlock<Infoset *> m_infosets;
  int m_refCount;

  gbt_efg_player_rep(efgGame *, int);
  ~gbt_efg_player_rep();
};

gbt_efg_player_rep::gbt_efg_player_rep(efgGame *p_efg, int p_id)
  : m_id(p_id), m_efg(p_efg), m_deleted(false), 
    m_refCount(1)
{ }


gbt_efg_player_rep::~gbt_efg_player_rep()
{
  // Temporarily we will leak these information sets while API is in
  // transition.
  /*
  while (m_infosets.Length()) {
    delete m_infosets.Remove(1);
  }
  */
}

gbtEfgPlayer::gbtEfgPlayer(void)
  : rep(0)
{ }

gbtEfgPlayer::gbtEfgPlayer(gbt_efg_player_rep *p_rep)
  : rep(p_rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtEfgPlayer::gbtEfgPlayer(const gbtEfgPlayer &p_outcome)
  : rep(p_outcome.rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtEfgPlayer::~gbtEfgPlayer()
{
  if (rep) {
    if (--rep->m_refCount == 0) {
      delete rep;
    }
  }
}

gbtEfgPlayer &gbtEfgPlayer::operator=(const gbtEfgPlayer &p_outcome)
{
  if (this == &p_outcome) {
    return *this;
  }

  if (rep && --rep->m_refCount == 0) {
    delete rep;
  }

  if ((rep = p_outcome.rep) != 0) {
    rep->m_refCount++;
  }
  return *this;
}

bool gbtEfgPlayer::operator==(const gbtEfgPlayer &p_outcome) const
{
  return (rep == p_outcome.rep);
} 

bool gbtEfgPlayer::operator!=(const gbtEfgPlayer &p_outcome) const
{
  return (rep != p_outcome.rep);
} 

bool gbtEfgPlayer::IsNull(void) const
{
  return (rep == 0);
}

efgGame *gbtEfgPlayer::GetGame(void) const
{
  return (rep) ? rep->m_efg : 0;
}

int gbtEfgPlayer::GetId(void) const
{
  return (rep) ? rep->m_id : -1;
}

gText gbtEfgPlayer::GetLabel(void) const
{
  if (rep) {
    return rep->m_label;
  }
  else {
    return "";
  }
}

void gbtEfgPlayer::SetLabel(const gText &p_label)
{
  if (rep) {
    rep->m_label = p_label;
  }
}

int gbtEfgPlayer::NumInfosets(void) const
{
  if (rep) {
    return rep->m_infosets.Length();
  }
  else {
    return 0;
  }
}

Infoset *gbtEfgPlayer::GetInfoset(int p_index) const
{
  if (rep) {
    return rep->m_infosets[p_index];
  }
  else {
    return 0;
  }
}


bool gbtEfgPlayer::IsChance(void) const
{
  return (rep && rep->m_id == 0);
}

gOutput &operator<<(gOutput &p_stream, const gbtEfgPlayer &)
{ 
  return p_stream;
}

//----------------------------------------------------------------------
//           gbtEfgPlayerIterator: Member function definitions
//----------------------------------------------------------------------

gbtEfgPlayerIterator::gbtEfgPlayerIterator(efgGame &p_efg)
  : m_index(1), m_efg(p_efg)
{ }

gbtEfgPlayer gbtEfgPlayerIterator::operator*(void) const
{ return m_efg.GetPlayer(m_index); }

gbtEfgPlayerIterator &gbtEfgPlayerIterator::operator++(int)
{ m_index++; return *this; }

bool gbtEfgPlayerIterator::Begin(void)
{ m_index = 1; return true; }

bool gbtEfgPlayerIterator::End(void) const
{ return m_index > m_efg.NumPlayers(); }


//----------------------------------------------------------------------
//           gbtEfgInfosetIterator: Member function definitions
//----------------------------------------------------------------------

gbtEfgInfosetIterator::gbtEfgInfosetIterator(const gbtEfgPlayer &p_player)
  : m_index(1), m_player(p_player)
{ }

Infoset *gbtEfgInfosetIterator::operator*(void) const
{ return m_player.GetInfoset(m_index); }

gbtEfgInfosetIterator &gbtEfgInfosetIterator::operator++(int)
{ m_index++; return *this; }

bool gbtEfgInfosetIterator::Begin(void)
{ m_index = 1; return true; }

bool gbtEfgInfosetIterator::End(void) const
{ return m_index > m_player.NumInfosets(); }

//----------------------------------------------------------------------
//                 Action: Member function definitions
//----------------------------------------------------------------------

bool Action::Precedes(const Node * n) const
{
  while ( n != n->Game()->RootNode() ) {
    if ( n->GetAction() == this )
      return true;
    else
      n = n->GetParent();
  }
  return false;
}

//----------------------------------------------------------------------
//                 Infoset: Member function definitions
//----------------------------------------------------------------------

bool Infoset::Precedes(const Node * n) const
{
  while ( n != n->Game()->RootNode() ) {
    if ( n->GetInfoset() == this )
      return true;
    else
      n = n->GetParent();
  }
  return false;
}

Infoset::Infoset(efgGame *e, int n, gbt_efg_player_rep *p, int br)
  : E(e), number(n), player(p), actions(br), flag(0) 
{
  while (br)   {
    actions[br] = new Action(br, "", this);
    br--; 
  }
}

Infoset::~Infoset()  
{
  for (int i = 1; i <= actions.Length(); i++)  delete actions[i];
}

void Infoset::PrintActions(gOutput &f) const
{ 
  f << "{ ";
  for (int i = 1; i <= actions.Length(); i++)
    f << '"' << EscapeQuotes(actions[i]->name) << "\" ";
  f << "}";
}

Action *Infoset::InsertAction(int where)
{
  Action *action = new Action(where, "", this);
  actions.Insert(action, where);
  for (; where <= actions.Length(); where++)
    actions[where]->number = where;
  return action;
}

void Infoset::RemoveAction(int which)
{
  delete actions.Remove(which);
  for (; which <= actions.Length(); which++)
    actions[which]->number = which;
}

gbtEfgPlayer Infoset::GetPlayer(void) const
{
  return player;
}

//------------------------------------------------------------------------
//           ChanceInfoset: Member function definitions
//------------------------------------------------------------------------

ChanceInfoset::ChanceInfoset(efgGame *E, int n, gbt_efg_player_rep *p, int br)
  : Infoset(E, n, p, br), probs(br)
{
  for (int i = 1; i <= br; probs[i++] = gRational(1, br));
}

Action *ChanceInfoset::InsertAction(int where)
{ 
  Action *action = Infoset::InsertAction(where);
  probs.Insert((gNumber) 0, where);
  return action;
}

void ChanceInfoset::RemoveAction(int which)
{
  Infoset::RemoveAction(which);
  probs.Remove(which);
}

void ChanceInfoset::PrintActions(gOutput &f) const
{ 
  f << "{ ";
  for (int i = 1; i <= actions.Length(); i++) {
    f << '"' << EscapeQuotes(actions[i]->GetName()) << "\" ";
    f << probs[i] << ' ';
  }
  f << "}";
}

//----------------------------------------------------------------------
//                   Node: Member function definitions
//----------------------------------------------------------------------

Node::Node(efgGame *e, Node *p)
  : mark(false), number(0), E(e), infoset(0), parent(p), outcome(0),
    gameroot((p) ? p->gameroot : this)
{ }

Node::~Node()
{
  for (int i = children.Length(); i; delete children[i--]);
}


int Node::NumberInInfoset(void) const
{
  for (int i = 1; i <= GetInfoset()->NumMembers(); i++)
    if (GetInfoset()->GetMember(i) == this)
      return i;
  //  This could be speeded up by adding a member to Node to keep track of this
  throw efgGame::Exception();
}


Node *Node::NextSibling(void) const  
{
  if (!parent)   return 0;
  if (parent->children.Find((Node * const) this) == parent->children.Length())
    return 0;
  else
    return parent->children[parent->children.Find((Node * const)this) + 1];
}

Node *Node::PriorSibling(void) const
{ 
  if (!parent)   return 0;
  if (parent->children.Find((Node * const)this) == 1)
    return 0;
  else
    return parent->children[parent->children.Find((Node * const)this) - 1];

}

Action *Node::GetAction(void) const
{
  if (this == Game()->RootNode()) {
    return 0;
  }
  
  Infoset *infoset = GetParent()->GetInfoset();
  for (int i = 1; i <= infoset->NumActions(); i++) {
    if (this == GetParent()->GetChild(infoset->GetAction(i))) {
      return infoset->GetAction(i);
    }
  }

  return 0;
}

void Node::DeleteOutcome(gbt_efg_outcome_rep *p_outcome)
{
  if (p_outcome == outcome) {
    outcome = 0;
  }
  for (int i = 1; i <= children.Length(); i++) {
    children[i]->DeleteOutcome(p_outcome);
  }
}

gbtEfgPlayer Node::GetPlayer(void) const
{
  if (!infoset) {
    return 0;
  }
  else {
    return infoset->GetPlayer();
  }
}

//------------------------------------------------------------------------
//       Efg: Constructors, destructor, constructive operators
//------------------------------------------------------------------------

#ifdef MEMCHECK
int efgGame::_NumObj = 0;
#endif // MEMCHECK

efgGame::efgGame(void)
  : sortisets(true), m_dirty(false), m_revision(0), 
    m_outcome_revision(-1), title("UNTITLED"),
    chance(new gbt_efg_player_rep(this, 0)), afg(0), lexicon(0)
{
  root = new Node(this, 0);
#ifdef MEMCHECK
  _NumObj++;
  gout << "--- Efg Ctor: " << _NumObj << "\n";
#endif // MEMCHECK

  SortInfosets();
}

efgGame::efgGame(const efgGame &E, Node *n /* = 0 */)
  : sortisets(false), m_dirty(false), m_revision(0), 
    m_outcome_revision(-1), title(E.title), comment(E.comment),
    players(E.players.Length()), outcomes(0, E.outcomes.Last()),
    chance(new gbt_efg_player_rep(this, 0)),
    afg(0), lexicon(0) 
{
  for (int i = 1; i <= players.Length(); i++)  {
    (players[i] = new gbt_efg_player_rep(this, i))->m_label = E.players[i]->m_label;
    for (int j = 1; j <= E.players[i]->m_infosets.Length(); j++)   {
      Infoset *s = new Infoset(this, j, players[i],
			       E.players[i]->m_infosets[j]->actions.Length());
      s->name = E.players[i]->m_infosets[j]->name;
      for (int k = 1; k <= s->actions.Length(); k++)
	s->actions[k]->name = E.players[i]->m_infosets[j]->actions[k]->name;
      players[i]->m_infosets.Append(s);
    }
  }

  for (int i = 1; i <= E.GetChance().NumInfosets(); i++)   {
    ChanceInfoset *t = (ChanceInfoset *) E.GetChance().GetInfoset(i);
    ChanceInfoset *s = new ChanceInfoset(this, i, chance, t->NumActions());
    s->name = t->GetName();
    for (int act = 1; act <= s->probs.Length(); act++) {
      s->probs[act] = t->probs[act];
      s->actions[act]->name = t->actions[act]->name;
    }
    chance->m_infosets.Append(s);
  }

  for (int outc = 1; outc <= E.NumOutcomes(); outc++)  {
    outcomes[outc] = new gbt_efg_outcome_rep(this, outc);
    outcomes[outc]->m_label = E.outcomes[outc]->m_label;
    outcomes[outc]->m_payoffs = E.outcomes[outc]->m_payoffs;
  }

  root = new Node(this, 0);
  CopySubtree(root, (n ? n : E.RootNode()));
  
  if (n)   {
    for (int pl = 1; pl <= players.Length(); pl++)  {
      for (int i = 1; i <= players[pl]->m_infosets.Length(); i++)  {
	if (players[pl]->m_infosets[i]->members.Length() == 0)
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

Infoset *efgGame::GetInfosetByIndex(gbtEfgPlayer p, int index) const
{
  for (int i = 1; i <= p.rep->m_infosets.Length(); i++) {
    if (p.rep->m_infosets[i]->number == index) {
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
      p->m_infosets[j]->number = j;
    }
  }

  for (i = 1; i <= outcomes.Last(); i++) {
    outcomes[i]->m_id = i;
  }
}


void efgGame::NumberNodes(Node *n, int &index)
{
  n->number = index++;
  for (int child = 1; child <= n->children.Length();
       NumberNodes(n->children[child++], index));
} 

void efgGame::SortInfosets(void)
{
  if (!sortisets)  return;

  int pl;

  for (pl = 0; pl <= players.Length(); pl++)  {
    gList<Node *> nodes;

    Nodes(*this, nodes);

    gbt_efg_player_rep *player = (pl) ? players[pl] : chance;

    int i, isets = 0;

    // First, move all empty infosets to the back of the list so
    // we don't "lose" them
    int foo = player->m_infosets.Length();
    i = 1;
    while (i < foo)   {
      if (player->m_infosets[i]->members.Length() == 0)  {
	Infoset *bar = player->m_infosets[i];
	player->m_infosets[i] = player->m_infosets[foo];
	player->m_infosets[foo--] = bar;
      }
      else
	i++;
    }

    // This will give empty infosets their proper number; the nonempty
    // ones will be renumbered by the next loop
    for (i = 1; i <= player->m_infosets.Length(); i++)
      if (player->m_infosets[i]->members.Length() == 0)
	player->m_infosets[i]->number = i;
      else
	player->m_infosets[i]->number = 0;
  
    for (i = 1; i <= nodes.Length(); i++)  {
      Node *n = nodes[i];
      if (n->GetPlayer() == player && n->GetInfoset()->number == 0)  {
	n->GetInfoset()->number = ++isets;
	player->m_infosets[isets] = n->GetInfoset();
      }
    }  
  }

  // Now, we sort the nodes within the infosets
  
  gList<Node *> nodes;
  Nodes(*this, nodes);

  for (pl = 0; pl <= players.Length(); pl++)  {
    gbt_efg_player_rep *player = (pl) ? players[pl] : chance;

    for (int iset = 1; iset <= player->m_infosets.Length(); iset++)  {
      Infoset *s = player->m_infosets[iset];
      for (int i = 1, j = 1; i <= nodes.Length(); i++)  {
	if (nodes[i]->infoset == s)
	  s->members[j++] = nodes[i];
      }
    }
  }

  int nodeindex = 1;
  NumberNodes(root, nodeindex);
}
  
Infoset *efgGame::CreateInfoset(int n, gbtEfgPlayer p, int br)
{
  Infoset *s = (p.IsChance()) ? new ChanceInfoset(this, n, p.rep, br) :
               new Infoset(this, n, p.rep, br);
  p.rep->m_infosets.Append(s);
  return s;
}

gbtEfgOutcome efgGame::CreateOutcomeByIndex(int index)
{
  NewOutcome(index);
  return outcomes[outcomes.Last()];
}

void efgGame::CopySubtree(Node *n, Node *m)
{
  n->name = m->name;

  if (m->gameroot == m)
    n->gameroot = n;

  if (m->outcome) {
    n->outcome = m->outcome;
  }

  if (m->infoset)   {
    gbt_efg_player_rep *p;
    if (m->infoset->player->m_id) {
      p = players[m->infoset->player->m_id];
    }
    else {
      p = chance;
    }

    Infoset *s = p->m_infosets[m->infoset->number];
    AppendNode(n, s);

    for (int i = 1; i <= n->children.Length(); i++)
      CopySubtree(n->children[i], m->children[i]);
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

void efgGame::WriteEfgFile(gOutput &f, Node *n) const
{
  if (n->children.Length() == 0)   {
    f << "t \"" << EscapeQuotes(n->name) << "\" ";
    if (n->outcome)  {
      f << n->outcome->m_id << " \"" <<
	EscapeQuotes(n->outcome->m_label) << "\" ";
      f << "{ ";
      for (int pl = 1; pl <= NumPlayers(); pl++)  {
	f << n->outcome->m_payoffs[pl];
	if (pl < NumPlayers())
	  f << ", ";
	else
	  f << " }\n";
      }
    }
    else
      f << "0\n";
  }

  else if (n->infoset->player->m_id)   {
    f << "p \"" << EscapeQuotes(n->name) << "\" " <<
      n->infoset->player->m_id << ' ';
    f << n->infoset->number << " \"" <<
      EscapeQuotes(n->infoset->name) << "\" ";
    n->infoset->PrintActions(f);
    f << " ";
    if (n->outcome)  {
      f << n->outcome->m_id << " \"" <<
	EscapeQuotes(n->outcome->m_label) << "\" ";
      f << "{ ";
      for (int pl = 1; pl <= NumPlayers(); pl++)  {
	f << n->outcome->m_payoffs[pl];
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
    f << "c \"" << n->name << "\" ";
    f << n->infoset->number << " \"" <<
      EscapeQuotes(n->infoset->name) << "\" ";
    n->infoset->PrintActions(f);
    f << " ";
    if (n->outcome)  {
      f << n->outcome->m_id << " \"" <<
	EscapeQuotes(n->outcome->m_label) << "\" ";
      f << "{ ";
      for (int pl = 1; pl <= NumPlayers(); pl++)  {
	f << n->outcome->m_payoffs[pl];
        if (pl < NumPlayers()) 
          f << ", ";
        else
          f << " }\n";
      }
    }
    else
      f << "0\n";
  }

  for (int i = 1; i <= n->children.Length(); i++)
    WriteEfgFile(f, n->children[i]);
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

gbtEfgOutcome efgGame::GetOutcome(const Node *p_node) const
{
  return p_node->outcome;
}

void efgGame::SetOutcome(Node *p_node, const gbtEfgOutcome &p_outcome)
{
  p_node->outcome = p_outcome.rep;
}

void efgGame::SetLabel(gbtEfgOutcome &p_outcome, const gText &p_label)
{
  p_outcome.rep->m_label = p_label;
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

gNumber efgGame::Payoff(const gbtEfgOutcome &p_outcome,
			const gbtEfgPlayer &p_player) const
{
  if (p_outcome.IsNull() || p_player.IsNull()) {
    return gNumber(0);
  }

  return p_outcome.rep->m_payoffs[p_player.rep->m_id];
}

gNumber efgGame::Payoff(const Node *p_node, const gbtEfgPlayer &p_player) const
{
  if (p_player.IsNull()) {
    return gNumber(0);
  }

  return ((p_node->outcome) ? 
	  p_node->outcome->m_payoffs[p_player.rep->m_id] : gNumber(0));
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

Node *efgGame::RootNode(void) const
{ return root; }

bool efgGame::IsSuccessor(const Node *n, const Node *from) const
{ return IsPredecessor(from, n); }

bool efgGame::IsPredecessor(const Node *n, const Node *of) const
{
  while (of && n != of)    of = of->parent;

  return (n == of);
}

void efgGame::DescendantNodes(const Node* n, 
			      const EFSupport& supp,
			      gList<Node *> &current) const
{
  current += const_cast<Node *>(n);
  if (n->IsNonterminal()) {
    for (gbtActionIterator action(supp, n->GetInfoset());
	 !action.End(); action++) {
      DescendantNodes(n->GetChild(*action), supp, current);
    }
  }
}

void efgGame::NonterminalDescendants(const Node* n, 
				     const EFSupport& supp,
				     gList<const Node*>& current) const
{
  if (n->IsNonterminal()) {
    current += n;
    for (gbtActionIterator action(supp, n->GetInfoset());
	 !action.End(); action++) {
      NonterminalDescendants(n->GetChild(*action), supp, current);
    }
  }
}

void efgGame::TerminalDescendants(const Node* n, 
				  const EFSupport& supp,
				  gList<Node *> &current) const
{
  if (n->IsTerminal()) { 
    // casting away const to silence compiler warning
    current += (Node *) n;
  }
  else {
    for (gbtActionIterator action(supp, n->GetInfoset());
	 !action.End(); action++) {
      TerminalDescendants(n->GetChild(*action), supp, current);
    }
  }
}

gList<Node *>
efgGame::DescendantNodes(const Node &p_node, const EFSupport &p_support) const
{
  gList<Node *> answer;
  DescendantNodes(&p_node, p_support, answer);
  return answer;
}

gList<const Node*> 
efgGame::NonterminalDescendants(const Node& n, const EFSupport& supp) const
{
  gList<const Node*> answer;
  NonterminalDescendants(&n,supp,answer);
  return answer;
}

gList<Node *> 
efgGame::TerminalDescendants(const Node& n, const EFSupport& supp) const
{
  gList<Node *> answer;
  TerminalDescendants(&n,supp,answer);
  return answer;
}

gList<Node *> efgGame::TerminalNodes() const
{
  return TerminalDescendants(*(RootNode()),EFSupport(*this));
}

gList<Infoset*> efgGame::DescendantInfosets(const Node& n, 
					const EFSupport& supp) const
{
  gList<Infoset*> answer;
  gList<const Node*> nodelist = NonterminalDescendants(n,supp);
  int i;
  for (i = 1; i <= nodelist.Length(); i++) {
      Infoset* iset = nodelist[i]->GetInfoset();
      if (!answer.Contains(iset))
	answer += iset;
  }
  return answer;
}

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

Infoset *efgGame::AppendNode(Node *n, gbtEfgPlayer p, int count)
{
  if (!n || p.IsNull() || count == 0)
    throw Exception();

  m_revision++;
  m_dirty = true;

  if (n->children.Length() == 0)   {
    n->infoset = CreateInfoset(p.rep->m_infosets.Length() + 1, p.rep, count);
    n->infoset->members.Append(n);
    while (count--)
      n->children.Append(new Node(this, n));
  }

  DeleteLexicon();
  SortInfosets();
  return n->infoset;
}  

Infoset *efgGame::AppendNode(Node *n, Infoset *s)
{
  if (!n || !s)   throw Exception();
  
  // Can't bridge subgames...
  if (s->members.Length() > 0 && n->gameroot != s->members[1]->gameroot)
    return 0;

  if (n->children.Length() == 0)   {
    m_revision++;
    m_dirty = true;
    n->infoset = s;
    s->members.Append(n);
    for (int i = 1; i <= s->actions.Length(); i++)
      n->children.Append(new Node(this, n));
  }

  DeleteLexicon();
  SortInfosets();
  return s;
}
  
Node *efgGame::DeleteNode(Node *n, Node *keep)
{
  if (!n || !keep)   throw Exception();

  if (keep->parent != n)   return n;

  if (n->gameroot == n)
    MarkSubgame(keep, keep);

  m_revision++;
  m_dirty = true;
  // turn infoset sorting off during tree deletion -- problems will occur
  sortisets = false;

  n->children.Remove(n->children.Find(keep));
  DeleteTree(n);
  keep->parent = n->parent;
  if (n->parent)
    n->parent->children[n->parent->children.Find(n)] = keep;
  else
    root = keep;

  delete n;
  DeleteLexicon();

  sortisets = true;

  SortInfosets();
  return keep;
}

Infoset *efgGame::InsertNode(Node *n, gbtEfgPlayer p, int count)
{
  if (!n || p.IsNull() || count <= 0)  {
    throw Exception();
  }

  m_revision++;
  m_dirty = true;

  Node *m = new Node(this, n->parent);
  m->infoset = CreateInfoset(p.rep->m_infosets.Length() + 1, p, count);
  m->infoset->members.Append(m);
  if (n->parent)
    n->parent->children[n->parent->children.Find(n)] = m;
  else
    root = m;
  m->children.Append(n);
  n->parent = m;
  while (--count)
    m->children.Append(new Node(this, m));

  DeleteLexicon();
  SortInfosets();
  return m->infoset;
}

Infoset *efgGame::InsertNode(Node *n, Infoset *s)
{
  if (!n || !s)  throw Exception();

  // can't bridge subgames
  if (s->members.Length() > 0 && n->gameroot != s->members[1]->gameroot)
    return 0;
  
  m_revision++;
  m_dirty = true;

  Node *m = new Node(this, n->parent);
  m->infoset = s;
  s->members.Append(m);
  if (n->parent)
    n->parent->children[n->parent->children.Find(n)] = m;
  else
    root = m;
  m->children.Append(n);
  n->parent = m;
  int count = s->actions.Length();
  while (--count)
    m->children.Append(new Node(this, m));

  DeleteLexicon();
  SortInfosets();
  return m->infoset;
}

Infoset *efgGame::CreateInfoset(gbtEfgPlayer p, int br)
{
  if (p.IsNull() || p.GetGame() != this) {
    throw Exception();
  }
  m_revision++;
  m_dirty = true;
  return CreateInfoset(p.rep->m_infosets.Length() + 1, p.rep, br);
}

Infoset *efgGame::JoinInfoset(Infoset *s, Node *n)
{
  if (!n || !s)  throw Exception();

  // can't bridge subgames
  if (s->members.Length() > 0 && n->gameroot != s->members[1]->gameroot)
    return 0;
  
  if (!n->infoset)   return 0; 
  if (n->infoset == s)   return s;
  if (s->actions.Length() != n->children.Length())  return n->infoset;

  m_revision++;
  m_dirty = true;

  Infoset *t = n->infoset;

  t->members.Remove(t->members.Find(n));
  s->members.Append(n);

  n->infoset = s;

  DeleteLexicon();
  SortInfosets();
  return s;
}

Infoset *efgGame::LeaveInfoset(Node *n)
{
  if (!n)  throw Exception();

  if (!n->infoset)   return 0;

  Infoset *s = n->infoset;
  if (s->members.Length() == 1)   return s;

  m_revision++;
  m_dirty = true;

  gbt_efg_player_rep *p = s->player;
  s->members.Remove(s->members.Find(n));
  n->infoset = CreateInfoset(p->m_infosets.Length() + 1, p,
			     n->children.Length());
  n->infoset->name = s->name;
  n->infoset->members.Append(n);
  for (int i = 1; i <= s->actions.Length(); i++)
    n->infoset->actions[i]->name = s->actions[i]->name;

  DeleteLexicon();
  SortInfosets();
  return n->infoset;
}

Infoset *efgGame::SplitInfoset(Node *n)
{
  if (!n)  throw Exception();

  if (!n->infoset)   return 0;

  Infoset *s = n->infoset;
  if (s->members.Length() == 1)   return s;

  m_revision++;
  m_dirty = true;

  gbt_efg_player_rep *p = s->player;
  Infoset *ns = CreateInfoset(p->m_infosets.Length() + 1, p,
			      n->children.Length());
  ns->name = s->name;
  int i;
  for (i = s->members.Length(); i > s->members.Find(n); i--)   {
    Node *nn = s->members.Remove(i);
    ns->members.Append(nn);
    nn->infoset = ns;
  }
  for (i = 1; i <= s->actions.Length(); i++) {
    ns->actions[i]->name = s->actions[i]->name;
    if (p == chance) {
      SetChanceProb(ns, i, GetChanceProb(s, i));
    }
  }
  DeleteLexicon();
  SortInfosets();
  return n->infoset;
}

Infoset *efgGame::MergeInfoset(Infoset *to, Infoset *from)
{
  if (!to || !from)  throw Exception();

  if (to == from ||
      to->actions.Length() != from->actions.Length())   return from;

  if (to->members[1]->gameroot != from->members[1]->gameroot) 
    return from;

  m_revision++;
  m_dirty = true;

  to->members += from->members;
  for (int i = 1; i <= from->members.Length(); i++)
    from->members[i]->infoset = to;

  from->members.Flush();

  DeleteLexicon();
  SortInfosets();
  return to;
}

bool efgGame::DeleteEmptyInfoset(Infoset *s)
{
  if (!s)  throw Exception();

  if (s->NumMembers() > 0)   return false;

  m_revision++;
  m_dirty = true;
  s->player->m_infosets.Remove(s->player->m_infosets.Find(s));
  delete s;

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

Infoset *efgGame::SwitchPlayer(Infoset *s, gbtEfgPlayer p)
{
  if (!s || p.IsNull()) {
    throw Exception();
  }
  if (s->GetPlayer().IsChance() || p.IsChance()) {
    throw Exception();
  }
  
  if (s->player == p.rep)   return s;

  m_revision++;
  m_dirty = true;
  s->player->m_infosets.Remove(s->player->m_infosets.Find(s));
  s->player = p.rep;
  p.rep->m_infosets.Append(s);

  DeleteLexicon();
  SortInfosets();
  return s;
}

void efgGame::CopySubtree(Node *src, Node *dest, Node *stop)
{
  if (src == stop) {
    dest->outcome = src->outcome;
    return;
  }

  if (src->children.Length())  {
    AppendNode(dest, src->infoset);
    for (int i = 1; i <= src->children.Length(); i++)
      CopySubtree(src->children[i], dest->children[i], stop);
  }

  dest->name = src->name;
  dest->outcome = src->outcome;
}

//
// MarkSubtree: sets the Node::mark flag on all children of p_node
//
void efgGame::MarkSubtree(Node *p_node)
{
  p_node->mark = true;
  for (int i = 1; i <= p_node->children.Length(); i++) {
    MarkSubtree(p_node->children[i]);
  }
}

//
// UnmarkSubtree: clears the Node::mark flag on all children of p_node
//
void efgGame::UnmarkSubtree(Node *p_node)
{
  p_node->mark = false;
  for (int i = 1; i <= p_node->children.Length(); i++) {
    UnmarkSubtree(p_node->children[i]);
  }
}

void efgGame::Reveal(Infoset *where, gbtEfgPlayer who)
{
  if (where->actions.Length() <= 1)  {
    // only one action; nothing to reveal!
    return;
  }

  UnmarkSubtree(root);  // start with a clean tree
  
  m_revision++;
  m_dirty = true;

  for (int i = 1; i <= where->actions.Length(); i++) {
    for (int j = 1; j <= where->members.Length(); j++) { 
      MarkSubtree(where->members[j]->children[i]);
    }

    for (int k = 1; k <= who.rep->m_infosets.Length(); k++) {
      // iterate over each member of information set 'k'
      // make copy of members to iterate correctly 
      // (since the information set may be changed in the process)
      gArray<Node *> members = who.rep->m_infosets[k]->members;
      Infoset *newiset = 0;
      
      for (int m = 1; m <= members.Length(); m++) {
	Node *n = members[m];
	if (n->mark) {
	  // If node is marked, is descendant of action 'i'
	  n->mark = false;   // unmark so tree is clean at end
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

  Reindex();
}

Node *efgGame::CopyTree(Node *src, Node *dest)
{
  if (!src || !dest)  throw Exception();
  if (src == dest || dest->children.Length())   return src;
  if (src->gameroot != dest->gameroot)  return src;

  if (src->children.Length())  {
    m_revision++;
    m_dirty = true;

    AppendNode(dest, src->infoset);
    for (int i = 1; i <= src->children.Length(); i++)
      CopySubtree(src->children[i], dest->children[i], dest);

    DeleteLexicon();
    SortInfosets();
  }

  return dest;
}

Node *efgGame::MoveTree(Node *src, Node *dest)
{
  if (!src || !dest)  throw Exception();
  if (src == dest || dest->children.Length() || IsPredecessor(src, dest))
    return src;
  if (src->gameroot != dest->gameroot)  return src;

  m_revision++;
  m_dirty = true;

  if (src->parent == dest->parent) {
    int srcChild = src->parent->children.Find(src);
    int destChild = src->parent->children.Find(dest);
    src->parent->children[srcChild] = dest;
    src->parent->children[destChild] = src;
  }
  else {
    Node *parent = src->parent; 
    parent->children[parent->children.Find(src)] = dest;
    dest->parent->children[dest->parent->children.Find(dest)] = src;
    src->parent = dest->parent;
    dest->parent = parent;
  }

  dest->name = "";
  dest->outcome = 0;
  
  DeleteLexicon();
  SortInfosets();
  return dest;
}

Node *efgGame::DeleteTree(Node *n)
{
  if (!n)  throw Exception();

  m_revision++;
  m_dirty = true;

  while (n->NumChildren() > 0)   {
    DeleteTree(n->children[1]);
    delete n->children.Remove(1);
  }
  
  if (n->infoset)  {
    n->infoset->members.Remove(n->infoset->members.Find(n));
    n->infoset = 0;
  }
  n->outcome = 0;
  n->name = "";

  DeleteLexicon();
  SortInfosets();
  return n;
}

Action *efgGame::InsertAction(Infoset *s)
{
  if (!s)  throw Exception();

  m_revision++;
  m_dirty = true;
  Action *action = s->InsertAction(s->NumActions() + 1);
  for (int i = 1; i <= s->members.Length(); i++) {
    s->members[i]->children.Append(new Node(this, s->members[i]));
  }
  DeleteLexicon();
  SortInfosets();
  return action;
}

Action *efgGame::InsertAction(Infoset *s, const Action *a)
{
  if (!a || !s)  throw Exception();

  m_revision++;
  m_dirty = true;

  int where;
  for (where = 1; where <= s->actions.Length() && s->actions[where] != a;
       where++);
  if (where > s->actions.Length())   return 0;
  Action *action = s->InsertAction(where);
  for (int i = 1; i <= s->members.Length(); i++)
    s->members[i]->children.Insert(new Node(this, s->members[i]), where);

  DeleteLexicon();
  SortInfosets();
  return action;
}

Infoset *efgGame::DeleteAction(Infoset *s, const Action *a)
{
  if (!a || !s)  throw Exception();

  m_revision++;
  m_dirty = true;
  int where;
  for (where = 1; where <= s->actions.Length() && s->actions[where] != a;
       where++);
  if (where > s->actions.Length() || s->actions.Length() == 1)   return s;
  s->RemoveAction(where);
  for (int i = 1; i <= s->members.Length(); i++)   {
    DeleteTree(s->members[i]->children[where]);
    delete s->members[i]->children.Remove(where);
  }
  DeleteLexicon();
  SortInfosets();
  return s;
}

void efgGame::SetChanceProb(Infoset *infoset, int act, const gNumber &value)
{
  if (infoset->IsChanceInfoset()) {
    m_revision++;
    m_dirty = true;
    ((ChanceInfoset *) infoset)->SetActionProb(act, value);
  }
}

gNumber efgGame::GetChanceProb(Infoset *infoset, int act) const
{
  if (infoset->IsChanceInfoset())
    return ((ChanceInfoset *) infoset)->GetActionProb(act);
  else
    return (gNumber) 0;
}

gNumber efgGame::GetChanceProb(const Action *a) const
{
  return GetChanceProbs(a->BelongsTo())[a->GetNumber()];
}

gArray<gNumber> efgGame::GetChanceProbs(Infoset *infoset) const
{
  if (infoset->IsChanceInfoset())
    return ((ChanceInfoset *) infoset)->GetActionProbs();
  else
    return gArray<gNumber>(infoset->NumActions());
}

//---------------------------------------------------------------------
//                     Subgame-related functions
//---------------------------------------------------------------------

void efgGame::MarkTree(Node *n, Node *base)
{
  n->ptr = base;
  for (int i = 1; i <= n->NumChildren(); i++)
    MarkTree(n->GetChild(i), base);
}

bool efgGame::CheckTree(Node *n, Node *base)
{
  int i;

  if (n->NumChildren() == 0)   return true;

  for (i = 1; i <= n->NumChildren(); i++)
    if (!CheckTree(n->GetChild(i), base))  return false;

  if (n->GetPlayer().IsChance())   return true;

  for (i = 1; i <= n->GetInfoset()->NumMembers(); i++)
    if (n->GetInfoset()->GetMember(i)->ptr != base)
      return false;

  return true;
}

bool efgGame::IsLegalSubgame(Node *n)
{
  if (n->NumChildren() == 0)  
    return false;

  MarkTree(n, n);
  return CheckTree(n, n);
}

bool efgGame::MarkSubgame(Node *n)
{
  if(n->gameroot == n) return true;

  if (n->gameroot != n && IsLegalSubgame(n))  {
    n->gameroot = 0;
    MarkSubgame(n, n);
    return true;
  }

  return false;
}

void efgGame::UnmarkSubgame(Node *n)
{
  if (n->gameroot == n && n->parent)  {
    n->gameroot = 0;
    MarkSubgame(n, n->parent->gameroot);
  }
}
  

void efgGame::MarkSubgame(Node *n, Node *base)
{
  if (n->gameroot == n)  return;
  n->gameroot = base;
  for (int i = 1; i <= n->NumChildren(); i++)
    MarkSubgame(n->GetChild(i), base);
}

void efgGame::MarkSubgames(const gList<Node *> &list)
{
  for (int i = 1; i <= list.Length(); i++)  {
    if (IsLegalSubgame(list[i]))  {
      list[i]->gameroot = 0;
      MarkSubgame(list[i], list[i]);
    }
  }
}

void efgGame::MarkSubgames(void)
{
  gList<Node *> subgames;
  LegalSubgameRoots(*this, subgames);

  for (int i = 1; i <= subgames.Length(); i++)  {
    subgames[i]->gameroot = 0;
    MarkSubgame(subgames[i], subgames[i]);
  }
}

void efgGame::UnmarkSubgames(Node *n)
{
  if (n->NumChildren() == 0)   return;

  for (int i = 1; i <= n->NumChildren(); i++)
    UnmarkSubgames(n->GetChild(i));
  
  if (n->gameroot == n && n->parent)  {
    n->gameroot = 0;
    MarkSubgame(n, n->parent->gameroot);
  }
}


int efgGame::ProfileLength(void) const
{
  int sum = 0;

  for (int i = 1; i <= players.Length(); i++) {
    for (int j = 1; j <= players[i]->m_infosets.Length(); j++) {
      sum += players[i]->m_infosets[j]->actions.Length();
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
      bar(i, j) = players[i]->m_infosets[j]->NumActions();
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
      bar(i, j) = players[i]->m_infosets[j]->NumMembers();
    }
  }

  return bar;
}

//------------------------------------------------------------------------
//                       Efg: Payoff computation
//------------------------------------------------------------------------

void efgGame::Payoff(Node *n, gNumber prob, const gPVector<int> &profile,
		 gVector<gNumber> &payoff) const
{
  if (n->outcome)  {
    for (int i = 1; i <= players.Length(); i++)
      payoff[i] += prob * n->outcome->m_payoffs[i];
  }

  if (n->infoset && n->infoset->player->m_id == 0) {
    for (int i = 1; i <= n->children.Length(); i++) {
      Payoff(n->children[i],
	     prob * GetChanceProb(n->infoset, i),
	     profile, payoff);
    }
  }
  else if (n->infoset) {
    Payoff(n->children[profile(n->infoset->player->m_id, n->infoset->number)],
	   prob, profile, payoff);
  }
}

void efgGame::InfosetProbs(Node *n, gNumber prob, const gPVector<int> &profile,
			  gPVector<gNumber> &probs) const
{
  if (n->infoset && n->infoset->player->m_id == 0) {
    for (int i = 1; i <= n->children.Length(); i++) {
      InfosetProbs(n->children[i],
		   prob * GetChanceProb(n->infoset, i),
		   profile, probs);
    }
  }
  else if (n->infoset)  {
    probs(n->infoset->player->m_id, n->infoset->number) += prob;
    InfosetProbs(n->children[profile(n->infoset->player->m_id, 
				     n->infoset->number)],
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

void efgGame::Payoff(Node *n, gNumber prob, const gArray<gArray<int> *> &profile,
		    gArray<gNumber> &payoff) const
{
  if (n->outcome)   {
    for (int i = 1; i <= players.Length(); i++)
      payoff[i] += prob * n->outcome->m_payoffs[i];
  }
  
  if (n->infoset && n->infoset->player->m_id == 0) {
    for (int i = 1; i <= n->children.Length(); i++) {
      Payoff(n->children[i],
	     prob * GetChanceProb(n->infoset, i),
	     profile, payoff);
    }
  }
  else if (n->infoset) {
    Payoff(n->children[(*profile[n->infoset->player->m_id])[n->infoset->number]],
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

