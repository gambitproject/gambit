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

namespace Gambit {

//----------------------------------------------------------------------
//                 gbtEfgPlayer: Member function definitions
//----------------------------------------------------------------------

GamePlayerRep::~GamePlayerRep()
{
  while (m_infosets.Length())  m_infosets.Remove(1)->Invalidate();
}


//----------------------------------------------------------------------
//                 gbtEfgAction: Member function definitions
//----------------------------------------------------------------------

bool GameActionRep::Precedes(const GameNode &n) const
{
  GameNode node = n;

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

GameInfosetRep::GameInfosetRep(GameTreeRep *p_efg, int p_number,
			       GamePlayerRep *p_player, int p_actions)
  : m_efg(p_efg), m_number(p_number), m_player(p_player), 
    m_actions(p_actions), flag(0) 
{
  while (p_actions)   {
    m_actions[p_actions] = new GameActionRep(p_actions, "", this);
    p_actions--; 
  }

  if (p_player->IsChance()) {
    for (int act = 1; act <= m_actions.Length(); act++) {
      m_ratProbs.Append(gbtRational(1, m_actions.Length()));
      m_textProbs.Append(ToText(m_ratProbs[act]));
    }
  }
}

GameInfosetRep::~GameInfosetRep()  
{
  for (int act = 1; act <= m_actions.Length(); m_actions[act++]->Invalidate());
}

bool GameInfosetRep::IsChanceInfoset(void) const
{ return m_player->IsChance(); }

bool GameInfosetRep::Precedes(GameNode p_node) const
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

GameAction GameInfosetRep::InsertAction(int where)
{
  GameActionRep *action = new GameActionRep(where, "", this);
  m_actions.Insert(action, where);
  for (; where <= m_actions.Length(); where++)
    m_actions[where]->m_number = where;

  if (m_player->IsChance()) {
    m_textProbs.Insert("0", where);
    m_ratProbs.Insert(gbtRational(0), where);
  }
  return action;
}

void GameInfosetRep::RemoveAction(int which)
{
  m_actions.Remove(which)->Invalidate();
  for (; which <= m_actions.Length(); which++)
    m_actions[which]->m_number = which;

  if (m_player->IsChance()) {
    m_textProbs.Remove(which);
    m_ratProbs.Remove(which);
  }
}

void GameInfosetRep::SetActionProb(int act, const std::string &p_value)
{
  m_textProbs[act] = p_value;
  m_ratProbs[act] = ToRational(p_value);
  m_efg->ClearComputedValues();
}

//----------------------------------------------------------------------
//                 GameNode: Member function definitions
//----------------------------------------------------------------------

GameNodeRep::GameNodeRep(GameTreeRep *e, GameNodeRep *p)
  : mark(false), number(0), m_efg(e), infoset(0), parent(p), outcome(0)
{ }

GameNodeRep::~GameNodeRep()
{
  for (int i = children.Length(); i; children[i--]->Invalidate());
}


GameNode GameNodeRep::GetNextSibling(void) const  
{
  if (!parent)   return 0;
  if (parent->children.Find(const_cast<GameNodeRep *>(this)) == 
      parent->children.Length())
    return 0;
  else
    return parent->children[parent->children.Find(const_cast<GameNodeRep *>(this)) + 1];
}

GameNode GameNodeRep::GetPriorSibling(void) const
{ 
  if (!parent)   return 0;
  if (parent->children.Find(const_cast<GameNodeRep *>(this)) == 1)
    return 0;
  else
    return parent->children[parent->children.Find(const_cast<GameNodeRep *>(this)) - 1];

}

GameAction GameNodeRep::GetPriorAction(void) const
{
  if (!parent) {
    return 0;
  }
  
  GameInfosetRep *infoset = GetParent()->GetInfoset();
  for (int i = 1; i <= infoset->NumActions(); i++) {
    if (this == GetParent()->GetChild(i)) {
      return infoset->GetAction(i);
    }
  }

  return 0;
}

void GameNodeRep::DeleteOutcome(Gambit::GameOutcomeRep *outc)
{
  if (outc == outcome)   outcome = 0;
  for (int i = 1; i <= children.Length(); i++)
    children[i]->DeleteOutcome(outc);
}

void GameNodeRep::SetOutcome(const Gambit::GameOutcome &p_outcome)
{
  if (p_outcome != outcome) {
    outcome = p_outcome;
    m_efg->ClearComputedValues();
  }
}

bool GameNodeRep::IsSuccessorOf(GameNode p_node) const
{
  GameNode n = const_cast<GameNodeRep *>(this);
  while (n && n != p_node) n = n->parent;
  return (n == p_node);
}

bool GameNodeRep::IsSubgameRoot(void) const
{
  if (children.Length() == 0)  return false;

  m_efg->MarkTree(const_cast<GameNodeRep *>(this), 
		  const_cast<GameNodeRep *>(this));
  return m_efg->CheckTree(const_cast<GameNodeRep *>(this),
			  const_cast<GameNodeRep *>(this));
}

//------------------------------------------------------------------------
//       Efg: Constructors, destructor, constructive operators
//------------------------------------------------------------------------

GameTreeRep::GameTreeRep(void)
  : title("Untitled extensive game"),
    chance(new GamePlayerRep(this, 0)), m_reducedNfg(0)
{
  m_root = new GameNodeRep(this, 0);
}

GameTreeRep::~GameTreeRep()
{
  m_root->Invalidate();
  chance->Invalidate();

  for (int i = 1; i <= players.Length(); players[i++]->Invalidate());
  for (int i = 1; i <= outcomes.Last(); outcomes[i++]->Invalidate());

  if (m_reducedNfg) {
    m_reducedNfg = 0;
  }
}

//------------------------------------------------------------------------
//                  Efg: Private member functions
//------------------------------------------------------------------------

void GameTreeRep::ClearComputedValues(void) const
{
  if (!m_reducedNfg)  return;

  m_reducedNfg = 0;
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl]->m_strategies = gbtList<gbtArray<int> >();
  }
}

