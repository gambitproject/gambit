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

#include "game.h"
#include "efgsupport.h"
#include "behav.h"
#include "gamebase.h"

class gbtEfgSupportInfoset   {
public:
  gbtGameInfoset m_infoset;
  gbtBlock<gbtGameAction> m_actions;

  gbtEfgSupportInfoset(const gbtGameInfoset &p_infoset);

  bool operator==(const gbtEfgSupportInfoset &a) const;

  void Set(int i, const gbtGameAction &action) { m_actions[i] = action; }

  // Implementation of information set interface
  gbtGameAction GetAction(int act) const { return m_actions[act]; }
  int NumActions(void) const { return m_actions.Length(); }

  void DeleteInfoset(void) const { throw gbtGameUndefinedOperation(); }
};

//----------------------------------------------------
// gbtEfgSupportInfoset: Constructors, Destructor, operators
// ---------------------------------------------------

gbtEfgSupportInfoset::gbtEfgSupportInfoset(const gbtGameInfoset &p_infoset)
  : m_infoset(p_infoset)
{
  for (int act = 1; act <= m_infoset->NumActions(); act++) {
    m_actions += m_infoset->GetAction(act);
  }
}

#ifdef __BORLANDC__
bool operator==(const gbtArray<gbtGameAction> &a, const gbtArray<gbtGameAction> &b)
{
  if (a.First() != b.First() || a.Last() != b.Last())  {
    return false;
  }

  for (int i = a.First(); i <= a.Last(); i++) {
    if (a[i] != b[i])   return false;
  }
  return true;
}
#endif

bool gbtEfgSupportInfoset::operator==(const gbtEfgSupportInfoset &a) const
{
  return (m_actions == a.m_actions);
}

class gbtEfgSupportPlayer  {
public:
  gbtGamePlayer m_player;
  gbtArray<gbtEfgSupportInfoset *> m_infosets;
  
  //----------------------------------------
  // Constructors, Destructor, operators
  //----------------------------------------

  gbtEfgSupportPlayer(const gbtEfgSupportPlayer &);
  gbtEfgSupportPlayer(const gbtGamePlayer &);
  virtual ~gbtEfgSupportPlayer();

  bool operator==(const gbtEfgSupportPlayer &s) const;

  //--------------------
  // Member Functions
  //--------------------

  // Append an action to an infoset;
  void AddAction(int iset, const gbtGameAction &);

  // Remove an action from an infoset . 
  // Returns true if the action was successfully removed, false otherwise.
  bool RemoveAction(int iset, const gbtGameAction &);

  // returns the index of the action if it is in the ActionSet
  int Find(const gbtGameAction &) const;

  int NumInfosets(void) const { return m_infosets.Length(); }
  gbtEfgSupportInfoset *GetInfoset(int p_index) const
  { return m_infosets[p_index]; }

  // checks for a valid gbtEfgSupportPlayer
  bool HasActiveActionsAtAllInfosets(void) const;
};

//--------------------------------------------------
// gbtEfgSupportPlayer: Constructors, Destructor, operators
//--------------------------------------------------

gbtEfgSupportPlayer::gbtEfgSupportPlayer(const gbtGamePlayer &p)
  : m_player(p), m_infosets(p->NumInfosets())
{
  for (int i = 1; i <= p->NumInfosets(); i++) {
    m_infosets[i] = new gbtEfgSupportInfoset(p->GetInfoset(i));
    for (int j = 1; j <= p->GetInfoset(i)->NumActions(); j++) {
      m_infosets[i]->Set(j, p->GetInfoset(i)->GetAction(j));
    }
  }
}

gbtEfgSupportPlayer::gbtEfgSupportPlayer(const gbtEfgSupportPlayer &s)
  : m_player(s.m_player), m_infosets(s.m_infosets.Length())
{
  for (int i = 1; i <= s.m_infosets.Length(); i++){
    m_infosets[i] = new gbtEfgSupportInfoset(*(s.m_infosets[i]));
  }
}

