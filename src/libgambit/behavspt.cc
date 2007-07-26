//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of supports for extensive forms
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

#include "libgambit.h"

namespace Gambit {

template <class T> void RemoveRedundancies(List<T> &p_list)
{
  int i = 1; int j = 2;		
  while (i < p_list.Length()) {
    if (p_list[i] == p_list[j])
      p_list.Remove(j);
    else 
      j++;
    if (j > p_list.Length()) { i++; j = i+1; }
  }
}


//========================================================================
//                      BehavSupport: Lifecycle
//========================================================================

BehavSupport::BehavSupport(const Game &p_efg) 
  : m_efg(p_efg),
    m_infosetActive(0, p_efg->NumPlayers()), 
    m_nonterminalActive(0, p_efg->NumPlayers())
{
  for (int pl = 1; pl <= p_efg->NumPlayers(); pl++) {
    m_actions.Append(Array<Array<GameAction> >());
    GamePlayer player = p_efg->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      GameInfoset infoset = player->GetInfoset(iset);
      m_actions[pl].Append(Array<GameAction>());
      for (int act = 1; act <= infoset->NumActions(); act++) {
	m_actions[pl][iset].Append(infoset->GetAction(act));
      }
    }
  }

  // Initialize the list of reachable information sets and nodes
  for (int pl = 0; pl <= GetGame()->NumPlayers(); pl++) {
    GamePlayer player = (pl == 0) ? GetGame()->GetChance() : GetGame()->GetPlayer(pl);
    List<bool> is_players_infoset_active;
    List<List<bool> > is_players_node_active;

    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      is_players_infoset_active.Append(false);

      List<bool> is_infosets_node_active;
      for (int n = 1; n <= player->GetInfoset(iset)->NumMembers()
; n++)
	is_infosets_node_active.Append(false);
      is_players_node_active.Append(is_infosets_node_active);
    }
    m_infosetActive[pl] = is_players_infoset_active;
    m_nonterminalActive[pl] = is_players_node_active;
  }

  ActivateSubtree(GetGame()->GetRoot());
}

//========================================================================
//                 BehavSupport: Operator overloading
//========================================================================

bool BehavSupport::operator==(const BehavSupport &p_support) const
{
  return (m_actions == p_support.m_actions);
}

//========================================================================
//                 BehavSupport: General information
//========================================================================

PVector<int> BehavSupport::NumActions(void) const
{
  PVector<int> answer(m_efg->NumInfosets());
  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++) {
    for (int iset = 1; iset <= m_efg->GetPlayer(pl)->NumInfosets(); iset++) {
      answer(pl, iset) = NumActions(pl, iset);
    }
  }

  return answer;
}  

int BehavSupport::GetIndex(const GameAction &a) const
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

int BehavSupport::NumDegreesOfFreedom(void) const
{
  int answer = 0;

  List<GameInfoset> active_infosets = ReachableInfosets(GetGame()->GetRoot());
  for (int i = 1; i <= active_infosets.Length(); i++)
    answer += NumActions(active_infosets[i]->GetPlayer()->GetNumber(),
			 active_infosets[i]->GetNumber()) - 1;

  return answer;  
}

bool BehavSupport::HasActiveActionAt(const GameInfoset &infoset) const
{
  return (m_actions[infoset->GetPlayer()->GetNumber()][infoset->GetNumber()].Length() > 0);
}

bool BehavSupport::HasActiveActionsAtAllInfosets(void) const
{
  for (int pl = 1; pl <= m_actions.Length(); pl++) {
    for (int iset = 1; iset <= m_actions[pl].Length(); iset++) {
      if (m_actions[pl][iset].Length() == 0) return false;
    }
  }

  return true;
}

bool BehavSupport::RemoveAction(const GameAction &s)
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

bool BehavSupport::RemoveAction(const GameAction &s, List<GameInfoset> &list)
{
  List<GameNode> startlist(ReachableMembers(s->GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++) {
    DeactivateSubtree(startlist[i]->GetChild(s->GetNumber()), list);
  }

  // the following returns false if s was not in the support
  return RemoveAction(s);
}

void BehavSupport::AddAction(const GameAction &s)
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
    actions.Append(s);
  }

  List<GameNode> startlist(ReachableMembers(s->GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++)
    DeactivateSubtree(startlist[i]);
}

