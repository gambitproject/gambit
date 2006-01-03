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

//========================================================================
//                       class GameActionRep
//========================================================================

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

//========================================================================
//                       class GameInfosetRep
//========================================================================

GameInfosetRep::GameInfosetRep(GameRep *p_efg, int p_number,
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

//========================================================================
//                      class GameStrategyRep
//========================================================================

void GameStrategyRep::DeleteStrategy(void)
{
  if (m_player->NumStrategies() == 1)  return;

  m_player->m_strategies.Remove(m_player->m_strategies.Find(this));
  for (int st = 1; st <= m_player->m_strategies.Length(); st++) {
    m_player->m_strategies[st]->m_number = st;
  }
  //m_player->m_game->RebuildTable();
  this->Invalidate();
}

GamePlayer GameStrategyRep::GetPlayer(void) const
{ return m_player; }


//========================================================================
//                       class GamePlayerRep
//========================================================================

GamePlayerRep::GamePlayerRep(GameRep *p_game, int p_id, int p_strats)
  : m_game(p_game), m_number(p_id), m_strategies(p_strats)
{ 
  for (int j = 1; j <= p_strats; j++) {
    m_strategies[j] = new GameStrategyRep(this);
    m_strategies[j]->m_number = j;
  }
}

GamePlayerRep::~GamePlayerRep()
{ 
  for (int j = 1; j <= m_infosets.Length(); m_infosets[j++]->Invalidate());
  for (int j = 1; j <= m_strategies.Length(); m_strategies[j++]->Invalidate());
}


GameStrategy GamePlayerRep::NewStrategy(void)
{
  GameStrategyRep *strategy = new GameStrategyRep(this);
  m_strategies.Append(strategy);
  strategy->m_number = m_strategies.Length();
  strategy->m_index = -1;   // this flags this action as new
  //m_nfg->RebuildTable();
  return strategy;
}

//========================================================================
//                         class GameNodeRep
//========================================================================

GameNodeRep::GameNodeRep(GameRep *e, GameNodeRep *p)
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

//========================================================================
//                           class GameRep
//========================================================================

//------------------------------------------------------------------------
//                         GameRep: Lifecycle
//------------------------------------------------------------------------

GameRep::GameRep(void)
{
  m_root = new GameNodeRep(this, 0);
  m_chance = new GamePlayerRep(this, 0);
}

/// This convenience function computes the Cartesian product of the
/// elements in dim.
static int Product(const gbtArray<int> &dim)
{
  int accum = 1;
  for (int i = 1; i <= dim.Length(); accum *= dim[i++]);
  return accum;
}
  
GameRep::GameRep(const gbtArray<int> &dim)
  : m_root(0), m_chance(0)
{
  m_results = gbtArray<GameOutcomeRep *>(Product(dim));
  for (int pl = 1; pl <= dim.Length(); pl++)  {
    m_players.Append(new GamePlayerRep(this, pl, dim[pl]));
    m_players[pl]->m_label = ToText(pl);
    for (int st = 1; st <= m_players[pl]->NumStrategies(); st++) {
      m_players[pl]->m_strategies[st]->m_name = ToText(st);
    }
  }
  IndexStrategies();

  for (int cont = 1; cont <= m_results.Length();
       m_results[cont++] = 0);
}


GameRep::~GameRep()
{
  if (m_root) m_root->Invalidate();
  if (m_chance) m_chance->Invalidate();

  for (int pl = 1; pl <= m_players.Length(); m_players[pl++]->Invalidate());
  for (int outc = 1; outc <= m_outcomes.Length(); 
       m_outcomes[outc++]->Invalidate());
}


//------------------------------------------------------------------------
//                     GameRep: General data access
//------------------------------------------------------------------------

bool GameRep::IsConstSum(void) const
{
  if (m_root) {
    EfgContIter iter(gbtEfgSupport(const_cast<GameRep *>(this)));

    gbtRational sum(0);
    for (int pl = 1; pl <= m_players.Length(); pl++) {
      sum += iter.Payoff(pl);
    }

    while (iter.NextContingency()) {
      gbtRational newsum(0);
      for (int pl = 1; pl <= m_players.Length(); pl++) {
	newsum += iter.Payoff(pl);
      }
      
      if (newsum != sum) {
	return false;
      }
    }

    return true;
  }
  else {
    gbtNfgContingencyIterator iter(gbtNfgSupport(const_cast<GameRep *>(this)));

    gbtRational sum(0);
    for (int pl = 1; pl <= m_players.Length(); pl++) {
      sum += iter.GetPayoff(pl);
    }

    while (iter.NextContingency()) {
      gbtRational newsum(0);
      for (int pl = 1; pl <= m_players.Length(); pl++) {
	newsum += iter.GetPayoff(pl);
      }

      if (newsum != sum) {
	return false;
      }
    }

    return true;
  }
}

gbtRational GameRep::GetMinPayoff(int player) const
{
  int index, p, p1, p2;
  
  if (m_outcomes.Length() == 0)  return gbtRational(0);

  if (player) {
    p1 = p2 = player;
  }
  else {
    p1 = 1;
    p2 = NumPlayers();
  }
  
  gbtRational minpay = m_outcomes[1]->GetPayoff(p1);
  for (index = 1; index <= m_outcomes.Length(); index++)  {
    for (p = p1; p <= p2; p++) {
      if (m_outcomes[index]->GetPayoff(p) < minpay) {
	minpay = m_outcomes[index]->GetPayoff(p);
      }
    }
  }
  return minpay;
}

gbtRational GameRep::GetMaxPayoff(int player) const
{
  int index, p, p1, p2;

  if (m_outcomes.Length() == 0)  return gbtRational(0);

  if (player) {
    p1 = p2 = player;
  }
  else {
    p1 = 1;
    p2 = NumPlayers();
  }

  gbtRational maxpay = m_outcomes[1]->GetPayoff(p1);
  for (index = 1; index <= m_outcomes.Length(); index++)  {
    for (p = p1; p <= p2; p++)
      if (m_outcomes[index]->GetPayoff(p) > maxpay)
	maxpay = m_outcomes[index]->GetPayoff(p);
  }
  return maxpay;
}

bool GameRep::IsPerfectRecall(GameInfoset &s1, GameInfoset &s2) const
{
  for (int pl = 1; pl <= m_players.Length(); pl++)   {
    GamePlayerRep *player = m_players[pl];
    
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
//                  GameRep: Managing the representation
//------------------------------------------------------------------------

void GameRep::NumberNodes(GameNodeRep *n, int &index)
{
  n->number = index++;
  for (int child = 1; child <= n->children.Length();
       NumberNodes(n->children[child++], index));
} 

void GameRep::Canonicalize(void)
{
  if (!m_root)  return;

  int nodeindex = 1;
  NumberNodes(m_root, nodeindex);

  for (int pl = 0; pl <= m_players.Length(); pl++) {
    GamePlayerRep *player = (pl) ? m_players[pl] : m_chance;
    
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

void GameRep::ClearComputedValues(void) const
{
  if (!m_root)  return;

  for (int pl = 1; pl <= m_players.Length(); pl++) {
    while (m_players[pl]->m_strategies.Length() > 0) {
      m_players[pl]->m_strategies.Remove(1)->Invalidate();
    }
  }
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
  
  GameStrategyRep *strategy = p->NewStrategy();
  strategy->m_behav = c;
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

void GameRep::BuildComputedValues(void)
{
  for (int i = 1; i <= m_players.Length(); i++) {
    MakeReducedStrats(m_players[i], m_root, NULL);
  }

  gbtArray<int> dim(m_players.Length());
  for (int i = 1; i <= m_players.Length(); i++)
    dim[i] = (m_players[i]->m_strategies.Length()) ? m_players[i]->m_strategies.Length() : 1;

  for (int i = 1; i <= NumPlayers(); i++)   {
    for (int j = 1; j <= m_players[i]->m_strategies.Length(); j++)   {
      std::string name;
      if (m_players[i]->m_strategies[j]->m_behav.Length() > 0) {
	for (int k = 1; k <= m_players[i]->m_strategies[j]->m_behav.Length(); k++)
	  if (m_players[i]->m_strategies[j]->m_behav[k] > 0)
	    name += ToText(m_players[i]->m_strategies[j]->m_behav[k]);
	  else
	    name += "*";
      }
      else {
	name = "*";
      }
      GetPlayer(i)->GetStrategy(j)->SetName(name);
    }
  }

  gbtNfgSupport S(Game(const_cast<GameRep *>(this)));
  gbtNfgContingencyIterator iter(S);
  gbtArray<gbtArray<int> > corr(NumPlayers());
  gbtArray<int> corrs(NumPlayers());
  for (int i = 1; i <= NumPlayers(); i++)  {
    corrs[i] = 1;
    corr[i] = m_players[i]->m_strategies[1]->m_behav;
  }

  gbtArray<gbtRational> value(NumPlayers());

  int pl = NumPlayers();
  while (1)  {
    Payoff(corr, value);

    iter.SetOutcome(NewOutcome());
    for (int j = 1; j <= NumPlayers(); j++) {
      iter.GetOutcome()->SetPayoff(j, ToText(value[j]));
    }

    iter.NextContingency();
    while (pl > 0)   {
      corrs[pl]++;
      if (corrs[pl] <= m_players[pl]->m_strategies.Length())  {
	corr[pl] = m_players[pl]->m_strategies[corrs[pl]]->m_behav;
	break;
      }
      corrs[pl] = 1;
      corr[pl] = m_players[pl]->m_strategies[1]->m_behav;
      pl--;
    }

    if (pl == 0)  break;
    pl = NumPlayers();
  }
}

bool GameRep::HasComputedValues(void) const
{
  return (!m_root || m_players[1]->m_strategies.Length() > 0);
}


//------------------------------------------------------------------------
//                     GameRep: Writing data files
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

static void WriteEfgFile(std::ostream &f, GameNodeRep *n)
{
  if (n->NumChildren() == 0)   {
    f << "t \"" << EscapeQuotes(n->GetLabel()) << "\" ";
    if (n->GetOutcome())  {
      f << n->GetOutcome()->GetNumber() << " \"" <<
	EscapeQuotes(n->GetOutcome()->GetLabel()) << "\" ";
      f << "{ ";
      for (int pl = 1; pl <= n->GetGame()->NumPlayers(); pl++)  {
	f << n->GetOutcome()->GetPayoffText(pl);

	if (pl < n->GetGame()->NumPlayers())
	  f << ", ";
	else
	  f << " }\n";
      }
    }
    else
      f << "0\n";
  }

  if (n->GetInfoset()->IsChanceInfoset()) {
    f << "c \"";
  }
  else {
    f << "p \"";
  }

  f << EscapeQuotes(n->GetLabel()) << "\" " <<
    n->GetInfoset()->GetPlayer()->GetNumber() << ' ';
  f << n->GetInfoset()->GetNumber() << " \"" <<
    EscapeQuotes(n->GetInfoset()->GetLabel()) << "\" ";
  PrintActions(f, n->GetInfoset());
  f << " ";
  if (n->GetOutcome())  {
    f << n->GetOutcome()->GetNumber() << " \"" <<
      EscapeQuotes(n->GetOutcome()->GetLabel()) << "\" ";
    f << "{ ";
    for (int pl = 1; pl <= n->GetGame()->NumPlayers(); pl++)  {
      f << n->GetOutcome()->GetPayoffText(pl);
      
      if (pl < n->GetGame()->NumPlayers())
	f << ", ";
      else
	f << " }\n";
    }
  }
  else
    f << "0\n";

  for (int i = 1; i <= n->NumChildren(); WriteEfgFile(f, n->GetChild(i++)));
}

void GameRep::WriteEfgFile(std::ostream &p_file) const
{
  p_file << "EFG 2 R";
  p_file << " \"" << EscapeQuotes(GetTitle()) << "\" { ";
  for (int i = 1; i <= m_players.Length(); i++)
    p_file << '"' << EscapeQuotes(m_players[i]->m_label) << "\" ";
  p_file << "}\n";
  p_file << "\"" << EscapeQuotes(GetComment()) << "\"\n\n";

  Gambit::WriteEfgFile(p_file, m_root);
}

void GameRep::WriteNfgFile(std::ostream &p_file) const
{ 
  p_file << "NFG 1 R";
  p_file << " \"" << EscapeQuotes(GetTitle()) << "\" { ";

  for (int i = 1; i <= NumPlayers(); i++)
    p_file << '"' << EscapeQuotes(GetPlayer(i)->GetLabel()) << "\" ";

  p_file << "}\n\n{ ";
  
  for (int i = 1; i <= NumPlayers(); i++)   {
    GamePlayerRep *player = GetPlayer(i);
    p_file << "{ ";
    for (int j = 1; j <= player->NumStrategies(); j++)
      p_file << '"' << EscapeQuotes(player->GetStrategy(j)->GetName()) << "\" ";
    p_file << "}\n";
  }
  
  p_file << "}\n";

  p_file << "\"" << EscapeQuotes(m_comment) << "\"\n\n";

  int ncont = 1;
  for (int i = 1; i <= NumPlayers(); i++)
    ncont *= m_players[i]->m_strategies.Length();

  p_file << "{\n";
  for (int outc = 1; outc <= m_outcomes.Length(); outc++)   {
    p_file << "{ \"" << EscapeQuotes(m_outcomes[outc]->m_label) << "\" ";
    for (int pl = 1; pl <= m_players.Length(); pl++)  {
      p_file << m_outcomes[outc]->m_textPayoffs[pl];

      if (pl < m_players.Length())
	p_file << ", ";
      else
	p_file << " }\n";
    }
  }
  p_file << "}\n";
  
  for (int cont = 1; cont <= ncont; cont++)  {
    if (m_results[cont] != 0)
      p_file << m_results[cont]->m_number << ' ';
    else
      p_file << "0 ";
  }

  p_file << '\n';
}

//------------------------------------------------------------------------
//                   GameRep: Dimensions of the game
//------------------------------------------------------------------------

gbtPVector<int> GameRep::NumActions(void) const
{
  gbtArray<int> foo(m_players.Length());
  int i;
  for (i = 1; i <= m_players.Length(); i++)
    foo[i] = m_players[i]->m_infosets.Length();

  gbtPVector<int> bar(foo);
  for (i = 1; i <= m_players.Length(); i++) {
    for (int j = 1; j <= m_players[i]->m_infosets.Length(); j++) {
      bar(i, j) = m_players[i]->m_infosets[j]->NumActions();
    }
  }

  return bar;
}  

gbtPVector<int> GameRep::NumMembers(void) const
{
  gbtArray<int> foo(m_players.Length());

  for (int i = 1; i <= m_players.Length(); i++) {
    foo[i] = m_players[i]->NumInfosets();
  }

  gbtPVector<int> bar(foo);
  for (int i = 1; i <= m_players.Length(); i++) {
    for (int j = 1; j <= m_players[i]->NumInfosets(); j++) {
      bar(i, j) = m_players[i]->m_infosets[j]->NumMembers();
    }
  }

  return bar;
}

gbtArray<int> GameRep::NumStrategies(void) const
{
  gbtArray<int> dim(m_players.Length());
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    dim[pl] = m_players[pl]->m_strategies.Length();
  }
  return dim;
}

int GameRep::BehavProfileLength(void) const
{
  int sum = 0;

  for (int i = 1; i <= m_players.Length(); i++)
    for (int j = 1; j <= m_players[i]->m_infosets.Length(); j++)
      sum += m_players[i]->m_infosets[j]->m_actions.Length();

  return sum;
}

int GameRep::MixedProfileLength(void) const
{
  int strats = 0;
  for (int i = 1; i <= m_players.Length();
       strats += m_players[i++]->m_strategies.Length());
  return strats;
}


//------------------------------------------------------------------------
//                         GameRep: Players
//------------------------------------------------------------------------

GamePlayer GameRep::NewPlayer(void)
{
  if (m_root) {
    GamePlayerRep *player = new GamePlayerRep(this, m_players.Length() + 1);
    m_players.Append(player);
    
    for (int outc = 1; outc <= m_outcomes.Last(); outc++) {
      m_outcomes[outc]->m_textPayoffs.Append("0");
      m_outcomes[outc]->m_ratPayoffs.Append(0);
    }

    ClearComputedValues();
    return player;
  }
  else {
    GamePlayerRep *player = new GamePlayerRep(this, m_players.Length() + 1, 1);
    m_players.Append(player);

    for (int outc = 1; outc <= m_outcomes.Length(); outc++) {
      m_outcomes[outc]->m_textPayoffs.Append("0");
      m_outcomes[outc]->m_ratPayoffs.Append(0);
    }

    return player;
  }
}

//------------------------------------------------------------------------
//                     GameRep: Information sets
//------------------------------------------------------------------------

GameInfoset GameRep::GetInfoset(int p_index) const
{
  int index = 1;
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    GamePlayerRep *player = m_players[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      if (index++ == p_index) {
	return player->GetInfoset(iset);
      }
    }
  }
  throw gbtIndexException();
}

gbtArray<int> GameRep::NumInfosets(void) const
{
  gbtArray<int> foo(m_players.Length());
  
  for (int i = 1; i <= foo.Length(); i++) {
    foo[i] = m_players[i]->NumInfosets();
  }

  return foo;
}

GameAction GameRep::GetAction(int p_index) const
{
  int index = 1;
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    GamePlayerRep *player = m_players[pl];
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

//------------------------------------------------------------------------
//                        GameRep: Outcomes
//------------------------------------------------------------------------

GameOutcome GameRep::NewOutcome(void)
{
  m_outcomes.Append(new GameOutcomeRep(this, m_outcomes.Length() + 1));
  return m_outcomes[m_outcomes.Last()];
}

void GameRep::DeleteOutcome(const GameOutcome &p_outcome)
{
  if (m_root) {
    m_root->DeleteOutcome(p_outcome);
  }
  else {
    for (int i = 1; i <= m_results.Length(); i++) {
      if (m_results[i] == p_outcome) {
	m_results[i] = 0;
      }
    }
  }

  m_outcomes.Remove(m_outcomes.Find(p_outcome))->Invalidate();

  for (int outc = 1; outc <= m_outcomes.Length(); outc++) {
    m_outcomes[outc]->m_number = outc;
  }
  ClearComputedValues();
}


//------------------------------------------------------------------------
//                          GameRep: Nodes
//------------------------------------------------------------------------

static int CountNodes(GameNodeRep *p_node)
{
  int num = 1;
  for (int i = 1; i <= p_node->NumChildren(); 
       num += CountNodes(p_node->GetChild(i++)));
  return num;
}

int GameRep::NumNodes(void) const
{
  return CountNodes(m_root);
}

//------------------------------------------------------------------------
//                    GameRep: Editing game trees
//------------------------------------------------------------------------

GameInfoset GameRep::AppendNode(GameNode n, GamePlayer p, int count)
{
  if (!n || !p || count == 0)
    throw gbtEfgException();

  if (n->children.Length() == 0)   {
    n->infoset = new GameInfosetRep(this, p->m_infosets.Length() + 1, p, count);
    p->m_infosets.Append(n->infoset);
    n->infoset->m_members.Append(n);
    while (count--)
      n->children.Append(new GameNodeRep(this, n));
  }

  ClearComputedValues();
  return n->infoset;
}  

GameInfoset GameRep::AppendNode(GameNode n, GameInfoset s)
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
  
GameNode GameRep::DeleteNode(GameNode n, GameNode keep)
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

GameInfoset GameRep::InsertNode(GameNode n, GamePlayer p, int count)
{
  if (!n || !p || count <= 0)  throw gbtEfgException();

  GameNodeRep *m = new GameNodeRep(this, n->parent);
  m->infoset = new GameInfosetRep(this, p->m_infosets.Length() + 1, p, count);
  p->m_infosets.Append(m->infoset);
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

GameInfoset GameRep::InsertNode(GameNode n, GameInfoset s)
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

GameInfoset GameRep::JoinInfoset(GameInfoset s, GameNode n)
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

GameInfoset GameRep::LeaveInfoset(GameNode n)
{
  if (!n)  throw gbtEfgException();

  if (!n->infoset)   return 0;

  GameInfosetRep *s = n->infoset;
  if (s->m_members.Length() == 1)   return s;

  GamePlayerRep *p = s->m_player;
  s->m_members.Remove(s->m_members.Find(n));
  n->infoset = new GameInfosetRep(this, p->m_infosets.Length() + 1, p,
				  n->children.Length());
  p->m_infosets.Append(n->infoset);
  n->infoset->m_label = s->m_label;
  n->infoset->m_members.Append(n);
  for (int i = 1; i <= s->m_actions.Length(); i++)
    n->infoset->m_actions[i]->m_label = s->m_actions[i]->m_label;

  ClearComputedValues();
  return n->infoset;
}

GameInfoset GameRep::SplitInfoset(GameNode n)
{
  if (!n)  throw gbtEfgException();

  if (!n->infoset)   return 0;

  GameInfosetRep *s = n->infoset;
  if (s->m_members.Length() == 1)   return s;

  GamePlayerRep *p = s->m_player;
  GameInfosetRep *ns = new GameInfosetRep(this, p->m_infosets.Length() + 1, p,
					  n->children.Length());
  p->m_infosets.Append(ns);
  ns->m_label = s->m_label;
  int i;
  for (i = s->m_members.Length(); i > s->m_members.Find(n); i--)   {
    GameNodeRep *nn = s->m_members.Remove(i);
    ns->m_members.Append(nn);
    nn->infoset = ns;
  }
  for (i = 1; i <= s->m_actions.Length(); i++) {
    ns->m_actions[i]->m_label = s->m_actions[i]->m_label;
    if (p == m_chance) {
      ns->SetActionProb(i, s->GetActionProbText(i));
    }
  }
  ClearComputedValues();
  return n->infoset;
}

GameInfoset GameRep::MergeInfoset(GameInfoset to, GameInfoset from)
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

GameInfoset GameRep::SwitchPlayer(GameInfoset s, GamePlayer p)
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

void GameRep::CopySubtree(GameNodeRep *src, GameNodeRep *dest, 
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

void GameRep::Reveal(GameInfoset where, 
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

GameNode GameRep::CopyTree(GameNode src, GameNode dest)
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

GameNode GameRep::MoveTree(GameNode src, GameNode dest)
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

GameNode GameRep::DeleteTree(GameNode n)
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

GameAction GameRep::InsertAction(GameInfoset s)
{
  if (!s)  throw gbtEfgException();

  GameActionRep *action = s->InsertAction(s->NumActions() + 1);
  for (int i = 1; i <= s->m_members.Length(); i++) {
    s->m_members[i]->children.Append(new GameNodeRep(this, s->m_members[i]));
  }
  ClearComputedValues();
  return action;
}

GameAction GameRep::InsertAction(GameInfoset s, const GameAction &a)
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

GameInfoset GameRep::DeleteAction(GameInfoset s, const GameAction &a)
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


//------------------------------------------------------------------------
//           GameRep: Computing payoffs of pure behavior profiles
//------------------------------------------------------------------------

void GameRep::Payoff(GameNodeRep *n, gbtRational prob, 
		     const gbtPVector<int> &profile,
		     gbtVector<gbtRational> &payoff) const
{
  if (n->outcome)  {
    for (int i = 1; i <= m_players.Length(); i++)
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

void GameRep::InfosetProbs(GameNodeRep *n, gbtRational prob, 
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

void GameRep::Payoff(const gbtPVector<int> &profile, 
		     gbtVector<gbtRational> &payoff) const
{
  ((gbtVector<gbtRational> &) payoff).operator=(gbtRational(0));
  Payoff(m_root, 1, profile, payoff);
}

void GameRep::InfosetProbs(const gbtPVector<int> &profile,
			   gbtPVector<gbtRational> &probs) const
{
  ((gbtVector<gbtRational> &) probs).operator=(gbtRational(0));
  InfosetProbs(m_root, 1, profile, probs);
}

void GameRep::Payoff(GameNodeRep *n, gbtRational prob, 
		     const gbtArray<gbtArray<int> > &profile,
		     gbtArray<gbtRational> &payoff) const
{
  if (n->outcome)   {
    for (int i = 1; i <= m_players.Length(); i++)
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

void GameRep::Payoff(const gbtArray<gbtArray<int> > &profile,
		     gbtArray<gbtRational> &payoff) const
{
  for (int i = 1; i <= payoff.Length(); i++)
    payoff[i] = 0;
  Payoff(m_root, 1, profile, payoff);
}


//------------------------------------------------------------------------
//                  GameRep: Private auxiliary functions
//------------------------------------------------------------------------

//
// MarkSubtree: sets the Node::mark flag on all children of p_node
//
void GameRep::MarkSubtree(GameNodeRep *p_node)
{
  p_node->mark = true;
  for (int i = 1; i <= p_node->children.Length(); i++) {
    MarkSubtree(p_node->children[i]);
  }
}

//
// UnmarkSubtree: clears the Node::mark flag on all children of p_node
//
void GameRep::UnmarkSubtree(GameNodeRep *p_node)
{
  p_node->mark = false;
  for (int i = 1; i <= p_node->children.Length(); i++) {
    UnmarkSubtree(p_node->children[i]);
  }
}

void GameRep::MarkTree(GameNodeRep *n, GameNodeRep *base)
{
  n->ptr = base;
  for (int i = 1; i <= n->NumChildren(); i++)
    MarkTree(n->GetChild(i), base);
}

bool GameRep::CheckTree(GameNodeRep *n, GameNodeRep *base)
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

void GameRep::IndexStrategies(void)
{
  long offset = 1L;

  for (int i = 1; i <= NumPlayers(); i++)  {
    int j;
    for (j = 1; j <= m_players[i]->NumStrategies(); j++)  {
      GameStrategyRep *s = m_players[i]->m_strategies[j];
      s->m_number = j;
      s->m_index = (j - 1) * offset;
    }
    offset *= (j - 1);
  }
}

/// This rebuilds a new table of outcomes after the game has been
/// redimensioned (change in the number of strategies).  Strategies
/// numbered -1 are identified as the new strategies.
void GameRep::RebuildTable(void)
{
  long size = 1L;
  gbtArray<long> offsets(m_players.Length());
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    offsets[pl] = size;
    size *= m_players[pl]->NumStrategies();
  }

  gbtArray<Gambit::GameOutcomeRep *> newResults(size);
  for (int i = 1; i <= newResults.Length(); newResults[i++] = 0);

  gbtNfgContingencyIterator iter(gbtNfgSupport(const_cast<GameRep *>(this)));

  do {
    long newindex = 0L;
    for (int pl = 1; pl <= m_players.Length(); pl++) {
      if (iter.profile.GetStrategy(pl)->m_index < 0) {
	// This is a contingency involving a new strategy... skip
	newindex = -1L;
	break;
      }
      else {
	newindex += (iter.profile.GetStrategy(pl)->m_number - 1) * offsets[pl];
      }
    }

    if (newindex >= 0) {
      newResults[newindex+1] = m_results[iter.profile.m_index+1];
    }
  } while (iter.NextContingency());

  m_results = newResults;

  IndexStrategies();
}



}  // end namespace Gambit
