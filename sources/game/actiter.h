//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of iterator class for actions in a support
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

#ifndef ACTITER_H
#define ACTITER_H

#include "efstrat.h"

class gbtActionIterator {
protected:
  EFSupport m_support;
  int pl, iset, act;

public:
  // LIFECYCLE
  gbtActionIterator(const EFSupport &);
  ~gbtActionIterator() { }

  // OPERATORS
  bool operator==(const gbtActionIterator &) const;
  bool operator!=(const gbtActionIterator &p_iter) const 
  { return !(*this == p_iter); }

  // ITERATION
  bool GoToNext(void);

  // ACCESS TO CURRENT STATE
  Action *GetAction(void) const;
  int GetActionId(void) const { return act; }
  Infoset *GetInfoset(void) const;
  int GetInfosetId(void) const { return iset; }
  EFPlayer *GetPlayer(void) const;
  int GetPlayerId(void) const { return pl; }

  bool IsLast(void) const;
  bool IsSubsequentTo(const Action *) const;
};

#endif  // ACTITER_H
