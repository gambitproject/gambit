//
// FILE: efstrat.cc -- Implementation of supports for the extensive form
//
// $Id$
//

//#include "efstrat.h"
#include "efg.h"
#include "efplayer.h"

//----------------------------------------------------
// EFActionArray: Constructors, Destructor, operators
// ---------------------------------------------------

EFActionArrays::EFActionArrays ( const gArray <Action *> &a )
  : acts(a.Length())
{
  for (int j = 1; j <= (a.Length()); j++)
    acts[j] = a[j];
}

EFActionArrays::EFActionArrays ( const EFActionArrays &a)
  : acts(a.acts)
{ }

EFActionArrays::~EFActionArrays ()
{ }

EFActionArrays &EFActionArrays::operator=( const EFActionArrays &a)
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

bool EFActionArrays::operator==(const EFActionArrays &a)
{
  return (acts == a.acts);
}

//--------------------------------------------------
// EFActionSet: Constructors, Destructor, operators
//--------------------------------------------------

EFActionSet::EFActionSet( EFPlayer &p )
  : infosets(p.NumInfosets())
{
  efp = &p;
  for (int i = 1; i <= p.NumInfosets(); i++){
    infosets[i] = new EFActionArrays(p.infosets[i]->GetActionList());
  }
}

EFActionSet::EFActionSet( const EFActionSet &s )
: infosets(s.infosets.Length())
{
  efp = s.efp;
  for (int i = 1; i <= s.infosets.Length(); i++){
    infosets[i] = new EFActionArrays(*(s.infosets[i]));
  }
}

EFActionSet::~EFActionSet()
{ 
  for (int i = 1; i <= infosets.Length(); i++)
    delete infosets[i];
}

EFActionSet &EFActionSet::operator=(const EFActionSet &s)
{
  if (this != &s) {
    efp = s.efp;
    for (int i = 1; i<=s.infosets.Length(); i++)
      infosets[i] = new EFActionArrays(*(s.infosets[i]));
  }    
  return (*this);
  
}

bool EFActionSet::operator==(const EFActionSet &s)
{
   if (infosets.Length() != s.infosets.Length() ||
       efp != s.efp) return (false);
   else {
     int i;
     for (i = 1; i <= infosets.Length() && 
	  *(infosets[i]) == *(s.infosets[i]);  i++);
     if ( i > infosets.Length()) return (true);
     else return(false);
   }
}

//------------------------------------------
// EFActionSet: Member functions 
//------------------------------------------

// Append an action to a particular infoset;
void EFActionSet::AddAction(int iset, Action *s) 
{ 
  
  infosets[iset]->acts.Append(s); 
}

// Insert an action  to a particular infoset at a particular place;
void EFActionSet::AddAction(int iset, Action *s, int index) 
{ 
  infosets[iset]->acts.Insert(s,index); 
}

// Remove an action from infoset iset at int i, 
// returns the removed Infoset pointer
Action* EFActionSet::RemoveAction(int iset, int i) 
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
Action *EFActionSet::GetAction(int iset, int index)
{
  return (infosets[iset]->acts)[index];
}

// Number of Infosets
int EFActionSet::NumInfosets(void) const
{
  return (infosets.Length());
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

// Returns true if action is in the ActionSet
int EFActionSet::IsActionInActionSet( int iset, Action *a)
{
  return (infosets[iset]->acts.Find(a));
}

// Returns the number of the action in the original support
int EFActionSet::OriNumber ( int iset, Action *a)
{
  int i;
  for (i = 1; i <= infosets[iset]->acts.Length() && 
       (infosets[iset]->acts)[i] != a; i++);
  if (i > infosets[iset]->acts.Length()) return 0;
  else return i;
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

EFSupport::EFSupport (const BaseEfg &E) : sets(E.NumPlayers())
{
  befg = &E;
  for (int i = 1; i <= sets.Length(); i++)
    sets[i] = new EFActionSet(*(E.PlayerList()[i]));
}

EFSupport::EFSupport(const EFSupport &s)
: name(s.name), befg(s.befg), sets(s.sets.Length())
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
  if (this != &s) {
    befg = s.befg;
    name = s.name;
    for (int i = 1; i <= sets.Length(); i++)
      *(sets[i]) = (*(sets[i])); 
  }
  return (*this);
}

bool EFSupport::operator==(const EFSupport &s)
{
  if (sets.Length() != s.sets.Length()) return (false);
  else {
    int i;
    for (i = 1; i <= sets.Length() && *(sets[i]) == *(s.sets[i]); i++);
    if (i > sets.Length()) return (true);
    else return (false);
  }
}

//-----------------------------
// EFSupport: Member Functions 
//-----------------------------

int EFSupport::NumActions(int pl, int iset) const
{
  return sets[pl]->NumActions(iset);
}

const BaseEfg &EFSupport::BelongsTo(void) const
{
  return (*befg);
}

int EFSupport::Contains(Action *a) const
{
  if (a->BelongsTo()->BelongsTo() != befg)   return 0;

  int pl = a->BelongsTo()->GetPlayer()->GetNumber();
  int iset = a->BelongsTo()->GetNumber();

  return sets[pl]->IsActionInActionSet(iset, a);
}

int EFSupport::OriNumber( int pl, int iset, Action *a)
{
  assert (pl <= sets.Length() && pl >= 1);
  return sets[pl]->OriNumber(iset ,a);
}

bool EFSupport::IsValid(void) const
{
  if (sets.Length() == 0)   return false;
  for (int i = 1; i <= sets.Length(); i++)
    if (!sets[i]->IsValid())  return false;

  return true;
}

gPVector<int> EFSupport::Dimensionality(bool trunc) const
{
  gArray<int> foo(befg->NumPlayers());
  int i;
  for (i = 1; i <= befg->NumPlayers(); i++)
    foo[i] = sets[i]->GetPlayer().NumInfosets();

  gPVector<int> bar(foo);
  for (i = 1; i <= befg->NumPlayers(); i++)
    for (int j = 1; j <= sets[i]->GetPlayer().NumInfosets(); j++)
      bar(i, j) = NumActions(i,j) -((trunc) ? 1 : 0);

  return bar;
}  

Action * EFSupport::RemoveAction( int i, int j, int k)
{
  return sets[i]->RemoveAction(k,j);
}

bool EFSupport::RemoveAction( int i, int j, Action *s)
{
  return sets[i]->RemoveAction(j,s);
}

void EFSupport::AddAction( int i, int j, Action *s)
{
  sets[i]->AddAction(j,s);
}

void EFSupport::AddAction( int i, int j, Action *s, int k)
{
  sets[i]->AddAction(j,s,k);
}

int EFSupport::NumSequences(int j) const
{
  if(j<befg->PlayerList().First() || j>befg->PlayerList().Last()) return 1;
  gArray<Infoset *> isets;
  isets = (befg->PlayerList())[j]->InfosetList();
  int num = 1;
  for(int i = isets.First();i<= isets.Last();i++)
    num+=NumActions(j,i);
  return num;
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
      Infoset* infoset = player.InfosetList()[j];
      s << '"' << infoset->GetName() << "\" { ";
      for (k = 1; k <= NumActions(i, j); k++)  {
	Action* action = sets[i]->ActionList(j)[k];
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

