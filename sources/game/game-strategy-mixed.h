//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of mixed strategy profile class
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

#ifndef GAME_STRATEGY_MIXED_H
#define GAME_STRATEGY_MIXED_H

#include "base/gmisc.h"
#include "math/gpvector.h"

template <class T> class gbtRectArray;
template <class T> class gbtBehavProfile;

template <class T> class gbtMixedProfileRep : public gbtGameObject {
public:
  //!
  //! @name Constructors and destructor
  //!
  //@{
  /// Creates a copy of the profile
  virtual gbtMixedProfileRep<T> *Copy(void) const = 0;
  //@}

  //!
  //! @name General information about the profile
  //!
  //@{
  /// Returns the game on which the profile is defined
  virtual gbtGame GetGame(void) const = 0;
  /// Returns the number of probabilities in the profile
  virtual int StrategyProfileLength(void) const = 0;

  /// Returns true if the profiles are equal
  virtual bool operator==(const gbtMixedProfileRep<T> &) const = 0;
  /// Returns true if the profiles are not equal
  bool operator!=(const gbtMixedProfileRep<T> &p_profile) const
    { return !(*this == p_profile); }
  //@}

  //!
  //! @name Accessing the profile probabilities
  //!
  //@{
  /// Get the probability assigned to the strategy
  virtual T GetStrategyProb(const gbtGameStrategy &) const = 0;
  /// Set the probability assigned to the strategy
  virtual void SetStrategyProb(const gbtGameStrategy &, const T &) = 0;
  /// Set the profile equal to the centroid
  virtual void SetCentroid(void) = 0;
  //@}

  //!
  //! @name Vector-like operations for accessing probabilities
  //!
  //@{
  virtual T &operator[](int) = 0;
  virtual const T &operator[](int) const = 0;

  virtual T &operator()(int, int) = 0;
  virtual const T &operator()(int, int) const = 0;

  virtual operator gbtVector<T>(void) const = 0;
  //@}

  //!
  //! @name Payoff information
  //!
  //@{
  /// Returns the payoff to the player under the profile
  virtual T GetPayoff(const gbtGamePlayer &) const = 0;

  /// Returns the payoff to the player if the strategy is used
  //!
  //! Returns the expected payoff to the player if the strategy is
  //! played, holding fixed the behavior of all other players as
  //! specified in the profile.  This is equivalent to the derivative
  //! of the player's payoff with respect to the probability that the
  //! strategy is played.
  //!
  virtual T GetPayoffDeriv(const gbtGamePlayer &, 
			   const gbtGameStrategy &) const = 0;
  /// Returns the payoff to the player if the strategies are used
  //!
  //! Returns the expected payoff to the player if the strategies are
  //! played, holding fixed the behavior of all other players as
  //! specified in the profile.  This is equivalent to the (second) derivative
  //! of the player's payoff with respect to the probabilities that the
  //! strategies are played.  Note that this is defined to be zero when
  //! the two strategies specified belong to the same player.
  //!
  virtual T GetPayoffDeriv(const gbtGamePlayer &, 
			   const gbtGameStrategy &,
			   const gbtGameStrategy &) const = 0;

  /// Returns the payoff to playing strategy, holding fixed others' play
  T GetStrategyValue(const gbtGameStrategy &p_strategy) const
    { return GetPayoffDeriv(p_strategy->GetPlayer(), p_strategy); }

  /// Computes the Liapunov value of the profile.
  virtual T GetLiapValue(bool penalty) const = 0;
  //@}
};

//
// This is really a gbtGameSingleHandle, but we can't typedef to
// a template-looking name.  
//
template <class T> class gbtMixedProfile {
private:
  gbtMixedProfileRep<T> *m_rep;

public:
  gbtMixedProfile(void) : m_rep(0) { }
  gbtMixedProfile(gbtMixedProfileRep<T> *p_rep)
    : m_rep(p_rep) { }
  gbtMixedProfile(const gbtMixedProfile<T> &p_handle)
    : m_rep(p_handle.m_rep->Copy()) { }
  ~gbtMixedProfile() { if (m_rep) delete m_rep; }

  gbtMixedProfile<T> &operator=(const gbtMixedProfile<T> &p_handle) {
    if (this != &p_handle) {
      if (m_rep) delete m_rep;
      m_rep = p_handle.m_rep->Copy();
    }
    return *this;
  }

  gbtMixedProfileRep<T> *operator->(void) 
  { if (!m_rep) throw gbtGameNullException(); return m_rep; }
  const gbtMixedProfileRep<T> *operator->(void) const 
  { if (!m_rep) throw gbtGameNullException(); return m_rep; }
  
  bool operator==(const gbtMixedProfile<T> &p_profile) const
    { return *m_rep == *p_profile.m_rep; }
  bool operator!=(const gbtMixedProfile<T> &p_profile) const
    { return *m_rep != *p_profile.m_rep; }

  // Some additional operations to make it more gbtPVector-like;
  // these should probably be viewed as deprecated, and we should
  // find better ways to write code to do the things that these
  // are used for.
  const T &operator()(int a, int b) const { return (*m_rep)(a, b); }
  T &operator()(int a, int b) { return (*m_rep)(a, b); }
  const T &operator[](int a) const { return (*m_rep)[a]; }
  T &operator[](int a) { return (*m_rep)[a]; }
  operator gbtVector<T>(void) const { return (gbtVector<T>) *m_rep; }
  
  gbtMixedProfileRep<T> *Get(void) const { return m_rep; }
};

#ifndef __BORLANDC__
template <class T> std::ostream &operator<<(std::ostream &f, 
					    const gbtMixedProfile<T> &);
#endif

#endif  // GAME_STRATEGY_MIXED_H
