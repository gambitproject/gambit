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

#include "efg.h"
#include "efplayer.h"
#include "efstrat.h"

class EFActionArray   {
  friend class EFActionSet;
protected:
  gBlock<gbtEfgAction> acts;

public:
  EFActionArray ( const gArray<gbtEfgAction> &a);
  EFActionArray ( const EFActionArray &a);
  virtual ~EFActionArray();
  EFActionArray &operator=( const EFActionArray &a);
  bool operator==( const EFActionArray &a) const;
  const gbtEfgAction &operator[](int i) const { return acts[i]; }
  gbtEfgAction operator[](int i) { return acts[i]; }
  void Set(int i, const gbtEfgAction &action) { acts[i] = action; }

  // Information
  int Length() const { return acts.Length(); }
};

//----------------------------------------------------
// EFActionArray: Constructors, Destructor, operators
// ---------------------------------------------------

EFActionArray::EFActionArray(const gArray<gbtEfgAction> &a)
  : acts(a.Length())
{
  for (int i = 1; i <= acts.Length(); i++)
    acts[i] = a[i];
 }

EFActionArray::EFActionArray(const EFActionArray &a)
  : acts(a.acts)
{ }

EFActionArray::~EFActionArray ()
{ }

EFActionArray &EFActionArray::operator=( const EFActionArray &a)
{
  acts = a.acts; 
  return *this;
}

#ifdef __BORLANDC__
bool operator==(const gArray<Action *> &a, const gArray<Action *> &b)
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

bool EFActionArray::operator==(const EFActionArray &a) const
{
  return (acts == a.acts);
}

class EFActionSet  {
protected:
  gbtEfgPlayer efp;
  gArray < EFActionArray *> infosets;
public:
  
  //----------------------------------------
  // Constructors, Destructor, operators
  //----------------------------------------

//  EFActionSet();
  EFActionSet(const EFActionSet &);
  EFActionSet(const gbtEfgPlayer &);
  virtual ~EFActionSet();

  EFActionSet &operator=(const EFActionSet &);
  bool operator==(const EFActionSet &s) const;

  //--------------------
  // Member Functions
  //--------------------

  // Append an action to an infoset;
  void AddAction(int iset, const gbtEfgAction &);

  // Insert an action in a particular place in an infoset;
  void AddAction(int iset, const gbtEfgAction &, int index);


  // Remove an action at int i, returns the removed action pointer
  gbtEfgAction RemoveAction(int iset, int i);

  // Remove an action from an infoset . 
  // Returns true if the action was successfully removed, false otherwise.
  bool RemoveAction(int iset, const gbtEfgAction &);

  // Get a garray of the actions in an Infoset
  //  const gArray<Action *> &ActionList(int iset) const
  //   { return infosets[iset]->acts; }

  // Get the EFActionArray of an iset
  const EFActionArray *ActionArray(int iset) const
     { return infosets[iset]; }

  // Get the EFActionArray of an Infoset
  const EFActionArray *ActionArray(const Infoset *i) const
     { return infosets[i->GetNumber()]; }
  
  // Get an Action
  gbtEfgAction GetAction(int iset, int index);

  // returns the index of the action if it is in the ActionSet
  int Find(const gbtEfgAction &) const;

  // Number of Actions in a particular infoset
  int NumActions(int iset) const;

  // return the player of the EFActionSet
  gbtEfgPlayer GetPlayer(void) const;

  // checks for a valid EFActionSet
  bool HasActiveActionsAtAllInfosets(void) const;
  bool HasActiveActionAt(const int &iset) const;

};

//--------------------------------------------------
// EFActionSet: Constructors, Destructor, operators
//--------------------------------------------------

EFActionSet::EFActionSet(const gbtEfgPlayer &p)
  : efp(p), infosets(p.NumInfosets())
{
  for (int i = 1; i <= p.NumInfosets(); i++) {
    infosets[i] = new EFActionArray(p.GetInfoset(i)->NumActions());
    for (int j = 1; j <= p.GetInfoset(i)->NumActions(); j++) {
      infosets[i]->Set(j, p.GetInfoset(i)->GetAction(j));
    }
  }
}

