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

#include <iostream>

#include "libgambit.h"

//----------------------------------------------------------------------
//                 gbtEfgPlayer: Member function definitions
//----------------------------------------------------------------------

gbtEfgPlayerRep::~gbtEfgPlayerRep()
{
  while (m_infosets.Length())  m_infosets.Remove(1)->Invalidate();
}


//----------------------------------------------------------------------
//                 gbtEfgAction: Member function definitions
//----------------------------------------------------------------------

bool gbtEfgActionRep::Precedes(const gbtEfgNode &n) const
{
  gbtEfgNode node = n;

  while (node != node->GetGame()->GetRoot()) {
    if (node->GetPriorAction() == this) {
      return true;
    }
    else {
      node = node->GetParent();
    }
  }
  return false;
}

//----------------------------------------------------------------------
//                 Infoset: Member function definitions
//----------------------------------------------------------------------

gbtEfgInfosetRep::gbtEfgInfosetRep(gbtEfgGame *p_efg, int p_number,
				   gbtEfgPlayerRep *p_player, int p_actions)
  : m_efg(p_efg), m_number(p_number), m_player(p_player), 
    m_actions(p_actions), flag(0) 
{
  while (p_actions)   {
    m_actions[p_actions] = new gbtEfgActionRep(p_actions, "", this);
    p_actions--; 
  }

  if (p_player->IsChance()) {
    for (int act = 1; act <= m_actions.Length(); act++) {
      m_ratProbs.Append(gbtRational(1, m_actions.Length()));
      m_textProbs.Append(ToText(m_ratProbs[act]));
    }
  }
}

gbtEfgInfosetRep::~gbtEfgInfosetRep()  
{
  for (int act = 1; act <= m_actions.Length(); m_actions[act++]->Invalidate());
}

bool gbtEfgInfosetRep::IsChanceInfoset(void) const
{ return m_player->IsChance(); }

bool gbtEfgInfosetRep::Precedes(gbtEfgNode p_node) const
{
  while (p_node != p_node->GetGame()->GetRoot()) {
    if (p_node->GetInfoset() == this) {
      return true;
    }
    else {
      p_node = p_node->GetParent();
    }
  }
  return false;
}

gbtEfgAction gbtEfgInfosetRep::InsertAction(int where)
{
  gbtEfgActionRep *action = new gbtEfgActionRep(where, "", this);
  m_actions.Insert(action, where);
  for (; where <= m_actions.Length(); where++)
    m_actions[where]->m_number = where;

  if (m_player->IsChance()) {
    m_textProbs.Insert("0", where);
    m_ratProbs.Insert(gbtRational(0), where);
  }
  return action;
}

void gbtEfgInfosetRep::RemoveAction(int which)
{
  m_actions.Remove(which)->Invalidate();
  for (; which <= m_actions.Length(); which++)
    m_actions[which]->m_number = which;

  if (m_player->IsChance()) {
    m_textProbs.Remove(which);
    m_ratProbs.Remove(which);
  }
}

void gbtEfgInfosetRep::SetActionProb(int act, const std::string &p_value)
{
  m_textProbs[act] = p_value;
  m_ratProbs[act] = ToRational(p_value);
  m_efg->DeleteLexicon();
}

//----------------------------------------------------------------------
//                 gbtEfgNode: Member function definitions
//----------------------------------------------------------------------

gbtEfgNodeRep::gbtEfgNodeRep(gbtEfgGame *e, gbtEfgNodeRep *p)
  : mark(false), number(0), m_efg(e), infoset(0), parent(p), outcome(0)
{ }

gbtEfgNodeRep::~gbtEfgNodeRep()
{
  for (int i = children.Length(); i; children[i--]->Invalidate());
}


gbtEfgNode gbtEfgNodeRep::GetNextSibling(void) const  
{
  if (!parent)   return 0;
  if (parent->children.Find(const_cast<gbtEfgNodeRep *>(this)) == 
      parent->children.Length())
    return 0;
  else
    return parent->children[parent->children.Find(const_cast<gbtEfgNodeRep *>(this)) + 1];
}

gbtEfgNode gbtEfgNodeRep::GetPriorSibling(void) const
{ 
  if (!parent)   return 0;
  if (parent->children.Find(const_cast<gbtEfgNodeRep *>(this)) == 1)
    return 0;
  else
    return parent->children[parent->children.Find(const_cast<gbtEfgNodeRep *>(this)) - 1];

}

gbtEfgAction gbtEfgNodeRep::GetPriorAction(void) const
{
  if (!parent) {
    return 0;
  }
  
  gbtEfgInfosetRep *infoset = GetParent()->GetInfoset();
  for (int i = 1; i <= infoset->NumActions(); i++) {
    if (this == GetParent()->GetChild(i)) {
      return infoset->GetAction(i);
    }
  }

  return 0;
}

void gbtEfgNodeRep::DeleteOutcome(gbtEfgOutcomeRep *outc)
{
  if (outc == outcome)   outcome = 0;
  for (int i = 1; i <= children.Length(); i++)
    children[i]->DeleteOutcome(outc);
}

void gbtEfgNodeRep::SetOutcome(const gbtEfgOutcome &p_outcome)
{
  if (p_outcome != outcome) {
    outcome = p_outcome;
    m_efg->DeleteLexicon();
  }
}

bool gbtEfgNodeRep::IsSuccessorOf(gbtEfgNode p_node) const
{
  gbtEfgNode n = const_cast<gbtEfgNodeRep *>(this);
  while (n && n != p_node) n = n->parent;
  return (n == p_node);
}

bool gbtEfgNodeRep::IsSubgameRoot(void) const
{
  if (children.Length() == 0)  return false;

  m_efg->MarkTree(const_cast<gbtEfgNodeRep *>(this), 
		  const_cast<gbtEfgNodeRep *>(this));
  return m_efg->CheckTree(const_cast<gbtEfgNodeRep *>(this),
			  const_cast<gbtEfgNodeRep *>(this));
}

//------------------------------------------------------------------------
//       Efg: Constructors, destructor, constructive operators
//------------------------------------------------------------------------

