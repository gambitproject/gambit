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

template <class T> void RemoveRedundancies(gbtList<T> &p_list)
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



class BehavSupportInfoset   {
  friend class BehavSupportPlayer;
protected:
  gbtArray<GameAction> acts;

public:
  BehavSupportInfoset(GameInfoset);
  BehavSupportInfoset ( const BehavSupportInfoset &a);
  virtual ~BehavSupportInfoset();
  BehavSupportInfoset &operator=( const BehavSupportInfoset &a);
  bool operator==( const BehavSupportInfoset &a) const;
  inline const GameAction operator[](const int &i) const { return acts[i]; }

  // Information
  inline const int Length() const { return acts.Length(); }
};

//----------------------------------------------------
// BehavSupportInfoset: Constructors, Destructor, operators
// ---------------------------------------------------

BehavSupportInfoset::BehavSupportInfoset(GameInfoset p_infoset)
{
  for (int i = 1; i <= p_infoset->NumActions(); i++) {
    acts.Append(p_infoset->GetAction(i));
  }
 }

BehavSupportInfoset::BehavSupportInfoset(const BehavSupportInfoset &a)
  : acts(a.acts)
{ }

BehavSupportInfoset::~BehavSupportInfoset ()
{ }

BehavSupportInfoset &BehavSupportInfoset::operator=( const BehavSupportInfoset &a)
{
  acts = a.acts; 
  return *this;
}

bool BehavSupportInfoset::operator==(const BehavSupportInfoset &a) const
{
  return (acts == a.acts);
}

class BehavSupportPlayer{

protected:
  GamePlayer efp;
  gbtArray < BehavSupportInfoset *> infosets;
public:
  
  //----------------------------------------
  // Constructors, Destructor, operators
  //----------------------------------------

//  BehavSupportPlayer();
  BehavSupportPlayer(const BehavSupportPlayer &);
  BehavSupportPlayer(GamePlayer);
  virtual ~BehavSupportPlayer();

  BehavSupportPlayer &operator=(const BehavSupportPlayer &);
  bool operator==(const BehavSupportPlayer &s) const;

  //--------------------
  // Member Functions
  //--------------------

  // Append an action to an infoset;
  void AddAction(int iset, const GameAction &);

  // Insert an action in a particular place in an infoset;
  void AddAction(int iset, const GameAction &, int index);


  // Remove an action at int i, returns the removed action pointer
  GameAction RemoveAction(int iset, int i);

  // Remove an action from an infoset . 
  // Returns true if the action was successfully removed, false otherwise.
  bool RemoveAction(int iset, GameAction);

  // Get a garray of the actions in an Infoset
  const gbtArray<GameAction> &ActionList(int iset) const
     { return infosets[iset]->acts; }

  // Get the BehavSupportInfoset of an iset
  const BehavSupportInfoset *ActionArray(int iset) const
     { return infosets[iset]; }

  // Get the BehavSupportInfoset of an Infoset
  const BehavSupportInfoset *ActionArray(const GameInfoset &i) const
     { return infosets[i->GetNumber()]; }
  
  // Get an Action
  GameAction GetAction(int iset, int index);

  // returns the index of the action if it is in the ActionSet
  int Find(const GameAction &) const;
  int Find(int, GameAction) const;

  // Number of Actions in a particular infoset
  int NumActions(int iset) const;

  // return the GamePlayer of the BehavSupportPlayer
  GamePlayer GetPlayer(void) const;

  // checks for a valid BehavSupportPlayer
  bool HasActiveActionsAtAllInfosets(void) const;
  bool HasActiveActionAt(const int &iset) const;

};

//--------------------------------------------------
// BehavSupportPlayer: Constructors, Destructor, operators
//--------------------------------------------------

BehavSupportPlayer::BehavSupportPlayer(GamePlayer p)
  : infosets(p->NumInfosets())
{
  efp = p;
  for (int i = 1; i <= p->NumInfosets(); i++) {
    infosets[i] = new BehavSupportInfoset(p->GetInfoset(i));
  }
}

BehavSupportPlayer::BehavSupportPlayer( const BehavSupportPlayer &s )
: infosets(s.infosets.Length())
{
  efp = s.efp;
  for (int i = 1; i <= s.infosets.Length(); i++){
    infosets[i] = new BehavSupportInfoset(*(s.infosets[i]));
  }
}

BehavSupportPlayer::~BehavSupportPlayer()
{ 
  for (int i = 1; i <= infosets.Length(); i++)
    delete infosets[i];
}