EFActionSet::EFActionSet( const EFActionSet &s )
: infosets(s.infosets.Length())
{
  efp = s.efp;
  for (int i = 1; i <= s.infosets.Length(); i++){
    infosets[i] = new EFActionArray(*(s.infosets[i]));
  }
}

EFActionSet::~EFActionSet()
{ 
  for (int i = 1; i <= infosets.Length(); i++)
    delete infosets[i];
}

EFActionSet &EFActionSet::operator=(const EFActionSet &s)
{
  if (this != &s && efp == s.efp) {
    for (int i = 1; i<= infosets.Length(); i++)  {
      delete infosets[i];
      infosets[i] = new EFActionArray(*(s.infosets[i]));
    }
  }    
  return *this;
}

bool EFActionSet::operator==(const EFActionSet &s) const
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
// EFActionSet: Member functions 
//------------------------------------------

// Append an action to a particular infoset;
void EFActionSet::AddAction(int iset, const gbtEfgAction &s)
{ 
  if (infosets[iset]->acts.Find(s))
    return;

  if (infosets[iset]->acts.Length() == 0) {
    infosets[iset]->acts.Append(s); 
  }
  else {
    int index = 1;
    while (index <= infosets[iset]->acts.Length() &&
	   infosets[iset]->acts[index].GetId() < s.GetId()) 
      index++;
    infosets[iset]->acts.Insert(s,index);
  }
}

// Insert an action  to a particular infoset at a particular place;
void EFActionSet::AddAction(int iset, const gbtEfgAction &s, int index)
{ 
  if (!infosets[iset]->acts.Find(s)) {
    infosets[iset]->acts.Insert(s,index); 
  }
}

// Remove an action from infoset iset at int i, 
// returns the removed Infoset pointer
gbtEfgAction EFActionSet::RemoveAction(int iset, int i) 
{ 
  return (infosets[iset]->acts.Remove(i)); 
}

// Removes an action from infoset iset . Returns true if the 
//Action was successfully removed, false otherwise.
bool EFActionSet::RemoveAction(int iset, const gbtEfgAction &s)
{ 
  int t = infosets[iset]->acts.Find(s); 
  if (t>0) infosets[iset]->acts.Remove(t); 
  return (t>0); 
} 

// Get an action
gbtEfgAction EFActionSet::GetAction(int iset, int index)
{
  return (infosets[iset]->acts)[index];
}

// Number of Actions in a particular infoset
int EFActionSet::NumActions(int iset) const
{
  return (infosets[iset]->acts.Length());
}

// Return the player of this EFActionSet
gbtEfgPlayer EFActionSet::GetPlayer(void) const
{
  return efp;
}

int EFActionSet::Find(const gbtEfgAction &a) const
{
  return (infosets[a.GetInfoset()->GetNumber()]->acts.Find(a));
}

// checks for a valid EFActionSet
bool EFActionSet::HasActiveActionsAtAllInfosets(void) const
{
  if (infosets.Length() != efp.NumInfosets())   return false;

  for (int i = 1; i <= infosets.Length(); i++)
    if (infosets[i]->acts.Length() == 0)   return false;

  return true;
}

// checks for a valid EFActionSet
bool EFActionSet::HasActiveActionAt(const int &iset) const
{
  if (iset > efp.NumInfosets())   return false;

  if (infosets[iset]->acts.Length() == 0)   return false;

  return true;
}


//--------------------------------------------------
// EFSupport: Constructors, Destructors, Operators
//--------------------------------------------------

EFSupport::EFSupport(const efgGame &p_efg) 
  : m_efg((efgGame *) &p_efg), m_players(p_efg.NumPlayers())
{
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    m_players[pl] = new EFActionSet(p_efg.GetPlayer(pl));
  }
}

EFSupport::EFSupport(const EFSupport &p_support)
  : m_name(p_support.m_name), m_efg(p_support.m_efg),
    m_players(p_support.m_players.Length())
{
  for (int pl = 1; pl <= m_players.Length(); pl++)
    m_players[pl] = new EFActionSet(*(p_support.m_players[pl]));
}

EFSupport::~EFSupport()
{
  for (int pl = 1; pl <= m_players.Length(); pl++)
    delete m_players[pl];
}

