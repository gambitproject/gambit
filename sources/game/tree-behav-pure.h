//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Definition of pure behavior profile for trees
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

#ifndef TREE_BEHAV_PURE_H
#define TREE_BEHAV_PURE_H

#include "game.h"

class gbtPureBehavProfile   {
protected:
  gbtGame m_efg;
  gbtArray<gbtArray<gbtGameAction> *> profile;

  void Payoff(const gbtGameNode &n, const gbtRational &, 
	      gbtArray<gbtRational> &) const;
  void InfosetProbs(const gbtGameNode &n, const gbtRational &,
		    gbtPVector<gbtRational> &) const;

public:
  gbtPureBehavProfile(const gbtGame &);
  gbtPureBehavProfile(const gbtPureBehavProfile &);
  ~gbtPureBehavProfile();

  // Operators
  gbtPureBehavProfile &operator=(const gbtPureBehavProfile &);
  gbtRational operator()(const gbtGameAction &) const;

  // Manipulation
  void Set(const gbtGameAction &);

  // Information
  gbtGameAction GetAction(const gbtGameInfoset &) const;
    
  gbtRational Payoff(const gbtGameNode &, int pl) const;
  void Payoff(gbtArray<gbtRational> &payoff) const;
  void InfosetProbs(gbtPVector<gbtRational> &prob) const;
  gbtGame GetGame(void) const   { return m_efg; }
};

#endif  // TREE_BEHAV_PURE_H
