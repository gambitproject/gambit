//#
//# FILE: nfstrat.cc -- Implementation of strategy representation for nfg
//#
//# $Id$
//#

#ifdef __GNUG__
#pragma implementation "nfstrat.h"
#endif   // __GNUG__

#include "nfstrat.h"
#include "nfplayer.h"
#include "nfg.h"

//--------------------------------------
// Strategy:  Constructors, Destructors
//--------------------------------------

Strategy::Strategy(void) : number (0), dominator(NULL), index(0L)
{ }

Strategy::Strategy(const Strategy &s) : dominator(NULL), name(s.name)
{ }

Strategy::~Strategy()
{ }


//--------------------------------------------------------
// StrategyProfile: Constructors, Destructors, Operators
//--------------------------------------------------------

StrategyProfile::StrategyProfile(int pl)
  : index(0L), profile(pl)
{
  for (int i = 1; i <= pl; profile[i++] = (Strategy *) 0);
}

StrategyProfile::StrategyProfile(const StrategyProfile &p)
: index(p.index), profile(p.profile)
{ }

StrategyProfile::~StrategyProfile()
{ }

StrategyProfile &StrategyProfile::operator=(const StrategyProfile &p)
{
  if (this != &p) {
    index = p.index;
    profile = p.profile;
  }
  return *this;  
}

//-----------------------------------------
// StrategyProfile: Members
//-----------------------------------------

int StrategyProfile::IsValid(void) const
{
  for (int i = profile.Length(); i > 0 && profile[i] != 0; i--);
  return i;
}

long StrategyProfile::GetIndex(void) const 
{ 
  return index; 
}

Strategy *const StrategyProfile::operator[](int p) const 
{ 
  return profile[p];
}

Strategy *const StrategyProfile::Get(int p) const 
{ 
return profile[p];
}

void StrategyProfile::Set(int p, Strategy *const s)
{
  index += (((s) ? s->index : 0L) - ((profile[p]) ? profile[p]->index : 0L));
  profile[p] = s;
}

//-----------------------------------------------
// Support: Constructors, Destructors, Operators
//-----------------------------------------------

Support::Support()
{
  nfp = NULL;
}

Support::Support(const NFPlayer &p)
  : strategies(p.strategies.Length()) {
  nfp = &p; 
  for (int i = 1; i <= p.strategies.Length(); i++) 
    strategies[i] = (p.strategies)[i];
} 

Support::Support(const Support &s)
: strategies(s.strategies)
{
  nfp = s.nfp;
}

Support::~Support()
{ }

Support &Support::operator=(const Support &s)
{
  if (this != &s) {
    nfp = s.nfp;
    strategies = s.strategies;
  }
  return (*this);
}


//------------------------------------------
// Support: Member functions 
//------------------------------------------

// Append a strategies to the support
void Support::AddStrategy(Strategy *s) 
{ 
  strategies.Append(s); 
}

// Insert a strategy to a particular place in the gBlock;
void Support::AddStrategy(Strategy *s, int i) 
{ 
  strategies.Insert(s,i); 
}

// Remove a strategy at int i, returns the removed strategy pointer
Strategy* Support::RemoveStrategy(int i) 
{ 
  return (strategies.Remove(i)); 
}

// Removes a strategy pointer. Returns true if the strategy was successfully
// removed, false otherwise.
bool Support::RemoveStrategy( Strategy *s ) 
{ 
  int t; 
  t = strategies.Find(s); 
  if (t>0) strategies.Remove(t); 
  return (t>0); 
} 

// Get a Strategy
Strategy *Support::GetStrategy(int num) const
{
  return strategies[num];
}

// Number of Strategies in a support
int Support::NumStrats(void)
{
  return (strategies.Length());
}

//-----------------------------------------------
// StrategySet: Ctors, Dtor, Operators
//-----------------------------------------------

StrategySet::StrategySet(const BaseNfg &N) : sups(N.NumPlayers())
{ 
  for (int i = 1; i <= sups.Length(); i++)
    sups[i] = new Support(*(N.PlayerList()[i]));
}

StrategySet::StrategySet(const StrategySet &s)
: name(s.name), sups(s.sups)
{ }

StrategySet::~StrategySet()
{ 
  for (int i = 1; i <= sups.Length(); i++)
    delete sups[i];
}

StrategySet &StrategySet::operator=(const StrategySet &s)
{
  if (this != &s) {
    name = s.name; 
    sups = s.sups;
  }
  return (*this);
}

//------------------------
// StrategySet: Members
//------------------------

Strategy *StrategySet::GetStrategy(int pl, int num) const
{
  return (sups[pl]->GetStrategy(num));
}









