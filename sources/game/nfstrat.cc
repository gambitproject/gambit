//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of strategy classes for normal forms
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

#include "base/base.h"
#include "nfstrat.h"
#include "nfplayer.h"
#include "nfg.h"

//--------------------------------------------------------
// StrategyProfile: Constructors, Destructors, Operators
//--------------------------------------------------------

StrategyProfile::StrategyProfile(const Nfg &N)
  : index(0L), profile(N.NumPlayers())
{
  for (int pl = 1; pl <= N.NumPlayers(); pl++)   {
    profile[pl] = N.Strategies(pl)[1];
    index += profile[pl]->Index();
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

void StrategyProfile::Set(int p, const Strategy *const s)
{
  index += (((s) ? s->Index() : 0L) - ((profile[p]) ? profile[p]->Index() : 0L));
  profile[p] = (Strategy *)s;
}

class nfgSupportPlayer  {
protected:
  NFPlayer *nfp;
  gBlock<Strategy *> strategies;
  
public:
  nfgSupportPlayer(const nfgSupportPlayer &s); 
  nfgSupportPlayer(NFPlayer &p);
  
  nfgSupportPlayer &operator=(const nfgSupportPlayer &s); 
  bool operator==(const nfgSupportPlayer &s);

  virtual ~nfgSupportPlayer();

  // Add a strategy to the nfgSupportPlayer
  void AddStrategy(Strategy *s);

  // Removes a strategy. Returns true if the strategy was successfully
  // removed, false otherwise.
  bool RemoveStrategy(Strategy *s); 

  // Number of strategies in the nfgSupportPlayer
  int NumStrats(void) const;

  //  return the entire strategy set in a const gArray
  const gBlock<Strategy *> &Strategies(void) const;
};


//-----------------------------------------------
// nfgSupportPlayer: Constructors, Destructors, Operators
//-----------------------------------------------

nfgSupportPlayer::nfgSupportPlayer(NFPlayer &p)
  : nfp(&p), strategies(p.NumStrats())
{
  for (int st = 1; st <= p.NumStrats(); st++)
    strategies[st] = p.Strategies()[st];  
}

nfgSupportPlayer::nfgSupportPlayer(const nfgSupportPlayer &s)
  : nfp(s.nfp), strategies(s.strategies)
{ }

nfgSupportPlayer::~nfgSupportPlayer()
{ }

nfgSupportPlayer &nfgSupportPlayer::operator=(const nfgSupportPlayer &s)
{
  if (this != &s) {
    nfp = s.nfp;
    strategies = s.strategies;
  }
  return *this;
}

bool nfgSupportPlayer::operator==(const nfgSupportPlayer &s)
{
  if (strategies.Length() != s.strategies.Length()) return (false);
  int i;
  for (i = 1; i <= strategies. Length() 
       && strategies[i] == s.strategies[i]; i++);
  if (i > strategies.Length()) return (true);
  else return (false);
}

//------------------------------------------
// nfgSupportPlayer: Member functions 
//------------------------------------------

// Add a strategy to the nfgSupportPlayer
void nfgSupportPlayer::AddStrategy(Strategy *s)
{ 
  if (nfp == s->Player() && !strategies.Find(s)) {
    int index;
    for (index = 1; (index <= strategies.Length() &&
		     strategies[index]->Number() < s->Number()); index++);
    strategies.Insert(s, index);
  }
}

// Removes a strategy. Returns true if the strategy was successfully
// removed, false otherwise.
bool nfgSupportPlayer::RemoveStrategy(Strategy *s) 
{ 
  if (nfp != s->Player())  return false;
  int t = strategies.Find(s); 
  if (t > 0) 
    strategies.Remove(t); 
  return (t > 0); 
} 

// Number of Strategies in a nfgSupportPlayer
int nfgSupportPlayer::NumStrats(void) const
{
  return (strategies.Length());
}

// Return the entire strategy set
const gBlock<Strategy *> &nfgSupportPlayer::Strategies(void) const
{
  return strategies;
}

//-----------------------------------------------
// NFSupport: Ctors, Dtor, Operators
//-----------------------------------------------

NFSupport::NFSupport(const Nfg &N) : bnfg(&N), sups(N.NumPlayers())
{ 
  for (int i = 1; i <= sups.Length(); i++)
    sups[i] = new nfgSupportPlayer(*(N.Players()[i]));
}

NFSupport::NFSupport(const NFSupport &s)
  : bnfg(s.bnfg), sups(s.sups.Length()), m_name(s.m_name)
{
  for (int i = 1; i <= sups.Length(); i++)
    sups[i] = new nfgSupportPlayer(*s.sups[i]);
}

NFSupport::~NFSupport()
{ 
  for (int i = 1; i <= sups.Length(); i++)
    delete sups[i];
}

NFSupport &NFSupport::operator=(const NFSupport &s)
{
  if (this != &s && bnfg == s.bnfg) {
    m_name = s.m_name;
    for (int i = 1; i <= sups.Length(); i++)  {
      delete sups[i];
      sups[i] = new nfgSupportPlayer(*s.sups[i]);
    }
  }
  return *this;
}

bool NFSupport::operator==(const NFSupport &s) const
{
  if (bnfg != s.bnfg)  return false;
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
  return (sups[pl]->Strategies());
}

int NFSupport::GetNumber(const Strategy *s) const
{
  int pl = s->Player()->GetNumber();
  gBlock<Strategy *> strats = Strategies(pl);
  for (int i = 1; i <= strats.Length(); i++)
    if (strats[i] == s)
      return i;
  //  gout << "Looking for the number of a strategy not in the support.\n";
  exit(0);
  return 0;
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

int NFSupport::TotalNumStrats(void) const
{
  int total = 0;
  for (int i = 1 ; i <= sups.Length(); i++)
    total += sups[i]->NumStrats();
  return total;
}

int NFSupport::Find(Strategy *s) const
{
  return sups[s->Player()->GetNumber()]->Strategies().Find(s);
}

bool NFSupport::StrategyIsActive(Strategy *s) const
{
  if (Find(s) > 0)
    return true;
  else
    return false;
}

void NFSupport::AddStrategy(Strategy *s)
{
  sups[s->Player()->GetNumber()]->AddStrategy(s);
}

bool NFSupport::RemoveStrategy(Strategy *s)
{
  return sups[s->Player()->GetNumber()]->RemoveStrategy(s);
}


// Returns true if all strategies in _THIS_ belong to _S_
bool NFSupport::IsSubset(const NFSupport &s) const
{
  if (bnfg != s.bnfg)  return false;
  for (int i = 1; i <= sups.Length(); i++)
    if (NumStrats(i) > s.NumStrats(i))
      return false;
    else  {
      const gBlock<Strategy *> &strats =
        sups[i]->Strategies();

      for (int j = 1; j <= NumStrats(i); j++)
	if (!s.sups[i]->Strategies().Find(strats[j]))
	  return false;
    }
  return true;
}

bool NFSupport::IsValid(void) const
{
  for (int pl = 1; pl <= bnfg->NumPlayers(); pl++) {
    if (NumStrats(pl) == 0)
      return false;
  }
  return true;
}

void NFSupport::Dump(gOutput &p_output) const
{
  p_output << '"' << m_name << "\" { ";
  for (int pl = 1; pl <= Game().NumPlayers(); pl++) {
    p_output << "{ ";
    const gArray<Strategy *> &strategies = Strategies(pl);
    for (int st = 1; st <= strategies.Length(); st++) {
      p_output << "\"" << strategies[st]->Name() << "\" ";
    }
    p_output << "} ";
  }
  p_output << "} ";
}

gOutput& operator<<(gOutput& s, const NFSupport& n)
{
  n.Dump(s);
  return s;
}

