//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of class to represent a normal form (pure) strategy
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

#ifndef STRATEGY_H
#define STRATEGY_H

struct gbt_nfg_strategy_rep;
class gbtNfgGame;
class gbtNfgPlayer;

class gbtNfgStrategy   {
friend class gbtNfgGame;
private:
  gbt_nfg_strategy_rep *rep;

public:
  gbtNfgStrategy(void);
  gbtNfgStrategy(gbt_nfg_strategy_rep *);
  gbtNfgStrategy(const gbtNfgStrategy &);
  ~gbtNfgStrategy();

  gbtNfgStrategy &operator=(const gbtNfgStrategy &);

  bool operator==(const gbtNfgStrategy &) const;
  bool operator!=(const gbtNfgStrategy &) const;

  bool IsNull(void) const;

  gText GetLabel(void) const;
  void SetLabel(const gText &);
  int GetId(void) const;

  gbtNfgPlayer GetPlayer(void) const;
  long GetIndex(void) const;

  const gArray<int> *const GetBehavior(void) const;
};

gOutput &operator<<(gOutput &, const gbtNfgStrategy &);

#endif  // STRATEGY_H