gbtEfgGame::gbtEfgGame(void)
  : sortisets(true), title("Untitled extensive form game"),
    chance(new gbtEfgPlayerRep(this, 0)), m_reducedNfg(0)
{
  m_root = new gbtEfgNodeRep(this, 0);
  SortInfosets();
}

gbtEfgGame::gbtEfgGame(const gbtEfgGame &E, gbtEfgNode n /* = 0 */)
  : sortisets(false), title(E.title), comment(E.comment),
    players(E.players.Length()), outcomes(0, E.outcomes.Last()),
    chance(new gbtEfgPlayerRep(this, 0)), m_reducedNfg(0)
{
  for (int i = 1; i <= players.Length(); i++)  {
    (players[i] = new gbtEfgPlayerRep(this, i))->m_label = E.players[i]->m_label;
    for (int j = 1; j <= E.players[i]->m_infosets.Length(); j++)   {
      gbtEfgInfosetRep *s = new gbtEfgInfosetRep(this, j, players[i],
						 E.players[i]->m_infosets[j]->m_actions.Length());
      s->m_label = E.players[i]->m_infosets[j]->m_label;
      for (int k = 1; k <= s->m_actions.Length(); k++)
	s->m_actions[k]->m_label = E.players[i]->m_infosets[j]->m_actions[k]->m_label;
      players[i]->m_infosets.Append(s);
    }
  }

  for (int i = 1; i <= E.GetChance()->NumInfosets(); i++)   {
    gbtEfgInfosetRep *t = E.GetChance()->GetInfoset(i);
    gbtEfgInfosetRep *s = new gbtEfgInfosetRep(this, i, chance,
					       t->NumActions());
    s->m_label = t->m_label;
    for (int act = 1; act <= s->m_textProbs.Length(); act++) {
      s->m_textProbs[act] = t->m_textProbs[act];
      s->m_ratProbs[act] = t->m_ratProbs[act];
      s->m_actions[act]->m_label = t->m_actions[act]->m_label;
    }
    chance->m_infosets.Append(s);
  }

  for (int outc = 1; outc <= E.NumOutcomes(); outc++)  {
    outcomes[outc] = new gbtEfgOutcomeRep(this, outc);
    outcomes[outc]->m_label = E.outcomes[outc]->m_label;
    outcomes[outc]->m_textPayoffs = E.outcomes[outc]->m_textPayoffs;
    outcomes[outc]->m_ratPayoffs = E.outcomes[outc]->m_ratPayoffs;
    outcomes[outc]->m_doublePayoffs = E.outcomes[outc]->m_doublePayoffs;
  }

  m_root = new gbtEfgNodeRep(this, 0);
  CopySubtree(m_root, (n ? n : E.GetRoot()));
  
  if (n)   {
    for (int pl = 1; pl <= players.Length(); pl++)  {
      for (int i = 1; i <= players[pl]->m_infosets.Length(); i++)  {
	if (players[pl]->m_infosets[i]->m_members.Length() == 0)
	  players[pl]->m_infosets.Remove(i--)->Invalidate();
      }
    }
  }

  sortisets = true;
  SortInfosets();
}

gbtEfgGame::~gbtEfgGame()
{
  m_root->Invalidate();
  chance->Invalidate();

  for (int i = 1; i <= players.Length(); players[i++]->Invalidate());
  for (int i = 1; i <= outcomes.Last(); outcomes[i++]->Invalidate());

  if (m_reducedNfg) {
    delete m_reducedNfg;
    m_reducedNfg = 0;
  }
}

//------------------------------------------------------------------------
//                  Efg: Private member functions
//------------------------------------------------------------------------

void gbtEfgGame::DeleteLexicon(void) const
{
  if (!m_reducedNfg)  return;

  delete m_reducedNfg;
  m_reducedNfg = 0;
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl]->m_strategies = gbtList<gbtArray<int> >();
  }
}

void gbtEfgGame::NumberNodes(gbtEfgNodeRep *n, int &index)
{
  n->number = index++;
  for (int child = 1; child <= n->children.Length();
       NumberNodes(n->children[child++], index));
} 

static void NDoChild(const gbtEfgGame &e, 
		     gbtEfgNodeRep *n, gbtList<gbtEfgNode> &list)
{ 
  list.Append(n);
  for (int i = 1; i <= n->NumChildren(); i++)
    NDoChild (e, n->GetChild(i), list);
}

static void Nodes (const gbtEfgGame &befg, gbtList<gbtEfgNode> &list)
{
  list = gbtList<gbtEfgNode>();
  NDoChild(befg, befg.GetRoot(), list); 
}

static void Nodes (const gbtEfgGame &efg, gbtEfgNodeRep *n, gbtList<gbtEfgNode> &list)
{
  list = gbtList<gbtEfgNode>();
  NDoChild(efg,n, list);
}

