//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Contingency iterator class for extensive forms
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

#ifndef EFGCITER_H
#define EFGCITER_H

#include "base/base.h"

#include "game.h"
#include "efgcont.h"
#include "efgsupport.h"

class gbtEfgIterator;

//
// N.B.: Currently, the contingency iterator only allows one information
//       set to be "frozen".  There is a list of "active" information
//       sets, which are those whose actions are cycled over, the idea
//       being that behavior at inactive information sets is irrelevant.
//
//       Iterating across all contingencies can be achieved by freezing
//       player number 0 (this is the default state on initialization)
//
class gbtEfgContIterator    {
  friend class gbtEfgIterator;
private:
  int m_frozenPlayer, m_frozenInfoset;
  gbtEfgSupport m_support;
  gbtEfgContingency m_profile;
  gbtPVector<int> m_current;
  gbtBlock<gbtBlock<bool> > m_isActive;
  gbtBlock<int> m_numActiveInfosets;

public:
  gbtEfgContIterator(const gbtEfgSupport &);
  gbtEfgContIterator(const gbtEfgSupport &, const gbtList<gbtGameInfoset> &);
  ~gbtEfgContIterator();
  
  void First(void);
  
  void Freeze(int pl, int iset);
  
  // These next two only work on frozen infosets
  void Set(int pl, int iset, int act);
  void Set(const gbtGameAction &);
  int Next(int pl, int iset);
  
  const gbtEfgContingency &GetProfile(void) const { return m_profile; }

  int NextContingency(void);
  
  gbtNumber GetPayoff(int pl) const;
};

// The following class is like the above, but intended for iteration
// over contingencies that are relevant once a particular node 
// has been reached.
class gbtEfgConditionalContIterator    {
  friend class gbtEfgIterator;
  private:
    gbtEfgSupport _support;
    gbtEfgContingency _profile;
    gbtPVector<int> _current;
    gbtBlock<gbtBlock<bool> > _is_active;
    gbtBlock<int> _num_active_infosets;
    mutable gbtVector<gbtNumber> _payoff;

  public:
    gbtEfgConditionalContIterator(const gbtEfgSupport &);
    gbtEfgConditionalContIterator(const gbtEfgSupport &, const gbtList<gbtGameInfoset> &);
    ~gbtEfgConditionalContIterator();
  
    void First(void); // Sets each infoset's action to the first in the support
  
    void Set(int pl, int iset, int act);
    void Set(const gbtGameAction &);
    int Next(int pl, int iset); 
  
    const gbtEfgContingency &GetProfile(void) const   
      { return _profile; }

    int NextContingency(void);   // Needs rewriting
  
    gbtNumber Payoff(int pl) const;
    gbtNumber Payoff(const gbtGameNode &, int pl) const;
  
    void Dump(gbtOutput &) const;
};

#endif   // EFGCITER_H
