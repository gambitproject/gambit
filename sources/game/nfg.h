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
#include "math/gmath.h"
#include "strategy.h"
#include "player.h"
#include "outcome.h"

class gbtNfgContingency;
class gbtEfgGame;
class gbtEfgSupport;
class gbtNfgSupport;
template <class T> class gbtMixedProfile;

struct gbt_nfg_game_rep;

class gbtNfgGame : public gbtGame {
friend class Lexicon;
friend class NfgFileReader;
friend class gbtNfgContingency;
friend class gbtMixedProfile<double>;
friend class gbtMixedProfile<gbtRational>;
friend class gbtMixedProfile<gbtNumber>;
#if GBT_WITH_MP_FLOAT
 friend class gbtMixedProfile<gbtMPFloat>;
#endif // GBT_WITH_MP_FLOAT
friend void SetEfg(gbtNfgGame, gbtEfgGame);
protected:
  gbt_nfg_game_rep *rep;

  // PRIVATE AUXILIARY MEMBER FUNCTIONS
  void IndexStrategies(void);

  void BreakLink(void);

public:
  // CONSTRUCTORS, DESTRUCTORS, CONSTRUCTIVE OPERATORS
  gbtNfgGame(const gbtArray<int> &dim);
  gbtNfgGame(const gbtNfgGame &);
  gbtNfgGame(gbt_nfg_game_rep *);
  ~gbtNfgGame();
    
  gbtNfgGame &operator=(const gbtNfgGame &);

  bool operator==(const gbtNfgGame &) const;
  bool operator!=(const gbtNfgGame &) const;

  // GENERAL DATA ACCESS AND MANIPULATION  
  void SetLabel(const gbtText &s);
  gbtText GetLabel(void) const;

  void SetComment(const gbtText &);
  gbtText GetComment(void) const;

  bool IsConstSum(void) const;
  long RevisionNumber(void) const;

  void WriteNfg(gbtOutput &p_file) const;

  // PLAYERS AND STRATEGIES
  int NumPlayers(void) const;
  gbtNfgPlayer GetPlayer(int i) const;

  int NumStrats(int pl) const;
  const gbtArray<int> &NumStrats(void) const; 
  int ProfileLength(void) const;

  // OUTCOMES
  gbtNfgOutcome NewOutcome(void);
  gbtNfgOutcome GetOutcome(int p_id) const;
  int NumOutcomes(void) const;

  void SetOutcomeIndex(int index, const gbtNfgOutcome &outcome);
  gbtNfgOutcome GetOutcomeIndex(int index) const;

  void InitPayoffs(void) const;

  // SUPPORTS
  gbtNfgSupport NewSupport(void) const;

    // defined in nfgutils.cc
  friend void RandomNfg(gbtNfgGame);
  friend gbtNumber MinPayoff(const gbtNfgGame &, int pl = 0);
  friend gbtNumber MaxPayoff(const gbtNfgGame &, int pl = 0);

  gbtEfgGame AssociatedEfg(void) const;
  bool HasAssociatedEfg(void) const;
};

// Exception thrown by ReadNfg if not valid .nfg file
class gbtNfgParserError { };
gbtNfgGame ReadNfg(gbtInput &);

#include "nfstrat.h"
#include "mixed.h"

extern gbtNfgGame CompressNfg(const gbtNfgGame &nfg, const gbtNfgSupport &S);

#endif    // NFG_H
