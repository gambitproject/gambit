//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of normal form game representation
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

#ifndef NFG_H
#define NFG_H

#include "base/base.h"
#include "math/gnumber.h"
#include "strategy.h"
#include "player.h"
#include "outcome.h"

class StrategyProfile;
class Lexicon;
class efgGame;

struct gbt_nfg_game_rep;

class Nfg  {
friend class Lexicon;
friend class NfgFileReader;
friend void SetEfg(Nfg *, efgGame *);
protected:
  gbt_nfg_game_rep *rep;

  // PRIVATE AUXILIARY MEMBER FUNCTIONS
  void IndexStrategies(void);

  void BreakLink(void);


public:
  // CONSTRUCTORS, DESTRUCTORS, CONSTRUCTIVE OPERATORS
  Nfg(const gArray<int> &dim);
  Nfg(const Nfg &b);
  ~Nfg();
    
  // GENERAL DATA ACCESS AND MANIPULATION  
  void SetTitle(const gText &s);
  const gText &GetTitle(void) const;

  void SetComment(const gText &);
  const gText &GetComment(void) const;

  long RevisionNumber(void) const;
  bool IsDirty(void) const;
  void SetIsDirty(bool p_dirty);

  void WriteNfgFile(gOutput &, int) const;

  // PLAYERS AND STRATEGIES
  int NumPlayers(void) const;
  gbtNfgPlayer GetPlayer(int i) const;

  int NumStrats(int pl) const;
  const gArray<int> &NumStrats(void) const; 
  int ProfileLength(void) const;

  // OUTCOMES
  gbtNfgOutcome NewOutcome(void);
  void DeleteOutcome(gbtNfgOutcome);

  gbtNfgOutcome GetOutcomeId(int p_id) const;
  int NumOutcomes(void) const;

  void SetOutcome(const gArray<int> &profile, const gbtNfgOutcome &outcome);
  gbtNfgOutcome GetOutcome(const gArray<int> &profile) const;
  void SetOutcome(const StrategyProfile &p, const gbtNfgOutcome &outcome);
  gbtNfgOutcome GetOutcome(const StrategyProfile &p) const;

  void SetOutcomeIndex(int index, const gbtNfgOutcome &outcome);
  gbtNfgOutcome GetOutcomeIndex(int index) const;

  void InitPayoffs(void) const;

    // defined in nfgutils.cc
  friend void RandomNfg(Nfg &);
  friend bool IsConstSum(const Nfg &);
  friend gNumber MinPayoff(const Nfg &, int pl = 0);
  friend gNumber MaxPayoff(const Nfg &, int pl = 0);

  const efgGame *AssociatedEfg(void) const;
};

Nfg *ReadNfgFile(gInput &);
Nfg *ReadComLabSfg(gInput &);
void WriteComLabSfg(gOutput &, Nfg *);

#include "nfstrat.h"
#include "mixed.h"

extern Nfg *CompressNfg(const Nfg &nfg, const gbtNfgSupport &S);

#endif    // NFG_H
