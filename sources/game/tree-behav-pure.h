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

#include "game-behav-pure.h"
#include "tree-game.h"

class gbtTreeBehavContingencyRep : public gbtGameBehavContingencyRep  {
public:
  int m_refCount;
  gbtTreeGameRep *m_efg;
  gbtArray<gbtArray<gbtTreeActionRep *> > m_profile;

  /// @name Constructors and destructor
  //@{
  gbtTreeBehavContingencyRep(gbtTreeGameRep *);
  virtual ~gbtTreeBehavContingencyRep();
  
  gbtGameBehavContingencyRep *Copy(void) const;
  //@}

  /// @name Mechanism for reference counting
  //@{
  void Reference(void);
  bool Dereference(void);
  bool IsDeleted(void) const { return false; }
  //@}

  /// @name Accessing the state
  //@{
  gbtGameAction GetAction(const gbtGameInfoset &) const;
  void SetAction(const gbtGameAction &);

  gbtRational GetPayoff(gbtTreeNodeRep *, gbtTreePlayerRep *) const;
  gbtRational GetPayoff(const gbtGamePlayer &) const;
  gbtRational GetRealizProb(const gbtGameNode &) const;
  //@}
};

class gbtTreeBehavProfileIteratorRep : public gbtGameBehavProfileIteratorRep {
public:
  int m_refCount;
  gbtTreeGameRep *m_efg;
  gbtTreeBehavContingencyRep m_profile;

  /// @name Constructors and destructor
  //@{
  gbtTreeBehavProfileIteratorRep(gbtTreeGameRep *);
  virtual ~gbtTreeBehavProfileIteratorRep() { }

  gbtGameBehavProfileIteratorRep *Copy(void) const;
  //@}
  
  /// @name Mechanism for reference counting
  //@{
  void Reference(void);
  bool Dereference(void);
  bool IsDeleted(void) const { return false; }
  //@}

  /// @name Iteration
  //@{
  void First(void);
  bool NextContingency(void);
  //@}

  /// @name Accessing the state
  //@{
  gbtGameAction GetAction(const gbtGameInfoset &) const;
  gbtRational GetPayoff(const gbtGamePlayer &) const;
  gbtRational GetRealizProb(const gbtGameNode &) const;
  //@}
};

#endif  // TREE_BEHAV_PURE_H