BehavSupportPlayer &BehavSupportPlayer::operator=(const BehavSupportPlayer &s)
{
  if (this != &s && efp == s.efp) {
    for (int i = 1; i<= infosets.Length(); i++)  {
      delete infosets[i];
      infosets[i] = new BehavSupportInfoset(*(s.infosets[i]));
    }
  }    
  return *this;
}

bool BehavSupportPlayer::operator==(const BehavSupportPlayer &s) const
{
  if (infosets.Length() != s.infosets.Length() ||
      efp != s.efp)
    return false;
  
  int i;
  for (i = 1; i <= infosets.Length() && 
       *(infosets[i]) == *(s.infosets[i]);  i++);
  return (i > infosets.Length());
}

//------------------------------------------
// BehavSupportPlayer: Member functions 
//------------------------------------------

// Append an action to a particular infoset;
void BehavSupportPlayer::AddAction(int iset, const GameAction &s)
{ 
  if (infosets[iset]->acts.Find(s))
    return;

  if (infosets[iset]->acts.Length() == 0) {
    infosets[iset]->acts.Append(s); 
  }
  else {
    int index = 1;
    while (index <= infosets[iset]->acts.Length() &&
	   infosets[iset]->acts[index]->GetNumber() < s->GetNumber()) 
      index++;
    infosets[iset]->acts.Insert(s,index);
  }
}

// Insert an action  to a particular infoset at a particular place;
void BehavSupportPlayer::AddAction(int iset, const GameAction &s, int index)
{ 
  if (!infosets[iset]->acts.Find(s))
    infosets[iset]->acts.Insert(s,index); 
}

// Remove an action from infoset iset at int i, 
// returns the removed Infoset pointer
GameAction BehavSupportPlayer::RemoveAction(int iset, int i) 
{ 
  return (infosets[iset]->acts.Remove(i)); 
}

// Removes an action from infoset iset . Returns true if the 
//Action was successfully removed, false otherwise.
bool BehavSupportPlayer::RemoveAction(int  iset, GameAction s )
{ 
  int t = infosets[iset]->acts.Find(s); 
  if (t>0) infosets[iset]->acts.Remove(t); 
  return (t>0); 
} 

// Get an action
GameAction BehavSupportPlayer::GetAction(int iset, int index)
{
  return (infosets[iset]->acts)[index];
}

// Number of Actions in a particular infoset
int BehavSupportPlayer::NumActions(int iset) const
{
  return (infosets[iset]->acts.Length());
}

// Return the GamePlayer of this BehavSupportPlayer
GamePlayer BehavSupportPlayer::GetPlayer(void) const
{
  return efp;
}

int BehavSupportPlayer::Find(const GameAction &a) const
{
  return (infosets[a->GetInfoset()->GetNumber()]->acts.Find(a));
}

int BehavSupportPlayer::Find(int p_infoset, GameAction a) const
{
  return (infosets[p_infoset]->acts.Find(a));
}

// checks for a valid BehavSupportPlayer
bool BehavSupportPlayer::HasActiveActionsAtAllInfosets(void) const
{
  if (infosets.Length() != efp->NumInfosets())   return false;

  for (int i = 1; i <= infosets.Length(); i++)
    if (infosets[i]->acts.Length() == 0)   return false;

  return true;
}

// checks for a valid BehavSupportPlayer
bool BehavSupportPlayer::HasActiveActionAt(const int &iset) const
{
  if (iset > efp->NumInfosets())   return false;

  if (infosets[iset]->acts.Length() == 0)   return false;

  return true;
}


//--------------------------------------------------
// BehavSupport: Constructors, Destructors, Operators
//--------------------------------------------------

BehavSupport::BehavSupport(const Game &p_efg) 
  : m_efg(p_efg), m_players(p_efg->NumPlayers())
{
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    m_players[pl] = new BehavSupportPlayer(p_efg->GetPlayer(pl));
  }
}

BehavSupport::BehavSupport(const BehavSupport &p_support)
  : m_name(p_support.m_name), m_efg(p_support.m_efg),
    m_players(p_support.m_players.Length())
{
  for (int pl = 1; pl <= m_players.Length(); pl++)
    m_players[pl] = new BehavSupportPlayer(*(p_support.m_players[pl]));
}

BehavSupport::~BehavSupport()
{
  for (int pl = 1; pl <= m_players.Length(); pl++)
    delete m_players[pl];
}

BehavSupport &BehavSupport::operator=(const BehavSupport &p_support)
{
  if (this != &p_support && m_efg == p_support.m_efg) {
    m_name = p_support.m_name;
    for (int pl = 1; pl <= m_players.Length(); pl++)  {
      delete m_players[pl];
      m_players[pl] = new BehavSupportPlayer(*(p_support.m_players[pl]));
    }
  }
  return *this;
}