gbtEfgSupportPlayer::~gbtEfgSupportPlayer()
{ 
  for (int i = 1; i <= m_infosets.Length(); delete m_infosets[i++]);
}

bool gbtEfgSupportPlayer::operator==(const gbtEfgSupportPlayer &s) const
{
  if (m_infosets.Length() != s.m_infosets.Length() ||
      m_player != s.m_player) {
    return false;
  }
  
  int i;
  for (i = 1; i <= m_infosets.Length() && 
       *(m_infosets[i]) == *(s.m_infosets[i]);  i++);
  return (i > m_infosets.Length());
}

//------------------------------------------
// gbtEfgSupportPlayer: Member functions 
//------------------------------------------

// Append an action to a particular infoset;
void gbtEfgSupportPlayer::AddAction(int iset, const gbtGameAction &s)
{ 
  if (m_infosets[iset]->m_actions.Find(s))
    return;

  if (m_infosets[iset]->m_actions.Length() == 0) {
    m_infosets[iset]->m_actions.Append(s); 
  }
  else {
    int index = 1;
    while (index <= m_infosets[iset]->m_actions.Length() &&
	   m_infosets[iset]->m_actions[index]->GetId() < s->GetId()) 
      index++;
    m_infosets[iset]->m_actions.Insert(s,index);
  }
}


// Removes an action from infoset iset . Returns true if the 
//Action was successfully removed, false otherwise.
bool gbtEfgSupportPlayer::RemoveAction(int iset, const gbtGameAction &s)
{ 
  int t = m_infosets[iset]->m_actions.Find(s); 
  if (t>0) m_infosets[iset]->m_actions.Remove(t); 
  return (t>0); 
} 

int gbtEfgSupportPlayer::Find(const gbtGameAction &a) const
{
  return (m_infosets[a->GetInfoset()->GetId()]->m_actions.Find(a));
}

// checks for a valid gbtEfgSupportPlayer
bool gbtEfgSupportPlayer::HasActiveActionsAtAllInfosets(void) const
{
  if (m_infosets.Length() != m_player->NumInfosets())   return false;

  for (int i = 1; i <= m_infosets.Length(); i++)
    if (m_infosets[i]->m_actions.Length() == 0)   return false;

  return true;
}

//--------------------------------------------------
// gbtEfgSupportBase: Constructors, Destructors, Operators
//--------------------------------------------------

gbtEfgSupportBase::gbtEfgSupportBase(const gbtGame &p_efg)
  : m_efg(p_efg), m_players(p_efg->NumPlayers()),
    is_infoset_active(0, p_efg->NumPlayers()), 
    is_nonterminal_node_active(0, p_efg->NumPlayers())
{
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    m_players[pl] = new gbtEfgSupportPlayer(p_efg->GetPlayer(pl));
  }
  InitializeActiveLists();
}


gbtEfgSupportBase::gbtEfgSupportBase(const gbtEfgSupportBase &p_support)
  : m_label(p_support.m_label), m_efg(p_support.m_efg),
    m_players(p_support.m_players.Length()),
    is_infoset_active(p_support.is_infoset_active),
    is_nonterminal_node_active(p_support.is_nonterminal_node_active)
{
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    m_players[pl] = new gbtEfgSupportPlayer(*(p_support.m_players[pl]));
  }
}

gbtEfgSupportBase::~gbtEfgSupportBase()
{
  for (int pl = 1; pl <= m_players.Length(); pl++)
    delete m_players[pl];
}

bool gbtEfgSupportBase::operator==(const gbtEfgSupportRep &p_support) const
{
  const gbtEfgSupportBase &support = dynamic_cast<const gbtEfgSupportBase &>(p_support);
  if (m_players.Length() != support.m_players.Length())
    return false;

  if (is_infoset_active != support.is_infoset_active ||
      is_nonterminal_node_active != support.is_nonterminal_node_active) {
    return false;
  }

  int pl;
  for (pl = 1; (pl <= m_players.Length() &&
		*(m_players[pl]) == *(support.m_players[pl])); pl++);
  return (pl > m_players.Length());
}

