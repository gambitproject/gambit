//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to normal form contingency class
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

#ifndef NFGCONT_H
#define NFGCONT_H

#include "game.h"

class gbtNfgContingencyRep : public gbtGameObject {
friend class gbtNfgContingency;
public:
  virtual ~gbtNfgContingencyRep() { }

  virtual gbtNfgContingencyRep *Copy(void) const = 0;
  
  virtual gbtGameStrategy GetStrategy(const gbtGamePlayer &) const = 0;
  virtual void SetStrategy(gbtGameStrategy) = 0;

  virtual gbtGameOutcome GetOutcome(void) const = 0;
  virtual void SetOutcome(const gbtGameOutcome &) const = 0;

  virtual gbtNumber GetPayoff(const gbtGamePlayer &) const = 0;
};

class gbtNfgContingency {
private:
  gbtNfgContingencyRep *m_rep;

public:
  gbtNfgContingency(void) : m_rep(0) { }
  gbtNfgContingency(gbtNfgContingencyRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtNfgContingency(const gbtNfgContingency &p_cont)
    : m_rep(p_cont.m_rep->Copy()) { if (m_rep) m_rep->Reference(); }
  ~gbtNfgContingency() { if (m_rep && m_rep->Dereference()) delete m_rep; }

  gbtNfgContingency &operator=(const gbtNfgContingency &p_cont) {
    if (this != &p_cont) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_cont.m_rep->Copy();
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  gbtNfgContingencyRep *operator->(void) 
  { if (!m_rep) throw gbtGameNullObject(); return m_rep; }
  const gbtNfgContingencyRep *operator->(void) const 
  { if (!m_rep) throw gbtGameNullObject(); return m_rep; }
  
  gbtNfgContingencyRep *Get(void) const { return m_rep; }

  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};

#endif // NFGCONT_H