void GameTreeRep::NumberNodes(GameNodeRep *n, int &index)
{
  n->number = index++;
  for (int child = 1; child <= n->children.Length();
       NumberNodes(n->children[child++], index));
} 

static void NDoChild(const GameTree &e, 
		     GameNodeRep *n, gbtList<GameNode> &list)
{ 
  list.Append(n);
  for (int i = 1; i <= n->NumChildren(); i++)
    NDoChild (e, n->GetChild(i), list);
}

static void Nodes (const GameTree &befg, gbtList<GameNode> &list)
{
  list = gbtList<GameNode>();
  NDoChild(befg, befg->GetRoot(), list); 
}

static void Nodes (const GameTree &efg, GameNodeRep *n, gbtList<GameNode> &list)
{
  list = gbtList<GameNode>();
  NDoChild(efg,n, list);
}

void GameTreeRep::Canonicalize(void)
{
  int nodeindex = 1;
  NumberNodes(m_root, nodeindex);

  for (int pl = 0; pl <= players.Length(); pl++) {
    GamePlayerRep *player = (pl) ? players[pl] : chance;
    
    // Sort nodes within information sets according to ID.
    // Coded using a bubble sort for simplicity; large games might
    // find a quicksort worthwhile.
    for (int iset = 1; iset <= player->m_infosets.Length(); iset++) {
      GameInfosetRep *infoset = player->m_infosets[iset];
      for (int i = 1; i < infoset->m_members.Length(); i++) {
	for (int j = 1; j < infoset->m_members.Length() - i - 1; j++) {
	  if (infoset->m_members[j+1]->number < infoset->m_members[j]->number) {
	    GameNodeRep *tmp = infoset->m_members[j];
	    infoset->m_members[j] = infoset->m_members[j+1];
	    infoset->m_members[j+1] = tmp;
	  }
	}
      }
    }

    // Sort information sets by the smallest ID among their members
    // Coded using a bubble sort for simplicity; large games might
    // find a quicksort worthwhile.
    for (int i = 1; i < player->m_infosets.Length(); i++) {
      for (int j = 1; j < player->m_infosets.Length() - i - 1; j++) {
	int a = ((player->m_infosets[j+1]->m_members.Length()) ?
		 player->m_infosets[j+1]->m_members[1]->number : 0);
	int b = ((player->m_infosets[j]->m_members.Length()) ?
		 player->m_infosets[j]->m_members[1]->number : 0);

	if (a < b || b == 0) {
	  GameInfosetRep *tmp = player->m_infosets[j];
	  player->m_infosets[j] = player->m_infosets[j+1];
	  player->m_infosets[j+1] = tmp;
	}
      }
    }

    // Reassign information set IDs
    for (int iset = 1; iset <= player->m_infosets.Length(); iset++) {
      player->m_infosets[iset]->m_number = iset;
    }
  }

}