bool BehavSupport::operator==(const BehavSupport &p_support) const
{
  if (m_players.Length() != p_support.m_players.Length())
    return false;

  int pl;
  for (pl = 1; (pl <= m_players.Length() &&
		*(m_players[pl]) == *(p_support.m_players[pl])); pl++);
  return (pl > m_players.Length());
}

bool BehavSupport::operator!=(const BehavSupport &p_support) const
{
  return !(*this == p_support);
}

//-----------------------------
// BehavSupport: Member Functions 
//-----------------------------

int BehavSupport::NumActions(int pl, int iset) const
{
  return m_players[pl]->NumActions(iset);
}

int BehavSupport::NumActions(const GameInfoset &i) const
{
  if (i->GetPlayer()->IsChance())
    return i->NumActions();
  else
    return m_players[i->GetPlayer()->GetNumber()]->NumActions(i->GetNumber());
}

const gbtArray<GameAction> &BehavSupport::Actions(int pl, int iset) const
{
  return m_players[pl]->ActionList(iset);
}

gbtArray<GameAction> BehavSupport::Actions(const GameInfoset &i) const
{
  if (i->GetPlayer()->IsChance()) {
    gbtArray<GameAction> actions;
    for (int act = 1; act <= i->NumActions(); act++) {
      actions.Append(i->GetAction(act));
    }
    return actions;
  }
  else
    return m_players[i->GetPlayer()->GetNumber()]->ActionList(i->GetNumber());
}

gbtList<GameAction> BehavSupport::ListOfActions(const GameInfoset &i) const
{
  gbtArray<GameAction> actions = Actions(i);
  gbtList<GameAction> answer;
  for (int i = 1; i <= actions.Length(); i++)
    answer.Append(actions[i]);
  return answer;
}

int BehavSupport::Find(const GameAction &a) const
{
  if (a->GetInfoset()->GetGame() != m_efg)  assert(0);

  int pl = a->GetInfoset()->GetPlayer()->GetNumber();

  return m_players[pl]->Find(a);
}

int BehavSupport::Find(int p_player, int p_infoset, GameAction p_action) const
{
  return m_players[p_player]->Find(p_infoset, p_action);
}

bool BehavSupport::ActionIsActive(GameAction a) const
{
  //DEBUG
  //  if (a == NULL) { gout << "Action* is null.\n"; exit(0); }

  if (a->GetInfoset()->GetGame() != m_efg)   
    return false;

  int pl = a->GetInfoset()->GetPlayer()->GetNumber();

  if (pl == 0) return true; // Chance

  int act = m_players[pl]->Find(a);
  if (act == 0) 
    return false;
  else
    return true;
}

bool BehavSupport::ActionIsActive(const int pl,
			       const int iset, 
			       const int act) const
{
  return 
    ActionIsActive(GetGame()->GetPlayer(pl)->GetInfoset(iset)->GetAction(act));
}

bool 
BehavSupport::AllActionsInSupportAtInfosetAreActive(const BehavSupport &S,
						 const GameInfoset &infset) const
{
  gbtArray<GameAction> support_actions = S.Actions(infset);
  for (int i = 1; i <= support_actions.Length(); i++) {
    if (!ActionIsActive(support_actions[i]))
      return false;
  }
  return true;
}

bool BehavSupport::HasActiveActionAt(const GameInfoset &infoset) const
{
  if 
    ( !m_players[infoset->GetPlayer()->GetNumber()]->
      HasActiveActionAt(infoset->GetNumber()) )
    return false;

  return true;
}

int BehavSupport::NumDegreesOfFreedom(void) const
{
  int answer(0);

  gbtList<GameInfoset> active_infosets = ReachableInfosets(GetGame()->GetRoot());
  for (int i = 1; i <= active_infosets.Length(); i++)
    answer += NumActions(active_infosets[i]) - 1;

  return answer;  
}

bool BehavSupport::HasActiveActionsAtAllInfosets(void) const
{
  if (m_players.Length() != m_efg->NumPlayers())   return false;
  for (int i = 1; i <= m_players.Length(); i++)
    if (!m_players[i]->HasActiveActionsAtAllInfosets())  return false;

  return true;
}

gbtPVector<int> BehavSupport::NumActions(void) const
{
  gbtArray<int> foo(m_efg->NumPlayers());
  int i;
  for (i = 1; i <= m_efg->NumPlayers(); i++)
    foo[i] = m_players[i]->GetPlayer()->NumInfosets();

  gbtPVector<int> bar(foo);
  for (i = 1; i <= m_efg->NumPlayers(); i++)
    for (int j = 1; j <= m_players[i]->GetPlayer()->NumInfosets(); j++)
      bar(i, j) = NumActions(i,j);

  return bar;
}  

