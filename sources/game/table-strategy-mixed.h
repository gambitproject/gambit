//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of mixed strategy profile class for table games
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

#ifndef TABLE_STRATEGY_MIXED_H
#define TABLE_STRATEGY_MIXED_H

template <class T> 
class gbtTableMixedProfileRep : public gbtMixedProfileRep<T> {
public:
  const gbtTableGameRep *m_game;
  gbtPVector<T> m_profile;

  /// @name Constructors and destructor
  //@{
  gbtTableMixedProfileRep(const gbtTableGameRep *);
  virtual ~gbtTableMixedProfileRep();

  gbtMixedProfileRep<T> *Copy(void) const;
  //@}

  /// @name General information about the profile
  //@{
  gbtGame GetGame(void) const { return const_cast<gbtTableGameRep *>(m_game); }
  int StrategyProfileLength(void) const { return m_profile.Length(); }

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
  T GetPayoffDeriv(const gbtGamePlayer &,
		   const gbtGameStrategy &, const gbtGameStrategy &) const;
  T GetLiapValue(bool penalize) const;

  //@}

  /// @name Member functions private to the implementation
  //@{
  T GetPayoff(gbtTablePlayerRep *, long index, int pl) const;
  T GetPayoffDeriv(gbtTablePlayerRep *, gbtTablePlayerRep *,
		   long index, int pl) const;
  T GetPayoffDeriv(gbtTablePlayerRep *, 
		   gbtTablePlayerRep *, gbtTablePlayerRep *,
		   long index, int pl) const;
  //@}
};

#endif  // TABLE_STRATEGY_MIXED_H
