//#
//# FILE: nfg.cc -- Implementation of normal form member functions
//#              -- and Implementation of NFPlayer member functions
//# $Date$ $Id$
//#

#include <assert.h>
#include <limits.h>
#include "nfg.h"
#include "nfg.imp"
#include "nfgiter.h"
#include "rational.h"
#include "gpset.h"
#include "nfstrat.h"
#include "nfplayer.h"


// ---------------------------------------------
// BaseNfg: Constructors, Destructors, Operators
// ---------------------------------------------


BaseNfg::BaseNfg( const gArray<int> &dim)
:players(dim.Length()), dimensions(dim)
{
  for (int i = 1; i <= players.Length(); i++) 
    players[i] = new NFPlayer(this, dim[i]);
  IndexStrategies();
}

BaseNfg::BaseNfg (const BaseNfg &b)
: title(b.title), players(b.players.Length()), dimensions(b.dimensions)
{
  for (int i = 1; i <= players.Length(); i++){
    players[i] = new NFPlayer(this, dimensions[i]);
    players[i]->name = b.players[i]->name;
    for (int j = 1; j <= players[i]->NumStrats(); j++)
      *(players[i]->strategies[j]) = *(b.players[i]->strategies[j]);
  }
  IndexStrategies();
}
BaseNfg::~BaseNfg() 
{ 
  for (int i = 1; i <= players.Length(); i++) 
    delete players[i];
}


const gArray<Strategy *> &BaseNfg::operator()(int p) const
{
  return (players[p]->StrategyList());
}

// -------------------------
// BaseNfg: Member Functions
// -------------------------


Strategy *BaseNfg::GetStrategy(int p, int s) const
{
  return ((players[p]->StrategyList())[s]);
}

const gArray<Strategy *> &BaseNfg::GetStrategy(int p) const
{
  return (players[p]->StrategyList());
}

void BaseNfg::SetTitle(const gString &s) 
{ 
  title = s; 
}

const gString &BaseNfg::GetTitle(void) const 
{ 
  return title; 
}

int BaseNfg::NumPlayers(void) const 
{ 
  return (players.Length()); 
}

const gArray<NFPlayer *> &BaseNfg::PlayerList(void) const
{
  return players;
}

int BaseNfg::NumStrats(int pl) const
{
  return ((pl > 0 && pl <= NumPlayers()) ? 
	  (players[pl])->strategies.Length() : 0);
}

int BaseNfg::ProfileLength(void) const
{
  int nprof = 0;
  for (int i = 1; i <= players.Length(); i++)
    nprof += players[i]->strategies.Length();
  return nprof;
}

// ---------------------------------------
// BaseNfg: Private member functions
// ---------------------------------------

void BaseNfg::IndexStrategies(void)
{
  long offset = 1L;

  for (int i = 1; i <= NumPlayers(); i++)  {
    for (int j = 1; j <= NumStrats(i); j++)  {
      Strategy *s = (players[i])->strategies[j];
      s->number = j;
      s->index = (j - 1) * offset;
    }
    assert(j - 1 == NumStrats(i));
    offset *= (j - 1);
  }
}

// --------------------------
// NFPlayer: Member functions 
// --------------------------

NFPlayer::NFPlayer(BaseNfg *n, int num)
: N(n), strategies(num)
{ 
  for (int j = 1; j <= num; j++)
    strategies[j] = new Strategy(this);
}

NFPlayer::~NFPlayer()
{ 
  for (int j = 1; j <= strategies.Length(); j++)
    delete strategies[j];
}

BaseNfg *NFPlayer::BelongsTo(void) const
{
  return N;
}

const gString &NFPlayer::GetName(void) const
{
  return name;
}

void NFPlayer::SetName(const gString &s)
{
  name = s;
}

int NFPlayer::NumStrats(void) const 
{
  return strategies.Length();
}


const gArray<Strategy *> &NFPlayer::StrategyList(void) const
{
  return strategies;
}



#include "mixed.h"

//---------------------------------------------------------------------------
//                    BaseMixedProfile member functions
//---------------------------------------------------------------------------

BaseMixedProfile::BaseMixedProfile(const BaseNfg &NF, bool trunc) 
: N(&NF), truncated(trunc), stratset(NF)   { }

BaseMixedProfile::BaseMixedProfile(const BaseNfg &NF, bool trunc,
				   NFSupport &s)
: N(&NF), truncated(trunc), stratset(s)   { }

BaseMixedProfile::BaseMixedProfile(const BaseMixedProfile &p)
: N(p.N), truncated(p.truncated), stratset(p.stratset)   { }

BaseMixedProfile::~BaseMixedProfile()   { }

BaseMixedProfile &BaseMixedProfile::operator=(const BaseMixedProfile &p)
{
  N = p.N;
  stratset = p.stratset;
  return *this;
}

DataType BaseMixedProfile::Type(void) const
{
  return N->Type();
}

const NFSupport &BaseMixedProfile::GetNFSupport(void) const
{
  return (stratset);
}
#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "garray.imp"
#include "gblock.imp"

TEMPLATE class gArray<NFStrategySet *>;
TEMPLATE class gArray<Strategy *>;
TEMPLATE class gArray<NFPlayer *>;
TEMPLATE class gBlock<Strategy *>;