int BehavSupport::NumSequences(int j) const
{
  if (j < 1 || j > m_efg->NumPlayers()) return 1;
  List<GameInfoset> isets = ReachableInfosets(m_efg->GetPlayer(j));
  int num = 1;
  for(int i = 1; i <= isets.Length(); i++)
    num+=NumActions(isets[i]->GetPlayer()->GetNumber(),
		    isets[i]->GetNumber());
  return num;
}

int BehavSupport::NumSequences(void) const
{
  int total = 0;
  for (int i = 1 ; i <= m_efg->NumPlayers(); i++)
    total += NumSequences(i);
  return total;
}

List<GameNode> BehavSupport::ReachableNonterminalNodes(const GameNode &n) const
{
  List<GameNode> answer;
  if (!n->IsTerminal()) {
    int pl = n->GetInfoset()->GetPlayer()->GetNumber();
    int iset = n->GetInfoset()->GetNumber();
    for (int i = 1; i <= NumActions(pl, iset); i++) {
      GameNode nn = n->GetChild(GetAction(pl, iset, i)->GetNumber());
      if (!nn->IsTerminal()) {
	answer.Append(nn);
	answer += ReachableNonterminalNodes(nn);
      }
    }
  }
  return answer;
}

List<GameNode> 
BehavSupport::ReachableNonterminalNodes(const GameNode &n,
					 const GameAction &a) const
{
  List<GameNode> answer;
  GameNode nn = n->GetChild(a->GetNumber());
  if (!nn->IsTerminal()) {
    answer.Append(nn);
    answer += ReachableNonterminalNodes(nn);
  }
  return answer;
}

List<GameInfoset> 
BehavSupport::ReachableInfosets(const GamePlayer &p) const
{ 
  Array<GameInfoset> isets;
  for (int iset = 1; iset <= p->NumInfosets(); iset++) {
    isets.Append(p->GetInfoset(iset));
  }
  List<GameInfoset> answer;

  for (int i = isets.First(); i <= isets.Last(); i++)
    if (MayReach(isets[i]))
      answer.Append(isets[i]);
  return answer;
}

List<GameInfoset> BehavSupport::ReachableInfosets(const GameNode &n) const
{
  List<GameInfoset> answer;
  List<GameNode> nodelist = ReachableNonterminalNodes(n);
  for (int i = 1; i <= nodelist.Length(); i++)
    answer.Append(nodelist[i]->GetInfoset());
  RemoveRedundancies(answer);
  return answer;
}

List<GameInfoset> 
BehavSupport::ReachableInfosets(const GameNode &n, 
				 const GameAction &a) const
{
  List<GameInfoset> answer;
  List<GameNode> nodelist = ReachableNonterminalNodes(n,a);
  for (int i = 1; i <= nodelist.Length(); i++)
    answer.Append(nodelist[i]->GetInfoset());
  RemoveRedundancies(answer);
  return answer;
}

bool BehavSupport::MayReach(const GameInfoset &i) const
{
  for (int j = 1; j <= i->NumMembers(); j++)
    if (MayReach(i->GetMember(j)))
      return true;
  return false;
}

bool BehavSupport::MayReach(const GameNode &n) const
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
class BehavConditionalIterator    {
private:
  Game _efg;
  BehavSupport _support;
  PureBehavProfile _profile;
  PVector<int> _current;
  Array<Array<bool> > _is_active;
  Array<int> _num_active_infosets;
  mutable Vector<Rational> _payoff;

public:
  BehavConditionalIterator(const BehavSupport &);
  BehavConditionalIterator(const BehavSupport &, const List<GameInfoset> &);
  ~BehavConditionalIterator();
  
  void First(void); // Sets each infoset's action to the first in the support
  
  void Set(int pl, int iset, int act);
  void Set(const GameAction &a);
  
  const PureBehavProfile &GetProfile(void) const   { return _profile; }

