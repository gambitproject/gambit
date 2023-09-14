//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/behavspt.cc
// Implementation of supports for extensive forms
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

#include "gambit.h"

namespace Gambit {

//========================================================================
//                      BehaviorSupportProfile: Lifecycle
//========================================================================

BehaviorSupportProfile::BehaviorSupportProfile(const Game &p_efg) 
  : m_efg(p_efg),
    m_infosetActive(0, p_efg->NumPlayers()), 
    m_nonterminalActive(0, p_efg->NumPlayers())
{
  for (int pl = 1; pl <= p_efg->NumPlayers(); pl++) {
    m_actions.push_back(Array<Array<GameAction> >());
    GamePlayer player = p_efg->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      GameInfoset infoset = player->GetInfoset(iset);
      m_actions[pl].push_back(Array<GameAction>());
      for (int act = 1; act <= infoset->NumActions(); act++) {
	m_actions[pl][iset].push_back(infoset->GetAction(act));
      }
    }
  }

  // Initialize the list of reachable information sets and nodes
  for (int pl = 0; pl <= GetGame()->NumPlayers(); pl++) {
    GamePlayer player = (pl == 0) ? GetGame()->GetChance() : GetGame()->GetPlayer(pl);
    List<bool> is_players_infoset_active;
    List<List<bool> > is_players_node_active;

    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      is_players_infoset_active.push_back(false);

      List<bool> is_infosets_node_active;
      for (int n = 1; n <= player->GetInfoset(iset)->NumMembers()
; n++)
	is_infosets_node_active.push_back(false);
      is_players_node_active.push_back(is_infosets_node_active);
    }
    m_infosetActive[pl] = is_players_infoset_active;
    m_nonterminalActive[pl] = is_players_node_active;
  }

  ActivateSubtree(GetGame()->GetRoot());
}

//========================================================================
//                 BehaviorSupportProfile: Operator overloading
//========================================================================

bool BehaviorSupportProfile::operator==(const BehaviorSupportProfile &p_support) const
{
  return (m_actions == p_support.m_actions);
}

//========================================================================
//                 BehaviorSupportProfile: General information
//========================================================================

PVector<int> BehaviorSupportProfile::NumActions() const
{
  PVector<int> answer(m_efg->NumInfosets());
  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++) {
    for (int iset = 1; iset <= m_efg->GetPlayer(pl)->NumInfosets(); iset++) {
      answer(pl, iset) = NumActions(pl, iset);
    }
  }

  return answer;
}  

int BehaviorSupportProfile::GetIndex(const GameAction &a) const
{
  if (a->GetInfoset()->GetGame() != m_efg)  throw MismatchException();

  int pl = a->GetInfoset()->GetPlayer()->GetNumber();
  if (pl == 0) {
    // chance action; all chance actions are always in the support
    return a->GetNumber();
  }
  else {
    return m_actions[pl][a->GetInfoset()->GetNumber()].Find(a);
  }
}

int BehaviorSupportProfile::NumDegreesOfFreedom() const
{
  int answer = 0;
  PVector<int> reachable(GetGame()->NumInfosets());
  reachable = 0;

  ReachableInfosets(GetGame()->GetRoot(), reachable);
  for (int i = 1; i <= reachable.Length(); i++) {
    answer += reachable[i];
  }

  return answer;  
}

bool BehaviorSupportProfile::HasActiveActionAt(const GameInfoset &infoset) const
{
  return (m_actions[infoset->GetPlayer()->GetNumber()][infoset->GetNumber()].Length() > 0);
}

bool BehaviorSupportProfile::HasActiveActionsAtAllInfosets() const
{
  for (int pl = 1; pl <= m_actions.Length(); pl++) {
    for (int iset = 1; iset <= m_actions[pl].Length(); iset++) {
      if (m_actions[pl][iset].Length() == 0) return false;
    }
  }

  return true;
}