bool BehavSupport::RemoveAction(const GameAction &s)
{
  GameInfoset infoset = s->GetInfoset();
  GamePlayer player = infoset->GetPlayer();
 
  return m_players[player->GetNumber()]->RemoveAction(infoset->GetNumber(), s);
}

void BehavSupport::AddAction(const GameAction &s)
{
  GameInfoset infoset = s->GetInfoset();
  GamePlayer player = infoset->GetPlayer();

  m_players[player->GetNumber()]->AddAction(infoset->GetNumber(), s);
				      
}

int BehavSupport::NumSequences(int j) const
{
  if (j < 1 || j > m_efg->NumPlayers()) return 1;
  gbtList<GameInfoset> isets = ReachableInfosets(m_efg->GetPlayer(j));
  int num = 1;
  for(int i = 1; i <= isets.Length(); i++)
    num+=NumActions(isets[i]);
  return num;
}

int BehavSupport::TotalNumSequences(void) const
{
  int total = 0;
  for (int i = 1 ; i <= m_efg->NumPlayers(); i++)
    total += NumSequences(i);
  return total;
}

gbtList<GameNode> BehavSupport::ReachableNonterminalNodes(const GameNode &n) const
{
  gbtList<GameNode> answer;
  if (!n->IsTerminal()) {
    const gbtArray<GameAction> &actions = Actions(n->GetInfoset());
    for (int i = 1; i <= actions.Length(); i++) {
      GameNode nn = n->GetChild(actions[i]->GetNumber());
      if (!nn->IsTerminal()) {
	answer.Append(nn);
	answer += ReachableNonterminalNodes(nn);
      }
    }
  }
  return answer;
}

gbtList<GameNode> 
BehavSupport::ReachableNonterminalNodes(const GameNode &n,
					 const GameAction &a) const
{
  gbtList<GameNode> answer;
  GameNode nn = n->GetChild(a->GetNumber());
  if (!nn->IsTerminal()) {
    answer.Append(nn);
    answer += ReachableNonterminalNodes(nn);
  }
  return answer;
}

gbtList<GameInfoset> 
BehavSupport::ReachableInfosets(const GamePlayer &p) const
{ 
  gbtArray<GameInfoset> isets;
  for (int iset = 1; iset <= p->NumInfosets(); iset++) {
    isets.Append(p->GetInfoset(iset));
  }
  gbtList<GameInfoset> answer;

  for (int i = isets.First(); i <= isets.Last(); i++)
    if (MayReach(isets[i]))
      answer.Append(isets[i]);
  return answer;
}

gbtList<GameInfoset> BehavSupport::ReachableInfosets(const GameNode &n) const
{
  gbtList<GameInfoset> answer;
  gbtList<GameNode> nodelist = ReachableNonterminalNodes(n);
  for (int i = 1; i <= nodelist.Length(); i++)
    answer.Append(nodelist[i]->GetInfoset());
  RemoveRedundancies(answer);
  return answer;
}

gbtList<GameInfoset> 
BehavSupport::ReachableInfosets(const GameNode &n, 
				 const GameAction &a) const
{
  gbtList<GameInfoset> answer;
  gbtList<GameNode> nodelist = ReachableNonterminalNodes(n,a);
  for (int i = 1; i <= nodelist.Length(); i++)
    answer.Append(nodelist[i]->GetInfoset());
  RemoveRedundancies(answer);
  return answer;
}

bool BehavSupport::AlwaysReaches(const GameInfoset &i) const
{
  return AlwaysReachesFrom(i, m_efg->GetRoot());
}

bool BehavSupport::AlwaysReachesFrom(const GameInfoset &i, const GameNode &n) const
{
  if (n->IsTerminal()) return false;
  else
    if (n->GetInfoset() == i) return true;
    else {
      gbtArray<GameAction> actions = Actions(n->GetInfoset());
      for (int j = 1; j <= actions.Length(); j++)
	if (!AlwaysReachesFrom(i,n->GetChild(actions[j]->GetNumber()))) 
	  return false;
    }
  return true;
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
    if (!ActionIsActive(n->GetPriorAction()))
      return false;
    else 
      return MayReach(n->GetParent());
  }
}

// This class iterates
// over contingencies that are relevant once a particular node 
// has been reached.
class BehavConditionalIterator    {
  friend class EfgIter;
private:
  Game _efg;
  BehavSupport _support;
  PureBehavProfile _profile;
  gbtPVector<int> _current;
  gbtArray<gbtArray<bool> > _is_active;
  gbtArray<int> _num_active_infosets;
  mutable gbtVector<gbtRational> _payoff;

public:
  BehavConditionalIterator(const BehavSupport &);
  BehavConditionalIterator(const BehavSupport &, const gbtList<GameInfoset> &);
  ~BehavConditionalIterator();
  