EFSupport &EFSupport::operator=(const EFSupport &p_support)
{
  if (this != &p_support && m_efg == p_support.m_efg) {
    m_name = p_support.m_name;
    for (int pl = 1; pl <= m_players.Length(); pl++)  {
      delete m_players[pl];
      m_players[pl] = new EFActionSet(*(p_support.m_players[pl]));
    }
  }
  return *this;
}

bool EFSupport::operator==(const EFSupport &p_support) const
{
  if (m_players.Length() != p_support.m_players.Length())
    return false;

  int pl;
  for (pl = 1; (pl <= m_players.Length() &&
		*(m_players[pl]) == *(p_support.m_players[pl])); pl++);
  return (pl > m_players.Length());
}

bool EFSupport::operator!=(const EFSupport &p_support) const
{
  return !(*this == p_support);
}

//-----------------------------
// EFSupport: Member Functions 
//-----------------------------

int EFSupport::NumActions(int pl, int iset) const
{
  if (pl == 0) {
    return m_efg->GetChance().GetInfoset(iset)->NumActions();
  }
  else {
    return m_players[pl]->NumActions(iset);
  }
}

int EFSupport::NumActions(const Infoset *i) const
{
  if (i->GetPlayer().IsChance()) {
    return i->NumActions();
  }
  else {
    return m_players[i->GetPlayer().GetId()]->NumActions(i->GetNumber());
  }
}

int EFSupport::GetIndex(const gbtEfgAction &a) const
{
  if (a.GetInfoset()->Game() != m_efg)  return 0;
  int pl = a.GetInfoset()->GetPlayer().GetId();
  return m_players[pl]->Find(a);
}

bool EFSupport::Contains(const gbtEfgAction &a) const
{
  if (a.GetInfoset()->Game() != m_efg)   
    return false;

  int pl = a.GetInfoset()->GetPlayer().GetId();

  if (pl == 0) return true; // Chance

  int act = m_players[pl]->Find(a);
  if (act == 0) 
    return false;
  else
    return true;
}

bool EFSupport::Contains(int pl, int iset, int act) const
{
  return Contains(GetGame().GetPlayer(pl).GetInfoset(iset)->GetAction(act));
}

gbtEfgAction EFSupport::GetAction(int pl, int iset, int act) const
{
  if (pl == 0) {
    return m_efg->GetChance().GetInfoset(iset)->GetAction(act);
  }
  else {
    return m_players[pl]->GetAction(iset, act);
  }
}

gbtEfgAction EFSupport::GetAction(Infoset *infoset, int act) const
{
  if (infoset->GetPlayer().IsChance()) {
    return infoset->GetAction(act);
  }
  else {
    return m_players[infoset->GetPlayer().GetId()]->GetAction(infoset->GetNumber(), act);
  }
}

bool EFSupport::HasActiveActionAt(const Infoset *infoset) const
{
  if (!m_players[infoset->GetPlayer().GetId()]->
      HasActiveActionAt(infoset->GetNumber()))
    return false;

  return true;
}

int EFSupport::NumDegreesOfFreedom(void) const
{
  int answer(0);

  gList<Infoset *> active_infosets = ReachableInfosets(GetGame().RootNode());
  for (int i = 1; i <= active_infosets.Length(); i++)
    answer += NumActions(active_infosets[i]) - 1;

  return answer;  
}

bool EFSupport::HasActiveActionsAtAllInfosets(void) const
{
  if (m_players.Length() != m_efg->NumPlayers())   return false;
  for (int i = 1; i <= m_players.Length(); i++)
    if (!m_players[i]->HasActiveActionsAtAllInfosets())  return false;

  return true;
}

gPVector<int> EFSupport::NumActions(void) const
{
  gArray<int> foo(m_efg->NumPlayers());
  int i;
  for (i = 1; i <= m_efg->NumPlayers(); i++)
    foo[i] = m_players[i]->GetPlayer().NumInfosets();

  gPVector<int> bar(foo);
  for (i = 1; i <= m_efg->NumPlayers(); i++)
    for (int j = 1; j <= m_players[i]->GetPlayer().NumInfosets(); j++)
      bar(i, j) = NumActions(i,j);

  return bar;
}  

