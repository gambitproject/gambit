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
#include "player.h"
#include "efstrat.h"

class gbtEfgActionArray   {
  friend class gbtEfgActionSet;
protected:
  gbtBlock<gbtEfgAction> acts;

public:
  gbtEfgActionArray ( const gbtArray<gbtEfgAction> &a);
  gbtEfgActionArray ( const gbtEfgActionArray &a);
  virtual ~gbtEfgActionArray();
  gbtEfgActionArray &operator=( const gbtEfgActionArray &a);
  bool operator==( const gbtEfgActionArray &a) const;
  const gbtEfgAction &operator[](int i) const { return acts[i]; }
  gbtEfgAction operator[](int i) { return acts[i]; }
  void Set(int i, const gbtEfgAction &action) { acts[i] = action; }

  // Information
  int Length() const { return acts.Length(); }
};

//----------------------------------------------------
// gbtEfgActionArray: Constructors, Destructor, operators
// ---------------------------------------------------

gbtEfgActionArray::gbtEfgActionArray(const gbtArray<gbtEfgAction> &a)
  : acts(a.Length())
{
  for (int i = 1; i <= acts.Length(); i++)
    acts[i] = a[i];
 }

gbtEfgActionArray::gbtEfgActionArray(const gbtEfgActionArray &a)
  : acts(a.acts)
{ }

gbtEfgActionArray::~gbtEfgActionArray ()
{ }

gbtEfgActionArray &gbtEfgActionArray::operator=( const gbtEfgActionArray &a)
{
  acts = a.acts; 
  return *this;
}

#ifdef __BORLANDC__
bool operator==(const gbtArray<gbtEfgAction> &a, const gbtArray<gbtEfgAction> &b)
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

bool gbtEfgActionArray::operator==(const gbtEfgActionArray &a) const
{
  return (acts == a.acts);
}

class gbtEfgActionSet  {
protected:
  gbtEfgPlayer efp;
  gbtArray < gbtEfgActionArray *> infosets;
public:
  
  //----------------------------------------
  // Constructors, Destructor, operators
  //----------------------------------------

//  gbtEfgActionSet();
  gbtEfgActionSet(const gbtEfgActionSet &);
  gbtEfgActionSet(const gbtEfgPlayer &);
  virtual ~gbtEfgActionSet();

  gbtEfgActionSet &operator=(const gbtEfgActionSet &);
  bool operator==(const gbtEfgActionSet &s) const;

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
  //  const gbtArray<Action *> &ActionList(int iset) const
  //   { return infosets[iset]->acts; }

  // Get the gbtEfgActionArray of an iset
  const gbtEfgActionArray *ActionArray(int iset) const
     { return infosets[iset]; }

  // Get the gbtEfgActionArray of an Infoset
  const gbtEfgActionArray *ActionArray(const gbtEfgInfoset &i) const
     { return infosets[i.GetId()]; }
  
  // Get an Action
  gbtEfgAction GetAction(int iset, int index);

  // returns the index of the action if it is in the ActionSet
  int Find(const gbtEfgAction &) const;

  // Number of Actions in a particular infoset
  int NumActions(int iset) const;

  // return the player of the gbtEfgActionSet
  gbtEfgPlayer GetPlayer(void) const;

  // checks for a valid gbtEfgActionSet
  bool HasActiveActionsAtAllInfosets(void) const;
  bool HasActiveActionAt(const int &iset) const;

};

//--------------------------------------------------
// gbtEfgActionSet: Constructors, Destructor, operators
//--------------------------------------------------

gbtEfgActionSet::gbtEfgActionSet(const gbtEfgPlayer &p)
  : efp(p), infosets(p->NumInfosets())
{
  for (int i = 1; i <= p->NumInfosets(); i++) {
    infosets[i] = new gbtEfgActionArray(p->GetInfoset(i).NumActions());
    for (int j = 1; j <= p->GetInfoset(i).NumActions(); j++) {
      infosets[i]->Set(j, p->GetInfoset(i).GetAction(j));
    }
  }
}

gbtEfgActionSet::gbtEfgActionSet( const gbtEfgActionSet &s )
: infosets(s.infosets.Length())
{
  efp = s.efp;
  for (int i = 1; i <= s.infosets.Length(); i++){
    infosets[i] = new gbtEfgActionArray(*(s.infosets[i]));
  }
}

