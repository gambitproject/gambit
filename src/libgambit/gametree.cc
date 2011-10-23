//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/gametree.cc
// Implementation of extensive game representation
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
#include <sstream>

#include "libgambit.h"
#include "gametree.h"

namespace Gambit {

//------------------------------------------------------------------------
//                     GameTreeRep: Lifecycle
//------------------------------------------------------------------------

GameTreeRep::GameTreeRep(void)
{
  m_computedValues = false;
  m_chance = new GamePlayerRep(this, 0);
  m_root = new GameTreeNodeRep(this, 0);
}

GameTreeRep::~GameTreeRep()
{
  m_root->Invalidate();
  m_chance->Invalidate();
}

Game GameTreeRep::Copy(void) const
{
  std::ostringstream os;
  WriteEfgFile(os);
  std::istringstream is(os.str());
  return ReadGame(is);
}

Game NewTree(void)  { return new GameTreeRep(); }

//------------------------------------------------------------------------
//                 GameTreeRep: General data access
//------------------------------------------------------------------------

namespace {

class NotZeroSumException : public Exception {
public:
  virtual ~NotZeroSumException() throw() { }
  const char *what(void) const throw() { return "Game is not constant sum"; }
};

Rational SubtreeSum(const GameNode &p_node)
{
  Rational sum(0);

  if (p_node->NumChildren() > 0) {
    sum = SubtreeSum(p_node->GetChild(1));
    for (int i = 2; i <= p_node->NumChildren(); i++) {
      if (SubtreeSum(p_node->GetChild(i)) != sum) {
	throw NotZeroSumException();
      }
    }
  }

  if (p_node->GetOutcome()) {
    for (int pl = 1; pl <= p_node->GetGame()->NumPlayers(); pl++) {
      sum += p_node->GetOutcome()->GetPayoff<Rational>(pl);
    }
  }
  return sum;
}
 
}  // end anonymous namespace


bool GameTreeRep::IsConstSum(void) const
{
  try {
    SubtreeSum(m_root);
    return true;
  }
  catch (NotZeroSumException) {
    return false;
  }
}

bool GameTreeRep::IsPerfectRecall(GameInfoset &s1, GameInfoset &s2) const
{
  for (int pl = 1; pl <= m_players.Length(); pl++)   {
    GamePlayerRep *player = m_players[pl];
    
    for (int i = 1; i <= player->NumInfosets(); i++)  {
      GameTreeInfosetRep *iset1 = player->m_infosets[i];
      for (int j = 1; j <= player->NumInfosets(); j++)   {
	GameTreeInfosetRep *iset2 = player->m_infosets[j];

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
//               GameTreeRep: Managing the representation
//------------------------------------------------------------------------

void GameTreeRep::NumberNodes(GameTreeNodeRep *n, int &index)
{
  n->number = index++;
  for (int child = 1; child <= n->children.Length();
       NumberNodes(n->children[child++], index));
} 

void GameTreeRep::Canonicalize(void)
{
  int nodeindex = 1;
  NumberNodes(m_root, nodeindex);

  for (int pl = 0; pl <= m_players.Length(); pl++) {
    GamePlayerRep *player = (pl) ? m_players[pl] : m_chance;
    
    // Sort nodes within information sets according to ID.
    // Coded using a bubble sort for simplicity; large games might
    // find a quicksort worthwhile.
    for (int iset = 1; iset <= player->m_infosets.Length(); iset++) {
      GameTreeInfosetRep *infoset = player->m_infosets[iset];
      for (int i = 1; i < infoset->m_members.Length(); i++) {
	for (int j = 1; j < infoset->m_members.Length() - i; j++) {
	  if (infoset->m_members[j+1]->number < infoset->m_members[j]->number) {
	    GameTreeNodeRep *tmp = infoset->m_members[j];
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
      for (int j = 1; j < player->m_infosets.Length() - i; j++) {
	int a = ((player->m_infosets[j+1]->m_members.Length()) ?
		 player->m_infosets[j+1]->m_members[1]->number : 0);
	int b = ((player->m_infosets[j]->m_members.Length()) ?
		 player->m_infosets[j]->m_members[1]->number : 0);

	if (a < b || b == 0) {
	  GameTreeInfosetRep *tmp = player->m_infosets[j];
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

void GameTreeRep::ClearComputedValues(void) const
{
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    while (m_players[pl]->m_strategies.Length() > 0) {
      m_players[pl]->m_strategies.Remove(1)->Invalidate();
    }
  }

  m_computedValues = false;
}

void GameTreeRep::BuildComputedValues(void)
{
  if (m_computedValues) return;

  Canonicalize();

  for (int pl = 1; pl <= m_players.Length(); pl++) {
    m_players[pl]->MakeReducedStrats(m_root, 0);
  }

  for (int pl = 1, id = 1; pl <= m_players.Length(); pl++) {
    for (int st = 1; st <= m_players[pl]->m_strategies.Length(); 
	 m_players[pl]->m_strategies[st++]->m_id = id++);
  }

  m_computedValues = true;
}

//------------------------------------------------------------------------
//                  GameTreeRep: Writing data files
//------------------------------------------------------------------------

namespace {

std::string EscapeQuotes(const std::string &s)
{
  std::string ret;
  
  for (unsigned int i = 0; i < s.length(); i++)  {
    if (s[i] == '"')   ret += '\\';
    ret += s[i];
  }

  return ret;
}

void PrintActions(std::ostream &p_stream, GameTreeInfosetRep *p_infoset)
{ 
  p_stream << "{ ";
  for (int act = 1; act <= p_infoset->NumActions(); act++) {
    p_stream << '"' << EscapeQuotes(p_infoset->GetAction(act)->GetLabel()) << "\" ";
    if (p_infoset->IsChanceInfoset()) {
      p_stream << p_infoset->GetActionProb(act, "") << ' ';
    }
  }
  p_stream << "}";
}

void WriteEfgFile(std::ostream &f, GameTreeNodeRep *n)
{
  if (n->NumChildren() == 0)   {
    f << "t \"" << EscapeQuotes(n->GetLabel()) << "\" ";
    if (n->GetOutcome())  {
      f << n->GetOutcome()->GetNumber() << " \"" <<
	EscapeQuotes(n->GetOutcome()->GetLabel()) << "\" ";
      f << "{ ";
      for (int pl = 1; pl <= n->GetGame()->NumPlayers(); pl++)  {
	f << n->GetOutcome()->GetPayoff<std::string>(pl);

	if (pl < n->GetGame()->NumPlayers()) {
	  f << ", ";
	}
	else {
	  f << " }\n";
	}
      }
    }
    else {
      f << "0\n";
    }
    return;
  }

  if (n->GetInfoset()->IsChanceInfoset()) {
    f << "c \"";
  }
  else {
    f << "p \"";
  }

  f << EscapeQuotes(n->GetLabel()) << "\" ";
  if (!n->GetInfoset()->IsChanceInfoset()) {
    f << n->GetInfoset()->GetPlayer()->GetNumber() << ' ';
  }
  f << n->GetInfoset()->GetNumber() << " \"" <<
    EscapeQuotes(n->GetInfoset()->GetLabel()) << "\" ";
  PrintActions(f, dynamic_cast<GameTreeInfosetRep *>(n->GetInfoset().operator->()));
  f << " ";
  if (n->GetOutcome())  {
    f << n->GetOutcome()->GetNumber() << " \"" <<
      EscapeQuotes(n->GetOutcome()->GetLabel()) << "\" ";
    f << "{ ";
    for (int pl = 1; pl <= n->GetGame()->NumPlayers(); pl++)  {
      f << n->GetOutcome()->GetPayoff<std::string>(pl);
      
      if (pl < n->GetGame()->NumPlayers())
	f << ", ";
      else
	f << " }\n";
    }
  }
  else
    f << "0\n";

  for (int i = 1; i <= n->NumChildren(); 
       WriteEfgFile(f, dynamic_cast<GameTreeNodeRep *>(n->GetChild(i++).operator->())));
}

} // end anonymous namespace

void GameTreeRep::WriteEfgFile(std::ostream &p_file) const
{
  p_file << "EFG 2 R";
  p_file << " \"" << EscapeQuotes(GetTitle()) << "\" { ";
  for (int i = 1; i <= m_players.Length(); i++)
    p_file << '"' << EscapeQuotes(m_players[i]->m_label) << "\" ";
  p_file << "}\n";
  p_file << "\"" << EscapeQuotes(GetComment()) << "\"\n\n";

  Gambit::WriteEfgFile(p_file, m_root);
}

void GameTreeRep::WriteEfgFile(std::ostream &p_file, const GameNode &p_root) const
{
  p_file << "EFG 2 R";
  p_file << " \"" << EscapeQuotes(GetTitle()) << "\" { ";
  for (int i = 1; i <= m_players.Length(); i++)
    p_file << '"' << EscapeQuotes(m_players[i]->m_label) << "\" ";
  p_file << "}\n";
  p_file << "\"" << EscapeQuotes(GetComment()) << "\"\n\n";

  Gambit::WriteEfgFile(p_file, 
		       dynamic_cast<GameTreeNodeRep *>(p_root.operator->()));
}

void GameTreeRep::WriteNfgFile(std::ostream &p_file) const
{ 
  // FIXME: Building computed values is logically const.
  const_cast<GameTreeRep *>(this)->BuildComputedValues();

  p_file << "NFG 1 R";
  p_file << " \"" << EscapeQuotes(GetTitle()) << "\" { ";

  for (int i = 1; i <= NumPlayers(); i++)
    p_file << '"' << EscapeQuotes(GetPlayer(i)->GetLabel()) << "\" ";

  p_file << "}\n\n{ ";
  
  for (int i = 1; i <= NumPlayers(); i++)   {
    GamePlayerRep *player = GetPlayer(i);
    p_file << "{ ";
    for (int j = 1; j <= player->NumStrategies(); j++)
      p_file << '"' << EscapeQuotes(player->GetStrategy(j)->GetLabel()) << "\" ";
    p_file << "}\n";
  }
  
  p_file << "}\n";

  p_file << "\"" << EscapeQuotes(m_comment) << "\"\n\n";

  // For trees, we write the payoff version, since there need not be
  // a one-to-one correspondence between outcomes and entries, when there
  // are chance moves.
  StrategyIterator iter(Game(const_cast<GameTreeRep *>(this)));
    
  for (; !iter.AtEnd(); iter++) {
    for (int pl = 1; pl <= NumPlayers(); pl++) {
      p_file << (*iter)->GetPayoff(pl) << " ";
    }
    p_file << "\n";
  }

  p_file << '\n';
}

//------------------------------------------------------------------------
//                 GameTreeRep: Dimensions of the game
//------------------------------------------------------------------------

PVector<int> GameTreeRep::NumActions(void) const
{
  Array<int> foo(m_players.Length());
  int i;
  for (i = 1; i <= m_players.Length(); i++)
    foo[i] = m_players[i]->m_infosets.Length();

  PVector<int> bar(foo);
  for (i = 1; i <= m_players.Length(); i++) {
    for (int j = 1; j <= m_players[i]->m_infosets.Length(); j++) {
      bar(i, j) = m_players[i]->m_infosets[j]->NumActions();
    }
  }

  return bar;
}  

PVector<int> GameTreeRep::NumMembers(void) const
{
  Array<int> foo(m_players.Length());

  for (int i = 1; i <= m_players.Length(); i++) {
    foo[i] = m_players[i]->NumInfosets();
  }

  PVector<int> bar(foo);
  for (int i = 1; i <= m_players.Length(); i++) {
    for (int j = 1; j <= m_players[i]->NumInfosets(); j++) {
      bar(i, j) = m_players[i]->m_infosets[j]->NumMembers();
    }
  }

  return bar;
}

int GameTreeRep::BehavProfileLength(void) const
{
  int sum = 0;
  for (int i = 1; i <= m_players.Length(); i++)
    for (int j = 1; j <= m_players[i]->m_infosets.Length(); j++)
      sum += m_players[i]->m_infosets[j]->m_actions.Length();
  return sum;
}

//------------------------------------------------------------------------
//                        GameTreeRep: Players
//------------------------------------------------------------------------

GamePlayer GameTreeRep::NewPlayer(void)
{
  GamePlayerRep *player = 0;
  player = new GamePlayerRep(this, m_players.Length() + 1);
  m_players.Append(player);
  for (int outc = 1; outc <= m_outcomes.Last(); outc++) {
    m_outcomes[outc]->m_payoffs.Append(Number());
  }
  ClearComputedValues();
  return player;
}

//------------------------------------------------------------------------
//                   GameTreeRep: Information sets
//------------------------------------------------------------------------

GameInfoset GameTreeRep::GetInfoset(int p_index) const
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
  throw IndexException();
}

Array<int> GameTreeRep::NumInfosets(void) const
{
  Array<int> foo(m_players.Length());
  for (int i = 1; i <= foo.Length(); i++) {
    foo[i] = m_players[i]->NumInfosets();
  }
  return foo;
}

GameAction GameTreeRep::GetAction(int p_index) const
{
  int index = 1;
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    GamePlayerRep *player = m_players[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      GameTreeInfosetRep *infoset = player->m_infosets[iset];
      for (int act = 1; act <= infoset->NumActions(); act++) {
	if (index++ == p_index) {
	  return infoset->GetAction(act);
	}
      }
    }
  }
  throw IndexException();
}


//------------------------------------------------------------------------
//                        GameTreeRep: Outcomes
//------------------------------------------------------------------------

void GameTreeRep::DeleteOutcome(const GameOutcome &p_outcome)
{
  m_root->DeleteOutcome(p_outcome);
  m_outcomes.Remove(m_outcomes.Find(p_outcome))->Invalidate();
  for (int outc = 1; outc <= m_outcomes.Length(); outc++) {
    m_outcomes[outc]->m_number = outc;
  }
  ClearComputedValues();
}

//------------------------------------------------------------------------
//                         GameTreeRep: Nodes
//------------------------------------------------------------------------

namespace {
int CountNodes(GameNode p_node)
{
  int num = 1;
  for (int i = 1; i <= p_node->NumChildren(); 
       num += CountNodes(p_node->GetChild(i++)));
  return num;
}

}  // end anonymous namespace

int GameTreeRep::NumNodes(void) const
{
  return CountNodes(m_root);
}

//------------------------------------------------------------------------
//                     GameTreeRep: Factory functions
//------------------------------------------------------------------------

MixedStrategyProfile<double> GameTreeRep::NewMixedStrategyProfile(double) const
{
  return StrategySupport(const_cast<GameTreeRep *>(this)).NewMixedStrategyProfile<double>();
}

MixedStrategyProfile<Rational> GameTreeRep::NewMixedStrategyProfile(const Rational &) const
{
  return StrategySupport(const_cast<GameTreeRep *>(this)).NewMixedStrategyProfile<Rational>();
}


//========================================================================
//                  class TreePureStrategyProfileRep
//========================================================================

class TreePureStrategyProfileRep : public PureStrategyProfileRep {
public:
  TreePureStrategyProfileRep(const Game &p_game);
  virtual PureStrategyProfile Copy(void) const;
  virtual void SetStrategy(const GameStrategy &);
  virtual GameOutcome GetOutcome(void) const
  { throw UndefinedException(); }
  virtual void SetOutcome(GameOutcome p_outcome)
  { throw UndefinedException(); }
  virtual Rational GetPayoff(int pl) const;
  virtual Rational GetStrategyValue(const GameStrategy &) const;
};

//------------------------------------------------------------------------
//              TreePureStrategyProfileRep: Lifecycle
//------------------------------------------------------------------------

TreePureStrategyProfileRep::TreePureStrategyProfileRep(const Game &p_nfg)
{
  m_nfg = p_nfg;
  m_profile = Array<GameStrategy>(m_nfg->NumPlayers());
  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++)   {
    m_profile[pl] = m_nfg->GetPlayer(pl)->GetStrategy(1);
  }
}

PureStrategyProfile TreePureStrategyProfileRep::Copy(void) const
{
  return PureStrategyProfile(new TreePureStrategyProfileRep(*this));
}

PureStrategyProfile GameTreeRep::NewPureStrategyProfile(void) const
{
  return PureStrategyProfile(new TreePureStrategyProfileRep(const_cast<GameTreeRep *>(this)));
}

//------------------------------------------------------------------------
//       TreePureStrategyProfileRep: Data access and manipulation
//------------------------------------------------------------------------

void TreePureStrategyProfileRep::SetStrategy(const GameStrategy &s)
{
  m_profile[s->GetPlayer()->GetNumber()] = s;
}

Rational TreePureStrategyProfileRep::GetPayoff(int pl) const
{
  PureBehavProfile behav(m_nfg);
  for (int i = 1; i <= m_nfg->NumPlayers(); i++) {
    GamePlayer player = m_nfg->GetPlayer(i);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      int act = m_profile[i]->m_behav[iset];
      if (act) {
	behav.SetAction(player->GetInfoset(iset)->GetAction(act));
      }
    }
  }
  return behav.GetPayoff<Rational>(pl);
}

Rational
TreePureStrategyProfileRep::GetStrategyValue(const GameStrategy &p_strategy) const
{
  PureStrategyProfile copy = Copy();
  copy->SetStrategy(p_strategy);
  return copy->GetPayoff(p_strategy->GetPlayer()->GetNumber());
}


}  // end namespace Gambit
