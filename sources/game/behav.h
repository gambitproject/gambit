//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of behavior strategy classes
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

#ifndef BEHAV_H
#define BEHAV_H

#include "game.h"
#include "efgsupport.h"

template <class T> class gbtMixedProfile;

//
//  gbtBehavProfile<T> implements a behavior profile on a game.
//
//  The class assumes that the underlying game does not change during the 
//  life of the profile, and will not correctly invalidate itself if 
//  the game does change.  
// 
//  The BehavSolution class should be used for interactive use, where 
//  the game payoffs or probabilities may change.  
// 

template <class T> class gbtBehavProfile;

template <class T> 
class gbtBehavProfileRep : public gbtConstEfgRep {
  friend class gbtBehavProfile<T>;
public:
  virtual ~gbtBehavProfileRep() { }

  virtual gbtBehavProfileRep<T> *Copy(void) const = 0;

  virtual bool operator==(const gbtBehavProfileRep<T> &) const = 0;
  
  // INITIALIZATION, VALIDATION
  virtual void SetCentroid(void) = 0;

  // GENERAL DATA ACCESS
  virtual const T &operator()(const gbtGameAction &) const = 0;
  virtual T &operator()(const gbtGameAction &) = 0;

  virtual const T &GetRealizProb(const gbtGameNode &node) const = 0;
  virtual const T &GetBeliefProb(const gbtGameNode &node) const = 0;
  virtual gbtVector<T> GetNodeValue(const gbtGameNode &node) const = 0;
  virtual T GetInfosetProb(const gbtGameInfoset &iset) const = 0;
  virtual const T &GetInfosetValue(const gbtGameInfoset &iset) const = 0;
  virtual T GetActionProb(const gbtGameAction &act) const = 0;
  virtual const T &GetActionValue(const gbtGameAction &act) const = 0;
  virtual const T &GetRegret(const gbtGameAction &act) const = 0;

  // COMPUTATION OF INTERESTING QUANTITIES
  virtual T GetPayoff(const gbtGamePlayer &p_player) const = 0;
  virtual T GetLiapValue(bool p_penalty = true) const = 0;

  virtual T DiffActionValue(const gbtGameAction &action, 
			    const gbtGameAction &oppAction) const = 0;
  virtual T DiffRealizProb(const gbtGameNode &node,
			   const gbtGameAction &oppAction) const = 0;
  virtual T DiffNodeValue(const gbtGameNode &node, const gbtGamePlayer &player,
			  const gbtGameAction &oppAction) const = 0;

  // IMPLEMENTATION OF gbtDPVector OPERATIONS
  virtual const T &operator()(int a, int b, int c) const = 0;
  virtual T &operator()(int a, int b, int c) = 0;
  virtual const T &operator[](int a) const = 0;
  virtual T &operator[](int a) = 0;

  virtual void operator=(const T &x) = 0; 

  virtual bool operator==(const gbtDPVector<T> &x) const = 0;
  virtual bool operator!=(const gbtDPVector<T> &x) const = 0;

  virtual operator gbtMixedProfile<T>(void) const = 0;
};

template <class T> class gbtBehavProfile {
private:
  gbtBehavProfileRep<T> *m_rep;

public:
  gbtBehavProfile(void) : m_rep(0) { }
  gbtBehavProfile(gbtBehavProfileRep<T> *p_rep)
    : m_rep(p_rep) { if (m_rep) m_rep->Reference(); }
  gbtBehavProfile(const gbtBehavProfile<T> &p_profile)
    : m_rep(p_profile.m_rep->Copy()) { if (m_rep) m_rep->Reference(); }
  ~gbtBehavProfile() { if (m_rep && m_rep->Dereference()) delete m_rep; }
  
  gbtBehavProfile &operator=(const gbtBehavProfile<T> &p_profile) {
    if (this != &p_profile) {
      if (m_rep && m_rep->Dereference()) delete m_rep;
      m_rep = p_profile.m_rep->Copy();
      if (m_rep) m_rep->Reference();
    }
    return *this;
  }

  // Equality semantics are defined as having the same profile, not
  // the same underlying object.
  bool operator==(const gbtBehavProfile<T> &p_profile) const
  { return (*m_rep == *p_profile.m_rep); }
  bool operator!=(const gbtBehavProfile<T> &p_profile) const
  { return !(*m_rep == *p_profile.m_rep); }

  // Direct access so that the traditional operators work without dereferencing
  const T &operator()(int pl, int iset, int act) const
  { return (*m_rep)(pl, iset, act); }
  T &operator()(int pl, int iset, int act) { return (*m_rep)(pl, iset, act); }

  const T &operator()(const gbtGameAction &p_action) const
  { return (*m_rep)(p_action); }
  T &operator()(const gbtGameAction &p_action) { return (*m_rep)(p_action); }

  // These almost certainly should be obsolete
  const T &operator[](int i) const { return (*m_rep)[i]; }
  T &operator[](int i) { return (*m_rep)[i]; }

  gbtBehavProfileRep<T> *operator->(void) 
  { if (!m_rep) throw gbtGameNullObject(); return m_rep; }
  const gbtBehavProfileRep<T> *operator->(void) const 
  { if (!m_rep) throw gbtGameNullObject(); return m_rep; }


  gbtBehavProfileRep<T> *Get(void) const { return m_rep; }

  operator gbtMixedProfile<T>(void) const 
  { return (gbtMixedProfile<T>) *m_rep; }

  operator gbtEfgGame(void) const { return m_rep; }

  // Questionable whether this should be provided
  bool IsNull(void) const { return (m_rep == 0); }
};


#ifndef __BORLANDC__
template <class T> gbtOutput &operator<<(gbtOutput &, 
					 const gbtBehavProfile<T> &);
#endif

#endif   // BEHAV_H
