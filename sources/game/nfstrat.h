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
#include "math/gpvector.h"
#include "player.h"

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

class gbtNfgSupport {
protected:
  const Nfg *m_nfg;
  // This really could be a gPVector<bool> probably, but we'll keep
  // it this way for now to placate possibly older compilers.
  gPVector<int> m_strategies;
  gText m_name;
  
  bool Undominated(gbtNfgSupport &newS, int pl, bool strong,
		   gOutput &tracefile, gStatus &status) const;

public:
  // LIFECYCLE
  gbtNfgSupport(const Nfg &);
  ~gbtNfgSupport() { }
  gbtNfgSupport &operator=(const gbtNfgSupport &);

  // OPERATORS
  bool operator==(const gbtNfgSupport &) const;
  bool operator!=(const gbtNfgSupport &p_support) const
  { return !(*this == p_support); }

  // DATA ACCESS: GENERAL
  const Nfg &Game(void) const   { return *m_nfg; }

  const gText &GetName(void) const { return m_name; }
  void SetName(const gText &p_name) { m_name = p_name; }
  
  // DATA ACCESS: STRATEGIES
  int NumStrats(int pl) const;
  int NumStrats(const gbtNfgPlayer &p_player) const 
    { return NumStrats(p_player.GetId()); }
  gArray<int> NumStrats(void) const;
  int ProfileLength(void) const;

  gArray<Strategy *> Strategies(int pl) const;
  Strategy *GetStrategy(int pl, int st) const
    { return Strategies(pl)[st]; }
  int GetIndex(const Strategy *) const;
  bool Contains(const Strategy *) const;

  // MANIPULATION
  void AddStrategy(Strategy *);
  void RemoveStrategy(Strategy *);
  
  // DATA ACCESS: PROPERTIES
  bool IsSubset(const gbtNfgSupport &s) const;
  bool IsValid(void) const;

  // DOMINANCE AND ELIMINATION OF STRATEGIES
  bool Dominates(Strategy *s, Strategy *t, bool strong) const;
  bool IsDominated(Strategy *s, bool strong) const; 

  gbtNfgSupport Undominated(bool strong, const gArray<int> &players,
			    gOutput &tracefile, gStatus &status) const;
  gbtNfgSupport MixedUndominated(bool strong, gPrecision precision,
				 const gArray<int> &players,
				 gOutput &, gStatus &status) const;

  // OUTPUT
  void Output(gOutput &) const;
};

gOutput &operator<<(gOutput &, const gbtNfgSupport &);

#endif  // NFSTRAT_H


