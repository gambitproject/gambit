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
  gBlock<const Action *> acts;

public:
  EFActionArray ( const gArray <const Action *> &a);
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

EFActionArray::EFActionArray(const gArray<const Action *> &a)
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
  void AddAction(int iset, const Action *);

  // Insert an action in a particular place in an infoset;
  void AddAction(int iset, const Action *, int index);


  // Remove an action at int i, returns the removed action pointer
  const Action *RemoveAction(int iset, int i);

  // Remove an action from an infoset . 
  // Returns true if the action was successfully removed, false otherwise.
  bool RemoveAction(int iset, const Action *);

  // Get a garray of the actions in an Infoset
  const gArray<const Action *> &ActionList(int iset) const
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
  int Find(const Action *) const;

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
void EFActionSet::AddAction(int iset, const Action *s) 
{ 
  infosets[iset]->acts.Append(s); 
}

// Insert an action  to a particular infoset at a particular place;
void EFActionSet::AddAction(int iset, const Action *s, int index) 
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
bool EFActionSet::RemoveAction(int  iset, const Action *s ) 
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

int EFActionSet::Find(const Action *a) const
{
  return (infosets[a->BelongsTo()->GetNumber()]->acts.Find(a));
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

const gArray<const Action *> &EFSupport::Actions(int pl, int iset) const
{
  return sets[pl]->ActionList(iset);
}

const gArray<const Action *>& EFSupport::Actions(const Infoset &i) const
{
  return sets[i.GetPlayer()->GetNumber()]->ActionList(i.GetNumber());
}

const gArray<const Action *>& EFSupport::Actions(const Infoset *i) const
{
  return sets[i->GetPlayer()->GetNumber()]->ActionList(i->GetNumber());
}

gList<const Action *> EFSupport::ListOfActions(const Infoset *i) const
{
  gArray<const Action *> actions = Actions(i);
  gList<const Action *> answer;
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

int EFSupport::Find(const Action *a) const
{
  if (a->BelongsTo()->Game() != befg)   return 0;

  int pl = a->BelongsTo()->GetPlayer()->GetNumber();

  return sets[pl]->Find(a);
}

bool EFSupport::ActionIsActive(const Action *a) const
{
  if (a->BelongsTo()->Game() != befg)   
    return false;

  int pl = a->BelongsTo()->GetPlayer()->GetNumber();

  int act = sets[pl]->Find(a);
  if (act == 0) 
    return false;
  else
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
 
  return sets[player->GetNumber()]->RemoveAction(infoset->GetNumber(), s);
}

void EFSupport::AddAction(const Action *s)
{
  Infoset *infoset = s->BelongsTo();
  EFPlayer *player = infoset->GetPlayer();

  sets[player->GetNumber()]->AddAction(infoset->GetNumber(), s);
}

int EFSupport::NumSequences(int j) const
{
  if (j < befg->Players().First() || j > befg->Players().Last()) return 1;
  gArray<Infoset *> isets;
  isets = befg->Players()[j]->Infosets();
  int num = 1;
  for(int i = isets.First();i<= isets.Last();i++)
    num+=NumActions(j,i);
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
EFSupport::ReachableNonterminalNodes(const Node *n, const Action *a) const
{
  gList<const Node *> answer;
  const Node *nn = n->GetChild(a);
  if (nn->IsNonterminal()) {
    answer += nn;
    answer += ReachableNonterminalNodes(nn);
  }
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
EFSupport::ReachableInfosets(const Node *n, const Action *a) const
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
      gArray<const Action *> actions = Actions(n->GetInfoset());
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
    gArray<const Action *> actions = Actions(n->GetParent()->GetInfoset());
    const Action * last = NULL;
    for (int i = 1; i <= actions.Length() && last == NULL; i++)
      if (n->GetParent()->GetChild(actions[i]) == n)
	last = actions[i];
    if (!ActionIsActive(last))
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
	s << '"' << action->GetName() << "\" ";
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
//                EFSupportWithActiveNodes
// ---------------------------------------------------

// The following class is an EFSupport that keeps track of which
// nodes can be reached using only actions in the support.

// Required instantiations
#include "gblock.imp"
#include "garray.imp"
template class gArray<gList<const Node * > >;
template class gBlock<gList<const Node *> >;
template gOutput &operator<<(gOutput &, const gBlock<gList<const Node *> > &);
template class gArray<gBlock<gList<const Node *> > >;
template class gBlock<gBlock<gList<const Node *> > >;

// Utilities
void EFSupportWithActiveNodes::generate_nonterminal_nodes(const Node *n)
{
  if (n->IsNonterminal()) {
    reachable_nt_nodes += n;
    reachable_nonterminal_nodes[n->GetPlayer()->GetNumber()]
      [n->GetInfoset()->GetNumber()] += n;
    gArray<const Action *> actions(Actions(n->GetInfoset()));
    for (int i = 1; i <= actions.Length(); i++) 
      generate_nonterminal_nodes(n->GetChild(actions[i]));    
  }
}

void EFSupportWithActiveNodes::delete_this_and_lower_nonterminal_nodes(
                                            const Node *n)
{
  if (n->IsNonterminal()) {
    for (int i = 1; i <= reachable_nt_nodes.Length(); i++)
      if (n == reachable_nt_nodes[i])
	reachable_nt_nodes.Remove(i);
    int pl = n->GetPlayer()->GetNumber();
    int iset = n->GetInfoset()->GetNumber();
    for (int i = 1; i <= reachable_nonterminal_nodes[pl][iset].Length(); i++)
      if (n == reachable_nonterminal_nodes[pl][iset][i])
	reachable_nonterminal_nodes[pl][iset].Remove(i);
    gArray<const Action *> actions(Actions(n->GetInfoset()));
    for (int i = 1; i <= actions.Length(); i++) 
      delete_this_and_lower_nonterminal_nodes(n->GetChild(actions[i]));
  }
}

// Constructors and Destructor
EFSupportWithActiveNodes::EFSupportWithActiveNodes(const Efg &E) 
  : EFSupport(E), reachable_nt_nodes(), reachable_nonterminal_nodes()
{
  const gArray<int> nos_infosets(E.NumInfosets());
  for (int pl = 1; pl <= E.NumPlayers(); pl++) {
    gBlock<gList<const Node *> > new_big_list;
    for (int j = 1; nos_infosets[pl]; j++) {
      gList<const Node *> new_list;
      new_big_list += new_list;
    }
    reachable_nonterminal_nodes += new_big_list;
  }

  generate_nonterminal_nodes(RootNode());
}

EFSupportWithActiveNodes::EFSupportWithActiveNodes(const EFSupport& given)
  : EFSupport(given), reachable_nt_nodes(), reachable_nonterminal_nodes()
{

  //DEBUG
  //  gout << "Got into the EFSupportWithActiveNodes ctor...\n";

  const gArray<int> nos_infosets(given.Game().NumInfosets());
  for (int pl = 1; pl <= given.Game().NumPlayers(); pl++) {
    gBlock<gList<const Node *> > new_big_list;
    for (int j = 1; j <= nos_infosets[pl]; j++) {

  //DEBUG
      //  gout << "Got into inner loop of the EFSupportWithActiveNodes ctor...\n";

      gList<const Node *> new_list;
      new_big_list += new_list;
    }
    reachable_nonterminal_nodes += new_big_list;
  }

  generate_nonterminal_nodes(RootNode());
}

EFSupportWithActiveNodes::EFSupportWithActiveNodes(
				  const EFSupportWithActiveNodes& given)
  : EFSupport(given), 
    reachable_nt_nodes(given.reachable_nt_nodes),
    reachable_nonterminal_nodes(given.reachable_nonterminal_nodes)
{}

EFSupportWithActiveNodes::~EFSupportWithActiveNodes()
{}

// Operators
EFSupportWithActiveNodes &
EFSupportWithActiveNodes::operator=(const EFSupportWithActiveNodes &s)
{
  if (this != &s) {
    ((EFSupport&) *this) = s;
    reachable_nt_nodes = s.reachable_nt_nodes;
    reachable_nonterminal_nodes = s.reachable_nonterminal_nodes;
  }
  return *this;
}

bool 
EFSupportWithActiveNodes::operator==(const EFSupportWithActiveNodes &s) const
{
  if ((EFSupport&) *this != (EFSupport&) s) 
    return false;
  if (reachable_nt_nodes != s.reachable_nt_nodes) 
    return false;
  if (reachable_nonterminal_nodes != s.reachable_nonterminal_nodes) 
    return false;
  return true;
}

bool 
EFSupportWithActiveNodes::operator!=(const EFSupportWithActiveNodes &s) const
{
  return !(*this == s);
}

// Member Function
const gList<const Node *> *
EFSupportWithActiveNodes::ReachableNonterminalNodes() const
{
  return &reachable_nt_nodes;
}

const gList<const Node *> 
EFSupportWithActiveNodes::ReachableNodesInInfoset(const Infoset * i) const
{
  /*
  gList<const Node *> answer;
  gArray<Node *> longlist(i->Members());
  for (int i = 1; i <= longlist.Length(); i++)
    if (reachable_nt_nodes.Contains(longlist[i]))
	answer += longlist[i];
  return answer;
  */
  return reachable_nonterminal_nodes[i->GetPlayer()->GetNumber()]
    [i->GetNumber()];
}

// Editing functions
void EFSupportWithActiveNodes::AddAction(Action *s)
{
  EFSupport::AddAction(s);

  Infoset *infoset = s->BelongsTo();
  gList<const Node *> startlist(ReachableNodesInInfoset(infoset));
  for (int i = 1; i <= startlist.Length(); i++)
    generate_nonterminal_nodes(startlist[i]->GetChild(s));
}

bool EFSupportWithActiveNodes::RemoveAction(Action *s)
{
  Infoset *infoset = s->BelongsTo();
  gList<const Node *> startlist(ReachableNodesInInfoset(infoset));
  for (int i = 1; i <= startlist.Length(); i++)
    delete_this_and_lower_nonterminal_nodes(startlist[i]->GetChild(s));  

  return EFSupport::RemoveAction(s);
}


//----------------------------------------------------
//                EFSupportWithActiveInfo
// ---------------------------------------------------

// Utilities
void EFSupportWithActiveInfo::generate_nonterminal_nodes(const Node *n)
{
  if (n->IsNonterminal()) {
    reachable_nt_nodes += n;
    reachable_nonterminal_nodes[n->GetPlayer()->GetNumber()]
      [n->GetInfoset()->GetNumber()] += n;
    gArray<const Action *> actions(Actions(n->GetInfoset()));
    for (int i = 1; i <= actions.Length(); i++) 
      generate_nonterminal_nodes(n->GetChild(actions[i]));    
  }
}

void EFSupportWithActiveInfo::delete_this_and_lower_nonterminal_nodes(
                                            const Node *n)
{
  if (n->IsNonterminal()) {
    for (int i = 1; i <= reachable_nt_nodes.Length(); i++)
      if (n == reachable_nt_nodes[i])
	reachable_nt_nodes.Remove(i);
    int pl = n->GetPlayer()->GetNumber();
    int iset = n->GetInfoset()->GetNumber();
    for (int i = 1; i <= reachable_nonterminal_nodes[pl][iset].Length(); i++)
      if (n == reachable_nonterminal_nodes[pl][iset][i])
	reachable_nonterminal_nodes[pl][iset].Remove(i);
    gArray<const Action *> actions(Actions(n->GetInfoset()));
    for (int i = 1; i <= actions.Length(); i++) 
      delete_this_and_lower_nonterminal_nodes(n->GetChild(actions[i]));
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
    is_infoset_active += is_players_infoset_active;
    is_nonterminal_node_active += is_players_node_active;
  }
}

void EFSupportWithActiveInfo::InitializeActiveLists()
{
  InitializeActiveListsToAllActive();


}

// Constructors and Destructor
EFSupportWithActiveInfo::EFSupportWithActiveInfo(const Efg &E) 
  : EFSupport(E), 
    is_infoset_active(), is_nonterminal_node_active(),
    reachable_nt_nodes(), reachable_nonterminal_nodes()
{
  const gArray<int> nos_infosets(E.NumInfosets());
  for (int pl = 1; pl <= E.NumPlayers(); pl++) {
    gBlock<gList<const Node *> > new_big_list;
    for (int j = 1; nos_infosets[pl]; j++) {
      gList<const Node *> new_list;
      new_big_list += new_list;
    }
    reachable_nonterminal_nodes += new_big_list;
  }

  generate_nonterminal_nodes(RootNode());
}

EFSupportWithActiveInfo::EFSupportWithActiveInfo(const EFSupport& given)
  : EFSupport(given), 
    is_infoset_active(), is_nonterminal_node_active(),
    reachable_nt_nodes(), reachable_nonterminal_nodes()
{
  const gArray<int> nos_infosets(given.Game().NumInfosets());
  for (int pl = 1; pl <= given.Game().NumPlayers(); pl++) {
    gBlock<gList<const Node *> > new_big_list;
    for (int j = 1; j <= nos_infosets[pl]; j++) {
      gList<const Node *> new_list;
      new_big_list += new_list;
    }
    reachable_nonterminal_nodes += new_big_list;
  }

  generate_nonterminal_nodes(RootNode());
}

EFSupportWithActiveInfo::EFSupportWithActiveInfo(
				  const EFSupportWithActiveInfo& given)
  : EFSupport(given), 
    is_infoset_active(is_infoset_active), 
    is_nonterminal_node_active(given.is_nonterminal_node_active),
    reachable_nt_nodes(given.reachable_nt_nodes),
    reachable_nonterminal_nodes(given.reachable_nonterminal_nodes)
{}

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
    reachable_nt_nodes = s.reachable_nt_nodes;
    reachable_nonterminal_nodes = s.reachable_nonterminal_nodes;
  }
  return *this;
}

bool 
EFSupportWithActiveInfo::operator==(const EFSupportWithActiveInfo &s) const
{
  if ((EFSupport&) *this != (EFSupport&) s) 
    return false;
  if (is_infoset_active != s.is_infoset_active)
    return false;
  if (is_nonterminal_node_active != s.is_nonterminal_node_active)
    return false;
  if (reachable_nt_nodes != s.reachable_nt_nodes) 
    return false;
  if (reachable_nonterminal_nodes != s.reachable_nonterminal_nodes) 
    return false;
  return true;
}

bool 
EFSupportWithActiveInfo::operator!=(const EFSupportWithActiveInfo &s) const
{
  return !(*this == s);
}

// Member Function
const gList<const Node *> *
EFSupportWithActiveInfo::ReachableNonterminalNodes() const
{
  return &reachable_nt_nodes;
}

const gList<const Node *> 
EFSupportWithActiveInfo::ReachableNodesInInfoset(const Infoset * i) const
{
  /*
  gList<const Node *> answer;
  gArray<Node *> longlist(i->Members());
  for (int i = 1; i <= longlist.Length(); i++)
    if (reachable_nt_nodes.Contains(longlist[i]))
	answer += longlist[i];
  return answer;
  */
  return reachable_nonterminal_nodes[i->GetPlayer()->GetNumber()]
    [i->GetNumber()];
}

// Editing functions
void EFSupportWithActiveInfo::AddAction(const Action *s)
{
  EFSupport::AddAction(s);

  Infoset *infoset = s->BelongsTo();
  gList<const Node *> startlist(ReachableNodesInInfoset(infoset));
  for (int i = 1; i <= startlist.Length(); i++)
    generate_nonterminal_nodes(startlist[i]->GetChild(s));
}

bool EFSupportWithActiveInfo::RemoveAction(const Action *s)
{
  Infoset *infoset = s->BelongsTo();
  gList<const Node *> startlist(ReachableNodesInInfoset(infoset));
  for (int i = 1; i <= startlist.Length(); i++)
    delete_this_and_lower_nonterminal_nodes(startlist[i]->GetChild(s));  

  return EFSupport::RemoveAction(s);
}

bool EFSupportWithActiveInfo::ActionIsActive(const int pl,
					     const int iset,
					     const int act) const
{
  return 
    EFSupport::ActionIsActive(Game().GetPlayer(pl)->GetInfoset(iset)->GetAction(act));
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
  return is_nonterminal_node_active[pl+1][iset][node];
}

bool EFSupportWithActiveInfo::NodeIsActive(const Node *n) const
{
  return NodeIsActive(n->GetInfoset()->GetPlayer()->GetNumber(),
		      n->GetInfoset()->GetNumber(),
		      n->NumberInInfoset());
}
