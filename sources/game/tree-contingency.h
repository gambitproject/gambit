//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Representation of strategy contingencies for explicit game trees
//
// This file is part of Gambit
// Copyright (c) 2003, The Gambit Project
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

#ifndef TREE_CONTINGENCY_H
#define TREE_CONTINGENCY_H

//!
//! Specialization of contingency concept to games represented by trees.
//! The current implementation is a bit inefficient, as it reconstructs
//! the entire path through the tree each time GetPayoff() is called,
//! rather than caching it.
//!
//! Note that GetOutcome() and SetOutcome() are not well-defined for
//! trees, and throw the gbtGameUndefined exception if called.
//!
class gbtNfgContingencyTree : public gbtNfgContingencyRep {
public:
  gbtTreeGameRep *m_efg;
  gbtArray<gbtTreeStrategyRep *> m_profile;

  gbtNfgContingencyTree(gbtTreeGameRep *);

  gbtNfgContingencyRep *Copy(void) const;
  gbtGameStrategy GetStrategy(const gbtGamePlayer &) const;
  void SetStrategy(const gbtGameStrategy &);
  void SetOutcome(const gbtGameOutcome &);
  gbtGameOutcome GetOutcome(void) const;

  gbtRational GetPayoff(const gbtGamePlayer &) const;
};

#endif  // TREE_CONTINGENCY_H