  int NextContingency(void);   // Needs rewriting
  
  Rational GetPayoff(int pl) const;
  Rational GetPayoff(const GameNode &, int pl) const;
};


BehavConditionalIterator::BehavConditionalIterator(const BehavSupport &s)
  : _efg(s.GetGame()), _support(s),
    _profile(s.GetGame()), _current(s.GetGame()->NumInfosets()),
    _is_active(),
    _num_active_infosets(_efg->NumPlayers()),
    _payoff(_efg->NumPlayers())
{
  for (int pl = 1; pl <= _efg->NumPlayers(); pl++) {
    _num_active_infosets[pl] = 0;
    Array<bool> active_for_pl(_efg->GetPlayer(pl)->NumInfosets());
    for (int iset = 1; iset <= _efg->GetPlayer(pl)->NumInfosets(); iset++) {
      active_for_pl[iset] = true;
      _num_active_infosets[pl]++;
    }
    _is_active.Append(active_for_pl);
  }
  First();
}

BehavConditionalIterator::BehavConditionalIterator(const BehavSupport &s, 
					       const List<GameInfoset>& active)
  : _efg(s.GetGame()), _support(s),
    _profile(s.GetGame()), _current(s.GetGame()->NumInfosets()),
    _is_active(),
    _num_active_infosets(_efg->NumPlayers()),
    _payoff(_efg->NumPlayers())
{
  for (int pl = 1; pl <= _efg->NumPlayers(); pl++) {
    _num_active_infosets[pl] = 0;
    Array<bool> active_for_pl(_efg->GetPlayer(pl)->NumInfosets());
    for (int iset = 1; iset <= _efg->GetPlayer(pl)->NumInfosets(); iset++) {
      if ( active.Contains(_efg->GetPlayer(pl)->GetInfoset(iset)) ) {
	active_for_pl[iset] = true;
	_num_active_infosets[pl]++;
      }
      else
	active_for_pl[iset] = false;
    }
    _is_active.Append(active_for_pl);
  }
  First();
}

BehavConditionalIterator::~BehavConditionalIterator()
{ }


void BehavConditionalIterator::First(void)
{
  for (int pl = 1; pl <= _efg->NumPlayers(); pl++)  {
    for (int iset = 1; iset <= _efg->GetPlayer(pl)->NumInfosets(); iset++) {
      _current(pl, iset) = 1;
      if (_is_active[pl][iset])
	_profile.SetAction(_support.GetAction(pl, iset, 1));
    }
  }
}

void BehavConditionalIterator::Set(int pl, int iset, int act)
{
  _current(pl, iset) = act;
  _profile.SetAction(_support.GetAction(pl, iset, act));
}

void BehavConditionalIterator::Set(const GameAction &a) 
{
  _profile.SetAction(a);
}

int BehavConditionalIterator::NextContingency(void)
{
  int pl = _efg->NumPlayers();
  while (pl > 0 && _num_active_infosets[pl] == 0)
    --pl;
  if (pl == 0)   return 0;
  int iset = _efg->GetPlayer(pl)->NumInfosets();
    
  while (true) {

    if (_is_active[pl][iset]) 
      if (_current(pl, iset) < _support.NumActions(pl, iset))  {
	_current(pl, iset) += 1;
	_profile.SetAction(_support.GetAction(pl, iset, _current(pl, iset)));
	return 1;
      }
      else {
	_current(pl, iset) = 1;
	_profile.SetAction(_support.GetAction(pl, iset, 1));
      }
    
    iset--;
    if (iset == 0)  {
      do  {
	--pl;
      }  while (pl > 0 && _num_active_infosets[pl] == 0);
      
      if (pl == 0)   return 0;
      iset = _efg->GetPlayer(pl)->NumInfosets();
    }
  }
}

Rational BehavConditionalIterator::GetPayoff(int pl) const
{
  return _profile.GetPayoff<Rational>(pl);
}

Rational BehavConditionalIterator::GetPayoff(const GameNode &n, int pl) const
{
  return _profile.GetNodeValue<Rational>(n, pl);
}


