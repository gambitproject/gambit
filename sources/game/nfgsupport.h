//
// $Source$
// $Date$n
// $Revision$
//
// DESCRIPTION:
// Interface to support class for normal forms
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

#ifndef NFGSUPPORT_H
#define NFGSUPPORT_H

#include "base/gstatus.h"
#include "game.h"

class gbtNfgSupport;

class gbtNfgSupportRep : public gbtConstGameRep, public gbtConstNfgRep {
friend class gbtNfgSupport;
private:
  bool gbtNfgSupportRep::Undominated(gbtNfgSupport &newS, int pl, bool strong,
				     gbtOutput &tracefile, 
				     gbtStatus &status) const;
public:
  // DATA ACCESS: GENERAL
  virtual gbtGame GetGame(void) const = 0;

  virtual gbtText GetLabel(void) const = 0;
  virtual void SetLabel(const gbtText &) = 0;
  
  virtual gbtNfgSupportRep *Copy(void) const = 0;

  virtual bool operator==(const gbtNfgSupportRep &) const = 0;
  virtual bool operator!=(const gbtNfgSupportRep &) const = 0;

  // DATA ACCESS: STRATEGIES
  virtual gbtGameStrategy GetStrategy(int pl, int st) const = 0;
  virtual int GetIndex(gbtGameStrategy) const = 0;
  virtual bool Contains(gbtGameStrategy) const = 0;

  // MANIPULATION
  virtual void AddStrategy(gbtGameStrategy) = 0;
  virtual void RemoveStrategy(gbtGameStrategy) = 0;
  
  // DATA ACCESS: PROPERTIES
  //  virtual bool IsValid(void) const = 0;

  // DOMINANCE AND ELIMINATION OF STRATEGIES
  // Not declared virtual; implementation of functions is generic already
  bool Dominates(gbtGameStrategy, gbtGameStrategy, bool strong) const;
  bool IsDominated(gbtGameStrategy, bool strong) const;

  gbtNfgSupport Undominated(bool strong, const gbtArray<int> &players,
			    gbtOutput &tracefile,
			    gbtStatus &status) const;
  gbtNfgSupport MixedUndominated(bool strong, gbtPrecision precision,
				 const gbtArray<int> &players,
				 gbtOutput &,
				 gbtStatus &status) const;
};

class gbtNfgSupport {
private:
  gbtNfgSupportRep *m_rep;

public:
  gbtNfgSupport(void) : m_rep(0) { }
  gbtNfgSupport(gbtNfgSupportRep *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtNfgSupport(const gbtNfgSupport &p_support)
    : m_rep(p_support.m_rep->Copy()) { if (m_rep) m_rep->Reference(); }
  ~gbtNfgSupport() { if (m_rep && m_rep->Dereference()) delete m_rep; }
  
  gbtNfgSupport &operator=(const gbtNfgSupport &p_support) {
    if (this != &p_support) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_support.m_rep->Copy();
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  // Equality semantics are defined as having the same support, not
  // the same underlying object.
  bool operator==(const gbtNfgSupport &p_support) const
  { return (*m_rep == *p_support.m_rep); }
  bool operator!=(const gbtNfgSupport &p_support) const
  { return (*m_rep != *p_support.m_rep); }
  
  gbtNfgSupportRep *operator->(void) 
  { if (!m_rep) throw gbtEfgNullGame(); return m_rep; }
  const gbtNfgSupportRep *operator->(void) const 
  { if (!m_rep) throw gbtEfgNullGame(); return m_rep; }

  //  gbtNfgSupportRep *Get(void) const { return m_rep; }
  
  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};

gbtOutput &operator<<(gbtOutput &, const gbtNfgSupport &);

#endif  // NFSTRAT_H
