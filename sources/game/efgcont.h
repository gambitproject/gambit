//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to extensive form contingency class
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

#ifndef EFGCONT_H
#define EFGCONT_H

#include "game.h"

class gbtEfgContingency   {
protected:
  gbtEfgGame m_efg;
  gbtArray<gbtArray<gbtGameAction> *> profile;

  //    void IndPayoff(const Node *n, const int &pl, const T, T &) const;
  // This aims at efficiency, but leads to a problem described in behav.imp

  void Payoff(const gbtGameNode &n, const gbtNumber &,
	      gbtArray<gbtNumber> &) const;
  void InfosetProbs(const gbtGameNode &n, gbtNumber, 
		    gbtPVector<gbtNumber> &) const;

public:
  gbtEfgContingency(const gbtEfgGame &);
  gbtEfgContingency(const gbtEfgContingency &);
  ~gbtEfgContingency();

  // Operators
  gbtEfgContingency &operator=(const gbtEfgContingency &);
  gbtNumber operator()(const gbtGameAction &) const;

  // Manipulation
  void Set(const gbtGameAction &);

  // Information
  gbtGameAction GetAction(const gbtGameInfoset &) const;
  
  gbtNumber Payoff(const gbtGameOutcome &, int pl) const;
  gbtNumber ChanceProb(const gbtGameInfoset &, int act) const;
  
  gbtNumber Payoff(const gbtGameNode &, int pl) const;
  void Payoff(gbtArray<gbtNumber> &payoff) const;
  void InfosetProbs(gbtPVector<gbtNumber> &prob) const;
};


#endif   // EFGCONT_H
