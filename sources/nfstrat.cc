//
// FILE: nfstrat.cc -- Implementation of strategy representation for nfg
//
// $Id$
//

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

StrategyProfile::StrategyProfile(NFGameForm &N)
  : index(0L), profile(N.NumPlayers())
{
  for (int pl = 1; pl <= N.NumPlayers(); pl++)   {
    profile[pl] = N.Strategies(pl)[1];
    index += profile[pl]->index;
  }
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

bool StrategyProfile::IsValid(void) const
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

class NFStrategySet {
protected:
  NFPlayer *nfp;
  gBlock <Strategy *> strategies;
  
public:
  NFStrategySet(const NFStrategySet &s); 
  NFStrategySet(NFPlayer &p);
  
  NFStrategySet &operator=(const NFStrategySet &s); 
  bool operator==(const NFStrategySet &s);

  virtual ~NFStrategySet();

  // Append a strategies to the NFStrategySet
  void AddStrategy(Strategy *s);

  // Removes a strategy pointer. Returns true if the strategy was successfully
  // removed, false otherwise.
  bool RemoveStrategy( Strategy *s ); 

  // Number of Strategies in the NFStrategySet
  int NumStrats(void) const;

  //  return the entire strategy set in a const gArray
  const gBlock<Strategy *> &GetNFStrategySet(void) const;
};


//-----------------------------------------------
// NFStrategySet: Constructors, Destructors, Operators
//-----------------------------------------------

NFStrategySet::NFStrategySet(NFPlayer &p)
  : nfp(&p), strategies(p.NumStrats())
{
  for (int st = 1; st <= p.NumStrats(); st++)
    strategies[st] = p.Strategies()[st];  
}

NFStrategySet::NFStrategySet(const NFStrategySet &s)
  : nfp(s.nfp), strategies(s.strategies)
{ }

NFStrategySet::~NFStrategySet()
{ }

NFStrategySet &NFStrategySet::operator=(const NFStrategySet &s)
{
  if (this != &s) {
    nfp = s.nfp;
    strategies = s.strategies;
  }
  return *this;
}

bool NFStrategySet::operator==(const NFStrategySet &s)
{
  if (strategies.Length() != s.strategies.Length()) return (false);
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

//-----------------------------------------------
// NFSupport: Ctors, Dtor, Operators
//-----------------------------------------------

NFSupport::NFSupport(const NFGameForm &N) : bnfg(&N), sups(N.NumPlayers())
{ 
  for (int i = 1; i <= sups.Length(); i++)
    sups[i] = new NFStrategySet(*(N.Players()[i]));
}

NFSupport::NFSupport(const NFSupport &s)
  : bnfg(s.bnfg), sups(s.sups.Length())
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
  if (this != &s && bnfg == s.bnfg) {
    for (int i = 1; i <= sups.Length(); i++)  {
      delete sups[i];
      sups[i] = new NFStrategySet(*s.sups[i]);
    }
  }
  return *this;
}

bool NFSupport::operator==(const NFSupport &s) const
{
  assert(sups.Length() == s.sups.Length());
  int i;
  for (i = 1; i <= sups.Length() && *sups[i] == *s.sups[i]; i++);
  if (i > sups.Length()) return (true);
  else return (false);
}
  
bool NFSupport::operator!=(const NFSupport &s) const
{
  return !(*this == s);
}

//------------------------
// NFSupport: Members
//------------------------

const gBlock<Strategy *> &NFSupport::Strategies(int pl) const
{
  return (sups[pl]->GetNFStrategySet());
}

int NFSupport::NumStrats(int pl) const
{
  return sups[pl]->NumStrats();
}

const gArray<int> NFSupport::NumStrats(void) const
{
  gArray<int> a(sups.Length());

  for (int i = 1 ; i <= a.Length(); i++)
    a[i] = sups[i]->NumStrats();
  return a;
}

int NFSupport::Find(Strategy *s) const
{
  return sups[s->nfp->GetNumber()]->GetNFStrategySet().Find(s);
}

void NFSupport::AddStrategy(Strategy *s)
{
  sups[s->nfp->GetNumber()]->AddStrategy(s);
}

bool NFSupport::RemoveStrategy(Strategy *s)
{
  return sups[s->nfp->GetNumber()]->RemoveStrategy(s);
}


// Returns true if all strategies in _THIS_ belong to _S_
bool NFSupport::IsSubset(const NFSupport &s) const
{
  assert(sups.Length() == s.sups.Length());
  for (int i = 1; i <= sups.Length(); i++)
    if (NumStrats(i) > s.NumStrats(i))
      return false;
    else  {
      const gBlock<Strategy *> &strats =
        sups[i]->GetNFStrategySet();

      for (int j = 1; j <= NumStrats(i); j++)
	if (!s.sups[i]->GetNFStrategySet().Find(strats[j]))
	  return false;
    }
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
    strat = Strategies( i );
    for (j = 1; j <= strat.Length(); j++ )
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