//-----------------------------
// gbtEfgSupportBase: Member Functions 
//-----------------------------

int gbtEfgSupportBase::NumActions(int pl, int iset) const
{
  if (pl == 0) {
    return m_efg->GetChance()->GetInfoset(iset)->NumActions();
  }
  else {
    return m_players[pl]->GetInfoset(iset)->NumActions();
  }
}

int gbtEfgSupportBase::NumActions(const gbtGameInfoset &i) const
{
  if (i->GetPlayer()->IsChance()) {
    return i->NumActions();
  }
  else {
    return m_players[i->GetPlayer()->GetId()]->GetInfoset(i->GetId())->NumActions();
  }
}

int gbtEfgSupportBase::GetIndex(const gbtGameAction &a) const
{
  if (a->GetInfoset()->GetGame() != m_efg)  return 0;
  int pl = a->GetInfoset()->GetPlayer()->GetId();
  return m_players[pl]->Find(a);
}

bool gbtEfgSupportBase::Contains(const gbtGameAction &a) const
{
  if (a->GetInfoset()->GetGame() != m_efg)   
    return false;

  int pl = a->GetInfoset()->GetPlayer()->GetId();

  if (pl == 0) return true; // Chance

  int act = m_players[pl]->Find(a);
  if (act == 0) 
    return false;
  else
    return true;
}

bool gbtEfgSupportBase::Contains(int pl, int iset, int act) const
{
  return Contains(GetPlayer(pl)->GetInfoset(iset)->GetAction(act));
}

gbtGameAction gbtEfgSupportBase::GetAction(int pl, int iset, int act) const
{
  if (pl == 0) {
    return m_efg->GetChance()->GetInfoset(iset)->GetAction(act);
  }
  else {
    return m_players[pl]->GetInfoset(iset)->GetAction(act);
  }
}

gbtGameAction gbtEfgSupportBase::GetAction(const gbtGameInfoset &infoset, int act) const
{
  if (infoset->GetPlayer()->IsChance()) {
    return infoset->GetAction(act);
  }
  else {
    return m_players[infoset->GetPlayer()->GetId()]->GetInfoset(infoset->GetId())->GetAction(act);
  }
}

bool gbtEfgSupportBase::HasActiveActionAt(const gbtGameInfoset &infoset) const
{
  if (m_players[infoset->GetPlayer()->GetId()]->GetInfoset(infoset->GetId())->NumActions() == 0) {
    return false;
  }
  else {
    return true;
  }
}

int gbtEfgSupportBase::NumDegreesOfFreedom(void) const
{
  int answer(0);

  gbtList<gbtGameInfoset> active_infosets = ReachableInfosets(GetRoot());
  for (int i = 1; i <= active_infosets.Length(); i++)
    answer += NumActions(active_infosets[i]) - 1;

  return answer;  
}

bool gbtEfgSupportBase::HasActiveActionsAtAllInfosets(void) const
{
  if (m_players.Length() != m_efg->NumPlayers())   return false;
  for (int i = 1; i <= m_players.Length(); i++)
    if (!m_players[i]->HasActiveActionsAtAllInfosets())  return false;

  return true;
}

gbtPVector<int> gbtEfgSupportBase::NumActions(void) const
{
  gbtArray<int> foo(m_efg->NumPlayers());
  for (int i = 1; i <= m_efg->NumPlayers(); i++) {
    foo[i] = m_players[i]->NumInfosets();
  }

  gbtPVector<int> bar(foo);
  for (int i = 1; i <= m_efg->NumPlayers(); i++)
    for (int j = 1; j <= m_players[i]->NumInfosets(); j++)
      bar(i, j) = NumActions(i,j);

  return bar;
}  

