//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of mixed profiles for table-represented normal forms
//
// This file is part of Gambit
// Copyright (c) 2003, The Gambit Project
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

#ifndef MIXEDTABLE_H
#define MIXEDTABLE_H

#include "mixed.h"
#include "gamebase.h"    // currently for gbtNfgSupportBase only

template <class T>
class gbtMixedProfileTable : public gbtMixedProfileRep<T> {
private:
  gbtPVector<T> m_profile;
  gbtNfgSupportBase *m_support;

  // Private Payoff functions

  T PPayoff(int pl, int index, int i) const;
  void PPayoff(int pl, int const_pl, int const_st, int cur_pl, long index,
	       T prob, T&value) const;
  void PPayoff(int pl, int const_pl1, int const_st1, int const_pl2, 
	       int const_st2, int cur_pl, long index, T prob, T &value) const;
  void PPayoff(int pl, int const_pl, int cur_pl, long index, T prob,
	       gbtVector<T> &value) const;
  T Payoff(const gbtGameOutcome &o, int pl) const;

public:
  gbtMixedProfileTable(const gbtNfgSupportBase &p_support);
  gbtMixedProfileTable(const gbtMixedProfileTable<T> &);
  virtual ~gbtMixedProfileTable() { delete m_support; }

  gbtMixedProfileRep<T> *Copy(void) const;
   
  bool operator==(const gbtMixedProfileRep<T> &) const;

  // Access to individual strategy probabilities
  T &operator()(const gbtGameStrategy &p_strategy);
  const T &operator()(const gbtGameStrategy &p_strategy) const;

  // The following implement the necessary gPVector-style operations
  // traditionally permitted on mixed profiles.
  const T &operator()(int pl, int st) const { return m_profile(pl, st); }
  T &operator()(int pl, int st) { return m_profile(pl, st); }

  const T &operator[](int index) const { return m_profile[index]; }
  T &operator[](int index) { return m_profile[index]; }

  // Computations of payoffs and other values
  T GetPayoff(const gbtGamePlayer &) const;
  T GetPayoff(const gbtGamePlayer &, const gbtGameStrategy &) const;
  T GetPayoff(const gbtGamePlayer &, 
	      const gbtGameStrategy &, const gbtGameStrategy &) const;

  T GetStrategyValue(const gbtGameStrategy &p_strategy) const;

  operator gbtBehavProfile<T>(void) const;

  gbtVector<T> GetStrategy(const gbtGamePlayer &p_player) const
  { return m_profile.GetRow(p_player->GetId()); }
  void SetStrategy(const gbtGamePlayer &p_player, 
		   const gbtVector<T> &p_strategy)
  { m_profile.SetRow(p_player->GetId(), p_strategy); }
  void CopyStrategy(const gbtGamePlayer &p_player, 
		    const gbtPVector<T> &p_from) 
  { m_profile.CopyRow(p_player->GetId(), p_from); }
  void CopyStrategy(const gbtGamePlayer &p_player,
		    const gbtMixedProfile<T> &p_from)
  { m_profile.CopyRow(p_player->GetId(),
		      dynamic_cast<gbtMixedProfileTable<T> *>(p_from.Get())->m_profile); }


  // IMPLEMENTATION OF gbtGameObject INTERFACE
  gbtText GetLabel(void) const { return ""; }
  void SetLabel(const gbtText &) { }

  gbtText GetComment(void) const { return ""; }

  // IMPLEMENTATION OF gbtConstGameRep INTERFACE
  bool IsTree(void) const { return m_support->IsTree(); }
  bool IsMatrix(void) const { return m_support->IsMatrix(); }
  
  int NumPlayers(void) const { return m_support->NumPlayers(); }
  gbtGamePlayer GetPlayer(int index) const { return m_support->GetPlayer(index); }
  
  int NumOutcomes(void) const { return m_support->NumOutcomes(); }
  gbtGameOutcome GetOutcome(int index) const 
  { return m_support->GetOutcome(index); }

  bool IsConstSum(void) const { return m_support->IsConstSum(); }
  gbtNumber GetMaxPayoff(void) const { return m_support->GetMaxPayoff(); }
  gbtNumber GetMinPayoff(void) const { return m_support->GetMinPayoff(); }

  gbtMixedProfile<double> NewMixedProfile(double) const;
  gbtMixedProfile<gbtRational> NewMixedProfile(const gbtRational &) const;
  gbtMixedProfile<gbtNumber> NewMixedProfile(const gbtNumber &) const;

  // IMPLEMENTATION OF gbtConstNfgRep INTERFACE
  virtual gbtArray<int> NumStrategies(void) const 
  { return m_support->NumStrategies(); }
  virtual int MixedProfileLength(void) const
  { return m_support->MixedProfileLength(); }

  gbtNfgContingency NewContingency(void) const;

  gbtNfgSupport NewNfgSupport(void) const { return m_support->NewNfgSupport(); }
};

#endif  // MIXEDTABLE_H
