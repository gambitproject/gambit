//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to normal form contingency iterator
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

#ifndef NFGCITER_H
#define NFGCITER_H

#include "base/base.h"
#include "player.h"
#include "nfstrat.h"

//
// This class is useful for iterating around the normal form.  This iterator
// allows the user choose to hold some players' strategies constant while
// iterating over the other players' strategies.  This is useful, for example,
// in displaying a 2D window into the n-D space of strategy profiles as in
// the normal form display code.  The constructor takes the normal form to
// work on and a gBlock<int> containing the numbers of players the user wants
// to hold constant.  The iteration is based on an index that goes from 1 to
// the total number of possible contingencies in increments of 1.
//
class gbtNfgContIterator    {
friend class NfgIter;
private:
  gbtNfgSupport m_support;
  gArray<int> m_current;
  gbtNfgGame m_nfg;
  StrategyProfile m_profile;
  gBlock<int> m_frozen, m_thawed;
  
public:
  gbtNfgContIterator(const gbtNfgSupport &);
  ~gbtNfgContIterator();
  
  void First(void);
  
  void Freeze(gbtNfgStrategy);
  void Thaw(gbtNfgPlayer);

  // This only has an effect if the player is currently frozen
  int Next(gbtNfgPlayer);
  
  const StrategyProfile &GetProfile(void) const { return m_profile; }

  int NextContingency(void);
  
  gbtNfgOutcome GetOutcome(void) const { return m_profile.GetOutcome(); }
  void SetOutcome(gbtNfgOutcome p_outcome) { m_profile.SetOutcome(p_outcome); }

  gNumber GetPayoff(const gbtNfgPlayer &p_player) const 
    { return m_profile.GetPayoff(p_player); }
};

#endif   // NFGCITER_H




