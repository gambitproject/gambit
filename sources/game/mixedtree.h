//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of mixed profiles for tree-represented normal forms
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

#ifndef MIXEDTREE_H
#define MIXEDTREE_H

#include "mixed.h"
#include "behavtree.h"
#include "gamebase.h"

template <class T>
class gbtMixedProfileTree : public gbtMixedProfileRep<T> {
public:
  gbtPVector<T> m_profile;
  gbtEfgSupportBase *m_efgSupport;
  gbtNfgSupportBase *m_nfgSupport;

  gbtMixedProfileTree(const gbtEfgSupportBase &);
  gbtMixedProfileTree(const gbtMixedProfileTree<T> &);
  gbtMixedProfileTree(const gbtBehavProfileTree<T> &);
  virtual ~gbtMixedProfileTree() { }

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
		      dynamic_cast<gbtMixedProfileTree<T> *>(p_from.Get())->m_profile); }

  // IMPLEMENTATION OF gbtGameObject INTERFACE
  gbtText GetLabel(void) const { return ""; }
  void SetLabel(const gbtText &) { }

  gbtText GetComment(void) const { return ""; }

  // IMPLEMENTATION OF gbtConstGameRep INTERFACE
  bool IsTree(void) const { return true; }
  bool IsMatrix(void) const { return false; }
  
  int NumPlayers(void) const { return m_nfgSupport->NumPlayers(); }
  gbtGamePlayer GetPlayer(int index) const { return m_nfgSupport->GetPlayer(index); }
  
  int NumOutcomes(void) const { return m_nfgSupport->NumOutcomes(); }
  gbtGameOutcome GetOutcome(int index) const 
  { return m_nfgSupport->GetOutcome(index); }

  bool IsConstSum(void) const { return m_nfgSupport->IsConstSum(); }
  gbtNumber GetMaxPayoff(void) const { return m_nfgSupport->GetMaxPayoff(); }
  gbtNumber GetMinPayoff(void) const { return m_nfgSupport->GetMinPayoff(); }

  gbtMixedProfile<double> NewMixedProfile(double) const;
  gbtMixedProfile<gbtRational> NewMixedProfile(const gbtRational &) const;
  gbtMixedProfile<gbtNumber> NewMixedProfile(const gbtNumber &) const;

  // IMPLEMENTATION OF gbtConstNfgRep INTERFACE
  gbtArray<int> NumStrategies(void) const 
  { return m_nfgSupport->NumStrategies(); }
  int MixedProfileLength(void) const
  { return m_nfgSupport->MixedProfileLength(); }

  gbtNfgContingency NewContingency(void) const;

  gbtNfgSupport NewNfgSupport(void) const 
  { return m_nfgSupport->NewNfgSupport(); }
};

#endif  // MIXEDTREE_H