bool EFSupport::RemoveAction(const gbtEfgAction &s)
{
  Infoset *infoset = s.GetInfoset();
  gbtEfgPlayer player = infoset->GetPlayer();
 
  return m_players[player.GetId()]->RemoveAction(infoset->GetNumber(), s); 
}

void EFSupport::AddAction(const gbtEfgAction &s)
{
  Infoset *infoset = s.GetInfoset();
  gbtEfgPlayer player = infoset->GetPlayer();

  m_players[player.GetId()]->AddAction(infoset->GetNumber(), s);
}

int EFSupport::NumSequences(int j) const
{
  if (j < 1 || j > m_efg->NumPlayers()) return 1;
  gList<Infoset *> isets = ReachableInfosets(m_efg->GetPlayer(j));
  int num = 1;
  for(int i = 1; i <= isets.Length(); i++)
    num+=NumActions(isets[i]);
  return num;
}

int EFSupport::TotalNumSequences(void) const
{
  int total = 0;
  for (int i = 1 ; i <= m_efg->NumPlayers(); i++)
    total += NumSequences(i);
  return total;
}

gList<Node *> EFSupport::ReachableNonterminalNodes(const Node *n) const
{
  gList<Node *> answer;
  if (n->IsNonterminal()) {
    for (int i = 1; i <= NumActions(n->GetInfoset()); i++) {
      Node *nn = n->GetChild(GetAction(n->GetInfoset(), i));
      if (nn->IsNonterminal()) {
	answer += nn;
	answer += ReachableNonterminalNodes(nn);
      }
    }
  }
  return answer;
}

gList<Node *> EFSupport::ReachableNonterminalNodes(const Node *n,
						   const gbtEfgAction &a) const
{
  gList<Node *> answer;
  Node *nn = n->GetChild(a);
  if (nn->IsNonterminal()) {
    answer += nn;
    answer += ReachableNonterminalNodes(nn);
  }
  return answer;
}

gList<Infoset *> EFSupport::ReachableInfosets(const gbtEfgPlayer &p) const
{ 
  gList<Infoset *> answer;

  for (int i = 1; i <= p.NumInfosets(); i++) {
    if (MayReach(p.GetInfoset(i))) {
      answer += p.GetInfoset(i);
    }
  }
  return answer;
}

gList<Infoset *> EFSupport::ReachableInfosets(const Node *n) const
{
  gList<Infoset *> answer;
  gList<Node *> nodelist = ReachableNonterminalNodes(n);
  for (int i = 1; i <= nodelist.Length(); i++)
    answer += nodelist[i]->GetInfoset();
  answer.RemoveRedundancies();
  return answer;
}

gList<Infoset *> EFSupport::ReachableInfosets(const Node *n, 
					      const gbtEfgAction &a) const
{
  gList<Infoset *> answer;
  gList<Node *> nodelist = ReachableNonterminalNodes(n,a);
  for (int i = 1; i <= nodelist.Length(); i++)
    answer += nodelist[i]->GetInfoset();
  answer.RemoveRedundancies();
  return answer;
}

bool EFSupport::AlwaysReaches(const Infoset *i) const
{
  return AlwaysReachesFrom(i, m_efg->RootNode());
}

bool EFSupport::AlwaysReachesFrom(const Infoset *i, const Node *n) const
{
  if (n->IsTerminal()) return false;
  else {
    if (n->GetInfoset() == i) return true;
    else {
      for (int j = 1; j <= NumActions(n->GetInfoset()); j++) {
	if (!AlwaysReachesFrom(i, n->GetChild(GetAction(n->GetInfoset(), j)))){
	  return false;
	}
      }
    }
  }
  return true;
}

bool EFSupport::MayReach(const Infoset *infoset) const
{
  for (int j = 1; j <= infoset->NumMembers(); j++) {
    if (MayReach(infoset->GetMember(j))) {
      return true;
    }
  }
  return false;
}

bool EFSupport::MayReach(const Node *n) const
{
  if (n == m_efg->RootNode())
    return true;
  else {
    if (!Contains(n->GetAction())) {
      return false;
    }
    else {
      return MayReach(n->GetParent());
    }
  }
}


