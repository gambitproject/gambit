//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to outcome representation classes
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

#ifndef OUTCOME_H
#define OUTCOME_H

struct gbt_efg_outcome_rep;
class efgGame;

class gbtEfgOutcome {
friend class efgGame;
protected:
  struct gbt_efg_outcome_rep *rep;

public:
  gbtEfgOutcome(void);
  gbtEfgOutcome(gbt_efg_outcome_rep *);
  gbtEfgOutcome(const gbtEfgOutcome &);
  ~gbtEfgOutcome();

  gbtEfgOutcome &operator=(const gbtEfgOutcome &);

  bool operator==(const gbtEfgOutcome &) const;
  bool operator!=(const gbtEfgOutcome &) const;

  bool IsNull(void) const;
  int GetId(void) const;
  efgGame *GetGame(void) const;
  gText GetLabel(void) const;

  gNumber GetPayoff(int p_playerId) const;
  double GetPayoffDouble(int p_playerId) const;
};

gOutput &operator<<(gOutput &, const gbtEfgOutcome &);

#endif  // OUTCOME_H