int gbtEfgSupportBase::BehavProfileLength(void) const
{
  gbtArray<int> actions = NumActions();
  int total = 0;
  for (int i = 1; i <= actions.Length(); total += actions[i++]);
  return total;
}

int gbtEfgSupportBase::NumSequences(int j) const
{
  if (j < 1 || j > m_efg->NumPlayers()) return 1;
  gbtList<gbtGameInfoset> isets = ReachableInfosets(m_efg->GetPlayer(j));
  int num = 1;
  for(int i = 1; i <= isets.Length(); i++)
    num+=NumActions(isets[i]);
  return num;
}

int gbtEfgSupportBase::TotalNumSequences(void) const
{
  int total = 0;
  for (int i = 1 ; i <= m_efg->NumPlayers(); i++)
    total += NumSequences(i);
  return total;
}

gbtList<gbtGameNode> 
gbtEfgSupportBase::ReachableNonterminalNodes(const gbtGameNode &n) const
{
  gbtList<gbtGameNode> answer;
  if (n->IsNonterminal()) {
    for (int i = 1; i <= NumActions(n->GetInfoset()); i++) {
      gbtGameNode nn = n->GetChild(GetAction(n->GetInfoset(), i));
      if (nn->IsNonterminal()) {
	answer += nn;
	answer += ReachableNonterminalNodes(nn);
      }
    }
  }
  return answer;
}

gbtList<gbtGameNode> 
gbtEfgSupportBase::ReachableNonterminalNodes(const gbtGameNode &n,
				     const gbtGameAction &a) const
{
  gbtList<gbtGameNode> answer;
  gbtGameNode nn = n->GetChild(a);
  if (nn->IsNonterminal()) {
    answer += nn;
    answer += ReachableNonterminalNodes(nn);
  }
  return answer;
}

gbtList<gbtGameInfoset> gbtEfgSupportBase::ReachableInfosets(const gbtGamePlayer &p) const
{ 
  gbtList<gbtGameInfoset> answer;

  for (int i = 1; i <= p->NumInfosets(); i++) {
    if (MayReach(p->GetInfoset(i))) {
      answer += p->GetInfoset(i);
    }
  }
  return answer;
}

gbtList<gbtGameInfoset> gbtEfgSupportBase::ReachableInfosets(const gbtGameNode &n) const
{
  gbtList<gbtGameInfoset> answer;
  gbtList<gbtGameNode> nodelist = ReachableNonterminalNodes(n);
  for (int i = 1; i <= nodelist.Length(); i++)
    answer += nodelist[i]->GetInfoset();
  answer.RemoveRedundancies();
  return answer;
}

gbtList<gbtGameInfoset> gbtEfgSupportBase::ReachableInfosets(const gbtGameNode &n, 
						  const gbtGameAction &a) const
{
  gbtList<gbtGameInfoset> answer;
  gbtList<gbtGameNode> nodelist = ReachableNonterminalNodes(n,a);
  for (int i = 1; i <= nodelist.Length(); i++)
    answer += nodelist[i]->GetInfoset();
  answer.RemoveRedundancies();
  return answer;
}

bool gbtEfgSupportBase::AlwaysReaches(const gbtGameInfoset &i) const
{
  return AlwaysReachesFrom(i, m_efg->GetRoot());
}

bool gbtEfgSupportBase::AlwaysReachesFrom(const gbtGameInfoset &i, 
				  const gbtGameNode &n) const
{
  if (n->IsTerminal()) return false;
  else {
    if (n->GetInfoset() == i) return true;
    else {
      for (int j = 1; j <= NumActions(n->GetInfoset()); j++) {
	if (!AlwaysReachesFrom(i, n->GetChild(GetAction(n->GetInfoset(), j)))) {
	  return false;
	}
      }
    }
  }
  return true;
}

bool gbtEfgSupportBase::MayReach(const gbtGameInfoset &infoset) const
{
  for (int j = 1; j <= infoset->NumMembers(); j++) {
    if (MayReach(infoset->GetMember(j))) {
      return true;
    }
  }
  return false;
}

