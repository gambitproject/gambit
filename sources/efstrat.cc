//
// FILE: efstrat.cc -- Implementation of supports for the extensive form
//
// $Id$
//

#include "efg.h"
#include "efplayer.h"

#include "glist.imp"

template class gList<const Infoset *>;

class EFActionArray   {
  friend class EFActionSet;
protected:
  gBlock<Action *> acts;

public:
  EFActionArray ( const gArray <Action *> &a);
  EFActionArray ( const EFActionArray &a);
  virtual ~EFActionArray();
  EFActionArray &operator=( const EFActionArray &a);
  bool operator==( const EFActionArray &a) const;
  inline const Action *operator[](const int &i) const { return acts[i]; }

  // Information
  inline const int Length() const { return acts.Length(); }
};

//----------------------------------------------------
// EFActionArray: Constructors, Destructor, operators
// ---------------------------------------------------

EFActionArray::EFActionArray(const gArray<Action *> &a)
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
  if (a.mindex != b.mindex || a.maxdex != b.maxdex)   return false;
  for (int i = a.mindex; i <= a.maxdex; i++)
    if (a[i] != b[i])   return false;
  return true;
}
#endif

bool EFActionArray::operator==(const EFActionArray &a) const
{
  return (acts == a.acts);
}

class EFActionSet{

protected:
  EFPlayer *efp;
  gArray < EFActionArray *> infosets;
public:
  
  //----------------------------------------
  // Constructors, Destructor, operators
  //----------------------------------------

//  EFActionSet();
  EFActionSet(const EFActionSet &);
  EFActionSet(EFPlayer &);
  virtual ~EFActionSet();

  EFActionSet &operator=(const EFActionSet &);
  bool operator==(const EFActionSet &s) const;

  //--------------------
  // Member Functions
  //--------------------

  // Append an action to an infoset;
  void AddAction(int iset, Action *);

  // Insert an action in a particular place in an infoset;
  void AddAction(int iset, Action *, int index);


  // Remove an action at int i, returns the removed action pointer
  const Action *RemoveAction(int iset, int i);

  // Remove an action from an infoset . 
  // Returns true if the action was successfully removed, false otherwise.
  bool RemoveAction(int iset, Action *);

  // Get a garray of the actions in an Infoset
  const gArray<Action *> &ActionList(int iset) const
     { return infosets[iset]->acts; }

  // Get the EFActionArray of an iset
  const EFActionArray *ActionArray(int iset) const
     { return infosets[iset]; }

  // Get the EFActionArray of an Infoset
  const EFActionArray *ActionArray(const Infoset *i) const
     { return infosets[i->GetNumber()]; }
  
  // Get an Action
  const Action *GetAction(int iset, int index);

  // returns the index of the action if it is in the ActionSet
  int Find(Action *) const;
  int Find(int, Action *) const;

  // Number of Actions in a particular infoset
  int NumActions(int iset) const;

  // return the EFPlayer of the EFActionSet
  EFPlayer &GetPlayer(void) const;

  // checks for a valid EFActionSet
  bool IsValid(void) const;

};

//--------------------------------------------------
// EFActionSet: Constructors, Destructor, operators
//--------------------------------------------------