gbtEfgActionSet::~gbtEfgActionSet()
{ 
  for (int i = 1; i <= infosets.Length(); i++)
    delete infosets[i];
}

gbtEfgActionSet &gbtEfgActionSet::operator=(const gbtEfgActionSet &s)
{
  if (this != &s && efp == s.efp) {
    for (int i = 1; i<= infosets.Length(); i++)  {
      delete infosets[i];
      infosets[i] = new gbtEfgActionArray(*(s.infosets[i]));
    }
  }    
  return *this;
}

bool gbtEfgActionSet::operator==(const gbtEfgActionSet &s) const
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
// gbtEfgActionSet: Member functions 
//------------------------------------------

// Append an action to a particular infoset;
void gbtEfgActionSet::AddAction(int iset, const gbtEfgAction &s)
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
void gbtEfgActionSet::AddAction(int iset, const gbtEfgAction &s, int index)
{ 
  if (!infosets[iset]->acts.Find(s)) {
    infosets[iset]->acts.Insert(s,index); 
  }
}

// Remove an action from infoset iset at int i, 
// returns the removed Infoset pointer
gbtEfgAction gbtEfgActionSet::RemoveAction(int iset, int i) 
{ 
  return (infosets[iset]->acts.Remove(i)); 
}

// Removes an action from infoset iset . Returns true if the 
//Action was successfully removed, false otherwise.
bool gbtEfgActionSet::RemoveAction(int iset, const gbtEfgAction &s)
{ 
  int t = infosets[iset]->acts.Find(s); 
  if (t>0) infosets[iset]->acts.Remove(t); 
  return (t>0); 
} 

// Get an action
gbtEfgAction gbtEfgActionSet::GetAction(int iset, int index)
{
  return (infosets[iset]->acts)[index];
}

// Number of Actions in a particular infoset
int gbtEfgActionSet::NumActions(int iset) const
{
  return (infosets[iset]->acts.Length());
}

// Return the player of this gbtEfgActionSet
gbtEfgPlayer gbtEfgActionSet::GetPlayer(void) const
{
  return efp;
}

int gbtEfgActionSet::Find(const gbtEfgAction &a) const
{
  return (infosets[a.GetInfoset().GetId()]->acts.Find(a));
}

// checks for a valid gbtEfgActionSet
bool gbtEfgActionSet::HasActiveActionsAtAllInfosets(void) const
{
  if (infosets.Length() != efp->NumInfosets())   return false;

  for (int i = 1; i <= infosets.Length(); i++)
    if (infosets[i]->acts.Length() == 0)   return false;

  return true;
}

// checks for a valid gbtEfgActionSet
bool gbtEfgActionSet::HasActiveActionAt(const int &iset) const
{
  if (iset > efp->NumInfosets())   return false;

  if (infosets[iset]->acts.Length() == 0)   return false;

  return true;
}


//--------------------------------------------------
// gbtEfgSupport: Constructors, Destructors, Operators
//--------------------------------------------------

gbtEfgSupport::gbtEfgSupport(const gbtEfgGame &p_efg) 
  : m_efg(p_efg), m_players(p_efg.NumPlayers())
{
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    m_players[pl] = new gbtEfgActionSet(p_efg.GetPlayer(pl));
  }
}

gbtEfgSupport::gbtEfgSupport(const gbtEfgSupport &p_support)
  : m_label(p_support.m_label), m_efg(p_support.m_efg),
    m_players(p_support.m_players.Length())
{
  for (int pl = 1; pl <= m_players.Length(); pl++)
    m_players[pl] = new gbtEfgActionSet(*(p_support.m_players[pl]));
}

gbtEfgSupport::~gbtEfgSupport()
{
  for (int pl = 1; pl <= m_players.Length(); pl++)
    delete m_players[pl];
}

gbtEfgSupport &gbtEfgSupport::operator=(const gbtEfgSupport &p_support)
{
  if (this != &p_support && m_efg == p_support.m_efg) {
    m_label = p_support.m_label;
    for (int pl = 1; pl <= m_players.Length(); pl++)  {
      delete m_players[pl];
      m_players[pl] = new gbtEfgActionSet(*(p_support.m_players[pl]));
    }
  }
  return *this;
}