bool BehavSupport::Dominates(const GameAction &a, const GameAction &b,
			      bool strong, bool conditional) const
{
  GameInfoset infoset = a->GetInfoset();
  if (infoset != b->GetInfoset()) {
    throw UndefinedException();
  }

  const BehavSupport SAct(*this);
  GamePlayer player = infoset->GetPlayer();
  int pl = player->GetNumber();
  bool equal = true;

  if (!conditional) {
    for (BehavIterator iter(*this, a); !iter.AtEnd(); iter++) {
      Rational ap = iter->GetActionValue<Rational>(a);  
      Rational bp = iter->GetActionValue<Rational>(b);

      if (strong)
	{ if (ap <= bp)  return false; }
      else
	if (ap < bp)   return false; 
	else if (ap > bp)  equal = false;
    }
  }

  else {
    List<GameNode> nodelist = SAct.ReachableMembers(infoset);  
    if (nodelist.Length() == 0) {
      // This may not be a good idea; I suggest checking for this
      // prior to entry
      for (int i = 1; i <= infoset->NumMembers(); i++) {
	nodelist.Append(infoset->GetMember(i));
      }
    }
    
    for (int n = 1; n <= nodelist.Length(); n++) {
      
      List<GameInfoset> L;
      L += ReachableInfosets(nodelist[n], a);
      L += ReachableInfosets(nodelist[n], b);
      RemoveRedundancies(L);
      
      BehavConditionalIterator A(*this,L), B(*this,L);
      A.Set(a);
      B.Set(b);
      
      do  {
	Rational ap = A.GetPayoff(nodelist[n],pl);  
	Rational bp = B.GetPayoff(nodelist[n],pl);
	
	if (strong)
	  { if (ap <= bp)  return false; }
	else
	  if (ap < bp)   return false; 
	  else if (ap > bp)  equal = false;
      } while (A.NextContingency() && B.NextContingency());
    }
  }
  
  if (strong) return true;
  else  return (!equal); 
  /*
  return ::Dominates(*this,player->GetNumber(),infoset->GetNumber(),
		   a->GetNumber(),b->GetNumber(),
		   strong, conditional);
  */
}