// FIXME: Backport more efficient canonicalization methods from 
// development version!
void gbtEfgGame::SortInfosets(void)
{
  if (!sortisets)  return;

  int pl;

  for (pl = 0; pl <= players.Length(); pl++)  {
    gbtList<gbtEfgNode> nodes;

    Nodes(*this, nodes);

    gbtEfgPlayerRep *player = (pl) ? players[pl] : chance;

    int i, isets = 0;

    // First, move all empty infosets to the back of the list so
    // we don't "lose" them
    int foo = player->m_infosets.Length();
    i = 1;
    while (i < foo)   {
      if (player->m_infosets[i]->m_members.Length() == 0)  {
	gbtEfgInfosetRep *bar = player->m_infosets[i];
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
	player->m_infosets[i]->m_number = i;
      else
	player->m_infosets[i]->m_number = 0;
  
    for (i = 1; i <= nodes.Length(); i++)  {
      gbtEfgNodeRep *n = nodes[i];
      if (n->GetPlayer() == player && n->GetInfoset()->m_number == 0)  {
	n->GetInfoset()->m_number = ++isets;
	player->m_infosets[isets] = n->GetInfoset();
      }
    }  
  }

  // Now, we sort the nodes within the infosets
  
  gbtList<gbtEfgNode> nodes;
  Nodes(*this, nodes);

  for (pl = 0; pl <= players.Length(); pl++)  {
    gbtEfgPlayerRep *player = (pl) ? players[pl] : chance;

    for (int iset = 1; iset <= player->m_infosets.Length(); iset++)  {
      gbtEfgInfosetRep *s = player->m_infosets[iset];
      for (int i = 1, j = 1; i <= nodes.Length(); i++)  {
	if (nodes[i]->infoset == s)
	  s->m_members[j++] = nodes[i];
      }
    }
  }

  int nodeindex = 1;
  NumberNodes(m_root, nodeindex);
}

gbtEfgInfosetRep *gbtEfgGame::CreateInfoset(int n, gbtEfgPlayerRep *p, int br)
{
  gbtEfgInfosetRep *s = new gbtEfgInfosetRep(this, n, p, br);
  p->m_infosets.Append(s);
  return s;
}


void gbtEfgGame::CopySubtree(gbtEfgNodeRep *n, gbtEfgNodeRep *m)
{
  n->m_label = m->m_label;

  if (m->outcome) {
    n->outcome = m->outcome;
  }

  if (m->infoset)   {
    gbtEfgPlayerRep *p;
    if (m->infoset->m_player->m_number)
      p = players[m->infoset->m_player->m_number];
    else
      p = chance;

    gbtEfgInfosetRep *s = p->GetInfoset(m->infoset->m_number);
    AppendNode(n, s);

    for (int i = 1; i <= n->children.Length(); i++)
      CopySubtree(n->children[i], m->children[i]);
  }
}

//------------------------------------------------------------------------
//               Efg: Title access and manipulation
//------------------------------------------------------------------------

void gbtEfgGame::SetTitle(const std::string &s)
{
  title = s; 
}

const std::string &gbtEfgGame::GetTitle(void) const
{ return title; }

void gbtEfgGame::SetComment(const std::string &s)
{
  comment = s;
}

const std::string &gbtEfgGame::GetComment(void) const
{ return comment; }
  

//------------------------------------------------------------------------
//                    Efg: Writing data files
//------------------------------------------------------------------------

static std::string EscapeQuotes(const std::string &s)
{
  std::string ret;
  
  for (unsigned int i = 0; i < s.length(); i++)  {
    if (s[i] == '"')   ret += '\\';
    ret += s[i];
  }

  return ret;
}

static void PrintActions(std::ostream &p_stream, gbtEfgInfosetRep *p_infoset)
{ 
  p_stream << "{ ";
  for (int act = 1; act <= p_infoset->NumActions(); act++) {
    p_stream << '"' << EscapeQuotes(p_infoset->GetAction(act)->GetLabel()) << "\" ";
    if (p_infoset->IsChanceInfoset()) {
      p_stream << p_infoset->GetActionProb(act) << ' ';
    }
  }
  p_stream << "}";
}

void gbtEfgGame::WriteEfgFile(std::ostream &f, gbtEfgNodeRep *n) const
{
  if (n->children.Length() == 0)   {
    f << "t \"" << EscapeQuotes(n->m_label) << "\" ";
    if (n->outcome)  {
      f << n->outcome->m_number << " \"" <<
	EscapeQuotes(n->outcome->m_label) << "\" ";
      f << "{ ";
      for (int pl = 1; pl <= NumPlayers(); pl++)  {
	f << n->outcome->m_textPayoffs[pl];

	if (pl < NumPlayers())
	  f << ", ";
	else
	  f << " }\n";
      }
    }
    else
      f << "0\n";
  }

  else if (n->infoset->m_player->m_number)   {
    f << "p \"" << EscapeQuotes(n->m_label) << "\" " <<
      n->infoset->m_player->m_number << ' ';
    f << n->infoset->m_number << " \"" <<
      EscapeQuotes(n->infoset->m_label) << "\" ";
    PrintActions(f, n->infoset);
    f << " ";
    if (n->outcome)  {
      f << n->outcome->m_number << " \"" <<
	EscapeQuotes(n->outcome->m_label) << "\" ";
      f << "{ ";
      for (int pl = 1; pl <= NumPlayers(); pl++)  {
	f << n->outcome->m_textPayoffs[pl];

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
    f << n->infoset->m_number << " \"" <<
      EscapeQuotes(n->infoset->m_label) << "\" ";
    PrintActions(f, n->infoset);
    f << " ";
    if (n->outcome)  {
      f << n->outcome->m_number << " \"" <<
	EscapeQuotes(n->outcome->m_label) << "\" ";
      f << "{ ";
      for (int pl = 1; pl <= NumPlayers(); pl++)  {
	f << n->outcome->m_textPayoffs[pl];

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

void gbtEfgGame::WriteEfgFile(std::ostream &p_file) const
{
  p_file << "EFG 2 R";
  p_file << " \"" << EscapeQuotes(title) << "\" { ";
  for (int i = 1; i <= players.Length(); i++)
    p_file << '"' << EscapeQuotes(players[i]->m_label) << "\" ";
  p_file << "}\n";
  p_file << "\"" << EscapeQuotes(comment) << "\"\n\n";

  WriteEfgFile(p_file, m_root);
}


//------------------------------------------------------------------------
//                    Efg: General data access
//------------------------------------------------------------------------

int gbtEfgGame::NumPlayers(void) const
{ return players.Length(); }

gbtEfgPlayer gbtEfgGame::NewPlayer(void)
{
  gbtEfgPlayerRep *ret = new gbtEfgPlayerRep(this, players.Length() + 1);
  players.Append(ret);

  for (int outc = 1; outc <= outcomes.Last(); outc++) {
    outcomes[outc]->m_textPayoffs.Append("0");
    outcomes[outc]->m_ratPayoffs.Append(0);
    outcomes[outc]->m_doublePayoffs.Append(0.0);
  }

  DeleteLexicon();
  return ret;
}

/*
gbtEfgArray<gbtEfgInfoset *> gbtEfgGame::Infosets() const
{
  gbtEfgArray<gbtEfgInfoset *> answer;

  gbtEfgArray<gbtEfgPlayer *> p = players;
  int i;
  for (i = 1; i <= p.Length(); i++) {
    gbtEfgArray<gbtEfgInfoset *> infosets_for_player = p[i]->Infosets();
    int j;
    for (j = 1; j <= infosets_for_player.Length(); j++)
      answer.Append(infosets_for_player[j]);
  }

  return answer;
}
*/

gbtEfgInfoset gbtEfgGame::GetInfoset(int p_index) const
{
  int index = 1;
  for (int pl = 1; pl <= players.Length(); pl++) {
    gbtEfgPlayerRep *player = players[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      if (index++ == p_index) {
	return player->GetInfoset(iset);
      }
    }
  }
  throw gbtIndexException();
}

gbtEfgAction gbtEfgGame::GetAction(int p_index) const
{
  int index = 1;
  for (int pl = 1; pl <= players.Length(); pl++) {
    gbtEfgPlayerRep *player = players[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      gbtEfgInfosetRep *infoset = player->GetInfoset(iset);
      for (int act = 1; act <= infoset->NumActions(); act++) {
	if (index++ == p_index) {
	  return infoset->GetAction(act);
	}
      }
    }
  }
  throw gbtIndexException();
}

int gbtEfgGame::NumOutcomes(void) const
{ return outcomes.Last(); }

gbtEfgOutcome gbtEfgGame::NewOutcome(void)
{
  outcomes.Append(new gbtEfgOutcomeRep(this, outcomes.Length() + 1));
  return outcomes[outcomes.Last()];
}

void gbtEfgGame::DeleteOutcome(const gbtEfgOutcome &p_outcome)
{
  m_root->DeleteOutcome(p_outcome);
  outcomes.Remove(outcomes.Find(p_outcome))->Invalidate();
  DeleteLexicon();
}

gbtEfgOutcome gbtEfgGame::GetOutcome(int p_index) const
{
  return outcomes[p_index];
}

bool gbtEfgGame::IsConstSum(void) const
{
  EfgContIter iter(*this);

  gbtRational sum(0);
  for (int pl = 1; pl <= players.Length(); pl++) {
    sum += iter.Payoff(pl);
  }

  while (iter.NextContingency()) {
    gbtRational newsum(0);
    for (int pl = 1; pl <= players.Length(); pl++) {
      newsum += iter.Payoff(pl);
    }

    if (newsum != sum) {
      return false;
    }
  }

  return true;
}

gbtRational gbtEfgGame::GetMinPayoff(int pl) const
{
  int index, p, p1, p2;
  gbtRational minpay;

  if (NumOutcomes() == 0)  return 0;

  if(pl) { p1=p2=pl;}
  else {p1=1;p2=players.Length();}

  minpay = outcomes[1]->m_ratPayoffs[p1];

  for (index = 1; index <= outcomes.Last(); index++)  {
    for (p = p1; p <= p2; p++)
      if (outcomes[index]->m_ratPayoffs[p] < minpay)
	minpay = outcomes[index]->m_ratPayoffs[p];
  }
  return minpay;
}

gbtRational gbtEfgGame::GetMaxPayoff(int pl) const
{
  int index, p, p1, p2;
  gbtRational maxpay;

  if (NumOutcomes() == 0)  return 0;

  if(pl) { p1=p2=pl;}
  else {p1=1;p2=players.Length();}

  maxpay = outcomes[1]->m_ratPayoffs[p1];

  for (index = 1; index <= outcomes.Last(); index++)  {
    for (p = p1; p <= p2; p++)
      if (outcomes[index]->m_ratPayoffs[p] > maxpay)
	maxpay = outcomes[index]->m_ratPayoffs[p];
  }
  return maxpay;
}

bool gbtEfgGame::IsPerfectRecall(gbtEfgInfoset &s1, gbtEfgInfoset &s2) const
{
  for (int pl = 1; pl <= players.Length(); pl++)   {
    gbtEfgPlayerRep *player = players[pl];
    
    for (int i = 1; i <= player->NumInfosets(); i++)  {
      gbtEfgInfosetRep *iset1 = player->GetInfoset(i);
      for (int j = 1; j <= player->NumInfosets(); j++)   {
	gbtEfgInfosetRep *iset2 = player->GetInfoset(j);

	bool precedes = false;
	int action = 0;
	
	for (int m = 1; m <= iset2->NumMembers(); m++)  {
	  int n;
	  for (n = 1; n <= iset1->NumMembers(); n++)  {
	    if (iset2->GetMember(m)->IsSuccessorOf(iset1->GetMember(n)) &&
	        iset1->GetMember(n) != iset2->GetMember(m))  {
	      precedes = true;
	      for (int act = 1; act <= iset1->NumActions(); act++)  {
		if (iset2->GetMember(m)->IsSuccessorOf(iset1->GetMember(n)->GetChild(act))) {
		  if (action != 0 && action != act)  {
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
	  
	  if (i == j && precedes)  {
	    s1 = iset1;
	    s2 = iset2;
	    return false;
	  }

	  if (n > iset1->NumMembers() && precedes)  {
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

//------------------------------------------------------------------------
//                     Efg: Operations on players
//------------------------------------------------------------------------

gbtEfgPlayer gbtEfgGame::GetChance(void) const
{
  return chance;
}

gbtEfgInfoset gbtEfgGame::AppendNode(gbtEfgNode n, gbtEfgPlayer p, int count)
{
  if (!n || !p || count == 0)
    throw gbtEfgException();

  if (n->children.Length() == 0)   {
    n->infoset = CreateInfoset(p->m_infosets.Length() + 1, p, count);
    n->infoset->m_members.Append(n);
    while (count--)
      n->children.Append(new gbtEfgNodeRep(this, n));
  }

  DeleteLexicon();
  SortInfosets();
  return n->infoset;
}  

gbtEfgInfoset gbtEfgGame::AppendNode(gbtEfgNode n, gbtEfgInfoset s)
{
  if (!n || !s)   throw gbtEfgException();
  
  if (n->children.Length() == 0)   {
    n->infoset = s;
    s->m_members.Append(n);
    for (int i = 1; i <= s->m_actions.Length(); i++)
      n->children.Append(new gbtEfgNodeRep(this, n));
  }

  DeleteLexicon();
  SortInfosets();
  return s;
}
  
gbtEfgNode gbtEfgGame::DeleteNode(gbtEfgNode n, gbtEfgNode keep)
{
  if (!n || !keep)   throw gbtEfgException();

  if (keep->parent != n)   return n;

  // turn infoset sorting off during tree deletion -- problems will occur
  sortisets = false;

  n->children.Remove(n->children.Find(keep));
  DeleteTree(n);
  keep->parent = n->parent;
  if (n->parent)
    n->parent->children[n->parent->children.Find(n)] = keep;
  else
    m_root = keep;

  n->Invalidate();
  DeleteLexicon();

  sortisets = true;

  SortInfosets();
  return keep;
}

gbtEfgInfoset gbtEfgGame::InsertNode(gbtEfgNode n, gbtEfgPlayer p, int count)
{
  if (!n || !p || count <= 0)  throw gbtEfgException();

  gbtEfgNodeRep *m = new gbtEfgNodeRep(this, n->parent);
  m->infoset = CreateInfoset(p->m_infosets.Length() + 1, p, count);
  m->infoset->m_members.Append(m);
  if (n->parent)
    n->parent->children[n->parent->children.Find(n)] = m;
  else
    m_root = m;
  m->children.Append(n);
  n->parent = m;
  while (--count)
    m->children.Append(new gbtEfgNodeRep(this, m));

  DeleteLexicon();
  SortInfosets();
  return m->infoset;
}

gbtEfgInfoset gbtEfgGame::InsertNode(gbtEfgNode n, gbtEfgInfoset s)
{
  if (!n || !s)  throw gbtEfgException();

  gbtEfgNodeRep *m = new gbtEfgNodeRep(this, n->parent);
  m->infoset = s;
  s->m_members.Append(m);
  if (n->parent)
    n->parent->children[n->parent->children.Find(n)] = m;
  else
    m_root = m;
  m->children.Append(n);
  n->parent = m;
  int count = s->m_actions.Length();
  while (--count)
    m->children.Append(new gbtEfgNodeRep(this, m));

  DeleteLexicon();
  SortInfosets();
  return m->infoset;
}

gbtEfgInfoset gbtEfgGame::CreateInfoset(gbtEfgPlayer p, int br)
{
  if (!p || p->GetGame() != this)  throw gbtEfgException();

  return CreateInfoset(p->m_infosets.Length() + 1, p, br);
}

gbtEfgInfoset gbtEfgGame::JoinInfoset(gbtEfgInfoset s, gbtEfgNode n)
{
  if (!n || !s)  throw gbtEfgException();

  if (!n->infoset)   return 0; 
  if (n->infoset == s)   return s;
  if (s->m_actions.Length() != n->children.Length())  return n->infoset;

  gbtEfgInfosetRep *t = n->infoset;

  t->m_members.Remove(t->m_members.Find(n));
  s->m_members.Append(n);

  n->infoset = s;

  DeleteLexicon();
  SortInfosets();
  return s;
}

gbtEfgInfoset gbtEfgGame::LeaveInfoset(gbtEfgNode n)
{
  if (!n)  throw gbtEfgException();

  if (!n->infoset)   return 0;

  gbtEfgInfosetRep *s = n->infoset;
  if (s->m_members.Length() == 1)   return s;

  gbtEfgPlayerRep *p = s->m_player;
  s->m_members.Remove(s->m_members.Find(n));
  n->infoset = CreateInfoset(p->m_infosets.Length() + 1, p,
			     n->children.Length());
  n->infoset->m_label = s->m_label;
  n->infoset->m_members.Append(n);
  for (int i = 1; i <= s->m_actions.Length(); i++)
    n->infoset->m_actions[i]->m_label = s->m_actions[i]->m_label;

  DeleteLexicon();
  SortInfosets();
  return n->infoset;
}

gbtEfgInfoset gbtEfgGame::SplitInfoset(gbtEfgNode n)
{
  if (!n)  throw gbtEfgException();

  if (!n->infoset)   return 0;

  gbtEfgInfosetRep *s = n->infoset;
  if (s->m_members.Length() == 1)   return s;

  gbtEfgPlayerRep *p = s->m_player;
  gbtEfgInfosetRep *ns = CreateInfoset(p->m_infosets.Length() + 1, p,
				       n->children.Length());
  ns->m_label = s->m_label;
  int i;
  for (i = s->m_members.Length(); i > s->m_members.Find(n); i--)   {
    gbtEfgNodeRep *nn = s->m_members.Remove(i);
    ns->m_members.Append(nn);
    nn->infoset = ns;
  }
  for (i = 1; i <= s->m_actions.Length(); i++) {
    ns->m_actions[i]->m_label = s->m_actions[i]->m_label;
    if (p == chance) {
      ns->SetActionProb(i, s->GetActionProbText(i));
    }
  }
  DeleteLexicon();
  SortInfosets();
  return n->infoset;
}

gbtEfgInfoset gbtEfgGame::MergeInfoset(gbtEfgInfoset to, gbtEfgInfoset from)
{
  if (!to || !from)  throw gbtEfgException();

  if (to == from ||
      to->m_actions.Length() != from->m_actions.Length())   return from;

  for (int i = 1; i <= from->m_members.Length(); i++) {
    to->m_members.Append(from->m_members[i]);
  }

  for (int i = 1; i <= from->m_members.Length(); i++)
    from->m_members[i]->infoset = to;

  from->m_members = gbtArray<gbtEfgNodeRep *>();

  DeleteLexicon();
  SortInfosets();
  return to;
}

bool gbtEfgGame::DeleteEmptyInfoset(gbtEfgInfoset s)
{
  if (!s)  throw gbtEfgException();

  if (s->NumMembers() > 0)   return false;

  s->m_player->m_infosets.Remove(s->m_player->m_infosets.Find(s));
  s->Invalidate();

  return true;
}

void gbtEfgGame::DeleteEmptyInfosets(void)
{
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    for (int iset = 1; iset <= NumInfosets()[pl]; iset++) {
      if (DeleteEmptyInfoset(players[pl]->GetInfoset(iset))) {
        iset--;
      }
    }
  }
} 

gbtEfgInfoset gbtEfgGame::SwitchPlayer(gbtEfgInfoset s, gbtEfgPlayer p)
{
  if (!s || !p)  throw gbtEfgException();
  if (s->GetPlayer()->IsChance() || p->IsChance())  throw gbtEfgException();
  
  if (s->m_player == p)   return s;

  s->m_player->m_infosets.Remove(s->m_player->m_infosets.Find(s));
  s->m_player = p;
  p->m_infosets.Append(s);

  DeleteLexicon();
  SortInfosets();
  return s;
}

void gbtEfgGame::CopySubtree(gbtEfgNodeRep *src, gbtEfgNodeRep *dest, 
			     gbtEfgNodeRep *stop)
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

  dest->m_label = src->m_label;
  dest->outcome = src->outcome;
}

//
// MarkSubtree: sets the Node::mark flag on all children of p_node
//
void gbtEfgGame::MarkSubtree(gbtEfgNodeRep *p_node)
{
  p_node->mark = true;
  for (int i = 1; i <= p_node->children.Length(); i++) {
    MarkSubtree(p_node->children[i]);
  }
}

//
// UnmarkSubtree: clears the Node::mark flag on all children of p_node
//
void gbtEfgGame::UnmarkSubtree(gbtEfgNodeRep *p_node)
{
  p_node->mark = false;
  for (int i = 1; i <= p_node->children.Length(); i++) {
    UnmarkSubtree(p_node->children[i]);
  }
}

void gbtEfgGame::Reveal(gbtEfgInfoset where, 
			const gbtArray<gbtEfgPlayer> &who)
{
  UnmarkSubtree(m_root);  // start with a clean tree
  
  for (int i = 1; i <= where->m_actions.Length(); i++) {
    for (int j = 1; j <= where->m_members.Length(); j++) { 
      MarkSubtree(where->m_members[j]->children[i]);
    }

    for (int j = who.First(); j <= who.Last(); j++) {
      // iterate over each information set of player 'j' in the list
      for (int k = 1; k <= who[j]->m_infosets.Length(); k++) {
	// iterate over each member of information set 'k'
	// make copy of members to iterate correctly 
	// (since the information set may be changed in the process)
	gbtArray<gbtEfgNodeRep *> members = who[j]->m_infosets[k]->m_members;
	gbtEfgInfoset newiset;

	for (int m = 1; m <= members.Length(); m++) {
	  gbtEfgNodeRep *n = members[m];
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
  }

  DeleteLexicon();
  SortInfosets();
}

gbtEfgNode gbtEfgGame::CopyTree(gbtEfgNode src, gbtEfgNode dest)
{
  if (!src || !dest)  throw gbtEfgException();
  if (src == dest || dest->children.Length())   return src;

  if (src->children.Length())  {

    AppendNode(dest, src->infoset);
    for (int i = 1; i <= src->children.Length(); i++)
      CopySubtree(src->children[i], dest->children[i], dest);

    DeleteLexicon();
    SortInfosets();
  }

  return dest;
}

gbtEfgNode gbtEfgGame::MoveTree(gbtEfgNode src, gbtEfgNode dest)
{
  if (!src || !dest)  throw gbtEfgException();
  if (src == dest || dest->children.Length() || dest->IsSuccessorOf(src)) {
    return src;
  }
  if (src->parent == dest->parent) {
    int srcChild = src->parent->children.Find(src);
    int destChild = src->parent->children.Find(dest);
    src->parent->children[srcChild] = dest;
    src->parent->children[destChild] = src;
  }
  else {
    gbtEfgNodeRep *parent = src->parent; 
    parent->children[parent->children.Find(src)] = dest;
    dest->parent->children[dest->parent->children.Find(dest)] = src;
    src->parent = dest->parent;
    dest->parent = parent;
  }

  dest->m_label = "";
  dest->outcome = 0;
  
  DeleteLexicon();
  SortInfosets();
  return dest;
}

gbtEfgNode gbtEfgGame::DeleteTree(gbtEfgNode n)
{
  if (!n)  throw gbtEfgException();

  while (n->NumChildren() > 0)   {
    DeleteTree(n->children[1]);
    n->children.Remove(1)->Invalidate();
  }
  
  if (n->infoset)  {
    n->infoset->m_members.Remove(n->infoset->m_members.Find(n));
    n->infoset = 0;
  }
  n->outcome = 0;
  n->m_label = "";

  DeleteLexicon();
  SortInfosets();
  return n;
}

gbtEfgAction gbtEfgGame::InsertAction(gbtEfgInfoset s)
{
  if (!s)  throw gbtEfgException();

  gbtEfgActionRep *action = s->InsertAction(s->NumActions() + 1);
  for (int i = 1; i <= s->m_members.Length(); i++) {
    s->m_members[i]->children.Append(new gbtEfgNodeRep(this, s->m_members[i]));
  }
  DeleteLexicon();
  SortInfosets();
  return action;
}

gbtEfgAction gbtEfgGame::InsertAction(gbtEfgInfoset s, const gbtEfgAction &a)
{
  if (!a || !s)  throw gbtEfgException();

  int where;
  for (where = 1; where <= s->m_actions.Length() && s->m_actions[where] != a;
       where++);
  if (where > s->m_actions.Length())   return 0;
  gbtEfgActionRep *action = s->InsertAction(where);
  for (int i = 1; i <= s->m_members.Length(); i++)
    s->m_members[i]->children.Insert(new gbtEfgNodeRep(this, s->m_members[i]), where);

  DeleteLexicon();
  SortInfosets();
  return action;
}

gbtEfgInfoset gbtEfgGame::DeleteAction(gbtEfgInfoset s, const gbtEfgAction &a)
{
  if (!a || !s)  throw gbtEfgException();

  int where;
  for (where = 1; where <= s->m_actions.Length() && s->m_actions[where] != a;
       where++);
  if (where > s->m_actions.Length() || s->m_actions.Length() == 1)   return s;
  s->RemoveAction(where);
  for (int i = 1; i <= s->m_members.Length(); i++)   {
    DeleteTree(s->m_members[i]->children[where]);
    s->m_members[i]->children.Remove(where)->Invalidate();
  }
  DeleteLexicon();
  SortInfosets();
  return s;
}

//---------------------------------------------------------------------
//                     Subgame-related functions
//---------------------------------------------------------------------

void gbtEfgGame::MarkTree(gbtEfgNodeRep *n, gbtEfgNodeRep *base)
{
  n->ptr = base;
  for (int i = 1; i <= n->NumChildren(); i++)
    MarkTree(n->GetChild(i), base);
}

bool gbtEfgGame::CheckTree(gbtEfgNodeRep *n, gbtEfgNodeRep *base)
{
  int i;

  if (n->NumChildren() == 0)   return true;

  for (i = 1; i <= n->NumChildren(); i++)
    if (!CheckTree(n->GetChild(i), base))  return false;

  if (n->GetPlayer()->IsChance())   return true;

  for (i = 1; i <= n->GetInfoset()->NumMembers(); i++)
    if (n->GetInfoset()->GetMember(i)->ptr != base)
      return false;

  return true;
}

int gbtEfgGame::ProfileLength(void) const
{
  int sum = 0;

  for (int i = 1; i <= players.Length(); i++)
    for (int j = 1; j <= players[i]->m_infosets.Length(); j++)
      sum += players[i]->m_infosets[j]->m_actions.Length();

  return sum;
}

gbtArray<int> gbtEfgGame::NumInfosets(void) const
{
  gbtArray<int> foo(players.Length());
  
  for (int i = 1; i <= foo.Length(); i++) {
    foo[i] = players[i]->NumInfosets();
  }

  return foo;
}

gbtPVector<int> gbtEfgGame::NumActions(void) const
{
  gbtArray<int> foo(players.Length());
  int i;
  for (i = 1; i <= players.Length(); i++)
    foo[i] = players[i]->m_infosets.Length();

  gbtPVector<int> bar(foo);
  for (i = 1; i <= players.Length(); i++) {
    for (int j = 1; j <= players[i]->m_infosets.Length(); j++) {
      bar(i, j) = players[i]->m_infosets[j]->NumActions();
    }
  }

  return bar;
}  

static int CountNodes(gbtEfgNodeRep *p_node)
{
  int num = 1;
  for (int i = 1; i <= p_node->NumChildren(); 
       num += CountNodes(p_node->GetChild(i++)));
  return num;
}

int gbtEfgGame::NumNodes(void) const
{
  return CountNodes(m_root);
}


gbtPVector<int> gbtEfgGame::NumMembers(void) const
{
  gbtArray<int> foo(players.Length());

  for (int i = 1; i <= players.Length(); i++) {
    foo[i] = players[i]->NumInfosets();
  }

  gbtPVector<int> bar(foo);
  for (int i = 1; i <= players.Length(); i++) {
    for (int j = 1; j <= players[i]->NumInfosets(); j++) {
      bar(i, j) = players[i]->m_infosets[j]->NumMembers();
    }
  }

  return bar;
}

//------------------------------------------------------------------------
//                       Efg: Payoff computation
//------------------------------------------------------------------------

void gbtEfgGame::Payoff(gbtEfgNodeRep *n, gbtRational prob, 
			const gbtPVector<int> &profile,
			gbtVector<gbtRational> &payoff) const
{
  if (n->outcome)  {
    for (int i = 1; i <= players.Length(); i++)
      payoff[i] += prob * n->outcome->m_ratPayoffs[i];
  }

  if (n->infoset && n->infoset->m_player->IsChance())
    for (int i = 1; i <= n->children.Length(); i++)
      Payoff(n->children[i],
	     prob * n->infoset->GetActionProb(i),
	     profile, payoff);
  else if (n->infoset)
    Payoff(n->children[profile(n->infoset->m_player->m_number,n->infoset->m_number)],
	   prob, profile, payoff);
}

void gbtEfgGame::InfosetProbs(gbtEfgNodeRep *n, gbtRational prob, 
			      const gbtPVector<int> &profile,
			      gbtPVector<gbtRational> &probs) const
{
  if (n->infoset && n->infoset->m_player->IsChance())
    for (int i = 1; i <= n->children.Length(); i++)
      InfosetProbs(n->children[i],
		   prob * n->infoset->GetActionProb(i),
		   profile, probs);
  else if (n->infoset)  {
    probs(n->infoset->m_player->m_number, n->infoset->m_number) += prob;
    InfosetProbs(n->children[profile(n->infoset->m_player->m_number,n->infoset->m_number)],
		 prob, profile, probs);
  }
}

void gbtEfgGame::Payoff(const gbtPVector<int> &profile, 
			gbtVector<gbtRational> &payoff) const
{
  ((gbtVector<gbtRational> &) payoff).operator=(gbtRational(0));
  Payoff(m_root, 1, profile, payoff);
}

void gbtEfgGame::InfosetProbs(const gbtPVector<int> &profile,
			      gbtPVector<gbtRational> &probs) const
{
  ((gbtVector<gbtRational> &) probs).operator=(gbtRational(0));
  InfosetProbs(m_root, 1, profile, probs);
}

void gbtEfgGame::Payoff(gbtEfgNodeRep *n, gbtRational prob, 
			const gbtArray<gbtArray<int> > &profile,
			gbtArray<gbtRational> &payoff) const
{
  if (n->outcome)   {
    for (int i = 1; i <= players.Length(); i++)
      payoff[i] += prob * n->outcome->m_ratPayoffs[i];
  }
  
  if (n->infoset && n->infoset->m_player->IsChance())
    for (int i = 1; i <= n->children.Length(); i++)
      Payoff(n->children[i],
	     prob * n->infoset->GetActionProb(i),
	     profile, payoff);
  else if (n->infoset)
    Payoff(n->children[profile[n->infoset->m_player->m_number][n->infoset->m_number]],
	   prob, profile, payoff);
}

void gbtEfgGame::Payoff(const gbtArray<gbtArray<int> > &profile,
			gbtArray<gbtRational> &payoff) const
{
  for (int i = 1; i <= payoff.Length(); i++)
    payoff[i] = 0;
  Payoff(m_root, 1, profile, payoff);
}

void MakeStrategy(gbtEfgPlayerRep *p)
{
  gbtArray<int> c(p->NumInfosets());
  
  for (int i = 1; i <= p->NumInfosets(); i++)  {
    if (p->GetInfoset(i)->flag == 1)
      c[i] = p->GetInfoset(i)->whichbranch;
    else
      c[i] = 0;
  }
  p->m_strategies.Append(c);
}

void MakeReducedStrats(gbtEfgPlayerRep *p, gbtEfgNodeRep *n, gbtEfgNodeRep *nn)
{
  int i;
  gbtEfgNodeRep *m, *mm;

  if (!n->GetParent())  n->ptr = 0;

  if (n->NumChildren() > 0)  {
    if (n->infoset->m_player == p)  {
      if (n->infoset->flag == 0)  {
	// we haven't visited this infoset before
	n->infoset->flag = 1;
	for (i = 1; i <= n->NumChildren(); i++)   {
	  gbtEfgNodeRep *m = n->GetChild(i);
	  n->whichbranch = m;
	  n->infoset->whichbranch = i;
	  MakeReducedStrats(p, m, nn);
	}
	n->infoset->flag = 0;
      }
      else  {
	// we have visited this infoset, take same action
	MakeReducedStrats(p, n->children[n->infoset->whichbranch], nn);
      }
    }
    else  {
      n->ptr = NULL;
      if (nn != NULL)
	n->ptr = nn->parent;
      n->whichbranch = n->children[1];
      if (n->infoset)
	n->infoset->whichbranch = 0;
      MakeReducedStrats(p, n->children[1], n->children[1]);
    }
  }
  else if (nn)  {
    for (; ; nn = nn->parent->ptr->whichbranch)  {
      m = nn->GetNextSibling();
      if (m || nn->parent->ptr == NULL)   break;
    }
    if (m)  {
      mm = m->parent->whichbranch;
      m->parent->whichbranch = m;
      MakeReducedStrats(p, m, m);
      m->parent->whichbranch = mm;
    }
    else
      MakeStrategy(p);
  }
  else
    MakeStrategy(p);
}

#include "nfgiter.h"

gbtNfgGame *gbtEfgGame::MakeReducedNfg(void)
{
  for (int i = 1; i <= players.Length(); i++) {
    MakeReducedStrats(players[i], m_root, NULL);
  }

  gbtArray<int> dim(players.Length());
  for (int i = 1; i <= players.Length(); i++)
    dim[i] = (players[i]->m_strategies.Length()) ? players[i]->m_strategies.Length() : 1;

  m_reducedNfg = new gbtNfgGame(dim);
  m_reducedNfg->efg = this;

  m_reducedNfg->SetTitle(GetTitle());

  for (int i = 1; i <= NumPlayers(); i++)   {
    m_reducedNfg->GetPlayer(i)->SetName(GetPlayer(i)->GetLabel());
    for (int j = 1; j <= players[i]->m_strategies.Length(); j++)   {
      std::string name;
      if (players[i]->m_strategies[j].Length() > 0) {
	for (int k = 1; k <= players[i]->m_strategies[j].Length(); k++)
	  if (players[i]->m_strategies[j][k] > 0)
	    name += ToText(players[i]->m_strategies[j][k]);
	  else
	    name += "*";
      }
      else {
	name = "*";
      }
      m_reducedNfg->GetPlayer(i)->GetStrategy(j)->SetName(name);
    }
  }

  gbtNfgSupport S(m_reducedNfg);
  gbtNfgContingencyIterator iter(S);
  gbtArray<gbtArray<int> > corr(NumPlayers());
  gbtArray<int> corrs(NumPlayers());
  for (int i = 1; i <= NumPlayers(); i++)  {
    corrs[i] = 1;
    corr[i] = players[i]->m_strategies[1];
  }

  gbtArray<gbtRational> value(NumPlayers());

  int pl = NumPlayers();
  while (1)  {
    Payoff(corr, value);

    iter.SetOutcome(m_reducedNfg->NewOutcome());
    for (int j = 1; j <= NumPlayers(); j++) {
      iter.GetOutcome()->SetPayoff(j, ToText(value[j]));
    }

    iter.NextContingency();
    while (pl > 0)   {
      corrs[pl]++;
      if (corrs[pl] <= players[pl]->m_strategies.Length())  {
	corr[pl] = players[pl]->m_strategies[corrs[pl]];
	break;
      }
      corrs[pl] = 1;
      corr[pl] = players[pl]->m_strategies[1];
      pl--;
    }

    if (pl == 0)  break;
    pl = NumPlayers();
  }

  return m_reducedNfg;
}

gbtEfgOutcomeRep::gbtEfgOutcomeRep(gbtEfgGame *p_efg, int p_number)
  : m_efg(p_efg), m_number(p_number), 
    m_textPayoffs(p_efg->NumPlayers()), 
    m_ratPayoffs(p_efg->NumPlayers()),
    m_doublePayoffs(p_efg->NumPlayers())
{
  for (int pl = 1; pl <= m_textPayoffs.Length(); pl++) {
    m_textPayoffs[pl] = "0";
    m_doublePayoffs[pl] = 0.0;
  }
}

void gbtEfgOutcomeRep::SetPayoff(int pl, const std::string &p_value)
{
  m_textPayoffs[pl] = p_value;
  // Note that ToRational() converts a decimal text string into
  // an exact fraction
  m_ratPayoffs[pl] = ToRational(p_value);
  m_doublePayoffs[pl] = (double) m_ratPayoffs[pl];
  m_efg->DeleteLexicon();
}