void EFSupport::Dump(gOutput &p_output) const
{
  p_output << '"' << m_name << "\" { ";
  for (int pl = 1; pl <= m_efg->NumPlayers(); pl++)  {
    gbtEfgPlayer player = m_players[pl]->GetPlayer();
    p_output << '"' << player.GetLabel() << "\" { ";
    for (int iset = 1; iset <= player.NumInfosets(); iset++)  {
      Infoset *infoset = player.GetInfoset(iset);
      p_output << '"' << infoset->GetName() << "\" { ";
      for (int act = 1; act <= NumActions(pl, iset); act++)  {
	gbtEfgAction action = m_players[pl]->GetAction(iset, act);
	p_output << action << ' ';
      }
      p_output << "} ";
    }
    p_output << "} ";
  }
  p_output << "} ";
}

gOutput& operator<<(gOutput&s, const EFSupport& e)
{
  e.Dump(s);
  return s;
}


//----------------------------------------------------
//                EFSupportWithActiveInfo
// ---------------------------------------------------

// Utilities 
bool EFSupportWithActiveInfo::infoset_has_active_nodes(const int pl,
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

bool EFSupportWithActiveInfo::infoset_has_active_nodes(const Infoset *i) const
{
  return infoset_has_active_nodes(i->GetPlayer().GetId(), i->GetNumber());
}

void EFSupportWithActiveInfo::activate(const Node *n)
{
  is_nonterminal_node_active[n->GetPlayer().GetId()]
                            [n->GetInfoset()->GetNumber()]
                            [n->NumberInInfoset()] = true;
}

void EFSupportWithActiveInfo::deactivate(const Node *n)
{
  is_nonterminal_node_active[n->GetPlayer().GetId()]
                            [n->GetInfoset()->GetNumber()]
                            [n->NumberInInfoset()] = false;
}

void EFSupportWithActiveInfo::activate(const Infoset * i)
{
  is_infoset_active[i->GetPlayer().GetId()][i->GetNumber()] = true;
}

void EFSupportWithActiveInfo::deactivate(const Infoset * i)
{
  is_infoset_active[i->GetPlayer().GetId()][i->GetNumber()] = false;
}

void EFSupportWithActiveInfo::activate_this_and_lower_nodes(const Node *n)
{
  if (n->IsNonterminal()) {
    activate(n); 
    activate(n->GetInfoset());
    for (int i = 1; i <= NumActions(n->GetInfoset()); i++) 
      activate_this_and_lower_nodes(n->GetChild(GetAction(n->GetInfoset(), i)));
  }
}

void EFSupportWithActiveInfo::deactivate_this_and_lower_nodes(const Node *n)
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

void EFSupportWithActiveInfo::
deactivate_this_and_lower_nodes_returning_deactivated_infosets(const Node *n, 
                                                gList<Infoset *> *list)
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

void EFSupportWithActiveInfo::InitializeActiveListsToAllActive()
{
  for (int pl = 0; pl <= GetGame().NumPlayers(); pl++) {
    gbtEfgPlayer player = (pl == 0) ? GetGame().GetChance() : GetGame().GetPlayer(pl); 
    gList<bool>         is_players_infoset_active;
    gList<gList<bool> > is_players_node_active;
    for (int iset = 1; iset <= player.NumInfosets(); iset++) {
      is_players_infoset_active += true;

      gList<bool> is_infosets_node_active;
      for (int n = 1; n <= player.GetInfoset(iset)->NumMembers(); n++)
	is_infosets_node_active += true;
      is_players_node_active += is_infosets_node_active;
    }
    is_infoset_active[pl] = is_players_infoset_active;
    is_nonterminal_node_active[pl] = is_players_node_active;
  }
}

void EFSupportWithActiveInfo::InitializeActiveListsToAllInactive()
{
  for (int pl = 0; pl <= GetGame().NumPlayers(); pl++) {
    gbtEfgPlayer player = (pl == 0) ? GetGame().GetChance() : GetGame().GetPlayer(pl);
    gList<bool>         is_players_infoset_active;
    gList<gList<bool> > is_players_node_active;

    for (int iset = 1; iset <= player.NumInfosets(); iset++) {
      is_players_infoset_active += false;

      gList<bool> is_infosets_node_active;
      for (int n = 1; n <= player.GetInfoset(iset)->NumMembers()
; n++)
	is_infosets_node_active += false;
      is_players_node_active += is_infosets_node_active;
    }
    is_infoset_active[pl] = is_players_infoset_active;
    is_nonterminal_node_active[pl] = is_players_node_active;
  }
}

void EFSupportWithActiveInfo::InitializeActiveLists()
{
  InitializeActiveListsToAllInactive();

  activate_this_and_lower_nodes(GetGame().RootNode());
}

// Constructors and Destructor
EFSupportWithActiveInfo::EFSupportWithActiveInfo(const efgGame &E) 
  : EFSupport(E), 
    is_infoset_active(0,E.NumPlayers()), 
    is_nonterminal_node_active(0,E.NumPlayers())
{
  InitializeActiveLists();
}

EFSupportWithActiveInfo::EFSupportWithActiveInfo(const EFSupport& given)
  : EFSupport(given), 
    is_infoset_active(0,given.GetGame().NumPlayers()), 
    is_nonterminal_node_active(0,given.GetGame().NumPlayers())
{
  InitializeActiveLists();
}

EFSupportWithActiveInfo::EFSupportWithActiveInfo(
				  const EFSupportWithActiveInfo& given)
  : EFSupport(given), 
    //is_infoset_active(0,given.GetGame().NumPlayers()), 
        is_infoset_active(is_infoset_active), 
    is_nonterminal_node_active(given.is_nonterminal_node_active)
{
  //  InitializeActiveLists();
}

EFSupportWithActiveInfo::~EFSupportWithActiveInfo()
{}

// Operators
EFSupportWithActiveInfo &
EFSupportWithActiveInfo::operator=(const EFSupportWithActiveInfo &s)
{
  if (this != &s) {
    ((EFSupport&) *this) = s;
    is_infoset_active = s.is_infoset_active;
    is_nonterminal_node_active = s.is_nonterminal_node_active;
  }
  return *this;
}

bool 
EFSupportWithActiveInfo::operator==(const EFSupportWithActiveInfo &s) const
{
  if ((EFSupport&) *this != (EFSupport&) s) {
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
EFSupportWithActiveInfo::operator!=(const EFSupportWithActiveInfo &s) const
{
  return !(*this == s);
}

// Member Function
const gList<const Node *> 
EFSupportWithActiveInfo::ReachableNodesInInfoset(const Infoset * i) const
{
  gList<const Node *> answer;
  int pl = i->GetPlayer().GetId();
  int iset = i->GetNumber();
  for (int j = 1; j <= i->NumMembers(); j++)
    if (is_nonterminal_node_active[pl][iset][j])
      answer += i->GetMember(j);
  return answer;
}

const gList<const Node *>
EFSupportWithActiveInfo::ReachableNonterminalNodes() const
{
  gList<const Node *> answer;
  for (int pl = 1; pl <= GetGame().NumPlayers(); pl++) {
    gbtEfgPlayer p = GetGame().GetPlayer(pl);
    for (int iset = 1; iset <= p.NumInfosets(); iset++) {
      answer += ReachableNodesInInfoset(p.GetInfoset(iset));
    }
  }
  return answer;
}

// Editing functions
void EFSupportWithActiveInfo::AddAction(const gbtEfgAction &s)
{
  EFSupport::AddAction(s);

  gList<const Node *> startlist(ReachableNodesInInfoset(s.GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++)
    activate_this_and_lower_nodes(startlist[i]);
}

bool EFSupportWithActiveInfo::RemoveAction(const gbtEfgAction &s)
{
  gList<const Node *> startlist(ReachableNodesInInfoset(s.GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++)
    deactivate_this_and_lower_nodes(startlist[i]->GetChild(s));

  // the following returns false if s was not active
  return EFSupport::RemoveAction(s);
}

bool 
EFSupportWithActiveInfo::
RemoveActionReturningDeletedInfosets(const gbtEfgAction &s,
				     gList<Infoset *> *list)
{
  gList<const Node *> startlist(ReachableNodesInInfoset(s.GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++)
    deactivate_this_and_lower_nodes_returning_deactivated_infosets(
                           startlist[i]->GetChild(s),list);

  // the following returns false if s was not active
  return EFSupport::RemoveAction(s);
}

int EFSupportWithActiveInfo::NumActiveNodes(int pl, int iset) const
{
  int answer = 0;
  for (int i = 1; i <= is_nonterminal_node_active[pl][iset].Length(); i++)
    if (is_nonterminal_node_active[pl][iset][i])
      answer++;
  return answer;
}

int EFSupportWithActiveInfo::NumActiveNodes(const Infoset *i) const
{
  return NumActiveNodes(i->GetPlayer().GetId(), i->GetNumber());
}

bool EFSupportWithActiveInfo::InfosetIsActive(const int pl,
					      const int iset) const
{
  return is_infoset_active[pl][iset];
}

bool EFSupportWithActiveInfo::InfosetIsActive(const Infoset *i) const
{
  return InfosetIsActive(i->GetPlayer().GetId(), i->GetNumber());
}

bool EFSupportWithActiveInfo::NodeIsActive(const int pl,
					   const int iset,
					   const int node) const
{
  return is_nonterminal_node_active[pl][iset][node];
}

bool EFSupportWithActiveInfo::NodeIsActive(const Node *n) const
{
  return NodeIsActive(n->GetInfoset()->GetPlayer().GetId(),
		      n->GetInfoset()->GetNumber(),
		      n->NumberInInfoset());
}

bool EFSupportWithActiveInfo::HasActiveActionsAtActiveInfosets()
{
  for (int pl = 1; pl <= GetGame().NumPlayers(); pl++)
    for (int iset = 1; iset <= GetGame().GetPlayer(pl).NumInfosets(); iset++) 
      if (InfosetIsActive(pl,iset))
        if ( NumActions(GetGame().GetPlayer(pl).GetInfoset(iset)) == 0 )
          return false;
  return true;
}

bool EFSupportWithActiveInfo::HasActiveActionsAtActiveInfosetsAndNoOthers()
{
  for (int pl = 1; pl <= GetGame().NumPlayers(); pl++)
    for (int iset = 1; iset <= GetGame().GetPlayer(pl).NumInfosets(); iset++) {
      if (InfosetIsActive(pl,iset))
        if ( NumActions(GetGame().GetPlayer(pl).GetInfoset(iset)) == 0 )
          return false;
      if (!InfosetIsActive(pl,iset))
        if ( NumActions(GetGame().GetPlayer(pl).GetInfoset(iset)) > 0 )
          return false;
      }
  return true;
}


void EFSupportWithActiveInfo::Dump(gOutput& s) const
{
  EFSupport::Dump(s);

  /*
  s << "\n";

  for (int pl = 0; pl <= GetGame().NumPlayers(); pl++) {
  
    if (pl == 0)
      s << " Chance:  ";
    else 
      s << "Player " << pl << ":";
    //    s << "(" << GetGame().Players()[pl]->NumInfosets() << ")";
    //    s << "\n";

    for (int iset = 1; iset <= GetGame().Players()[pl]->NumInfosets(); iset++) { 

      s << "  Infoset " << iset << " is ";
      if (InfosetIsActive(pl,iset))
	s << "Active  : ";
      else
	s << "inactive: ";
      
      s << "{";
      for (int n = 1; n <= GetGame().NumNodesInInfoset(pl,iset); n++) {
	if (NodeIsActive(pl,iset,n))
	  s << "+";
	else
	  s << "0";
	if (n < GetGame().NumNodesInInfoset(pl,iset))
	  s << ",";
      }
      s << "}";

      //      s << "\n";
    }
    s << "\n ";
  }
  //    s << "\n\n";
  */
}

gOutput& operator<<(gOutput&s, const EFSupportWithActiveInfo& e)
{
  e.Dump(s);
  return s;
}


// Instantiations
#include "base/glist.imp"

template class gList<EFSupport>;
template class gList<const EFSupport>;
template class gList<const EFSupportWithActiveInfo>;

#include "math/gvector.imp"
#include "math/gpvector.imp"
#include "math/gdpvect.imp"

template class gVector<bool>;
template class gPVector<bool>;
template class gDPVector<bool>;
