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

#include "game.h"

struct gbt_nfg_strategy_rep;
class gbtNfgGame;
class gbtNfgPlayer;

class gbtNfgAction : public gbtGameAction  {
friend class gbtNfgGame;
private:
  gbt_nfg_strategy_rep *rep;

public:
  gbtNfgAction(void);
  gbtNfgAction(gbt_nfg_strategy_rep *);
  gbtNfgAction(const gbtNfgAction &);
  ~gbtNfgAction();

  gbtNfgAction &operator=(const gbtNfgAction &);

  bool operator==(const gbtNfgAction &) const;
  bool operator!=(const gbtNfgAction &) const;

  bool IsNull(void) const;

  gbtText GetLabel(void) const;
  void SetLabel(const gbtText &);
  int GetId(void) const;

  gbtNfgPlayer GetPlayer(void) const;
  long GetIndex(void) const;

  const gbtArray<int> *const GetBehavior(void) const;
};

gbtOutput &operator<<(gbtOutput &, const gbtNfgAction &);

#endif  // STRATEGY_H
