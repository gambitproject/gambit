//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of behavior strategy 
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

#ifndef GAME_BEHAV_MIXED_H
#define GAME_BEHAV_MIXED_H

#include "base/base.h"
#include "math/gdpvect.h"
#include "math/gmatrix.h"
#include "game.h"

template <class T> class gbtMixedProfile;
template <class T> class gbtPVector;
template <class T> class gbtRectArray;

class gbtTreeGameRep;
class gbtTreeNodeRep;

template <class T> class gbtBehavProfileRep : public gbtGameObject {
public:
  //!
  //! @name Constructors and destructor
  //!
  //@{
  /// Creates a copy of the profile
  virtual gbtBehavProfileRep<T> *Copy(void) const = 0;
  //@}

  //!
  //! @name General information about the profile
  //!
  //@{
  /// Returns the game on which the profile is defined
  virtual gbtGame GetGame(void) const = 0;
  /// Returns the number of probabilities in the profile
  virtual int BehaviorProfileLength(void) const = 0;

  /// Returns true if the profiles are equal
  virtual bool operator==(const gbtBehavProfileRep<T> &) const = 0;
  /// Returns true if the profiles are not equal
  virtual bool operator!=(const gbtBehavProfileRep<T> &x) const
    { return !(operator==(x)); }  
  //@}

  //!
  //! @name Accessing the profile probabilities
  //!
  //@{
  /// Get the probability assigned to the action
  virtual T GetActionProb(const gbtGameAction &) const = 0;
  /// Set the probability assigned to the action
  virtual void SetActionProb(const gbtGameAction &, const T &) = 0;
  /// Set the profile equal to the centroid
  virtual void SetCentroid(void) = 0;
  //@}

  //!
  //! @name Vector-like operations for accessing probabilities
  //!
  //@{
  virtual const T &operator[](int a) const = 0;
  virtual T &operator[](int a) = 0;

  virtual const T &operator()(int a, int b, int c) const = 0;
  virtual T &operator()(int a, int b, int c) = 0; 

  virtual operator gbtVector<T>(void) const = 0;
  //@}

  //!
  //! @name Payoff information
  //!
  //@{
  /// Returns the payoff to the player under the profile
  virtual T GetPayoff(const gbtGamePlayer &) const = 0;

  virtual T GetNodeValue(const gbtGameNode &, const gbtGamePlayer &) const = 0;
  virtual T GetInfosetValue(const gbtGameInfoset &iset) const = 0;
  virtual T GetActionValue(const gbtGameAction &act) const = 0;

  virtual T DiffActionValue(const gbtGameAction &action, 
			    const gbtGameAction &oppAction) const = 0;
  virtual T DiffRealizProb(const gbtGameNode &node,
			   const gbtGameAction &oppAction) const = 0;
  virtual T DiffNodeValue(const gbtGameNode &node, const gbtGamePlayer &player,
			  const gbtGameAction &oppAction) const = 0;

  virtual T GetLiapValue(bool p_penalty) const = 0;
  //@}

  //!
  //! @name Probability information
  //!
  //@{
  virtual T GetRealizProb(const gbtGameNode &node) const = 0;
  virtual T GetBeliefProb(const gbtGameNode &node) const = 0;
  virtual T GetInfosetProb(const gbtGameInfoset &iset) const = 0;
  //@}
};

//
// This is really a gbtGameSingleHandle, but we can't typedef to
// a template-looking name.  
//
template <class T> class gbtBehavProfile {
private:
  gbtBehavProfileRep<T> *m_rep;

public:
  gbtBehavProfile(void) : m_rep(0) { }
  gbtBehavProfile(gbtBehavProfileRep<T> *p_rep)
    : m_rep(p_rep) { }
  gbtBehavProfile(const gbtBehavProfile<T> &p_handle)
    : m_rep(p_handle.m_rep->Copy()) { }
  ~gbtBehavProfile() { if (m_rep) delete m_rep; }

  gbtBehavProfile<T> &operator=(const gbtBehavProfile<T> &p_handle) {
    if (this != &p_handle) {
      if (m_rep) delete m_rep;
      m_rep = p_handle.m_rep->Copy();
    }
    return *this;
  }

  gbtBehavProfileRep<T> *operator->(void) 
  { if (!m_rep) throw gbtGameNullException(); return m_rep; }
  const gbtBehavProfileRep<T> *operator->(void) const 
  { if (!m_rep) throw gbtGameNullException(); return m_rep; }
  
  bool operator==(const gbtBehavProfile<T> &p_profile) const
    { return *m_rep == *p_profile.m_rep; }
  bool operator!=(const gbtBehavProfile<T> &p_profile) const
    { return *m_rep != *p_profile.m_rep; }

  // Some additional operations to make it more gbtDPVector-like;
  // these should probably be viewed as deprecated, and we should
  // find better ways to write code to do the things that these
  // are used for.
  const T &operator()(int a, int b, int c) const { return (*m_rep)(a, b, c); }
  T &operator()(int a, int b, int c) { return (*m_rep)(a, b, c); }
  const T &operator[](int a) const { return (*m_rep)[a]; }
  T &operator[](int a) { return (*m_rep)[a]; }

  operator gbtVector<T>(void) const { return (gbtVector<T>) *m_rep; }
  
  gbtBehavProfileRep<T> *Get(void) const { return m_rep; }
};


#ifndef __BORLANDC__
template <class T> std::ostream &operator<<(std::ostream &, 
					    const gbtBehavProfile<T> &);
#endif

#endif   // GAME_BEHAV_MIXED_H