bool gbtEfgSupport::operator==(const gbtEfgSupport &p_support) const
{
  if (m_players.Length() != p_support.m_players.Length())
    return false;

  int pl;
  for (pl = 1; (pl <= m_players.Length() &&
		*(m_players[pl]) == *(p_support.m_players[pl])); pl++);
  return (pl > m_players.Length());
}

bool gbtEfgSupport::operator!=(const gbtEfgSupport &p_support) const
{
  return !(*this == p_support);
}

//-----------------------------
// gbtEfgSupport: Member Functions 
//-----------------------------

int gbtEfgSupport::NumActions(int pl, int iset) const
{
  if (pl == 0) {
    return m_efg.GetChance()->GetInfoset(iset).NumActions();
  }
  else {
    return m_players[pl]->NumActions(iset);
  }
}

int gbtEfgSupport::NumActions(const gbtEfgInfoset &i) const
{
  if (i.GetPlayer()->IsChance()) {
    return i.NumActions();
  }
  else {
    return m_players[i.GetPlayer()->GetId()]->NumActions(i.GetId());
  }
}

int gbtEfgSupport::GetIndex(const gbtEfgAction &a) const
{
  if (a.GetInfoset().GetGame() != m_efg)  return 0;
  int pl = a.GetInfoset().GetPlayer()->GetId();
  return m_players[pl]->Find(a);
}

bool gbtEfgSupport::Contains(const gbtEfgAction &a) const
{
  if (a.GetInfoset().GetGame() != m_efg)   
    return false;

  int pl = a.GetInfoset().GetPlayer()->GetId();

  if (pl == 0) return true; // Chance

  int act = m_players[pl]->Find(a);
  if (act == 0) 
    return false;
  else
    return true;
}

bool gbtEfgSupport::Contains(int pl, int iset, int act) const
{
  return Contains(GetGame().GetPlayer(pl)->GetInfoset(iset).GetAction(act));
}

gbtEfgAction gbtEfgSupport::GetAction(int pl, int iset, int act) const
{
  if (pl == 0) {
    return m_efg.GetChance()->GetInfoset(iset).GetAction(act);
  }
  else {
    return m_players[pl]->GetAction(iset, act);
  }
}

gbtEfgAction gbtEfgSupport::GetAction(const gbtEfgInfoset &infoset, int act) const
{
  if (infoset.GetPlayer()->IsChance()) {
    return infoset.GetAction(act);
  }
  else {
    return m_players[infoset.GetPlayer()->GetId()]->GetAction(infoset.GetId(), act);
  }
}

bool gbtEfgSupport::HasActiveActionAt(const gbtEfgInfoset &infoset) const
{
  if (!m_players[infoset.GetPlayer()->GetId()]->
      HasActiveActionAt(infoset.GetId()))
    return false;

  return true;
}

int gbtEfgSupport::NumDegreesOfFreedom(void) const
{
  int answer(0);

  gbtList<gbtEfgInfoset> active_infosets = ReachableInfosets(GetGame().GetRoot());
  for (int i = 1; i <= active_infosets.Length(); i++)
    answer += NumActions(active_infosets[i]) - 1;

  return answer;  
}

bool gbtEfgSupport::HasActiveActionsAtAllInfosets(void) const
{
  if (m_players.Length() != m_efg.NumPlayers())   return false;
  for (int i = 1; i <= m_players.Length(); i++)
    if (!m_players[i]->HasActiveActionsAtAllInfosets())  return false;

  return true;
}

gbtPVector<int> gbtEfgSupport::NumActions(void) const
{
  gbtArray<int> foo(m_efg.NumPlayers());
  int i;
  for (i = 1; i <= m_efg.NumPlayers(); i++) {
    foo[i] = m_players[i]->GetPlayer()->NumInfosets();
  }

  gbtPVector<int> bar(foo);
  for (i = 1; i <= m_efg.NumPlayers(); i++)
    for (int j = 1; j <= m_players[i]->GetPlayer()->NumInfosets(); j++)
      bar(i, j) = NumActions(i,j);

  return bar;
}  

bool gbtEfgSupport::RemoveAction(const gbtEfgAction &s)
{
  gbtEfgInfoset infoset = s.GetInfoset();
  gbtEfgPlayer player = infoset.GetPlayer();
 
  return m_players[player->GetId()]->RemoveAction(infoset.GetId(), s); 
}

void gbtEfgSupport::AddAction(const gbtEfgAction &s)
{
  gbtEfgInfoset infoset = s.GetInfoset();
  gbtEfgPlayer player = infoset.GetPlayer();

  m_players[player->GetId()]->AddAction(infoset.GetId(), s);
}

