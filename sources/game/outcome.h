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

#include "game.h"

//
// The extensive form and normal form outcome classes are both included
// in this header file in the hopes of an eventual unification of the
// two.
//


class gbtEfgGame;
class gbtEfgPlayer;

class gbtEfgOutcomeRep : public gbtGameObject {
friend class gbtEfgOutcome;
friend class gbtEfgGame;
friend class gbtEfgNode;
public:
  virtual int GetId(void) const = 0;
  virtual gbtText GetLabel(void) const = 0;
  virtual void SetLabel(const gbtText &) = 0;

  virtual gbtArray<gbtNumber> GetPayoff(void) const = 0;
  virtual gbtNumber GetPayoff(const gbtEfgPlayer &) const = 0;
  virtual double GetPayoffDouble(int p_playerId) const = 0;
  virtual void SetPayoff(const gbtEfgPlayer &, const gbtNumber &) = 0;

  virtual void DeleteOutcome(void) = 0;
};

gbtOutput &operator<<(gbtOutput &, const gbtEfgOutcomeRep &);


class gbtEfgNullOutcome { };

class gbtEfgOutcome {
private:
  gbtEfgOutcomeRep *m_rep;

public:
  gbtEfgOutcome(void) : m_rep(0) { }
  gbtEfgOutcome(gbtEfgOutcomeRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtEfgOutcome(const gbtEfgOutcome &p_outcome)
    : m_rep(p_outcome.m_rep) { if (m_rep) m_rep->Reference(); }
  ~gbtEfgOutcome() { if (m_rep && m_rep->Dereference()) delete m_rep; }

  gbtEfgOutcome &operator=(const gbtEfgOutcome &p_outcome) {
    if (this != &p_outcome) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_outcome.m_rep;
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  bool operator==(const gbtEfgOutcome &p_outcome) const
  { return (m_rep == p_outcome.m_rep); }
  bool operator!=(const gbtEfgOutcome &p_outcome) const
  { return (m_rep != p_outcome.m_rep); }

  gbtEfgOutcomeRep *operator->(void) 
  { if (!m_rep) throw gbtEfgNullOutcome(); return m_rep; }
  const gbtEfgOutcomeRep *operator->(void) const 
  { if (!m_rep) throw gbtEfgNullOutcome(); return m_rep; }
  
  gbtEfgOutcomeRep *Get(void) const { return m_rep; }

  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};

inline gbtOutput &operator<<(gbtOutput &p_stream, const gbtEfgOutcome &) 
{ return p_stream; } 


class gbtNfgPlayer;

class gbtNfgOutcomeRep : public gbtGameObject {
friend class gbtNfgOutcome;
friend class gbtNfgGame;
public:
  virtual int GetId(void) const = 0;
  virtual gbtText GetLabel(void) const = 0;
  virtual void SetLabel(const gbtText &) = 0;

  virtual gbtArray<gbtNumber> GetPayoff(void) const = 0;
  virtual gbtNumber GetPayoff(const gbtNfgPlayer &) const = 0;
  virtual double GetPayoffDouble(int p_playerId) const = 0;
  virtual void SetPayoff(const gbtNfgPlayer &, const gbtNumber &) = 0;

  virtual void DeleteOutcome(void) = 0;
};

gbtOutput &operator<<(gbtOutput &, const gbtNfgOutcomeRep &);


class gbtNfgNullOutcome { };

class gbtNfgOutcome {
private:
  gbtNfgOutcomeRep *m_rep;

public:
  gbtNfgOutcome(void) : m_rep(0) { }
  gbtNfgOutcome(gbtNfgOutcomeRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtNfgOutcome(const gbtNfgOutcome &p_outcome)
    : m_rep(p_outcome.m_rep) { if (m_rep) m_rep->Reference(); }
  ~gbtNfgOutcome() { if (m_rep && m_rep->Dereference()) delete m_rep; }

  gbtNfgOutcome &operator=(const gbtNfgOutcome &p_outcome) {
    if (this != &p_outcome) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_outcome.m_rep;
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  bool operator==(const gbtNfgOutcome &p_outcome) const
  { return (m_rep == p_outcome.m_rep); }
  bool operator!=(const gbtNfgOutcome &p_outcome) const
  { return (m_rep != p_outcome.m_rep); }

  gbtNfgOutcomeRep *operator->(void) 
  { if (!m_rep) throw gbtNfgNullOutcome(); return m_rep; }
  const gbtNfgOutcomeRep *operator->(void) const 
  { if (!m_rep) throw gbtNfgNullOutcome(); return m_rep; }
  
  gbtNfgOutcomeRep *Get(void) const { return m_rep; }

  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};

inline gbtOutput &operator<<(gbtOutput &p_stream, const gbtNfgOutcome &) 
{ return p_stream; } 

#endif  // OUTCOME_H
