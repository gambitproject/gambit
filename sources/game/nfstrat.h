//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to strategy classes for normal forms
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

#ifndef NFSTRAT_H
#define NFSTRAT_H

#include "base/base.h"
#include "base/gstatus.h"
#include "math/gnumber.h"
#include "nfplayer.h"

class NFPlayer;
class Nfg;
class Strategy;

class StrategyProfile   {
  friend class Nfg;
private:
  long index;
  gArray<Strategy *> profile;
  
public:
  StrategyProfile(const Nfg &);
  StrategyProfile(const StrategyProfile &p);

  ~StrategyProfile();
  
  StrategyProfile &operator=(const StrategyProfile &);
  
  bool IsValid(void) const; 
  
  long GetIndex(void) const;
  
  Strategy *const operator[](int p) const;
  Strategy *const Get(int p) const;
  void Set(int p, const Strategy  *const s);
};


class Nfg;
class nfgSupportPlayer;

class NFSupport {
protected:
  const Nfg *bnfg;
  gArray <nfgSupportPlayer *> sups;
  gText m_name;
  
  bool Undominated(NFSupport &newS, int pl, bool strong,
		   gOutput &tracefile, gStatus &status) const;

public:
  NFSupport(const Nfg &);
  NFSupport(const NFSupport &s); 
  virtual ~NFSupport();
  NFSupport &operator=(const NFSupport &s);

  bool operator==(const NFSupport &s) const;
  bool operator!=(const NFSupport &s) const;

  const Nfg &Game(void) const   { return *bnfg; }
  const Nfg *GamePtr(void) const { return bnfg; }

  const gText &GetName(void) const { return m_name; }
  void SetName(const gText &p_name) { m_name = p_name; }
  
  const gBlock<Strategy *> &Strategies(int pl) const;
  inline Strategy *GetStrategy(int pl, int i) const
    { return Strategies(pl)[i]; }
  int GetNumber(const Strategy *s) const;

  int NumStrats(int pl) const;
  inline int NumStrats(const NFPlayer* p) const 
    { return NumStrats(p->GetNumber()); }
  const gArray<int> NumStrats(void) const;
  int TotalNumStrats(void) const;

  void AddStrategy(Strategy *);
  bool RemoveStrategy(Strategy *);
  
  bool IsSubset(const NFSupport &s) const;
  bool IsValid(void) const;

  // returns the index of the strategy in the support if it exists,
  // otherwise returns zero
  int Find(Strategy *) const; 
  bool StrategyIsActive(Strategy *) const;

  // Domination 
  bool Dominates(Strategy *s, Strategy *t, bool strong) const;
  bool IsDominated(Strategy *s, bool strong) const; 

  NFSupport Undominated(bool strong, const gArray<int> &players,
			gOutput &tracefile, gStatus &status) const;
  NFSupport MixedUndominated(bool strong, gPrecision precision,
			     const gArray<int> &players,
			     gOutput &, gStatus &status) const;

  void Dump(gOutput &) const;
};

gOutput &operator<<(gOutput &f, const NFSupport &);


#endif    // NFSTRAT_H


