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

//
// The extensive form and normal form outcome classes are both included
// in this header file in the hopes of an eventual unification of the
// two.
//

struct gbt_efg_outcome_rep;
class efgGame;
class gbtEfgPlayer;

class gbtEfgOutcome {
friend class efgGame;
friend class gbtEfgNode;
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
  void SetLabel(const gText &);

  gNumber GetPayoff(const gbtEfgPlayer &) const;
  double GetPayoffDouble(int p_playerId) const;
};

gOutput &operator<<(gOutput &, const gbtEfgOutcome &);


struct gbt_nfg_outcome_rep;
class Nfg;

class gbtNfgOutcome {
friend class Nfg;
protected:
  struct gbt_nfg_outcome_rep *rep;

public:
  gbtNfgOutcome(void);
  gbtNfgOutcome(gbt_nfg_outcome_rep *);
  gbtNfgOutcome(const gbtNfgOutcome &);
  ~gbtNfgOutcome();

  gbtNfgOutcome &operator=(const gbtNfgOutcome &);

  bool operator==(const gbtNfgOutcome &) const;
  bool operator!=(const gbtNfgOutcome &) const;

  bool IsNull(void) const;
  int GetId(void) const;
  Nfg *GetGame(void) const;
  gText GetLabel(void) const;
  void SetLabel(const gText &);
  
  gNumber GetPayoff(int p_playerId) const;
  double GetPayoffDouble(int p_playerId) const;
};

gOutput &operator<<(gOutput &, const gbtNfgOutcome &);

#endif  // OUTCOME_H