bool BehaviorSupportProfile::RemoveAction(const GameAction &s)
{
  List<GameNode> startlist(ReachableMembers(s->GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++)
    DeactivateSubtree(startlist[i]->GetChild(s->GetNumber()));

  GameInfoset infoset = s->GetInfoset();
  GamePlayer player = infoset->GetPlayer();
  Array<GameAction> &actions = m_actions[player->GetNumber()][infoset->GetNumber()];

  if (!actions.Contains(s)) {
    return false;
  }
  else if (actions.Length() == 1) {
    actions.Remove(actions.Find(s));
    return false;
  }
  else {
    actions.Remove(actions.Find(s));
    return true;
  }
}

bool BehaviorSupportProfile::RemoveAction(const GameAction &s, List<GameInfoset> &list)
{
  List<GameNode> startlist(ReachableMembers(s->GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++) {
    DeactivateSubtree(startlist[i]->GetChild(s->GetNumber()), list);
  }

  // the following returns false if s was not in the support
  return RemoveAction(s);
}

void BehaviorSupportProfile::AddAction(const GameAction &s)
{
  GameInfoset infoset = s->GetInfoset();
  GamePlayer player = infoset->GetPlayer();
  Array<GameAction> &actions = m_actions[player->GetNumber()][infoset->GetNumber()];

  int act = 1;
  while (act <= actions.Length()) {
    if (actions[act] == s) {
      break;
    }
    else if (actions[act]->GetNumber() > s->GetNumber()) {
      actions.Insert(s, act);
      break;
    }
    act++;
  }

  if (act > actions.Length()) {
    actions.push_back(s);
  }

  List<GameNode> startlist(ReachableMembers(s->GetInfoset()));
  for (const auto &node : ReachableMembers(s->GetInfoset())) {
    DeactivateSubtree(node);
  }
}

int BehaviorSupportProfile::NumSequences(int j) const
{
  if (j < 1 || j > m_efg->NumPlayers()) return 1;
  List<GameInfoset> isets = ReachableInfosets(m_efg->GetPlayer(j));
  int num = 1;
  for(int i = 1; i <= isets.Length(); i++)
    num+=NumActions(isets[i]->GetPlayer()->GetNumber(),
		    isets[i]->GetNumber());
  return num;
}

int BehaviorSupportProfile::NumSequences() const
{
  int total = 0;
  for (int i = 1 ; i <= m_efg->NumPlayers(); i++)
    total += NumSequences(i);
  return total;
}

List<GameNode> BehaviorSupportProfile::ReachableNonterminalNodes(const GameNode &n) const
{
  List<GameNode> answer;
  if (!n->IsTerminal()) {
    int pl = n->GetInfoset()->GetPlayer()->GetNumber();
    int iset = n->GetInfoset()->GetNumber();
    for (int i = 1; i <= NumActions(pl, iset); i++) {
      GameNode nn = n->GetChild(GetAction(pl, iset, i)->GetNumber());
      if (!nn->IsTerminal()) {
        answer.push_back(nn);
        answer += ReachableNonterminalNodes(nn);
      }
    }
  }
  return answer;
}

List<GameInfoset> 
BehaviorSupportProfile::ReachableInfosets(const GamePlayer &p) const
{ 
  Array<GameInfoset> isets;
  for (int iset = 1; iset <= p->NumInfosets(); iset++) {
    isets.push_back(p->GetInfoset(iset));
  }
  List<GameInfoset> answer;

  for (int i = isets.First(); i <= isets.Last(); i++)
    if (MayReach(isets[i]))
      answer.push_back(isets[i]);
  return answer;
}

void
BehaviorSupportProfile::ReachableInfosets(const GameNode &p_node,
				PVector<int> &p_reached) const
{
  if (p_node->NumChildren() == 0)  return;

  GameInfoset infoset = p_node->GetInfoset();
  if (!infoset->GetPlayer()->IsChance()) {
    p_reached(infoset->GetPlayer()->GetNumber(), infoset->GetNumber()) = 1;

    for (int act = 1; act <= NumActions(infoset); act++) {
      ReachableInfosets(p_node->GetChild(GetAction(infoset, act)->GetNumber()),
			p_reached);
    }
  }
  else {
    for (int act = 1; act <= infoset->NumActions(); act++) {
      ReachableInfosets(p_node->GetChild(act), p_reached);
    }
  }
}

bool BehaviorSupportProfile::MayReach(const GameInfoset &i) const
{
  for (int j = 1; j <= i->NumMembers(); j++)
    if (MayReach(i->GetMember(j)))
      return true;
  return false;
}

bool BehaviorSupportProfile::MayReach(const GameNode &n) const
{
  if (n == m_efg->GetRoot())
    return true;
  else {
    if (!Contains(n->GetPriorAction()))
      return false;
    else 
      return MayReach(n->GetParent());
  }
}

// This class iterates
// over contingencies that are relevant once a particular node 
// has been reached.
class BehavConditionalIterator  {
private:
  bool m_atEnd;
  BehaviorSupportProfile m_support;
  PVector<int> m_currentBehav;
  PureBehaviorProfile m_profile;
  PVector<int> m_isActive;
  Array<int> m_numActiveInfosets;

  /// Reset the iterator to the first contingency (this is called by ctors)
  void First(); 

public:
  /// @name Lifecycle
  //@{
  BehavConditionalIterator(const BehaviorSupportProfile &, const PVector<int> &);
  //@}

  /// @name Iteration and data access
  //@{
  /// Advance to the next contingency (prefix version) 
  void operator++();
  /// Advance to the next contingency (postfix version) 
  void operator++(int) { ++(*this); }
  /// Has iterator gone past the end?
  bool AtEnd() const { return m_atEnd; }
  /// Get the current behavior profile
  const PureBehaviorProfile *operator->() const { return &m_profile; }
  //@}
};


BehavConditionalIterator::BehavConditionalIterator(const BehaviorSupportProfile &p_support, 
						   const PVector<int> &p_active)
  : m_atEnd(false), m_support(p_support),
    m_currentBehav(m_support.GetGame()->NumInfosets()),
    m_profile(m_support.GetGame()), 
    m_isActive(p_active),
    m_numActiveInfosets(m_support.GetGame()->NumPlayers())
{
  for (int pl = 1; pl <= m_support.GetGame()->NumPlayers(); pl++) {
    m_numActiveInfosets[pl] = 0;
    GamePlayer player = m_support.GetGame()->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      if (m_isActive(pl, iset)) {
	m_numActiveInfosets[pl]++;
      }
    }
  }
  First();
}

void BehavConditionalIterator::First()
{
  for (int pl = 1; pl <= m_support.GetGame()->NumPlayers(); pl++)  {
    for (int iset = 1; iset <= m_support.GetGame()->GetPlayer(pl)->NumInfosets(); iset++) {
      m_currentBehav(pl, iset) = 1;
      if (m_isActive(pl, iset)) {
	m_profile.SetAction(m_support.GetAction(pl, iset, 1));
      }
    }
  }
}

void BehavConditionalIterator::operator++()
{
  int pl = m_support.GetGame()->NumPlayers();
  while (pl > 0 && m_numActiveInfosets[pl] == 0)
    --pl;
  if (pl == 0) {
    m_atEnd = true;
    return;
  }

  int iset = m_support.GetGame()->GetPlayer(pl)->NumInfosets();
    
  while (true) {
    if (m_isActive(pl, iset)) {
      if (m_currentBehav(pl, iset) < m_support.NumActions(pl, iset))  {
	m_profile.SetAction(m_support.GetAction(pl, iset, 
						++m_currentBehav(pl, iset)));
	return;
      }
      else {
	m_currentBehav(pl, iset) = 1;
	m_profile.SetAction(m_support.GetAction(pl, iset, 1));
      }
    }
    
    iset--;
    if (iset == 0)  {
      do  {
	--pl;
      }  while (pl > 0 && m_numActiveInfosets[pl] == 0);
      
      if (pl == 0) {
	m_atEnd = true;
	return;
      }
      iset = m_support.GetGame()->GetPlayer(pl)->NumInfosets();
    }
  }
}

bool BehaviorSupportProfile::Dominates(const GameAction &a, const GameAction &b,
			     bool p_strict, bool p_conditional) const
{
  GameInfoset infoset = a->GetInfoset();
  if (infoset != b->GetInfoset()) {
    throw UndefinedException();
  }

  GamePlayer player = infoset->GetPlayer();
  int pl = player->GetNumber();
  bool equal = true;

  if (!p_conditional) {
    for (BehaviorProfileIterator iter(*this, a); !iter.AtEnd(); iter++) {
      Rational ap = (*iter).GetPayoff<Rational>(a);
      Rational bp = (*iter).GetPayoff<Rational>(b);

      if (p_strict) {
	if (ap <= bp) {
	  return false;
	}
      }
      else {
	if (ap < bp) {
	  return false; 
	}
	else if (ap > bp) {
	  equal = false;
	}
      }
    }
  }

  else {
    List<GameNode> nodelist = ReachableMembers(infoset);  
    if (nodelist.empty()) {
      // This may not be a good idea; I suggest checking for this
      // prior to entry
      for (int i = 1; i <= infoset->NumMembers(); i++) {
        nodelist.push_back(infoset->GetMember(i));
      }
    }
    
    for (int n = 1; n <= nodelist.Length(); n++) {
      PVector<int> reachable(GetGame()->NumInfosets());
      reachable = 0;
      ReachableInfosets(nodelist[n]->GetChild(a->GetNumber()), reachable);
      ReachableInfosets(nodelist[n]->GetChild(b->GetNumber()), reachable);
      
      for (BehavConditionalIterator iter(*this, reachable); 
	   !iter.AtEnd(); iter++) {
	Rational ap = iter->GetPayoff<Rational>(nodelist[n]->GetChild(a->GetNumber()), pl);
	Rational bp = iter->GetPayoff<Rational>(nodelist[n]->GetChild(b->GetNumber()), pl);
	
	if (p_strict) {
	  if (ap <= bp) {
	    return false;
	  }
	}
	else {
	  if (ap < bp) { 
	    return false;
	  } 
	  else if (ap > bp) {
	    equal = false;
	  }
	}
      }
    }
  }
  
  if (p_strict) {
    return true;
  }
  else {
    return !equal; 
  }
}

bool SomeElementDominates(const BehaviorSupportProfile &S, 
			  const Array<GameAction> &array,
			  const GameAction &a, 
			  const bool strong,
			  const bool conditional)
{
  for (int i = 1; i <= array.Length(); i++)
    if (array[i] != a)
      if (S.Dominates(array[i],a,strong,conditional)) {
	return true;
      }
  return false;
}

bool BehaviorSupportProfile::IsDominated(const GameAction &a, 
			       bool strong, bool conditional) const
{
  int pl = a->GetInfoset()->GetPlayer()->GetNumber();
  int iset = a->GetInfoset()->GetNumber();
  Array<GameAction> array(m_actions[pl][iset]);
  return SomeElementDominates(*this,array,a,strong,conditional);
}

bool InfosetHasDominatedElement(const BehaviorSupportProfile &S, 
				const GameInfoset &p_infoset,
				bool strong,
				bool conditional)
{
  int pl = p_infoset->GetPlayer()->GetNumber();
  int iset = p_infoset->GetNumber();
  Array<GameAction> actions;
  for (int act = 1; act <= S.NumActions(pl, iset); act++) {
    actions.push_back(S.GetAction(pl, iset, act));
  }
  for (int i = 1; i <= actions.Length(); i++)
    if (SomeElementDominates(S,actions,actions[i],
			     strong,conditional))
      return true;

  return false;
}

bool ElimDominatedInInfoset(const BehaviorSupportProfile &S, BehaviorSupportProfile &T,
			    int pl, int iset, 
			    bool strong, bool conditional)
{
  Array<GameAction> actions;
  for (int act = 1; act <= S.NumActions(pl, iset); act++) {
    actions.push_back(S.GetAction(pl, iset, act));
  }
  Array<bool> is_dominated(actions.Length());
  for (int k = 1; k <= actions.Length(); k++)
    is_dominated[k] = false;

  for (int i = 1; i <= actions.Length(); i++)
    for (int j = 1; j <= actions.Length(); j++)
      if (i != j && !is_dominated[i] && !is_dominated[j]) 
	if (S.Dominates(actions[i], actions[j], strong, conditional)) {
	  is_dominated[j] = true;
	}
      
  bool action_was_eliminated = false;
  int k = 1;
  while (k <= actions.Length() && !action_was_eliminated) {
    if (is_dominated[k]) action_was_eliminated = true;
    else k++;
  }
  while (k <= actions.Length()) {
    if (is_dominated[k]) 
      T.RemoveAction(actions[k]);
    k++;
  }

  return action_was_eliminated;
}

bool ElimDominatedForPlayer(const BehaviorSupportProfile &S, BehaviorSupportProfile &T,
			    const int pl, int &cumiset,
			    const bool strong,
			    const bool conditional)
{
  bool action_was_eliminated = false;

  for (int iset = 1; iset <= S.GetGame()->GetPlayer(pl)->NumInfosets();
       iset++, cumiset++) {
    if (ElimDominatedInInfoset(S, T, pl, iset, strong, conditional)) 
      action_was_eliminated = true;
  }

  return action_was_eliminated;
}

BehaviorSupportProfile BehaviorSupportProfile::Undominated(bool strong, bool conditional,
				       const Array<int> &players,
				       std::ostream &) const
{
  BehaviorSupportProfile T(*this);
  int cumiset = 0;

  for (int i = 1; i <= players.Length(); i++)   {
    int pl = players[i];
    ElimDominatedForPlayer(*this, T, pl, cumiset, 
			   strong, conditional); 
  }

  return T;
}


// Utilities 
bool BehaviorSupportProfile::HasActiveMembers(int pl, int iset) const
{
  for (int i = 1; i <= m_nonterminalActive[pl][iset].Length(); i++) {
    if (m_nonterminalActive[pl][iset][i]) {
      return true;
    }
  }
  return false;
}

void BehaviorSupportProfile::activate(const GameNode &n)
{
  m_nonterminalActive[n->GetPlayer()->GetNumber()]
                            [n->GetInfoset()->GetNumber()]
                            [n->NumberInInfoset()] = true;
}

void BehaviorSupportProfile::deactivate(const GameNode &n)
{
  m_nonterminalActive[n->GetPlayer()->GetNumber()]
                            [n->GetInfoset()->GetNumber()]
                            [n->NumberInInfoset()] = false;
}

void BehaviorSupportProfile::activate(const GameInfoset &i)
{
  m_infosetActive[i->GetPlayer()->GetNumber()][i->GetNumber()] = true;
}

void BehaviorSupportProfile::deactivate(const GameInfoset &i)
{
  m_infosetActive[i->GetPlayer()->GetNumber()][i->GetNumber()] = false;
}

void BehaviorSupportProfile::ActivateSubtree(const GameNode &n)
{
  if (!n->IsTerminal()) {
    activate(n); 
    activate(n->GetInfoset());
    if (n->GetInfoset()->GetPlayer()->IsChance()) {
      for (int i = 1; i <= n->NumChildren(); i++) {
	ActivateSubtree(n->GetChild(i));
      }
    }
    else {
      const Array<GameAction> &actions(m_actions[n->GetInfoset()->GetPlayer()->GetNumber()][n->GetInfoset()->GetNumber()]);
      for (int i = 1; i <= actions.Length(); i++) {
	ActivateSubtree(n->GetChild(actions[i]->GetNumber()));    
      }
    }
  }
}

void BehaviorSupportProfile::DeactivateSubtree(const GameNode &n)
{
  if (!n->IsTerminal()) {  // THIS ALL LOOKS FISHY
    deactivate(n); 
    if ( !HasActiveMembers(n->GetInfoset()->GetPlayer()->GetNumber(),
			   n->GetInfoset()->GetNumber())) {
      deactivate(n->GetInfoset());
    }
    if (!n->GetPlayer()->IsChance()) {
      Array<GameAction> actions(m_actions[n->GetInfoset()->GetPlayer()->GetNumber()][n->GetInfoset()->GetNumber()]);
      for (int i = 1; i <= actions.Length(); i++) {
	DeactivateSubtree(n->GetChild(actions[i]->GetNumber()));    
      }
    }
    else {
      for (int i = 1; i <= n->GetInfoset()->NumActions(); i++) {
	DeactivateSubtree(n->GetChild(i));
      }
    }
  }
}

void 
BehaviorSupportProfile::DeactivateSubtree(const GameNode &n, List<GameInfoset> &list)
{
  if (!n->IsTerminal()) {
    deactivate(n); 
    if (!HasActiveMembers(n->GetInfoset()->GetPlayer()->GetNumber(),
			  n->GetInfoset()->GetNumber())) {
      list.push_back(n->GetInfoset());
      deactivate(n->GetInfoset());
    }
    Array<GameAction> actions(m_actions[n->GetInfoset()->GetPlayer()->GetNumber()][n->GetInfoset()->GetNumber()]);
    for (int i = 1; i <= actions.Length(); i++) {
      DeactivateSubtree(n->GetChild(actions[i]->GetNumber()), list);    
    }
  }
}

List<GameNode> 
BehaviorSupportProfile::ReachableMembers(const GameInfoset &i) const
{
  List<GameNode> answer;
  int pl = i->GetPlayer()->GetNumber();
  int iset = i->GetNumber();
  for (int j = 1; j <= i->NumMembers(); j++)
    if (m_nonterminalActive[pl][iset][j])
      answer.push_back(i->GetMember(j));
  return answer;
}

List<GameNode>
BehaviorSupportProfile::ReachableNonterminalNodes() const
{
  List<GameNode> answer;
  for (int pl = 1; pl <= GetGame()->NumPlayers(); pl++) {
    GamePlayer p = GetGame()->GetPlayer(pl);
    for (int iset = 1; iset <= p->NumInfosets(); iset++)
      answer += ReachableMembers(p->GetInfoset(iset));
  }
  return answer;
}

int BehaviorSupportProfile::NumActiveMembers(const GameInfoset &p_infoset) const
{
  int answer = 0;
  int pl = p_infoset->GetPlayer()->GetNumber();
  int iset = p_infoset->GetNumber();

  for (int i = 1; i <= m_nonterminalActive[pl][iset].Length(); i++) {
    if (m_nonterminalActive[pl][iset][i]) {
      answer++;
    }
  }
  return answer;
}

bool BehaviorSupportProfile::IsActive(const GameInfoset &i) const
{
  return m_infosetActive[i->GetPlayer()->GetNumber()][i->GetNumber()];
}


bool BehaviorSupportProfile::IsActive(const GameNode &n) const
{
  return m_nonterminalActive[n->GetInfoset()->GetPlayer()->GetNumber()]
    [n->GetInfoset()->GetNumber()][n->NumberInInfoset()];
}

bool BehaviorSupportProfile::HasActiveActionsAtActiveInfosets() const
{
  for (int pl = 1; pl <= GetGame()->NumPlayers(); pl++) {
    for (int iset = 1; iset <= GetGame()->GetPlayer(pl)->NumInfosets(); iset++) {
      if (m_infosetActive[pl][iset] && NumActions(pl, iset) == 0) {
	return false;
      }
    }
  }
  return true;
}

bool BehaviorSupportProfile::HasActiveActionsAtActiveInfosetsAndNoOthers() const
{
  for (int pl = 1; pl <= GetGame()->NumPlayers(); pl++) {
    for (int iset = 1; iset <= GetGame()->GetPlayer(pl)->NumInfosets(); iset++) {
      if (m_infosetActive[pl][iset] && NumActions(pl, iset) == 0) {
	return false;
      }

      if (!m_infosetActive[pl][iset] && NumActions(pl, iset) > 0) {
	return false;
      }
    }
  }
  return true;
}

} // end namespace Gambit
