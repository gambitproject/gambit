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

class gbtGame;
class gbtGamePlayer;

class gbtGameConstOutcomeRep : public gbtGameObject {
public:
  virtual int GetId(void) const = 0;
  virtual gbtText GetLabel(void) const = 0;

  virtual gbtNumber GetPayoff(const gbtGamePlayer &) const = 0;
  virtual gbtArray<gbtNumber> GetPayoff(void) const = 0;
};

class gbtGameOutcomeRep : public gbtGameConstOutcomeRep {
friend class gbtGameOutcome;
friend class gbtGame;
friend class gbtGameNode;
public:
  virtual void SetLabel(const gbtText &) = 0;

  //  virtual double GetPayoffDouble(int p_playerId) const = 0;
  virtual void SetPayoff(const gbtGamePlayer &, const gbtNumber &) = 0;

  virtual void DeleteOutcome(void) = 0;
};

gbtOutput &operator<<(gbtOutput &, const gbtGameOutcomeRep &);


class gbtGameOutcome {
private:
  gbtGameOutcomeRep *m_rep;

public:
  gbtGameOutcome(void) : m_rep(0) { }
  gbtGameOutcome(gbtGameOutcomeRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtGameOutcome(const gbtGameOutcome &p_outcome)
    : m_rep(p_outcome.m_rep) { if (m_rep) m_rep->Reference(); }
  ~gbtGameOutcome() { if (m_rep && m_rep->Dereference()) delete m_rep; }

  gbtGameOutcome &operator=(const gbtGameOutcome &p_outcome) {
    if (this != &p_outcome) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_outcome.m_rep;
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  bool operator==(const gbtGameOutcome &p_outcome) const
  { return (m_rep == p_outcome.m_rep); }
  bool operator!=(const gbtGameOutcome &p_outcome) const
  { return (m_rep != p_outcome.m_rep); }

  gbtGameOutcomeRep *operator->(void) 
  { if (!m_rep) throw gbtGameNullObject(); return m_rep; }
  const gbtGameOutcomeRep *operator->(void) const 
  { if (!m_rep) throw gbtGameNullObject(); return m_rep; }
  
  gbtGameOutcomeRep *Get(void) const { return m_rep; }

  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};

inline gbtOutput &operator<<(gbtOutput &p_stream, const gbtGameOutcome &) 
{ return p_stream; } 


#endif  // OUTCOME_H