int gbtEfgSupport::NumSequences(int j) const
{
  if (j < 1 || j > m_efg.NumPlayers()) return 1;
  gbtList<gbtEfgInfoset> isets = ReachableInfosets(m_efg.GetPlayer(j));
  int num = 1;
  for(int i = 1; i <= isets.Length(); i++)
    num+=NumActions(isets[i]);
  return num;
}

int gbtEfgSupport::TotalNumSequences(void) const
{
  int total = 0;
  for (int i = 1 ; i <= m_efg.NumPlayers(); i++)
    total += NumSequences(i);
  return total;
}

gbtList<gbtEfgNode> 
gbtEfgSupport::ReachableNonterminalNodes(const gbtEfgNode &n) const
{
  gbtList<gbtEfgNode> answer;
  if (n.IsNonterminal()) {
    for (int i = 1; i <= NumActions(n.GetInfoset()); i++) {
      gbtEfgNode nn = n.GetChild(GetAction(n.GetInfoset(), i));
      if (nn.IsNonterminal()) {
	answer += nn;
	answer += ReachableNonterminalNodes(nn);
      }
    }
  }
  return answer;
}

gbtList<gbtEfgNode> 
gbtEfgSupport::ReachableNonterminalNodes(const gbtEfgNode &n,
				     const gbtEfgAction &a) const
{
  gbtList<gbtEfgNode> answer;
  gbtEfgNode nn = n.GetChild(a);
  if (nn.IsNonterminal()) {
    answer += nn;
    answer += ReachableNonterminalNodes(nn);
  }
  return answer;
}

gbtList<gbtEfgInfoset> gbtEfgSupport::ReachableInfosets(const gbtEfgPlayer &p) const
{ 
  gbtList<gbtEfgInfoset> answer;

  for (int i = 1; i <= p->NumInfosets(); i++) {
    if (MayReach(p->GetInfoset(i))) {
      answer += p->GetInfoset(i);
    }
  }
  return answer;
}

gbtList<gbtEfgInfoset> gbtEfgSupport::ReachableInfosets(const gbtEfgNode &n) const
{
  gbtList<gbtEfgInfoset> answer;
  gbtList<gbtEfgNode> nodelist = ReachableNonterminalNodes(n);
  for (int i = 1; i <= nodelist.Length(); i++)
    answer += nodelist[i].GetInfoset();
  answer.RemoveRedundancies();
  return answer;
}

gbtList<gbtEfgInfoset> gbtEfgSupport::ReachableInfosets(const gbtEfgNode &n, 
						  const gbtEfgAction &a) const
{
  gbtList<gbtEfgInfoset> answer;
  gbtList<gbtEfgNode> nodelist = ReachableNonterminalNodes(n,a);
  for (int i = 1; i <= nodelist.Length(); i++)
    answer += nodelist[i].GetInfoset();
  answer.RemoveRedundancies();
  return answer;
}

bool gbtEfgSupport::AlwaysReaches(const gbtEfgInfoset &i) const
{
  return AlwaysReachesFrom(i, m_efg.GetRoot());
}

bool gbtEfgSupport::AlwaysReachesFrom(const gbtEfgInfoset &i, 
				  const gbtEfgNode &n) const
{
  if (n.IsTerminal()) return false;
  else {
    if (n.GetInfoset() == i) return true;
    else {
      for (int j = 1; j <= NumActions(n.GetInfoset()); j++) {
	if (!AlwaysReachesFrom(i, n.GetChild(GetAction(n.GetInfoset(), j)))) {
	  return false;
	}
      }
    }
  }
  return true;
}

bool gbtEfgSupport::MayReach(const gbtEfgInfoset &infoset) const
{
  for (int j = 1; j <= infoset.NumMembers(); j++) {
    if (MayReach(infoset.GetMember(j))) {
      return true;
    }
  }
  return false;
}

bool gbtEfgSupport::MayReach(const gbtEfgNode &n) const
{
  if (n == m_efg.GetRoot())
    return true;
  else {
    if (!Contains(n.GetPriorAction())) {
      return false;
    }
    else {
      return MayReach(n.GetParent());
    }
  }
}


