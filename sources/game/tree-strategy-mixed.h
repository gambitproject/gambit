//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of mixed strategy profile class for game trees
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

#ifndef TREE_STRATEGY_MIXED_H
#define TREE_STRATEGY_MIXED_H

template <class T> 
class gbtTreeMixedProfileRep : public gbtMixedProfileRep<T> {
public:
  int m_refCount;
  gbtTreeGameRep *m_game;
  gbtPVector<T> m_profile;

  /// @name Constructors and destructor
  //@{
  gbtTreeMixedProfileRep(const gbtTreeGameRep *);
  gbtTreeMixedProfileRep(const gbtTreeMixedProfileRep<T> &);
  gbtTreeMixedProfileRep(const gbtBehavProfile<T> &);
  virtual ~gbtTreeMixedProfileRep();

  gbtMixedProfileRep<T> *Copy(void) const;
  //@}
  
  /// @name Mechanism for reference counting
  //@{
  void Reference(void);
  bool Dereference(void);
  //@}

  /// @name General information about the profile
  //@{
  gbtGame GetGame(void) const  { return const_cast<gbtTreeGameRep *>(m_game); }
  int StrategyProfileLength(void) const { return m_profile.Length(); }
  bool IsDeleted(void) const { return false; }

  bool operator==(const gbtMixedProfileRep<T> &) const;
  //@}
 
  /// @name Accessing the profile probabilities
  //@{
  T GetStrategyProb(const gbtGameStrategy &) const;
  void SetStrategyProb(const gbtGameStrategy &, const T &);
  void SetCentroid(void);
  //@}

  /// @name Vector-like operations for accessing probabilities
  //@{
  T &operator[](int a) { return m_profile[a]; }
  const T &operator[](int a) const { return m_profile[a]; }

  T &operator()(int a, int b) { return m_profile(a, b); }
  const T &operator()(int a, int b) const { return m_profile(a, b); }

  operator gbtVector<T>(void) const { return m_profile; }
  //@}

  /// @name Payoff information
  //@{
  T GetPayoff(const gbtGamePlayer &) const;
  T GetPayoffDeriv(const gbtGamePlayer &, const gbtGameStrategy &) const;
  T GetPayoffDeriv(const gbtGamePlayer &, const gbtGameStrategy &,
		   const gbtGameStrategy &) const;

  T GetLiapValue(bool penalize) const;
  //@}
};

#endif  // TABLE_STRATEGY_MIXED_H