EFActionSet::EFActionSet(EFPlayer &p)
  : infosets(p.NumInfosets())
{
  efp = &p;
  for (int i = 1; i <= p.NumInfosets(); i++){
    infosets[i] = new EFActionArray(p.Infosets()[i]->Actions());
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
void EFActionSet::AddAction(int iset, Action *s)
{ 
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
void EFActionSet::AddAction(int iset, Action *s, int index)
{ 
  infosets[iset]->acts.Insert(s,index); 
}

// Remove an action from infoset iset at int i, 
// returns the removed Infoset pointer
const Action* EFActionSet::RemoveAction(int iset, int i) 
{ 
  return (infosets[iset]->acts.Remove(i)); 
}

// Removes an action from infoset iset . Returns true if the 
//Action was successfully removed, false otherwise.
bool EFActionSet::RemoveAction(int  iset, Action *s )
{ 
  int t = infosets[iset]->acts.Find(s); 
  if (t>0) infosets[iset]->acts.Remove(t); 
  return (t>0); 
} 

// Get an action
const Action *EFActionSet::GetAction(int iset, int index)
{
  return (infosets[iset]->acts)[index];
}

// Number of Actions in a particular infoset
int EFActionSet::NumActions(int iset) const
{
  return (infosets[iset]->acts.Length());
}

// Return the EFPlayer of this EFActionSet
EFPlayer &EFActionSet::GetPlayer(void) const
{
  return (*efp);
}

int EFActionSet::Find(Action *a) const
{
  return (infosets[a->BelongsTo()->GetNumber()]->acts.Find(a));
}

int EFActionSet::Find(int p_infoset, Action *a) const
{
  return (infosets[p_infoset]->acts.Find(a));
}

// checks for a valid EFActionSet
bool EFActionSet::IsValid(void) const
{
  if (infosets.Length() != efp->NumInfosets())   return false;

  for (int i = 1; i <= infosets.Length(); i++)
    if (infosets[i]->acts.Length() == 0)   return false;

  return true;
}

//--------------------------------------------------
// EFSupport: Constructors, Destructors, Operators
//--------------------------------------------------

EFSupport::EFSupport(const Efg &E) 
: befg(&E), sets(E.NumPlayers())
{
  for (int i = 1; i <= sets.Length(); i++)
    sets[i] = new EFActionSet(*(E.Players()[i]));
}

EFSupport::EFSupport(const EFSupport &s)
  : name(s.name), befg(s.befg), 
    sets(s.sets.Length())
{
  for (int i = 1; i <= sets.Length(); i++)
    sets[i] = new EFActionSet(*(s.sets[i]));
}

EFSupport::~EFSupport()
{
  for (int i = 1; i <= sets.Length(); i++)
    delete sets[i];
}

EFSupport &EFSupport::operator=(const EFSupport &s)
{
  if (this != &s && befg == s.befg) {
    name = s.name;
    for (int i = 1; i <= sets.Length(); i++)  {
      delete sets[i];
      sets[i] = new EFActionSet(*(s.sets[i]));
    }
  }
  return *this;
}

bool EFSupport::operator==(const EFSupport &s) const
{
  if (sets.Length() != s.sets.Length())
    return false;

  int i;
  for (i = 1; i <= sets.Length() && *(sets[i]) == *(s.sets[i]); i++);
  return (i > sets.Length());
}

bool EFSupport::operator!=(const EFSupport &s) const
{
  return !(*this == s);
}

//-----------------------------
// EFSupport: Member Functions 
//-----------------------------

int EFSupport::NumActions(int pl, int iset) const
{
  return sets[pl]->NumActions(iset);
}

int EFSupport::NumActions(const Infoset &i) const
{
  return sets[i.GetPlayer()->GetNumber()]->NumActions(i.GetNumber());
}

int EFSupport::NumActions(const Infoset *i) const
{
  return sets[i->GetPlayer()->GetNumber()]->NumActions(i->GetNumber());
}

const gArray<Action *> &EFSupport::Actions(int pl, int iset) const
{
  return sets[pl]->ActionList(iset);
}

const gArray<Action *> &EFSupport::Actions(const Infoset &i) const
{
  if (i.GetPlayer()->IsChance())
    return i.Actions();
  else
    return sets[i.GetPlayer()->GetNumber()]->ActionList(i.GetNumber());
}

const gArray<Action *> &EFSupport::Actions(const Infoset *i) const
{
  /*
  //DEBUG
  gout << "In Actions, i->GetNumber() = " << i->GetNumber() << ".\n";
  gout << "In Actions, i->GetPlayer() = " << i->GetPlayer() << ".\n";
  gout << "In Actions, i->GetPlayer()->GetNumber() = " << i->GetPlayer()->GetNumber() << ".\n";

  if (i->GetPlayer()->IsChance()) {
    gout << "Got in 1.\n";
    return i->Actions();
    gout << "Got out 1.\n";
  }
  else {
    gout << "Got in 2.\n";
    return sets[i->GetPlayer()->GetNumber()]->ActionList(i->GetNumber());
    gout << "Got out 2.\n";
  }
  */

  if (i->GetPlayer()->IsChance())
    return i->Actions();
  else
    return sets[i->GetPlayer()->GetNumber()]->ActionList(i->GetNumber());
}

gList<Action *> EFSupport::ListOfActions(const Infoset *i) const
{
  gArray<Action *> actions = Actions(i);
  gList<Action *> answer;
  for (int i = 1; i <= actions.Length(); i++)
    answer += actions[i];
  return answer;
}

const EFActionArray* EFSupport::ActionArray(const Infoset *i) const
{
  return sets[i->GetPlayer()->GetNumber()]->ActionArray(i);
}

const Efg &EFSupport::Game(void) const
{
  return *befg;
}

const Node *EFSupport::RootNode(void) const
{
  return befg->RootNode();
}

int EFSupport::Find(Action *a) const
{
  if (a->BelongsTo()->Game() != befg)  assert(0);

  int pl = a->BelongsTo()->GetPlayer()->GetNumber();

  return sets[pl]->Find(a);
}

int EFSupport::Find(int p_player, int p_infoset, Action *p_action) const
{
  return sets[p_player]->Find(p_infoset, p_action);
}

bool EFSupport::ActionIsActive(Action *a) const
{
  //DEBUG
  //  if (a == NULL) { gout << "Action* is null.\n"; exit(0); }

  if (a->BelongsTo()->Game() != befg)   
    return false;

  int pl = a->BelongsTo()->GetPlayer()->GetNumber();

  if (pl == 0) return true; // Chance

  int act = sets[pl]->Find(a);
  if (act == 0) 
    return false;
  else
    return true;
}

bool EFSupport::ActionIsActive(const int pl,
			       const int iset, 
			       const int act) const
{
  return 
    ActionIsActive(Game().GetPlayer(pl)->GetInfoset(iset)->GetAction(act));
}

bool 
EFSupport::AllActionsInSupportAtInfosetAreActive(const EFSupport &S,
						 const Infoset *infset) const
{
  gArray<Action *> support_actions = S.Actions(infset);
  for (int i = 1; i <= support_actions.Length(); i++) {
    if (!ActionIsActive(support_actions[i]))
      return false;
  }
  return true;
}

bool EFSupport::IsValid(void) const
{
  if (sets.Length() != befg->NumPlayers())   return false;
  for (int i = 1; i <= sets.Length(); i++)
    if (!sets[i]->IsValid())  return false;

  return true;
}

gPVector<int> EFSupport::NumActions(void) const
{
  gArray<int> foo(befg->NumPlayers());
  int i;
  for (i = 1; i <= befg->NumPlayers(); i++)
    foo[i] = sets[i]->GetPlayer().NumInfosets();

  gPVector<int> bar(foo);
  for (i = 1; i <= befg->NumPlayers(); i++)
    for (int j = 1; j <= sets[i]->GetPlayer().NumInfosets(); j++)
      bar(i, j) = NumActions(i,j);

  return bar;
}  

bool EFSupport::RemoveAction(const Action *s)
{
  Infoset *infoset = s->BelongsTo();
  EFPlayer *player = infoset->GetPlayer();
 
  return sets[player->GetNumber()]->RemoveAction(infoset->GetNumber(), 
						 (Action *)s);
}

void EFSupport::AddAction(const Action *s)
{
  Infoset *infoset = s->BelongsTo();
  EFPlayer *player = infoset->GetPlayer();

  sets[player->GetNumber()]->AddAction(infoset->GetNumber(), 
				       (Action *)s);
}

int EFSupport::NumSequences(int j) const
{
  if (j < befg->Players().First() || j > befg->Players().Last()) return 1;
  gList<const Infoset *> isets;
  isets = ReachableInfosets(befg->GetPlayer(j));
  int num = 1;
  for(int i = 1;i<= isets.Length();i++)
    num+=NumActions(isets[i]);
  return num;
}

int EFSupport::TotalNumSequences() const
{
  int total = 0;
  for (int i = 1 ; i <= befg->NumPlayers(); i++)
    total += NumSequences(i);
  return total;
}

const gList<const Node *> 
EFSupport::ReachableNonterminalNodes(const Node *n) const
{
  gList<const Node *> answer;
  if (n->IsNonterminal()) {
    const EFActionArray *actions = ActionArray(n->GetInfoset());
    for (int i = 1; i <= actions->Length(); i++) {
      const Node *nn = n->GetChild((*actions)[i]);
      if (nn->IsNonterminal()) {
	answer += nn;
	answer += ReachableNonterminalNodes(nn);
      }
    }
  }
  return answer;
}

const gList<const Node *> 
EFSupport::ReachableNonterminalNodes(const Node *n, Action *a) const
{
  gList<const Node *> answer;
  const Node *nn = n->GetChild(a);
  if (nn->IsNonterminal()) {
    answer += nn;
    answer += ReachableNonterminalNodes(nn);
  }
  return answer;
}

const gList<const Infoset *> EFSupport::ReachableInfosets(const EFPlayer *p) const
{ 
  gArray<Infoset *> isets;
  gList<const Infoset *> answer;

  isets = p->Infosets();
  for (int i = isets.First(); i <= isets.Last(); i++)
    if(MayReach(isets[i]))
      answer += isets[i];
  return answer;
}

const gList<const Infoset *> EFSupport::ReachableInfosets(const Node *n) const
{
  gList<const Infoset *> answer;
  gList<const Node *> nodelist = ReachableNonterminalNodes(n);
  for (int i = 1; i <= nodelist.Length(); i++)
    answer += nodelist[i]->GetInfoset();
  answer.RemoveRedundancies();
  return answer;
}

const gList<const Infoset *> 
EFSupport::ReachableInfosets(const Node *n, Action *a) const
{
  gList<const Infoset *> answer;
  gList<const Node *> nodelist = ReachableNonterminalNodes(n,a);
  for (int i = 1; i <= nodelist.Length(); i++)
    answer += nodelist[i]->GetInfoset();
  answer.RemoveRedundancies();
  return answer;
}

bool EFSupport::AlwaysReaches(const Infoset *i) const
{
  return AlwaysReachesFrom(i,RootNode());
}

bool EFSupport::AlwaysReachesFrom(const Infoset *i, const Node *n) const
{
  if (n->IsTerminal()) return false;
  else
    if (n->GetInfoset() == i) return true;
    else {
      gArray<Action *> actions = Actions(n->GetInfoset());
      for (int j = 1; j <= actions.Length(); j++)
	if (!AlwaysReachesFrom(i,n->GetChild(actions[j]))) 
	  return false;
    }
  return true;
}

bool EFSupport::MayReach(const Infoset *i) const
{
  gArray<Node *> members = i->Members();
  for (int j = 1; j <= members.Length(); j++)
    if (MayReach(members[j]))
      return true;
  return false;
}

bool EFSupport::MayReach(const Node *n) const
{
  if (n == RootNode())
    return true;
  else {
    if (!ActionIsActive((Action*)n->GetAction()))
      return false;
    else 
      return MayReach(n->GetParent());
  }
}


void EFSupport::Dump(gOutput& s) const
{
  int numplayers;
  int i;
  int j;
  int k;

  s << "{ ";
  numplayers = befg->NumPlayers();
  for (i = 1; i <= numplayers; i++)  {
    EFPlayer& player = sets[i]->GetPlayer();
    s << '"' << player.GetName() << "\" { ";
    for (j = 1; j <= player.NumInfosets(); j++)  {
      Infoset* infoset = player.Infosets()[j];
      s << '"' << infoset->GetName() << "\" { ";
      for (k = 1; k <= NumActions(i, j); k++)  {
	const Action *action = sets[i]->ActionList(j)[k];
	//	s << '"' << action->GetName() << "\" ";
	s << action << ' ';
      }
      s << "} ";
    }
    s << "} ";
  }
  s << "} ";
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
  return infoset_has_active_nodes(i->GetPlayer()->GetNumber(), i->GetNumber());
}

void EFSupportWithActiveInfo::activate(const Node *n)
{
  is_nonterminal_node_active[n->GetPlayer()->GetNumber()]
                            [n->GetInfoset()->GetNumber()]
                            [n->NumberInInfoset()] = true;
}

void EFSupportWithActiveInfo::deactivate(const Node *n)
{
  is_nonterminal_node_active[n->GetPlayer()->GetNumber()]
                            [n->GetInfoset()->GetNumber()]
                            [n->NumberInInfoset()] = false;
}

void EFSupportWithActiveInfo::activate(const Infoset * i)
{
  is_infoset_active[i->GetPlayer()->GetNumber()][i->GetNumber()] = true;
}

void EFSupportWithActiveInfo::deactivate(const Infoset * i)
{
  is_infoset_active[i->GetPlayer()->GetNumber()][i->GetNumber()] = false;
}

void EFSupportWithActiveInfo::activate_this_and_lower_nodes(const Node *n)
{
  if (n->IsNonterminal()) {
    activate(n); 
    activate(n->GetInfoset());
    gArray<Action *> actions(Actions(n->GetInfoset()));
    for (int i = 1; i <= actions.Length(); i++) 
      activate_this_and_lower_nodes(n->GetChild(actions[i]));    
  }
}

void EFSupportWithActiveInfo::deactivate_this_and_lower_nodes(const Node *n)
{
  if (n->IsNonterminal()) {  // THIS ALL LOOKS FISHY
    deactivate(n); 
    if ( !infoset_has_active_nodes(n->GetInfoset()) )
      deactivate(n->GetInfoset());
    gArray<Action *> actions(Actions(n->GetInfoset()));
      for (int i = 1; i <= actions.Length(); i++) 
	deactivate_this_and_lower_nodes(n->GetChild(actions[i]));    
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
    gArray<Action *> actions(Actions(n->GetInfoset()));
      for (int i = 1; i <= actions.Length(); i++) 
	deactivate_this_and_lower_nodes_returning_deactivated_infosets(
			     n->GetChild(actions[i]),list);    
  }
}

void EFSupportWithActiveInfo::InitializeActiveListsToAllActive()
{
  for (int pl = 0; pl <= Game().NumPlayers(); pl++) {
    gList<bool>         is_players_infoset_active;
    gList<gList<bool> > is_players_node_active;
    for (int iset = 1; iset <= Game().NumPlayersInfosets(pl); iset++) {

      is_players_infoset_active += true;

      gList<bool> is_infosets_node_active;
      for (int n = 1; n <= Game().NumNodesInInfoset(pl,iset); n++)
	is_infosets_node_active += true;
      is_players_node_active += is_infosets_node_active;
    }
    is_infoset_active[pl] = is_players_infoset_active;
    is_nonterminal_node_active[pl] = is_players_node_active;
  }
}

void EFSupportWithActiveInfo::InitializeActiveListsToAllInactive()
{
  for (int pl = 0; pl <= Game().NumPlayers(); pl++) {
    gList<bool>         is_players_infoset_active;
    gList<gList<bool> > is_players_node_active;

    for (int iset = 1; iset <= Game().NumPlayersInfosets(pl); iset++) {

      is_players_infoset_active += false;

      gList<bool> is_infosets_node_active;
      for (int n = 1; n <= Game().NumNodesInInfoset(pl,iset); n++)
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

  activate_this_and_lower_nodes(Game().RootNode());
}

// Constructors and Destructor
EFSupportWithActiveInfo::EFSupportWithActiveInfo(const Efg &E) 
  : EFSupport(E), 
    is_infoset_active(0,E.NumPlayers()), 
    is_nonterminal_node_active(0,E.NumPlayers())
{
  InitializeActiveLists();
}

EFSupportWithActiveInfo::EFSupportWithActiveInfo(const EFSupport& given)
  : EFSupport(given), 
    is_infoset_active(0,given.Game().NumPlayers()), 
    is_nonterminal_node_active(0,given.Game().NumPlayers())
{
  InitializeActiveLists();
}

EFSupportWithActiveInfo::EFSupportWithActiveInfo(
				  const EFSupportWithActiveInfo& given)
  : EFSupport(given.UnderlyingSupport()), 
    //is_infoset_active(0,given.Game().NumPlayers()), 
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
  if ((EFSupport&) *this != (EFSupport&) s) 
    {gout << "Underlying supports differ.\n"; return false; }
  if (is_infoset_active != s.is_infoset_active)
    {gout<< "Active infosets differ:\n"; 
   
    for(int i = 0; i < is_infoset_active.Length(); i++)
      gout << "is_infoset_active[" << i << "] = " << is_infoset_active[i];

    for(int i = 0; i < s.is_infoset_active.Length(); i++)
      gout << "s.is_infoset_active[" << i << "] = " << s.is_infoset_active[i];

    return false;}
  if (is_nonterminal_node_active != s.is_nonterminal_node_active)
    {gout << "Active nodes differ.\n"; return false;}
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
  int pl = i->GetPlayer()->GetNumber();
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
  for (int pl = 1; pl <= Game().NumPlayers(); pl++) {
    const EFPlayer *p = Game().GetPlayer(pl);
    for (int iset = 1; iset <= p->NumInfosets(); iset++)
      answer += ReachableNodesInInfoset(p->GetInfoset(iset));
  }
  return answer;
}

// Editing functions
void EFSupportWithActiveInfo::AddAction(const Action *s)
{
  EFSupport::AddAction(s);

  gList<const Node *> startlist(ReachableNodesInInfoset(s->BelongsTo()));
  for (int i = 1; i <= startlist.Length(); i++)
    activate_this_and_lower_nodes(startlist[i]);
}

bool EFSupportWithActiveInfo::RemoveAction(const Action *s)
{

  gList<const Node *> startlist(ReachableNodesInInfoset(s->BelongsTo()));
  for (int i = 1; i <= startlist.Length(); i++)
    deactivate_this_and_lower_nodes(startlist[i]->GetChild(s));

  // the following returns false if s was not active
  return EFSupport::RemoveAction(s);
}

bool 
EFSupportWithActiveInfo::RemoveActionReturningDeletedInfosets(const Action *s,
					   gList<Infoset *> *list)
{

  gList<const Node *> startlist(ReachableNodesInInfoset(s->BelongsTo()));
  for (int i = 1; i <= startlist.Length(); i++)
    deactivate_this_and_lower_nodes_returning_deactivated_infosets(
                           startlist[i]->GetChild(s),list);

  // the following returns false if s was not active
  return EFSupport::RemoveAction(s);
}

int EFSupportWithActiveInfo::NumActiveNodes(const int pl,
					     const int iset) const
{
  int answer = 0;
  for (int i = 1; i <= is_nonterminal_node_active[pl][iset].Length(); i++)
    if (is_nonterminal_node_active[pl][iset][i])
      answer++;
  return answer;
}

int EFSupportWithActiveInfo::NumActiveNodes(const Infoset *i) const
{
  return NumActiveNodes(i->GetPlayer()->GetNumber(),i->GetNumber());
}

bool EFSupportWithActiveInfo::InfosetIsActive(const int pl,
					      const int iset) const
{
  return is_infoset_active[pl][iset];
}

bool EFSupportWithActiveInfo::InfosetIsActive(const Infoset *i) const
{
  return InfosetIsActive(i->GetPlayer()->GetNumber(),i->GetNumber());
}

bool EFSupportWithActiveInfo::NodeIsActive(const int pl,
					   const int iset,
					   const int node) const
{
  return is_nonterminal_node_active[pl][iset][node];
}

bool EFSupportWithActiveInfo::NodeIsActive(const Node *n) const
{
  return NodeIsActive(n->GetInfoset()->GetPlayer()->GetNumber(),
		      n->GetInfoset()->GetNumber(),
		      n->NumberInInfoset());
}

bool EFSupportWithActiveInfo::HasActiveActionsAtActiveInfosets()
{
  for (int pl = 1; pl <= Game().NumPlayers(); pl++)
    for (int iset = 1; iset <= Game().NumPlayersInfosets(pl); iset++) 
      if (InfosetIsActive(pl,iset))
        if ( NumActions(Game().GetInfosetByIndex(pl,iset)) == 0 )
          return false;
  return true;
}

bool EFSupportWithActiveInfo::HasActiveActionsAtActiveInfosetsAndNoOthers()
{
  for (int pl = 1; pl <= Game().NumPlayers(); pl++)
    for (int iset = 1; iset <= Game().NumPlayersInfosets(pl); iset++) {
      if (InfosetIsActive(pl,iset))
        if ( NumActions(Game().GetInfosetByIndex(pl,iset)) == 0 )
          return false;
      if (!InfosetIsActive(pl,iset))
        if ( NumActions(Game().GetInfosetByIndex(pl,iset)) > 0 )
          return false;
      }
  return true;
}


void EFSupportWithActiveInfo::Dump(gOutput& s) const
{
  EFSupport::Dump(s);

  /*
  s << "\n";

  for (int pl = 0; pl <= Game().NumPlayers(); pl++) {
    if (pl == 0)
      s << " Chance:  ";
    else 
      s << "Player " << pl << ":";
    //    s << "(" << Game().NumPlayersInfosets(pl) << ")";
    //    s << "\n";

    for (int iset = 1; iset <= Game().NumPlayersInfosets(pl); iset++) { 

      s << "  Infoset " << iset << " is ";
      if (InfosetIsActive(pl,iset))
	s << "Active  : ";
      else
	s << "inactive: ";
      
      s << "{";
      for (int n = 1; n <= Game().NumNodesInInfoset(pl,iset); n++) {
	if (NodeIsActive(pl,iset,n))
	  s << "+";
	else
	  s << "0";
	if (n < Game().NumNodesInInfoset(pl,iset))
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
template class gList<const EFSupport>;
template class gList<const EFSupportWithActiveInfo>;