bool gbtEfgSupportBase::MayReach(const gbtGameNode &n) const
{
  if (n == m_efg->GetRoot())
    return true;
  else {
    if (!Contains(n->GetPriorAction())) {
      return false;
    }
    else {
      return MayReach(n->GetParent());
    }
  }
}


void gbtEfgSupportBase::Dump(gbtOutput &p_output) const
{
  p_output << '"' << m_label << "\" { ";
  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++)  {
    gbtGamePlayer player = m_players[pl]->m_player;
    p_output << '"' << player->GetLabel() << "\" { ";
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      gbtGameInfoset infoset = player->GetInfoset(iset);
      p_output << '"' << infoset->GetLabel() << "\" { ";
      for (int act = 1; act <= NumActions(pl, iset); act++)  {
	gbtGameAction action = m_players[pl]->GetInfoset(iset)->GetAction(act);
	p_output << action << ' ';
      }
      p_output << "} ";
    }
    p_output << "} ";
  }
  p_output << "} ";
}

gbtEfgSupportRep *gbtEfgSupportBase::Copy(void) const
{
  return new gbtEfgSupportBase(*this);
}

gbtEfgSupport gbtEfgSupportBase::NewEfgSupport(void) const
{
  return new gbtEfgSupportBase(*this);
}

gbtBehavProfile<double> gbtEfgSupportBase::NewBehavProfile(double) const
{
  return new gbtBehavProfileBase<double>(NewEfgSupport());
}

gbtBehavProfile<gbtRational>
gbtEfgSupportBase::NewBehavProfile(const gbtRational &) const
{
  return new gbtBehavProfileBase<gbtRational>(NewEfgSupport());
}

gbtBehavProfile<gbtNumber>
gbtEfgSupportBase::NewBehavProfile(const gbtNumber &) const
{
  return new gbtBehavProfileBase<gbtNumber>(NewEfgSupport());
}

gbtOutput& operator<<(gbtOutput&s, const gbtEfgSupport &p_support)
{
  p_support->Dump(s);
  return s;
}

//-------------------------------------------------------------------------
//
// The following member functions were combined from a derived class.
//
gbtList<gbtGameNode> 
gbtEfgSupportBase::ReachableNodesInInfoset(const gbtGameInfoset &i) const
{
  gbtList<gbtGameNode> answer;
  int pl = i->GetPlayer()->GetId();
  int iset = i->GetId();
  for (int j = 1; j <= i->NumMembers(); j++)
    if (is_nonterminal_node_active[pl][iset][j])
      answer += i->GetMember(j);
  return answer;
}

bool gbtEfgSupportBase::HasActiveActionsAtActiveInfosets(void)
{
  for (int pl = 1; pl <= NumPlayers(); pl++)
    for (int iset = 1; iset <= GetPlayer(pl)->NumInfosets(); iset++) 
      if (InfosetIsActive(GetPlayer(pl)->GetInfoset(iset)))
        if ( NumActions(GetPlayer(pl)->GetInfoset(iset)) == 0 )
          return false;
  return true;
}

bool gbtEfgSupportBase::HasActiveActionsAtActiveInfosetsAndNoOthers(void)
{
  for (int pl = 1; pl <= NumPlayers(); pl++)
    for (int iset = 1; iset <= GetPlayer(pl)->NumInfosets(); iset++) {
      if (InfosetIsActive(GetPlayer(pl)->GetInfoset(iset)))
        if ( NumActions(GetPlayer(pl)->GetInfoset(iset)) == 0 )
          return false;
      if (!InfosetIsActive(GetPlayer(pl)->GetInfoset(iset)))
        if ( NumActions(GetPlayer(pl)->GetInfoset(iset)) > 0 )
          return false;
      }
  return true;
}

void gbtEfgSupportBase::deactivate(const gbtGameNode &n)
{
  is_nonterminal_node_active[n->GetPlayer()->GetId()]
                            [n->GetInfoset()->GetId()]
                            [n->GetMemberId()] = false;
}

