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
#include "nfstrat.h"

class gbtNfgContingency   {
friend class gbtNfgGame;
private:
  gbtNfgGame m_nfg; 
  long m_index;
  gbtArray<gbtNfgStrategy> m_profile;
  
public:
  gbtNfgContingency(const gbtNfgGame &);
  gbtNfgContingency(const gbtNfgGame &, const gbtArray<int> &);
  gbtNfgContingency(const gbtNfgContingency &p);

  ~gbtNfgContingency();
  
  gbtNfgContingency &operator=(const gbtNfgContingency &);

  bool operator==(const gbtNfgContingency &p_cont) const;
  bool operator!=(const gbtNfgContingency &p_cont) const
    { return !(*this == p_cont); }

  bool IsValid(void) const; 
  long GetIndex(void) const;
  
  gbtNfgStrategy GetStrategy(int p_player) const
    { return m_profile[p_player]; }
  void SetStrategy(gbtNfgStrategy);

  void SetOutcome(const gbtNfgOutcome &);
  gbtNfgOutcome GetOutcome(void) const;

  gbtNumber GetPayoff(const gbtNfgPlayer &) const;
};

//
// We are in the process of migrating supports so they act like
// "views" on a game -- they should support (no pun intended) all the
// usual members of the underlying game (such as NumPlayers()) as well
// as extra editing features to show/hide actions
// 
// This will eventually derive from gbtNfgGame, providing the usual
// normal form operations
//
class gbtNfgSupport : public gbtGame {
protected:
  gbtNfgGame m_nfg;
  // This really could be a gbtPVector<bool> probably, but we'll keep
  // it this way for now to placate possibly older compilers.
  gbtPVector<int> m_strategies;
  gbtText m_label;
  
  bool Undominated(gbtNfgSupport &newS, int pl, bool strong,
		   gbtOutput &tracefile, gbtStatus &status) const;

public:
  // LIFECYCLE
  gbtNfgSupport(const gbtNfgGame &);
  ~gbtNfgSupport() { }
  gbtNfgSupport &operator=(const gbtNfgSupport &);

  // OPERATORS
  bool operator==(const gbtNfgSupport &) const;
  bool operator!=(const gbtNfgSupport &p_support) const
  { return !(*this == p_support); }

  // DATA ACCESS: GENERAL
  gbtNfgGame GetGame(void) const { return m_nfg; }

  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }
  
  // DATA ACCESS: STRATEGIES
  int NumStrats(int pl) const;
  int NumStrats(const gbtNfgPlayer &p_player) const 
    { return NumStrats(p_player.GetId()); }
  gbtArray<int> NumStrats(void) const;
  int ProfileLength(void) const;

  gbtNfgStrategy GetStrategy(int pl, int st) const;
  int GetIndex(gbtNfgStrategy) const;
  bool Contains(gbtNfgStrategy) const;

  // MANIPULATION
  void AddStrategy(gbtNfgStrategy);
  void RemoveStrategy(gbtNfgStrategy);
  
  // DATA ACCESS: PROPERTIES
  bool IsSubset(const gbtNfgSupport &s) const;
  bool IsValid(void) const;

  // DOMINANCE AND ELIMINATION OF STRATEGIES
  bool Dominates(gbtNfgStrategy, gbtNfgStrategy, bool strong) const;
  bool IsDominated(gbtNfgStrategy, bool strong) const; 

  gbtNfgSupport Undominated(bool strong, const gbtArray<int> &players,
			    gbtOutput &tracefile, gbtStatus &status) const;
  gbtNfgSupport MixedUndominated(bool strong, gbtPrecision precision,
				 const gbtArray<int> &players,
				 gbtOutput &, gbtStatus &status) const;

  // OUTPUT
  void Output(gbtOutput &) const;

  // The following are just echoed from the base game.  In the future,
  // derivation from gbtNfgGame will handle these.
  gbtText GetComment(void) const { return m_nfg.GetComment(); }
  void SetComment(const gbtText &p_comment) { m_nfg.SetComment(p_comment); }
  bool IsConstSum(void) const { return m_nfg.IsConstSum(); }
  int NumPlayers(void) const { return m_nfg.NumPlayers(); }
  int NumOutcomes(void) const { return m_nfg.NumOutcomes(); }
};

gbtOutput &operator<<(gbtOutput &, const gbtNfgSupport &);

#endif  // NFSTRAT_H