void gbtEfgSupport::Dump(gbtOutput &p_output) const
{
  p_output << '"' << m_label << "\" { ";
  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++)  {
    gbtEfgPlayer player = m_players[pl]->GetPlayer();
    p_output << '"' << player->GetLabel() << "\" { ";
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      gbtEfgInfoset infoset = player->GetInfoset(iset);
      p_output << '"' << infoset.GetLabel() << "\" { ";
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

gbtOutput& operator<<(gbtOutput&s, const gbtEfgSupport& e)
{
  e.Dump(s);
  return s;
}


//----------------------------------------------------
//                gbtEfgSupportWithActiveInfo
// ---------------------------------------------------

// Utilities 
bool gbtEfgSupportWithActiveInfo::infoset_has_active_nodes(const int pl,
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
gbtEfgSupportWithActiveInfo::infoset_has_active_nodes(const gbtEfgInfoset &i) const
{
  return infoset_has_active_nodes(i.GetPlayer()->GetId(), i.GetId());
}

void gbtEfgSupportWithActiveInfo::activate(const gbtEfgNode &n)
{
  is_nonterminal_node_active[n.GetPlayer()->GetId()]
                            [n.GetInfoset().GetId()]
                            [n.GetMemberId()] = true;
}

void gbtEfgSupportWithActiveInfo::deactivate(const gbtEfgNode &n)
{
  is_nonterminal_node_active[n.GetPlayer()->GetId()]
                            [n.GetInfoset().GetId()]
                            [n.GetMemberId()] = false;
}

void gbtEfgSupportWithActiveInfo::activate(const gbtEfgInfoset &i)
{
  is_infoset_active[i.GetPlayer()->GetId()][i.GetId()] = true;
}

void gbtEfgSupportWithActiveInfo::deactivate(const gbtEfgInfoset &i)
{
  is_infoset_active[i.GetPlayer()->GetId()][i.GetId()] = false;
}

void
gbtEfgSupportWithActiveInfo::activate_this_and_lower_nodes(const gbtEfgNode &n)
{
  if (n.IsNonterminal()) {
    activate(n); 
    activate(n.GetInfoset());
    for (int i = 1; i <= NumActions(n.GetInfoset()); i++) 
      activate_this_and_lower_nodes(n.GetChild(GetAction(n.GetInfoset(), i)));
  }
}

void
gbtEfgSupportWithActiveInfo::deactivate_this_and_lower_nodes(const gbtEfgNode &n)
{
  if (n.IsNonterminal()) {  // THIS ALL LOOKS FISHY
    deactivate(n); 
    if ( !infoset_has_active_nodes(n.GetInfoset()) )
      deactivate(n.GetInfoset());
    for (int i = 1; i <= NumActions(n.GetInfoset()); i++) {
      deactivate_this_and_lower_nodes(n.GetChild(GetAction(n.GetInfoset(), i)));
    }
  }
}

void gbtEfgSupportWithActiveInfo::
deactivate_this_and_lower_nodes_returning_deactivated_infosets(const gbtEfgNode &n, 
                                                gbtList<gbtEfgInfoset> *list)
{
  if (n.IsNonterminal()) {
    deactivate(n); 
    if ( !infoset_has_active_nodes(n.GetInfoset()) ) {

      //DEBUG
      /*
      gout << "We are deactivating infoset " << n->GetInfoset()->GetNumber()
	   << " with support \n" << *this << "\n";
      */

      (*list) += n.GetInfoset(); 
      deactivate(n.GetInfoset());
    }
    for (int i = 1; i <= NumActions(n.GetInfoset()); i++) 
      deactivate_this_and_lower_nodes_returning_deactivated_infosets(n.GetChild(GetAction(n.GetInfoset(), i)), list);    
  }
}

void gbtEfgSupportWithActiveInfo::InitializeActiveListsToAllActive()
{
  for (int pl = 0; pl <= GetGame().NumPlayers(); pl++) {
    gbtEfgPlayer player = (pl == 0) ? GetGame().GetChance() : GetGame().GetPlayer(pl); 
    gbtList<bool>         is_players_infoset_active;
    gbtList<gbtList<bool> > is_players_node_active;
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      is_players_infoset_active += true;

      gbtList<bool> is_infosets_node_active;
      for (int n = 1; n <= player->GetInfoset(iset).NumMembers(); n++)
	is_infosets_node_active += true;
      is_players_node_active += is_infosets_node_active;
    }
    is_infoset_active[pl] = is_players_infoset_active;
    is_nonterminal_node_active[pl] = is_players_node_active;
  }
}

void gbtEfgSupportWithActiveInfo::InitializeActiveListsToAllInactive()
{
  for (int pl = 0; pl <= GetGame().NumPlayers(); pl++) {
    gbtEfgPlayer player = (pl == 0) ? GetGame().GetChance() : GetGame().GetPlayer(pl);
    gbtList<bool>         is_players_infoset_active;
    gbtList<gbtList<bool> > is_players_node_active;

    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      is_players_infoset_active += false;

      gbtList<bool> is_infosets_node_active;
      for (int n = 1; n <= player->GetInfoset(iset).NumMembers()
; n++)
	is_infosets_node_active += false;
      is_players_node_active += is_infosets_node_active;
    }
    is_infoset_active[pl] = is_players_infoset_active;
    is_nonterminal_node_active[pl] = is_players_node_active;
  }
}