  void First(void); // Sets each infoset's action to the first in the support
  
  void Set(int pl, int iset, int act);
  void Set(const GameAction &a);
  int Next(int pl, int iset); 
  
  const PureBehavProfile &GetProfile(void) const   { return _profile; }

  int NextContingency(void);   // Needs rewriting
  
  gbtRational GetPayoff(int pl) const;
  gbtRational GetPayoff(const GameNode &, int pl) const;
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
    gbtArray<bool> active_for_pl(_efg->GetPlayer(pl)->NumInfosets());
    for (int iset = 1; iset <= _efg->GetPlayer(pl)->NumInfosets(); iset++) {
      active_for_pl[iset] = true;
      _num_active_infosets[pl]++;
    }
    _is_active.Append(active_for_pl);
  }
  First();
}

BehavConditionalIterator::BehavConditionalIterator(const BehavSupport &s, 
					       const gbtList<GameInfoset>& active)
  : _efg(s.GetGame()), _support(s),
    _profile(s.GetGame()), _current(s.GetGame()->NumInfosets()),
    _is_active(),
    _num_active_infosets(_efg->NumPlayers()),
    _payoff(_efg->NumPlayers())
{
  for (int pl = 1; pl <= _efg->NumPlayers(); pl++) {
    _num_active_infosets[pl] = 0;
    gbtArray<bool> active_for_pl(_efg->GetPlayer(pl)->NumInfosets());
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
	_profile.SetAction(_support.Actions(pl, iset)[1]);
    }
  }
}

void BehavConditionalIterator::Set(int pl, int iset, int act)
{
  _current(pl, iset) = act;
  _profile.SetAction(_support.Actions(pl, iset)[act]);
}

void BehavConditionalIterator::Set(const GameAction &a) 
{
  _profile.SetAction(a);
}