GameInfosetRep *GameTreeRep::CreateInfoset(int n, GamePlayerRep *p, int br)
{
  GameInfosetRep *s = new GameInfosetRep(this, n, p, br);
  p->m_infosets.Append(s);
  return s;
}



//------------------------------------------------------------------------
//               Efg: Title access and manipulation
//------------------------------------------------------------------------

void GameTreeRep::SetTitle(const std::string &s)
{
  title = s; 
}

const std::string &GameTreeRep::GetTitle(void) const
{ return title; }

void GameTreeRep::SetComment(const std::string &s)
{
  comment = s;
}

const std::string &GameTreeRep::GetComment(void) const
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

static void PrintActions(std::ostream &p_stream, GameInfosetRep *p_infoset)
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

void GameTreeRep::WriteEfgFile(std::ostream &f, GameNodeRep *n) const
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

void GameTreeRep::WriteEfgFile(std::ostream &p_file) const
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

int GameTreeRep::NumPlayers(void) const
{ return players.Length(); }

GamePlayer GameTreeRep::NewPlayer(void)
{
  GamePlayerRep *ret = new GamePlayerRep(this, players.Length() + 1);
  players.Append(ret);

  for (int outc = 1; outc <= outcomes.Last(); outc++) {
    outcomes[outc]->m_textPayoffs.Append("0");
    outcomes[outc]->m_ratPayoffs.Append(0);
  }

  ClearComputedValues();
  return ret;
}

GameInfoset GameTreeRep::GetInfoset(int p_index) const
{
  int index = 1;
  for (int pl = 1; pl <= players.Length(); pl++) {
    GamePlayerRep *player = players[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      if (index++ == p_index) {
	return player->GetInfoset(iset);
      }
    }
  }
  throw gbtIndexException();
}

GameAction GameTreeRep::GetAction(int p_index) const
{
  int index = 1;
  for (int pl = 1; pl <= players.Length(); pl++) {
    GamePlayerRep *player = players[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      GameInfosetRep *infoset = player->GetInfoset(iset);
      for (int act = 1; act <= infoset->NumActions(); act++) {
	if (index++ == p_index) {
	  return infoset->GetAction(act);
	}
      }
    }
  }
  throw gbtIndexException();
}

int GameTreeRep::NumOutcomes(void) const
{ return outcomes.Last(); }

Gambit::GameOutcome GameTreeRep::NewOutcome(void)
{
  outcomes.Append(new Gambit::GameOutcomeRep(this, outcomes.Length() + 1));
  return outcomes[outcomes.Last()];
}

void GameTreeRep::DeleteOutcome(const Gambit::GameOutcome &p_outcome)
{
  m_root->DeleteOutcome(p_outcome);
  outcomes.Remove(outcomes.Find(p_outcome))->Invalidate();
  ClearComputedValues();
}

Gambit::GameOutcome GameTreeRep::GetOutcome(int p_index) const
{
  return outcomes[p_index];
}