void gbtEfgSupportWithActiveInfo::InitializeActiveLists()
{
  InitializeActiveListsToAllInactive();

  activate_this_and_lower_nodes(GetGame().GetRoot());
}

// Constructors and Destructor
gbtEfgSupportWithActiveInfo::gbtEfgSupportWithActiveInfo(const gbtEfgGame &p_efg)
  : gbtEfgSupport(p_efg), 
    is_infoset_active(0, p_efg.NumPlayers()), 
    is_nonterminal_node_active(0, p_efg.NumPlayers())
{
  InitializeActiveLists();
}

gbtEfgSupportWithActiveInfo::gbtEfgSupportWithActiveInfo(const gbtEfgSupport& given)
  : gbtEfgSupport(given), 
    is_infoset_active(0,given.GetGame().NumPlayers()), 
    is_nonterminal_node_active(0,given.GetGame().NumPlayers())
{
  InitializeActiveLists();
}

gbtEfgSupportWithActiveInfo::gbtEfgSupportWithActiveInfo(
				  const gbtEfgSupportWithActiveInfo& given)
  : gbtEfgSupport(given), 
    //is_infoset_active(0,given.GetGame().NumPlayers()), 
        is_infoset_active(is_infoset_active), 
    is_nonterminal_node_active(given.is_nonterminal_node_active)
{
  //  InitializeActiveLists();
}

gbtEfgSupportWithActiveInfo::~gbtEfgSupportWithActiveInfo()
{}

// Operators
gbtEfgSupportWithActiveInfo &
gbtEfgSupportWithActiveInfo::operator=(const gbtEfgSupportWithActiveInfo &s)
{
  if (this != &s) {
    ((gbtEfgSupport&) *this) = s;
    is_infoset_active = s.is_infoset_active;
    is_nonterminal_node_active = s.is_nonterminal_node_active;
  }
  return *this;
}

