//#
//# FILE: nfstrat.cc -- Implementation of strategy representation for nfg
//#
//# $Id$
//#

#include "nfstrat.h"
#include "nfplayer.h"
#include "nfg.h"

//--------------------------------------
// Strategy:  Constructors, Destructors
//--------------------------------------

Strategy::Strategy(NFPlayer *p) : number (0), nfp(p), index(0L)
{ }

Strategy::Strategy(const Strategy &s) : nfp(s.nfp), name(s.name)
{ }

Strategy &Strategy::operator=(const Strategy &s)
{
  nfp = s.nfp;
  name = s.name;
  return (*this);
}

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
  int i;
  for (i = profile.Length(); i > 0 && profile[i] != 0; i--);
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
// NFStrategySet: Constructors, Destructors, Operators
//-----------------------------------------------

NFStrategySet::NFStrategySet()
{
  nfp = NULL;
}

NFStrategySet::NFStrategySet( NFPlayer &p)
  : strategies(p.strategies.Length()) 
{
  nfp = &p; 
  for (int i = 1; i <= p.strategies.Length(); i++) 
    strategies[i] = (p.strategies)[i];
} 

NFStrategySet::NFStrategySet(const NFStrategySet &s)
: strategies(s.strategies)
{
  nfp = s.nfp;
}

NFStrategySet::~NFStrategySet()
{ }

NFStrategySet &NFStrategySet::operator=(const NFStrategySet &s)
{
  if (this != &s) {
    nfp = s.nfp;
    strategies = s.strategies;
  }
  return (*this);
}

bool NFStrategySet::operator==(const NFStrategySet &s)
{
  if ( strategies.Length() != s.strategies.Length()) return (false);
  int i;
  for (i = 1; i <= strategies. Length() 
       && strategies[i] == s.strategies[i]; i++);
  if (i > strategies.Length()) return (true);
  else return (false);
}

//------------------------------------------
// NFStrategySet: Member functions 
//------------------------------------------

// Append a strategies to the NFStrategySet
void NFStrategySet::AddStrategy(Strategy *s) 
{ 
  assert (nfp == s->nfp);
  strategies.Append(s); 
}

// Insert a strategy to a particular place in the gBlock;
void NFStrategySet::AddStrategy(Strategy *s, int i) 
{ 
  assert (nfp == s->nfp);
  strategies.Insert(s,i); 
}

// Remove a strategy at int i, returns the removed strategy pointer
Strategy* NFStrategySet::RemoveStrategy(int i) 
{ 
  return (strategies.Remove(i)); 
}

// Removes a strategy pointer. Returns true if the strategy was successfully
// removed, false otherwise.
bool NFStrategySet::RemoveStrategy( Strategy *s ) 
{ 
  assert (nfp == s->nfp);
  int t; 
  t = strategies.Find(s); 
  if (t>0) strategies.Remove(t); 
  return (t>0); 
} 

// Get a Strategy
Strategy *NFStrategySet::GetStrategy(int num) const
{
  return strategies[num];
}

// Number of Strategies in a NFStrategySet
int NFStrategySet::NumStrats(void) const
{
  return (strategies.Length());
}

// Return the entire strategy set
const gBlock<Strategy *> &NFStrategySet::GetNFStrategySet(void) const
{
	return strategies;
}

// Return the NFPlayer of this NFStrategySet
NFPlayer &NFStrategySet::GetPlayer(void) const
{
  return (*nfp);
}

//-----------------------------------------------
// NFSupport: Ctors, Dtor, Operators
//-----------------------------------------------

NFSupport::NFSupport (const BaseNfg &N) : sups(N.NumPlayers())
{ 
  bnfg = &N;
  for (int i = 1; i <= sups.Length(); i++)
    sups[i] = new NFStrategySet(*(N.PlayerList()[i]));
}

NFSupport::NFSupport(const NFSupport &s)
: name(s.name), bnfg(s.bnfg), sups(s.sups.Length())
{
  for (int i = 1; i <= sups.Length(); i++)
    sups[i] = new NFStrategySet(*s.sups[i]);
}

NFSupport::~NFSupport()
{ 
  for (int i = 1; i <= sups.Length(); i++)
		delete sups[i];
}

NFSupport &NFSupport::operator=(const NFSupport &s)
{
  if (this != &s) {
    name = s.name; 
    sups = s.sups;
  }
  return (*this);
}

bool NFSupport::operator==(const NFSupport &s) const
{
  assert(sups.Length() == s.sups.Length());
  int i;
  for (i = 1; i <= sups.Length() && *sups[i] == *s.sups[i]; i++);
  if (i > sups.Length()) return (true);
  else return (false);
}
  
  
  

//------------------------
// NFSupport: Members
//------------------------

void NFSupport::SetNFStrategySet(int pl, NFStrategySet *s)
{
  assert (s->GetPlayer().BelongsTo() == bnfg);
  sups[pl] = s;
}
Strategy *NFSupport::GetStrategy(int pl, int num) const
{
  return (sups[pl]->GetStrategy(num));
}

const gBlock<Strategy *> &NFSupport::GetStrategy(int pl) const
{
  return (sups[pl]->GetNFStrategySet());
}

const BaseNfg &NFSupport::BelongsTo(void) const
{
  return (*bnfg);
}

const gArray<int> NFSupport::SupportDimensions(void) const
{
  gArray<int> a(sups.Length());

  for ( int i = 1 ; i <= a.Length(); i++)
    a[i] = sups[i]->NumStrats();
	return a;
}

// Returns true if all strategies in _THIS_ belong to _S_
bool NFSupport::IsSubset(const NFSupport &s) const
{
	assert(sups.Length() == s.sups.Length());
	for (int i = 1; i <= sups.Length(); i++)
		if (NumStrats(i) > s.NumStrats(i))
			return false;
		else
			for (int j = 1; j <= NumStrats(i); j++)
				if (!s.GetStrategy(i).Contains(GetStrategy(i,j)))
					return false;
	return true;
}


void NFSupport::Dump(gOutput&s) const
{
  int numplayers;
  int i;
  int j;
  gArray<Strategy *> strat;

  s << "{ ";
  numplayers = BelongsTo().NumPlayers();
  for( i = 1; i <= numplayers; i++ )
  {
    s << "{ ";
    strat = GetStrategy( i );
    for( j = 1; j <= strat.Length(); j++ )
    {
      s << "\"" << strat[ j ]->name << "\" ";
    }
    s << "} ";
  }
  s << "} ";
}

gOutput& operator<<(gOutput& s, const NFSupport& n)
{
  n.Dump(s);
  return s;
}