bool GameTreeRep::IsConstSum(void) const
{
  EfgContIter iter(gbtEfgSupport(const_cast<GameTreeRep *>(this)));

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

gbtRational GameTreeRep::GetMinPayoff(int pl) const
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

gbtRational GameTreeRep::GetMaxPayoff(int pl) const
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

bool GameTreeRep::IsPerfectRecall(GameInfoset &s1, GameInfoset &s2) const
{
  for (int pl = 1; pl <= players.Length(); pl++)   {
    GamePlayerRep *player = players[pl];
    
    for (int i = 1; i <= player->NumInfosets(); i++)  {
      GameInfosetRep *iset1 = player->GetInfoset(i);
      for (int j = 1; j <= player->NumInfosets(); j++)   {
	GameInfosetRep *iset2 = player->GetInfoset(j);

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

GamePlayer GameTreeRep::GetChance(void) const
{
  return chance;
}

GameInfoset GameTreeRep::AppendNode(GameNode n, GamePlayer p, int count)
{
  if (!n || !p || count == 0)
    throw gbtEfgException();

  if (n->children.Length() == 0)   {
    n->infoset = CreateInfoset(p->m_infosets.Length() + 1, p, count);
    n->infoset->m_members.Append(n);
    while (count--)
      n->children.Append(new GameNodeRep(this, n));
  }

  ClearComputedValues();
  return n->infoset;
}  

GameInfoset GameTreeRep::AppendNode(GameNode n, GameInfoset s)
{
  if (!n || !s)   throw gbtEfgException();
  
  if (n->children.Length() == 0)   {
    n->infoset = s;
    s->m_members.Append(n);
    for (int i = 1; i <= s->m_actions.Length(); i++)
      n->children.Append(new GameNodeRep(this, n));
  }

  ClearComputedValues();
  return s;
}
  
GameNode GameTreeRep::DeleteNode(GameNode n, GameNode keep)
{
  if (!n || !keep)   throw gbtEfgException();

  if (keep->parent != n)   return n;

  n->children.Remove(n->children.Find(keep));
  DeleteTree(n);
  keep->parent = n->parent;
  if (n->parent)
    n->parent->children[n->parent->children.Find(n)] = keep;
  else
    m_root = keep;

  n->Invalidate();
  ClearComputedValues();

  return keep;
}

GameInfoset GameTreeRep::InsertNode(GameNode n, GamePlayer p, int count)
{
  if (!n || !p || count <= 0)  throw gbtEfgException();

  GameNodeRep *m = new GameNodeRep(this, n->parent);
  m->infoset = CreateInfoset(p->m_infosets.Length() + 1, p, count);
  m->infoset->m_members.Append(m);
  if (n->parent)
    n->parent->children[n->parent->children.Find(n)] = m;
  else
    m_root = m;
  m->children.Append(n);
  n->parent = m;
  while (--count)
    m->children.Append(new GameNodeRep(this, m));

  ClearComputedValues();
  return m->infoset;
}

GameInfoset GameTreeRep::InsertNode(GameNode n, GameInfoset s)
{
  if (!n || !s)  throw gbtEfgException();

  GameNodeRep *m = new GameNodeRep(this, n->parent);
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
    m->children.Append(new GameNodeRep(this, m));

  ClearComputedValues();
  return m->infoset;
}

GameInfoset GameTreeRep::JoinInfoset(GameInfoset s, GameNode n)
{
  if (!n || !s)  throw gbtEfgException();

  if (!n->infoset)   return 0; 
  if (n->infoset == s)   return s;
  if (s->m_actions.Length() != n->children.Length())  return n->infoset;

  GameInfosetRep *t = n->infoset;

  t->m_members.Remove(t->m_members.Find(n));
  s->m_members.Append(n);

  n->infoset = s;

  if (t->m_members.Length() == 0) {
    t->m_player->m_infosets.Remove(t->m_player->m_infosets.Find(t));
    for (int i = 1; i <= t->m_player->m_infosets.Length(); i++) {
      t->m_player->m_infosets[i]->m_number = i;
    }
    t->Invalidate();
  }

  ClearComputedValues();
  return s;
}

GameInfoset GameTreeRep::LeaveInfoset(GameNode n)
{
  if (!n)  throw gbtEfgException();

  if (!n->infoset)   return 0;

  GameInfosetRep *s = n->infoset;
  if (s->m_members.Length() == 1)   return s;

  GamePlayerRep *p = s->m_player;
  s->m_members.Remove(s->m_members.Find(n));
  n->infoset = CreateInfoset(p->m_infosets.Length() + 1, p,
			     n->children.Length());
  n->infoset->m_label = s->m_label;
  n->infoset->m_members.Append(n);
  for (int i = 1; i <= s->m_actions.Length(); i++)
    n->infoset->m_actions[i]->m_label = s->m_actions[i]->m_label;

  ClearComputedValues();
  return n->infoset;
}

GameInfoset GameTreeRep::SplitInfoset(GameNode n)
{
  if (!n)  throw gbtEfgException();

  if (!n->infoset)   return 0;

  GameInfosetRep *s = n->infoset;
  if (s->m_members.Length() == 1)   return s;

  GamePlayerRep *p = s->m_player;
  GameInfosetRep *ns = CreateInfoset(p->m_infosets.Length() + 1, p,
				       n->children.Length());
  ns->m_label = s->m_label;
  int i;
  for (i = s->m_members.Length(); i > s->m_members.Find(n); i--)   {
    GameNodeRep *nn = s->m_members.Remove(i);
    ns->m_members.Append(nn);
    nn->infoset = ns;
  }
  for (i = 1; i <= s->m_actions.Length(); i++) {
    ns->m_actions[i]->m_label = s->m_actions[i]->m_label;
    if (p == chance) {
      ns->SetActionProb(i, s->GetActionProbText(i));
    }
  }
  ClearComputedValues();
  return n->infoset;
}

GameInfoset GameTreeRep::MergeInfoset(GameInfoset to, GameInfoset from)
{
  if (!to || !from)  throw gbtEfgException();

  if (to == from ||
      to->m_actions.Length() != from->m_actions.Length())   return from;

  for (int i = 1; i <= from->m_members.Length(); i++) {
    to->m_members.Append(from->m_members[i]);
  }

  for (int i = 1; i <= from->m_members.Length(); i++)
    from->m_members[i]->infoset = to;

  from->m_members = gbtArray<GameNodeRep *>();

  ClearComputedValues();
  return to;
}

GameInfoset GameTreeRep::SwitchPlayer(GameInfoset s, GamePlayer p)
{
  if (!s || !p)  throw gbtEfgException();
  if (s->GetPlayer()->IsChance() || p->IsChance())  throw gbtEfgException();
  
  if (s->m_player == p)   return s;

  s->m_player->m_infosets.Remove(s->m_player->m_infosets.Find(s));
  s->m_player = p;
  p->m_infosets.Append(s);

  ClearComputedValues();
  return s;
}

void GameTreeRep::CopySubtree(GameNodeRep *src, GameNodeRep *dest, 
			     GameNodeRep *stop)
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
void GameTreeRep::MarkSubtree(GameNodeRep *p_node)
{
  p_node->mark = true;
  for (int i = 1; i <= p_node->children.Length(); i++) {
    MarkSubtree(p_node->children[i]);
  }
}

//
// UnmarkSubtree: clears the Node::mark flag on all children of p_node
//
void GameTreeRep::UnmarkSubtree(GameNodeRep *p_node)
{
  p_node->mark = false;
  for (int i = 1; i <= p_node->children.Length(); i++) {
    UnmarkSubtree(p_node->children[i]);
  }
}

void GameTreeRep::Reveal(GameInfoset where, 
			const gbtArray<GamePlayer> &who)
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
	gbtArray<GameNodeRep *> members = who[j]->m_infosets[k]->m_members;
	GameInfoset newiset;

	for (int m = 1; m <= members.Length(); m++) {
	  GameNodeRep *n = members[m];
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

  ClearComputedValues();
}

GameNode GameTreeRep::CopyTree(GameNode src, GameNode dest)
{
  if (!src || !dest)  throw gbtEfgException();
  if (src == dest || dest->children.Length())   return src;

  if (src->children.Length())  {

    AppendNode(dest, src->infoset);
    for (int i = 1; i <= src->children.Length(); i++)
      CopySubtree(src->children[i], dest->children[i], dest);

    ClearComputedValues();
  }

  return dest;
}

GameNode GameTreeRep::MoveTree(GameNode src, GameNode dest)
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
    GameNodeRep *parent = src->parent; 
    parent->children[parent->children.Find(src)] = dest;
    dest->parent->children[dest->parent->children.Find(dest)] = src;
    src->parent = dest->parent;
    dest->parent = parent;
  }

  dest->m_label = "";
  dest->outcome = 0;
  
  ClearComputedValues();
  return dest;
}

GameNode GameTreeRep::DeleteTree(GameNode n)
{
  if (!n)  throw gbtEfgException();

  while (n->NumChildren() > 0)   {
    DeleteTree(n->children[1]);
    n->children.Remove(1)->Invalidate();
  }
  
  if (n->infoset)  {
    GameInfosetRep *infoset = n->infoset;
    GamePlayerRep *player = infoset->m_player;

    infoset->m_members.Remove(infoset->m_members.Find(n));
    if (infoset->m_members.Length() == 0) {
      player->m_infosets.Remove(player->m_infosets.Find(infoset));
      for (int i = 1; i <= player->m_infosets.Length(); i++) {
	player->m_infosets[i]->m_number = i;
      }
      infoset->Invalidate();
    }
    n->infoset = 0;
  }
  n->outcome = 0;
  n->m_label = "";

  ClearComputedValues();
  return n;
}

GameAction GameTreeRep::InsertAction(GameInfoset s)
{
  if (!s)  throw gbtEfgException();

  GameActionRep *action = s->InsertAction(s->NumActions() + 1);
  for (int i = 1; i <= s->m_members.Length(); i++) {
    s->m_members[i]->children.Append(new GameNodeRep(this, s->m_members[i]));
  }
  ClearComputedValues();
  return action;
}

GameAction GameTreeRep::InsertAction(GameInfoset s, const GameAction &a)
{
  if (!a || !s)  throw gbtEfgException();

  int where;
  for (where = 1; where <= s->m_actions.Length() && s->m_actions[where] != a;
       where++);
  if (where > s->m_actions.Length())   return 0;
  GameActionRep *action = s->InsertAction(where);
  for (int i = 1; i <= s->m_members.Length(); i++)
    s->m_members[i]->children.Insert(new GameNodeRep(this, s->m_members[i]), where);

  ClearComputedValues();
  return action;
}

GameInfoset GameTreeRep::DeleteAction(GameInfoset s, const GameAction &a)
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
  ClearComputedValues();
  return s;
}

//---------------------------------------------------------------------
//                     Subgame-related functions
//---------------------------------------------------------------------

void GameTreeRep::MarkTree(GameNodeRep *n, GameNodeRep *base)
{
  n->ptr = base;
  for (int i = 1; i <= n->NumChildren(); i++)
    MarkTree(n->GetChild(i), base);
}

bool GameTreeRep::CheckTree(GameNodeRep *n, GameNodeRep *base)
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

int GameTreeRep::ProfileLength(void) const
{
  int sum = 0;

  for (int i = 1; i <= players.Length(); i++)
    for (int j = 1; j <= players[i]->m_infosets.Length(); j++)
      sum += players[i]->m_infosets[j]->m_actions.Length();

  return sum;
}

gbtArray<int> GameTreeRep::NumInfosets(void) const
{
  gbtArray<int> foo(players.Length());
  
  for (int i = 1; i <= foo.Length(); i++) {
    foo[i] = players[i]->NumInfosets();
  }

  return foo;
}

gbtPVector<int> GameTreeRep::NumActions(void) const
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

static int CountNodes(GameNodeRep *p_node)
{
  int num = 1;
  for (int i = 1; i <= p_node->NumChildren(); 
       num += CountNodes(p_node->GetChild(i++)));
  return num;
}

int GameTreeRep::NumNodes(void) const
{
  return CountNodes(m_root);
}


gbtPVector<int> GameTreeRep::NumMembers(void) const
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

void GameTreeRep::Payoff(GameNodeRep *n, gbtRational prob, 
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

void GameTreeRep::InfosetProbs(GameNodeRep *n, gbtRational prob, 
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

void GameTreeRep::Payoff(const gbtPVector<int> &profile, 
			gbtVector<gbtRational> &payoff) const
{
  ((gbtVector<gbtRational> &) payoff).operator=(gbtRational(0));
  Payoff(m_root, 1, profile, payoff);
}

void GameTreeRep::InfosetProbs(const gbtPVector<int> &profile,
			      gbtPVector<gbtRational> &probs) const
{
  ((gbtVector<gbtRational> &) probs).operator=(gbtRational(0));
  InfosetProbs(m_root, 1, profile, probs);
}

void GameTreeRep::Payoff(GameNodeRep *n, gbtRational prob, 
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

void GameTreeRep::Payoff(const gbtArray<gbtArray<int> > &profile,
			gbtArray<gbtRational> &payoff) const
{
  for (int i = 1; i <= payoff.Length(); i++)
    payoff[i] = 0;
  Payoff(m_root, 1, profile, payoff);
}

void MakeStrategy(GamePlayerRep *p)
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

void MakeReducedStrats(GamePlayerRep *p, GameNodeRep *n, GameNodeRep *nn)
{
  int i;
  GameNodeRep *m, *mm;

  if (!n->GetParent())  n->ptr = 0;

  if (n->NumChildren() > 0)  {
    if (n->infoset->m_player == p)  {
      if (n->infoset->flag == 0)  {
	// we haven't visited this infoset before
	n->infoset->flag = 1;
	for (i = 1; i <= n->NumChildren(); i++)   {
	  GameNodeRep *m = n->GetChild(i);
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

Gambit::GameTable GameTreeRep::MakeReducedNfg(void)
{
  for (int i = 1; i <= players.Length(); i++) {
    MakeReducedStrats(players[i], m_root, NULL);
  }

  gbtArray<int> dim(players.Length());
  for (int i = 1; i <= players.Length(); i++)
    dim[i] = (players[i]->m_strategies.Length()) ? players[i]->m_strategies.Length() : 1;

  m_reducedNfg = new Gambit::GameTableRep(dim);
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

}  // end namespace Gambit