int BehavConditionalIterator::Next(int pl, int iset)
{
  const gbtArray<GameAction> &actions = _support.Actions(pl, iset);
  
  if (_current(pl, iset) == actions.Length())   {
    _current(pl, iset) = 1;
    _profile.SetAction(actions[1]);
    return 0;
  }

  _current(pl, iset)++;
  _profile.SetAction(actions[_current(pl, iset)]);
  return 1;
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
	_profile.SetAction(_support.Actions(pl, iset)[_current(pl, iset)]);
	return 1;
      }
      else {
	_current(pl, iset) = 1;
	_profile.SetAction(_support.Actions(pl, iset)[1]);
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

gbtRational BehavConditionalIterator::GetPayoff(int pl) const
{
  return _profile.GetPayoff(pl);
}

gbtRational BehavConditionalIterator::GetPayoff(const GameNode &n, int pl) const
{
  return _profile.GetNodeValue(n, pl);
}


bool BehavSupport::Dominates(const GameAction &a, const GameAction &b,
			      bool strong, bool conditional) const
{
  GameInfoset infoset = a->GetInfoset();
  if (infoset != b->GetInfoset()) {
    throw UndefinedException();
  }

  const BehavSupportWithActiveInfo SAct(*this);
  GamePlayer player = infoset->GetPlayer();
  int pl = player->GetNumber();
  bool equal = true;

  if (!conditional) {
    BehavIterator A(*this), B(*this);
    A.Freeze(player->GetNumber(), infoset->GetNumber()); 
    B.Freeze(player->GetNumber(), infoset->GetNumber());
    A.Set(a);
    B.Set(b);

    do  {
      gbtRational ap = A.GetPayoff(pl);  
      gbtRational bp = B.GetPayoff(pl);

      if (strong)
	{ if (ap <= bp)  return false; }
      else
	if (ap < bp)   return false; 
	else if (ap > bp)  equal = false;
    } while (A.NextContingency() && B.NextContingency());
  }

  else {
    gbtList<GameNode> nodelist = SAct.ReachableNodesInInfoset(infoset);  
    if (nodelist.Length() == 0) {
      // This may not be a good idea; I suggest checking for this
      // prior to entry
      for (int i = 1; i <= infoset->NumMembers(); i++) {
	nodelist.Append(infoset->GetMember(i));
      }
    }
    
    for (int n = 1; n <= nodelist.Length(); n++) {
      
      gbtList<GameInfoset> L;
      L += ReachableInfosets(nodelist[n], a);
      L += ReachableInfosets(nodelist[n], b);
      RemoveRedundancies(L);
      
      BehavConditionalIterator A(*this,L), B(*this,L);
      A.Set(a);
      B.Set(b);
      
      do  {
	gbtRational ap = A.GetPayoff(nodelist[n],pl);  
	gbtRational bp = B.GetPayoff(nodelist[n],pl);
	
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
			  const gbtArray<GameAction> &array,
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
  gbtArray<GameAction> array(Actions(a->GetInfoset()));
  return SomeElementDominates(*this,array,a,strong,conditional);
}

bool InfosetHasDominatedElement(const BehavSupport &S, 
				const GameInfoset &i,
				bool strong,
				bool conditional)
{
  gbtArray<GameAction> actions = S.Actions(i);
  for (int i = 1; i <= actions.Length(); i++)
    if (SomeElementDominates(S,actions,actions[i],
			     strong,conditional))
      return true;

  return false;
}

bool ElimDominatedInInfoset(const BehavSupport &S, BehavSupport &T,
			    const int pl, 
			    const int iset, 
			    const bool strong,
			    const bool conditional)
{
  const gbtArray<GameAction> &actions = S.Actions(pl, iset);

  gbtArray<bool> is_dominated(actions.Length());
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
				       const gbtArray<int> &players,
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

//----------------------------------------------------
//                BehavSupportWithActiveInfo
// ---------------------------------------------------

// Utilities 
bool BehavSupportWithActiveInfo::infoset_has_active_nodes(const int pl,
						       const int iset) const
{
  //DEBUG
  /*
  gout << "Got in with pl = " << pl << " and iset = " << iset << ".\n";
  if (InfosetIsActive(pl,iset))
    gout << "Apparently the infoset is active??\n";
  else
    gout << "Somehow it got deactivated.\n";
  */

  for (int i = 1; i <= is_nonterminal_node_active[pl][iset].Length(); i++)
    { //DEBUG
      /*
      gout << "With pl = " << pl << ", iset = " << iset
	   << ", and i = " << i << " the node is supposedly ";
      if (NodeIsActive(pl,iset,i))
	gout << "active.\n";
      else
	gout << "inactive.\n";
      */

    if (is_nonterminal_node_active[pl][iset][i])
      return true;
    }
  return false;
}

bool BehavSupportWithActiveInfo::infoset_has_active_nodes(const GameInfoset &i) const
{
  return infoset_has_active_nodes(i->GetPlayer()->GetNumber(), i->GetNumber());
}

void BehavSupportWithActiveInfo::activate(const GameNode &n)
{
  is_nonterminal_node_active[n->GetPlayer()->GetNumber()]
                            [n->GetInfoset()->GetNumber()]
                            [n->NumberInInfoset()] = true;
}

void BehavSupportWithActiveInfo::deactivate(const GameNode &n)
{
  is_nonterminal_node_active[n->GetPlayer()->GetNumber()]
                            [n->GetInfoset()->GetNumber()]
                            [n->NumberInInfoset()] = false;
}

void BehavSupportWithActiveInfo::activate(const GameInfoset &i)
{
  is_infoset_active[i->GetPlayer()->GetNumber()][i->GetNumber()] = true;
}

void BehavSupportWithActiveInfo::deactivate(const GameInfoset &i)
{
  is_infoset_active[i->GetPlayer()->GetNumber()][i->GetNumber()] = false;
}

void BehavSupportWithActiveInfo::activate_this_and_lower_nodes(const GameNode &n)
{
  if (!n->IsTerminal()) {
    activate(n); 
    activate(n->GetInfoset());
    gbtArray<GameAction> actions(Actions(n->GetInfoset()));
    for (int i = 1; i <= actions.Length(); i++) 
      activate_this_and_lower_nodes(n->GetChild(actions[i]->GetNumber()));    
  }
}

void BehavSupportWithActiveInfo::deactivate_this_and_lower_nodes(const GameNode &n)
{
  if (!n->IsTerminal()) {  // THIS ALL LOOKS FISHY
    deactivate(n); 
    if ( !infoset_has_active_nodes(n->GetInfoset()) )
      deactivate(n->GetInfoset());
    gbtArray<GameAction> actions(Actions(n->GetInfoset()));
      for (int i = 1; i <= actions.Length(); i++) 
	deactivate_this_and_lower_nodes(n->GetChild(actions[i]->GetNumber()));    
  }
}

void BehavSupportWithActiveInfo::
deactivate_this_and_lower_nodes_returning_deactivated_infosets(const GameNode &n, 
                                                gbtList<GameInfoset> *list)
{
  if (!n->IsTerminal()) {
    deactivate(n); 
    if ( !infoset_has_active_nodes(n->GetInfoset()) ) {

      //DEBUG
      /*
      gout << "We are deactivating infoset " << n->GetInfoset()->GetNumber()
	   << " with support \n" << *this << "\n";
      */

      list->Append(n->GetInfoset()); 
      deactivate(n->GetInfoset());
    }
    gbtArray<GameAction> actions(Actions(n->GetInfoset()));
      for (int i = 1; i <= actions.Length(); i++) 
	deactivate_this_and_lower_nodes_returning_deactivated_infosets(
			     n->GetChild(actions[i]->GetNumber()),list);    
  }
}

void BehavSupportWithActiveInfo::InitializeActiveListsToAllActive()
{
  for (int pl = 0; pl <= GetGame()->NumPlayers(); pl++) {
    GamePlayer player = (pl == 0) ? GetGame()->GetChance() : GetGame()->GetPlayer(pl); 
    gbtList<bool>         is_players_infoset_active;
    gbtList<gbtList<bool> > is_players_node_active;
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      is_players_infoset_active.Append(true);

      gbtList<bool> is_infosets_node_active;
      for (int n = 1; n <= player->GetInfoset(iset)->NumMembers(); n++)
	is_infosets_node_active.Append(true);
      is_players_node_active.Append(is_infosets_node_active);
    }
    is_infoset_active[pl] = is_players_infoset_active;
    is_nonterminal_node_active[pl] = is_players_node_active;
  }
}

void BehavSupportWithActiveInfo::InitializeActiveListsToAllInactive()
{
  for (int pl = 0; pl <= GetGame()->NumPlayers(); pl++) {
    GamePlayer player = (pl == 0) ? GetGame()->GetChance() : GetGame()->GetPlayer(pl);
    gbtList<bool>         is_players_infoset_active;
    gbtList<gbtList<bool> > is_players_node_active;

    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      is_players_infoset_active.Append(false);

      gbtList<bool> is_infosets_node_active;
      for (int n = 1; n <= player->GetInfoset(iset)->NumMembers()
; n++)
	is_infosets_node_active.Append(false);
      is_players_node_active.Append(is_infosets_node_active);
    }
    is_infoset_active[pl] = is_players_infoset_active;
    is_nonterminal_node_active[pl] = is_players_node_active;
  }
}

void BehavSupportWithActiveInfo::InitializeActiveLists()
{
  InitializeActiveListsToAllInactive();

  activate_this_and_lower_nodes(GetGame()->GetRoot());
}

// Constructors and Destructor
BehavSupportWithActiveInfo::BehavSupportWithActiveInfo(const Game &E) 
  : BehavSupport(E), 
    is_infoset_active(0,E->NumPlayers()), 
    is_nonterminal_node_active(0,E->NumPlayers())
{
  InitializeActiveLists();
}

BehavSupportWithActiveInfo::BehavSupportWithActiveInfo(const BehavSupport& given)
  : BehavSupport(given), 
    is_infoset_active(0,given.GetGame()->NumPlayers()), 
    is_nonterminal_node_active(0,given.GetGame()->NumPlayers())
{
  InitializeActiveLists();
}

BehavSupportWithActiveInfo::BehavSupportWithActiveInfo(
				  const BehavSupportWithActiveInfo& given)
  : BehavSupport(given),
    //is_infoset_active(0,given.GetGame()->NumPlayers()), 
        is_infoset_active(is_infoset_active), 
    is_nonterminal_node_active(given.is_nonterminal_node_active)
{
  //  InitializeActiveLists();
}

BehavSupportWithActiveInfo::~BehavSupportWithActiveInfo()
{}

// Operators
BehavSupportWithActiveInfo &
BehavSupportWithActiveInfo::operator=(const BehavSupportWithActiveInfo &s)
{
  if (this != &s) {
    ((BehavSupport&) *this) = s;
    is_infoset_active = s.is_infoset_active;
    is_nonterminal_node_active = s.is_nonterminal_node_active;
  }
  return *this;
}

bool 
BehavSupportWithActiveInfo::operator==(const BehavSupportWithActiveInfo &s) const
{
  if ((BehavSupport&) *this != (BehavSupport&) s) {
    //  gout << "Underlying supports differ.\n"; 
    return false; 
  }
  
  if (is_infoset_active != s.is_infoset_active) {
    //    gout<< "Active infosets differ:\n"; 
   
    //  for(int i = 0; i < is_infoset_active.Length(); i++)
    //  gout << "is_infoset_active[" << i << "] = " << is_infoset_active[i];

    //for(int i = 0; i < s.is_infoset_active.Length(); i++)
    //  gout << "s.is_infoset_active[" << i << "] = " << s.is_infoset_active[i];

    return false;
  }
  if (is_nonterminal_node_active != s.is_nonterminal_node_active) {
    // gout << "Active nodes differ.\n";
    return false;
  }
  return true;
}

bool 
BehavSupportWithActiveInfo::operator!=(const BehavSupportWithActiveInfo &s) const
{
  return !(*this == s);
}

gbtList<GameNode> 
BehavSupportWithActiveInfo::ReachableNodesInInfoset(const GameInfoset &i) const
{
  gbtList<GameNode> answer;
  int pl = i->GetPlayer()->GetNumber();
  int iset = i->GetNumber();
  for (int j = 1; j <= i->NumMembers(); j++)
    if (is_nonterminal_node_active[pl][iset][j])
      answer.Append(i->GetMember(j));
  return answer;
}

gbtList<GameNode>
BehavSupportWithActiveInfo::ReachableNonterminalNodes() const
{
  gbtList<GameNode> answer;
  for (int pl = 1; pl <= GetGame()->NumPlayers(); pl++) {
    GamePlayer p = GetGame()->GetPlayer(pl);
    for (int iset = 1; iset <= p->NumInfosets(); iset++)
      answer += ReachableNodesInInfoset(p->GetInfoset(iset));
  }
  return answer;
}

// Editing functions
void BehavSupportWithActiveInfo::AddAction(const GameAction &s)
{
  BehavSupport::AddAction(s);

  gbtList<GameNode> startlist(ReachableNodesInInfoset(s->GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++)
    activate_this_and_lower_nodes(startlist[i]);
}

bool BehavSupportWithActiveInfo::RemoveAction(const GameAction &s)
{
  gbtList<GameNode> startlist(ReachableNodesInInfoset(s->GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++)
    deactivate_this_and_lower_nodes(startlist[i]->GetChild(s->GetNumber()));

  // the following returns false if s was not active
  return BehavSupport::RemoveAction(s);
}

bool 
BehavSupportWithActiveInfo::RemoveActionReturningDeletedInfosets(const GameAction &s,
					   gbtList<GameInfoset> *list)
{

  gbtList<GameNode> startlist(ReachableNodesInInfoset(s->GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++)
    deactivate_this_and_lower_nodes_returning_deactivated_infosets(
                           startlist[i]->GetChild(s->GetNumber()),list);

  // the following returns false if s was not active
  return BehavSupport::RemoveAction(s);
}

int BehavSupportWithActiveInfo::NumActiveNodes(const int pl,
					     const int iset) const
{
  int answer = 0;
  for (int i = 1; i <= is_nonterminal_node_active[pl][iset].Length(); i++)
    if (is_nonterminal_node_active[pl][iset][i])
      answer++;
  return answer;
}

int BehavSupportWithActiveInfo::NumActiveNodes(const GameInfoset &i) const
{
  return NumActiveNodes(i->GetPlayer()->GetNumber(),i->GetNumber());
}

bool BehavSupportWithActiveInfo::InfosetIsActive(const int pl,
					      const int iset) const
{
  return is_infoset_active[pl][iset];
}

bool BehavSupportWithActiveInfo::InfosetIsActive(const GameInfoset &i) const
{
  return InfosetIsActive(i->GetPlayer()->GetNumber(),i->GetNumber());
}

bool BehavSupportWithActiveInfo::NodeIsActive(const int pl,
					   const int iset,
					   const int node) const
{
  return is_nonterminal_node_active[pl][iset][node];
}

bool BehavSupportWithActiveInfo::NodeIsActive(const GameNode &n) const
{
  return NodeIsActive(n->GetInfoset()->GetPlayer()->GetNumber(),
		      n->GetInfoset()->GetNumber(),
		      n->NumberInInfoset());
}

bool BehavSupportWithActiveInfo::HasActiveActionsAtActiveInfosets()
{
  for (int pl = 1; pl <= GetGame()->NumPlayers(); pl++)
    for (int iset = 1; iset <= GetGame()->GetPlayer(pl)->NumInfosets(); iset++) 
      if (InfosetIsActive(pl,iset))
        if ( NumActions(GetGame()->GetPlayer(pl)->GetInfoset(iset)) == 0 )
          return false;
  return true;
}

bool BehavSupportWithActiveInfo::HasActiveActionsAtActiveInfosetsAndNoOthers()
{
  for (int pl = 1; pl <= GetGame()->NumPlayers(); pl++)
    for (int iset = 1; iset <= GetGame()->GetPlayer(pl)->NumInfosets(); iset++) {
      if (InfosetIsActive(pl,iset))
        if ( NumActions(GetGame()->GetPlayer(pl)->GetInfoset(iset)) == 0 )
          return false;
      if (!InfosetIsActive(pl,iset))
        if ( NumActions(GetGame()->GetPlayer(pl)->GetInfoset(iset)) > 0 )
          return false;
      }
  return true;
}

} // end namespace Gambit
