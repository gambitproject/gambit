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


class gbtNfgGameRep : public gbtGameObject {
friend class gbtNfgGame;
friend class NfgFileReader;
friend class gbtNfgContingency;
friend class gbtMixedProfile<double>;
friend class gbtMixedProfile<gbtRational>;
friend class gbtMixedProfile<gbtNumber>;
#if GBT_WITH_MP_FLOAT
friend class gbtMixedProfile<gbtMPFloat>;
#endif // GBT_WITH_MP_FLOAT
friend void SetEfg(gbtNfgGame, gbtEfgGame);

public:
  // GENERAL DATA ACCESS AND MANIPULATION  
  virtual void SetLabel(const gbtText &s) = 0;
  virtual gbtText GetLabel(void) const = 0;

  virtual void SetComment(const gbtText &) = 0;
  virtual gbtText GetComment(void) const = 0;

  virtual bool IsConstSum(void) const = 0;
  virtual long RevisionNumber(void) const = 0;

  virtual void WriteNfg(gbtOutput &p_file) const = 0;

  // PLAYERS AND STRATEGIES
  virtual int NumPlayers(void) const = 0;
  virtual gbtNfgPlayer GetPlayer(int i) const =0;

  virtual int NumStrats(int pl) const = 0;
  virtual const gbtArray<int> &NumStrats(void) const = 0; 
  virtual int ProfileLength(void) const = 0;

  // OUTCOMES
  virtual gbtNfgOutcome NewOutcome(void) = 0;
  virtual gbtNfgOutcome GetOutcome(int p_id) const = 0;
  virtual int NumOutcomes(void) const = 0;

  virtual void SetOutcomeIndex(int index, const gbtNfgOutcome &outcome) = 0;
  virtual gbtNfgOutcome GetOutcomeIndex(int index) const = 0;

  virtual void InitPayoffs(void) const = 0;

  // SUPPORTS
  virtual gbtNfgSupport NewSupport(void) const = 0;

  virtual gbtEfgGame AssociatedEfg(void) const = 0;
  virtual bool HasAssociatedEfg(void) const = 0;
};

class gbtNfgNullGame { };

class gbtNfgGame {
private:
  gbtNfgGameRep *m_rep;

public:
  gbtNfgGame(void) : m_rep(0) { }
  gbtNfgGame(gbtNfgGameRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtNfgGame(const gbtNfgGame &p_player)
    : m_rep(p_player.m_rep) { if (m_rep) m_rep->Reference(); }
  ~gbtNfgGame() { if (m_rep && m_rep->Dereference()) delete m_rep; }

  gbtNfgGame &operator=(const gbtNfgGame &p_player) {
    if (this != &p_player) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_player.m_rep;
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  bool operator==(const gbtNfgGame &p_player) const
  { return (m_rep == p_player.m_rep); }
  bool operator!=(const gbtNfgGame &p_player) const
  { return (m_rep != p_player.m_rep); }

  gbtNfgGameRep *operator->(void) 
  { if (!m_rep) throw gbtNfgNullGame(); return m_rep; }
  const gbtNfgGameRep *operator->(void) const 
  { if (!m_rep) throw gbtNfgNullGame(); return m_rep; }
  
  gbtNfgGameRep *Get(void) const { return m_rep; }

  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};

gbtNumber MinPayoff(const gbtNfgGame &, int = 0);
gbtNumber MaxPayoff(const gbtNfgGame &, int = 0);
gbtNfgGame NewNfg(const gbtArray<int> &);

// Exception thrown by ReadNfg if not valid .nfg file
class gbtNfgParserError { };
gbtNfgGame ReadNfg(gbtInput &);

#include "nfstrat.h"
#include "mixed.h"

extern gbtNfgGame CompressNfg(const gbtNfgGame &nfg, const gbtNfgSupport &S);

#endif    // NFG_H