bool SomeElementDominates(const BehavSupport &S, 
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

bool BehavSupport::IsDominated(const GameAction &a, 
			       bool strong, bool conditional) const
{
  int pl = a->GetInfoset()->GetPlayer()->GetNumber();
  int iset = a->GetInfoset()->GetNumber();
  Array<GameAction> array(m_actions[pl][iset]);
  return SomeElementDominates(*this,array,a,strong,conditional);
}

bool InfosetHasDominatedElement(const BehavSupport &S, 
				const GameInfoset &p_infoset,
				bool strong,
				bool conditional)
{
  int pl = p_infoset->GetPlayer()->GetNumber();
  int iset = p_infoset->GetNumber();
  Array<GameAction> actions;
  for (int act = 1; act <= S.NumActions(pl, iset); act++) {
    actions.Append(S.GetAction(pl, iset, act));
  }
  for (int i = 1; i <= actions.Length(); i++)
    if (SomeElementDominates(S,actions,actions[i],
			     strong,conditional))
      return true;

  return false;
}

bool ElimDominatedInInfoset(const BehavSupport &S, BehavSupport &T,
			    int pl, int iset, 
			    bool strong, bool conditional)
{
  Array<GameAction> actions;
  for (int act = 1; act <= S.NumActions(pl, iset); act++) {
    actions.Append(S.GetAction(pl, iset, act));
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

bool ElimDominatedForPlayer(const BehavSupport &S, BehavSupport &T,
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

BehavSupport BehavSupport::Undominated(bool strong, bool conditional,
				       const Array<int> &players,
				       std::ostream &) const
{
  BehavSupport T(*this);
  int cumiset = 0;

  for (int i = 1; i <= players.Length(); i++)   {
    int pl = players[i];
    ElimDominatedForPlayer(*this, T, pl, cumiset, 
			   strong, conditional); 
  }

  return T;
}


// Utilities 
bool BehavSupport::HasActiveMembers(int pl, int iset) const
{
  for (int i = 1; i <= m_nonterminalActive[pl][iset].Length(); i++) {
    if (m_nonterminalActive[pl][iset][i]) {
      return true;
    }
  }
  return false;
}

void BehavSupport::activate(const GameNode &n)
{
  m_nonterminalActive[n->GetPlayer()->GetNumber()]
                            [n->GetInfoset()->GetNumber()]
                            [n->NumberInInfoset()] = true;
}

void BehavSupport::deactivate(const GameNode &n)
{
  m_nonterminalActive[n->GetPlayer()->GetNumber()]
                            [n->GetInfoset()->GetNumber()]
                            [n->NumberInInfoset()] = false;
}

void BehavSupport::activate(const GameInfoset &i)
{
  m_infosetActive[i->GetPlayer()->GetNumber()][i->GetNumber()] = true;
}

void BehavSupport::deactivate(const GameInfoset &i)
{
  m_infosetActive[i->GetPlayer()->GetNumber()][i->GetNumber()] = false;
}

void BehavSupport::ActivateSubtree(const GameNode &n)
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

void BehavSupport::DeactivateSubtree(const GameNode &n)
{
  if (!n->IsTerminal()) {  // THIS ALL LOOKS FISHY
    deactivate(n); 
    if ( !HasActiveMembers(n->GetInfoset()->GetPlayer()->GetNumber(),
			   n->GetInfoset()->GetNumber())) {
      deactivate(n->GetInfoset());
    }
    Array<GameAction> actions(m_actions[n->GetInfoset()->GetPlayer()->GetNumber()][n->GetInfoset()->GetNumber()]);
    for (int i = 1; i <= actions.Length(); i++) {
      DeactivateSubtree(n->GetChild(actions[i]->GetNumber()));    
    }
  }
}

void 
BehavSupport::DeactivateSubtree(const GameNode &n, List<GameInfoset> &list)
{
  if (!n->IsTerminal()) {
    deactivate(n); 
    if (!HasActiveMembers(n->GetInfoset()->GetPlayer()->GetNumber(),
			  n->GetInfoset()->GetNumber())) {
      list.Append(n->GetInfoset()); 
      deactivate(n->GetInfoset());
    }
    Array<GameAction> actions(m_actions[n->GetInfoset()->GetPlayer()->GetNumber()][n->GetInfoset()->GetNumber()]);
    for (int i = 1; i <= actions.Length(); i++) {
      DeactivateSubtree(n->GetChild(actions[i]->GetNumber()), list);    
    }
  }
}

List<GameNode> 
BehavSupport::ReachableMembers(const GameInfoset &i) const
{
  List<GameNode> answer;
  int pl = i->GetPlayer()->GetNumber();
  int iset = i->GetNumber();
  for (int j = 1; j <= i->NumMembers(); j++)
    if (m_nonterminalActive[pl][iset][j])
      answer.Append(i->GetMember(j));
  return answer;
}

List<GameNode>
BehavSupport::ReachableNonterminalNodes(void) const
{
  List<GameNode> answer;
  for (int pl = 1; pl <= GetGame()->NumPlayers(); pl++) {
    GamePlayer p = GetGame()->GetPlayer(pl);
    for (int iset = 1; iset <= p->NumInfosets(); iset++)
      answer += ReachableMembers(p->GetInfoset(iset));
  }
  return answer;
}

int BehavSupport::NumActiveMembers(const GameInfoset &p_infoset) const
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

bool BehavSupport::IsActive(const GameInfoset &i) const
{
  return m_infosetActive[i->GetPlayer()->GetNumber()][i->GetNumber()];
}


bool BehavSupport::IsActive(const GameNode &n) const
{
  return m_nonterminalActive[n->GetInfoset()->GetPlayer()->GetNumber()]
    [n->GetInfoset()->GetNumber()][n->NumberInInfoset()];
}

bool BehavSupport::HasActiveActionsAtActiveInfosets(void) const
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

bool BehavSupport::HasActiveActionsAtActiveInfosetsAndNoOthers(void) const
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