void gbtEfgSupportBase::
deactivate_this_and_lower_nodes_returning_deactivated_infosets(const gbtGameNode &n, 
                                                gbtList<gbtGameInfoset> *list)
{
  if (n->IsNonterminal()) {
    deactivate(n); 
    if ( !infoset_has_active_nodes(n->GetInfoset()) ) {

      //DEBUG
      /*
      gout << "We are deactivating infoset " << n->GetInfoset()->GetNumber()
	   << " with support \n" << *this << "\n";
      */

      (*list) += n->GetInfoset(); 
      deactivate(n->GetInfoset());
    }
    for (int i = 1; i <= NumActions(n->GetInfoset()); i++) 
      deactivate_this_and_lower_nodes_returning_deactivated_infosets(n->GetChild(GetAction(n->GetInfoset(), i)), list);    
  }
}

bool 
gbtEfgSupportBase::RemoveActionReturningDeletedInfosets(const gbtGameAction &s,
						    gbtList<gbtGameInfoset> *list)
{
  gbtList<gbtGameNode> startlist(ReachableNodesInInfoset(s->GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++)
    deactivate_this_and_lower_nodes_returning_deactivated_infosets(
                           startlist[i]->GetChild(s),list);

  // the following returns false if s was not active
  return RemoveAction(s);
}



//----------------------------------------------------
//                gbtEfgSupportBase
// ---------------------------------------------------

// Utilities 
bool gbtEfgSupportBase::infoset_has_active_nodes(const int pl,
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

bool
gbtEfgSupportBase::infoset_has_active_nodes(const gbtGameInfoset &i) const
{
  return infoset_has_active_nodes(i->GetPlayer()->GetId(), i->GetId());
}

void gbtEfgSupportBase::activate(const gbtGameNode &n)
{
  is_nonterminal_node_active[n->GetPlayer()->GetId()]
                            [n->GetInfoset()->GetId()]
                            [n->GetMemberId()] = true;
}

void gbtEfgSupportBase::activate(const gbtGameInfoset &i)
{
  is_infoset_active[i->GetPlayer()->GetId()][i->GetId()] = true;
}

void gbtEfgSupportBase::deactivate(const gbtGameInfoset &i)
{
  is_infoset_active[i->GetPlayer()->GetId()][i->GetId()] = false;
}

void
gbtEfgSupportBase::activate_this_and_lower_nodes(const gbtGameNode &n)
{
  if (n->IsNonterminal()) {
    activate(n); 
    activate(n->GetInfoset());
    for (int i = 1; i <= NumActions(n->GetInfoset()); i++) 
      activate_this_and_lower_nodes(n->GetChild(GetAction(n->GetInfoset(), i)));
  }
}

void
gbtEfgSupportBase::deactivate_this_and_lower_nodes(const gbtGameNode &n)
{
  if (n->IsNonterminal()) {  // THIS ALL LOOKS FISHY
    deactivate(n); 
    if ( !infoset_has_active_nodes(n->GetInfoset()) )
      deactivate(n->GetInfoset());
    for (int i = 1; i <= NumActions(n->GetInfoset()); i++) {
      deactivate_this_and_lower_nodes(n->GetChild(GetAction(n->GetInfoset(), i)));
    }
  }
}

void gbtEfgSupportBase::InitializeActiveListsToAllActive()
{
  for (int pl = 0; pl <= NumPlayers(); pl++) {
    gbtGamePlayer player = (pl == 0) ? GetChance() : GetPlayer(pl); 
    gbtList<bool>         is_players_infoset_active;
    gbtList<gbtList<bool> > is_players_node_active;
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      is_players_infoset_active += true;

      gbtList<bool> is_infosets_node_active;
      for (int n = 1; n <= player->GetInfoset(iset)->NumMembers(); n++)
	is_infosets_node_active += true;
      is_players_node_active += is_infosets_node_active;
    }
    is_infoset_active[pl] = is_players_infoset_active;
    is_nonterminal_node_active[pl] = is_players_node_active;
  }
}

void gbtEfgSupportBase::InitializeActiveListsToAllInactive()
{
  for (int pl = 0; pl <= NumPlayers(); pl++) {
    gbtGamePlayer player = (pl == 0) ? GetChance() : GetPlayer(pl);
    gbtList<bool>         is_players_infoset_active;
    gbtList<gbtList<bool> > is_players_node_active;

    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      is_players_infoset_active += false;

      gbtList<bool> is_infosets_node_active;
      for (int n = 1; n <= player->GetInfoset(iset)->NumMembers()
; n++)
	is_infosets_node_active += false;
      is_players_node_active += is_infosets_node_active;
    }
    is_infoset_active[pl] = is_players_infoset_active;
    is_nonterminal_node_active[pl] = is_players_node_active;
  }
}

