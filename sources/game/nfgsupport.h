//
// $Source$
// $Date$n
// $Revision$
//
// DESCRIPTION:
// Interface to support class for normal forms
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

#ifndef NFGSUPPORT_H
#define NFGSUPPORT_H

#include "base/gstatus.h"
#include "game.h"

//
// We are in the process of migrating supports so they act like
// "views" on a game -- they should support (no pun intended) all the
// usual members of the underlying game (such as NumPlayers()) as well
// as extra editing features to show/hide actions
// 
// This will eventually derive from gbtGame, providing the usual
// normal form operations
//
class gbtNfgSupport {
protected:
  gbtGame m_nfg;
  // This really could be a gbtPVector<bool> probably, but we'll keep
  // it this way for now to placate possibly older compilers.
  gbtPVector<int> m_strategies;
  gbtText m_label;
  
  bool Undominated(gbtNfgSupport &newS, int pl, bool strong,
		   gbtOutput &tracefile, gbtStatus &status) const;

public:
  // LIFECYCLE
  gbtNfgSupport(const gbtGame &);
  ~gbtNfgSupport() { }
  gbtNfgSupport &operator=(const gbtNfgSupport &);

  // OPERATORS
  bool operator==(const gbtNfgSupport &) const;
  bool operator!=(const gbtNfgSupport &p_support) const
  { return !(*this == p_support); }

  // DATA ACCESS: GENERAL
  gbtGame GetGame(void) const { return m_nfg; }

  gbtText GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }
  
  // DATA ACCESS: STRATEGIES
  int NumStrats(int pl) const;
  int NumStrats(const gbtGamePlayer &p_player) const 
    { return NumStrats(p_player->GetId()); }
  gbtArray<int> NumStrats(void) const;
  int ProfileLength(void) const;

  gbtGameStrategy GetStrategy(int pl, int st) const;
  int GetIndex(gbtGameStrategy) const;
  bool Contains(gbtGameStrategy) const;

  // MANIPULATION
  void AddStrategy(gbtGameStrategy);
  void RemoveStrategy(gbtGameStrategy);
  
  // DATA ACCESS: PROPERTIES
  bool IsSubset(const gbtNfgSupport &s) const;
  bool IsValid(void) const;

  // DOMINANCE AND ELIMINATION OF STRATEGIES
  bool Dominates(gbtGameStrategy, gbtGameStrategy, bool strong) const;
  bool IsDominated(gbtGameStrategy, bool strong) const; 

  gbtNfgSupport Undominated(bool strong, const gbtArray<int> &players,
			    gbtOutput &tracefile, gbtStatus &status) const;
  gbtNfgSupport MixedUndominated(bool strong, gbtPrecision precision,
				 const gbtArray<int> &players,
				 gbtOutput &, gbtStatus &status) const;

  // OUTPUT
  void Output(gbtOutput &) const;

  // The following are just echoed from the base game.  In the future,
  // derivation from gbtGame will handle these.
  gbtText GetComment(void) const { return m_nfg->GetComment(); }
  void SetComment(const gbtText &p_comment) { m_nfg->SetComment(p_comment); }
  bool IsConstSum(void) const { return m_nfg->IsConstSum(); }
  int NumPlayers(void) const { return m_nfg->NumPlayers(); }
  int NumOutcomes(void) const { return m_nfg->NumOutcomes(); }
};

gbtOutput &operator<<(gbtOutput &, const gbtNfgSupport &);

#endif  // NFSTRAT_H