bool 
gbtEfgSupportWithActiveInfo::operator==(const gbtEfgSupportWithActiveInfo &s) const
{
  if ((gbtEfgSupport&) *this != (gbtEfgSupport&) s) {
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
gbtEfgSupportWithActiveInfo::operator!=(const gbtEfgSupportWithActiveInfo &s) const
{
  return !(*this == s);
}

// Member Function
gbtList<gbtEfgNode> 
gbtEfgSupportWithActiveInfo::ReachableNodesInInfoset(const gbtEfgInfoset &i) const
{
  gbtList<gbtEfgNode> answer;
  int pl = i.GetPlayer()->GetId();
  int iset = i.GetId();
  for (int j = 1; j <= i.NumMembers(); j++)
    if (is_nonterminal_node_active[pl][iset][j])
      answer += i.GetMember(j);
  return answer;
}

gbtList<gbtEfgNode>
gbtEfgSupportWithActiveInfo::ReachableNonterminalNodes() const
{
  gbtList<gbtEfgNode> answer;
  for (int pl = 1; pl <= GetGame().NumPlayers(); pl++) {
    gbtEfgPlayer p = GetGame().GetPlayer(pl);
    for (int iset = 1; iset <= p->NumInfosets(); iset++) {
      answer += ReachableNodesInInfoset(p->GetInfoset(iset));
    }
  }
  return answer;
}

// Editing functions
void gbtEfgSupportWithActiveInfo::AddAction(const gbtEfgAction &s)
{
  gbtEfgSupport::AddAction(s);

  gbtList<gbtEfgNode> startlist(ReachableNodesInInfoset(s.GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++)
    activate_this_and_lower_nodes(startlist[i]);
}

bool gbtEfgSupportWithActiveInfo::RemoveAction(const gbtEfgAction &s)
{
  gbtList<gbtEfgNode> startlist(ReachableNodesInInfoset(s.GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++)
    deactivate_this_and_lower_nodes(startlist[i].GetChild(s));

  // the following returns false if s was not active
  return gbtEfgSupport::RemoveAction(s);
}

bool 
gbtEfgSupportWithActiveInfo::
RemoveActionReturningDeletedInfosets(const gbtEfgAction &s,
				     gbtList<gbtEfgInfoset> *list)
{
  gbtList<gbtEfgNode> startlist(ReachableNodesInInfoset(s.GetInfoset()));
  for (int i = 1; i <= startlist.Length(); i++)
    deactivate_this_and_lower_nodes_returning_deactivated_infosets(
                           startlist[i].GetChild(s),list);

  // the following returns false if s was not active
  return gbtEfgSupport::RemoveAction(s);
}

int gbtEfgSupportWithActiveInfo::NumActiveNodes(int pl, int iset) const
{
  int answer = 0;
  for (int i = 1; i <= is_nonterminal_node_active[pl][iset].Length(); i++)
    if (is_nonterminal_node_active[pl][iset][i])
      answer++;
  return answer;
}

int gbtEfgSupportWithActiveInfo::NumActiveNodes(const gbtEfgInfoset &i) const
{
  return NumActiveNodes(i.GetPlayer()->GetId(), i.GetId());
}

bool gbtEfgSupportWithActiveInfo::InfosetIsActive(const int pl,
					      const int iset) const
{
  return is_infoset_active[pl][iset];
}

bool gbtEfgSupportWithActiveInfo::InfosetIsActive(const gbtEfgInfoset &i) const
{
  return InfosetIsActive(i.GetPlayer()->GetId(), i.GetId());
}

bool gbtEfgSupportWithActiveInfo::NodeIsActive(const int pl,
					   const int iset,
					   const int node) const
{
  return is_nonterminal_node_active[pl][iset][node];
}

bool gbtEfgSupportWithActiveInfo::NodeIsActive(const gbtEfgNode &n) const
{
  return NodeIsActive(n.GetInfoset().GetPlayer()->GetId(),
		      n.GetInfoset().GetId(),
		      n.GetMemberId());
}

bool gbtEfgSupportWithActiveInfo::HasActiveActionsAtActiveInfosets()
{
  for (int pl = 1; pl <= GetGame().NumPlayers(); pl++)
    for (int iset = 1; iset <= GetGame().GetPlayer(pl)->NumInfosets(); iset++) 
      if (InfosetIsActive(pl,iset))
        if ( NumActions(GetGame().GetPlayer(pl)->GetInfoset(iset)) == 0 )
          return false;
  return true;
}

bool gbtEfgSupportWithActiveInfo::HasActiveActionsAtActiveInfosetsAndNoOthers()
{
  for (int pl = 1; pl <= GetGame().NumPlayers(); pl++)
    for (int iset = 1; iset <= GetGame().GetPlayer(pl)->NumInfosets(); iset++) {
      if (InfosetIsActive(pl,iset))
        if ( NumActions(GetGame().GetPlayer(pl)->GetInfoset(iset)) == 0 )
          return false;
      if (!InfosetIsActive(pl,iset))
        if ( NumActions(GetGame().GetPlayer(pl)->GetInfoset(iset)) > 0 )
          return false;
      }
  return true;
}


void gbtEfgSupportWithActiveInfo::Dump(gbtOutput& s) const
{
  gbtEfgSupport::Dump(s);

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

gbtOutput& operator<<(gbtOutput&s, const gbtEfgSupportWithActiveInfo& e)
{
  e.Dump(s);
  return s;
}


// Instantiations
#include "base/glist.imp"

template class gbtList<gbtEfgSupport>;
template class gbtList<const gbtEfgSupport>;
template class gbtList<const gbtEfgSupportWithActiveInfo>;

#include "math/gvector.imp"
#include "math/gpvector.imp"
#include "math/gdpvect.imp"

template class gbtVector<bool>;
template class gbtPVector<bool>;
template class gbtDPVector<bool>;