void gbtEfgSupportBase::InitializeActiveLists()
{
  InitializeActiveListsToAllInactive();

  activate_this_and_lower_nodes(GetRoot());
}

// Member Function
gbtList<gbtGameNode>
gbtEfgSupportBase::ReachableNonterminalNodes() const
{
  gbtList<gbtGameNode> answer;
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    gbtGamePlayer p = GetPlayer(pl);
    for (int iset = 1; iset <= p->NumInfosets(); iset++) {
      answer += ReachableNodesInInfoset(p->GetInfoset(iset));
    }
  }
  return answer;
}

// Editing functions
void gbtEfgSupportBase::AddAction(const gbtGameAction &s)
{
  gbtGameInfoset infoset = s->GetInfoset();
  gbtGamePlayer player = infoset->GetPlayer();

  m_players[player->GetId()]->AddAction(infoset->GetId(), s);

  gbtList<gbtGameNode> startlist(ReachableNodesInInfoset(s->GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++)
    activate_this_and_lower_nodes(startlist[i]);
}

bool gbtEfgSupportBase::RemoveAction(const gbtGameAction &s)
{
  gbtList<gbtGameNode> startlist(ReachableNodesInInfoset(s->GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++)
    deactivate_this_and_lower_nodes(startlist[i]->GetChild(s));

  gbtGameInfoset infoset = s->GetInfoset();
  gbtGamePlayer player = infoset->GetPlayer();
 
  return m_players[player->GetId()]->RemoveAction(infoset->GetId(), s); 
}

int gbtEfgSupportBase::NumActiveNodes(int pl, int iset) const
{
  int answer = 0;
  for (int i = 1; i <= is_nonterminal_node_active[pl][iset].Length(); i++)
    if (is_nonterminal_node_active[pl][iset][i])
      answer++;
  return answer;
}

int gbtEfgSupportBase::NumActiveNodes(const gbtGameInfoset &i) const
{
  return NumActiveNodes(i->GetPlayer()->GetId(), i->GetId());
}

bool gbtEfgSupportBase::InfosetIsActive(const int pl,
					      const int iset) const
{
  return is_infoset_active[pl][iset];
}

bool gbtEfgSupportBase::InfosetIsActive(const gbtGameInfoset &i) const
{
  return InfosetIsActive(i->GetPlayer()->GetId(), i->GetId());
}

bool gbtEfgSupportBase::NodeIsActive(const int pl,
					   const int iset,
					   const int node) const
{
  return is_nonterminal_node_active[pl][iset][node];
}

bool gbtEfgSupportBase::NodeIsActive(const gbtGameNode &n) const
{
  return NodeIsActive(n->GetInfoset()->GetPlayer()->GetId(),
		      n->GetInfoset()->GetId(),
		      n->GetMemberId());
}



// Instantiations
#include "base/glist.imp"

template class gbtList<gbtEfgSupport>;

#include "math/gvector.imp"
#include "math/gpvector.imp"
#include "math/gdpvect.imp"

template class gbtVector<bool>;
template class gbtPVector<bool>;
template class gbtDPVector<bool>;
