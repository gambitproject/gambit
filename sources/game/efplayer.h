//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to objects representing players in extensive forms
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

#ifndef EFPLAYER_H
#define EFPLAYER_H

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

struct gbt_efg_player_rep;
class gbtEfgGame;
class gbtEfgInfoset;

class gbtEfgPlayer {
friend class gbtEfgGame;
protected:
  struct gbt_efg_player_rep *rep;

public:
  gbtEfgPlayer(void);
  gbtEfgPlayer(gbt_efg_player_rep *);
  gbtEfgPlayer(const gbtEfgPlayer &);
  ~gbtEfgPlayer();

  gbtEfgPlayer &operator=(const gbtEfgPlayer &);

  bool operator==(const gbtEfgPlayer &) const;
  bool operator!=(const gbtEfgPlayer &) const;

  bool IsNull(void) const;

  gbtEfgGame GetGame(void) const;
  gText GetLabel(void) const;
  void SetLabel(const gText &);
  int GetId(void) const;

  bool IsChance(void) const;

  int NumInfosets(void) const;
  gbtEfgInfoset NewInfoset(int p_actions);
  gbtEfgInfoset GetInfoset(int p_index) const;
};

#endif    // EFPLAYER_H


