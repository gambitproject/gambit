//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of mixed strategy profile classes
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

#ifndef MIXED_H
#define MIXED_H

#include "game.h"
#include "nfgsupport.h"

template <class T> class gbtRectArray;
template <class T> class gbtBehavProfile;

template <class T>
class gbtMixedProfile : public gbtPVector<T>, public gbtConstNfgRep {
private:
  gbtNfgSupport m_support;

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
  gbtMixedProfile(const gbtNfgSupport &);
  gbtMixedProfile(const gbtMixedProfile<T> &);
  gbtMixedProfile(const gbtBehavProfile<T> &);
  virtual ~gbtMixedProfile();

  gbtMixedProfile<T> &operator=(const gbtMixedProfile<T> &);

  gbtGame GetGame(void) const  { return m_support->GetGame(); }
  
  T LiapValue(void) const;
  void Regret(gbtPVector<T> &value) const;
  T MaxRegret(void) const;
  void Centroid(void);

  T Payoff(int pl) const;
  T Payoff(int pl, gbtGameStrategy) const;
  T Payoff(int pl, int player1, int strat1, int player2, int strat2) const;
  void Payoff(int pl, int const_pl, gbtVector<T> &payoff) const;

  bool operator==(const gbtMixedProfile<T> &) const;

  const gbtNfgSupport &GetSupport(void) const   { return m_support; }


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
  gbtNumber MaxPayoff(int pl = 0) const { return m_support->MaxPayoff(pl); }
  gbtNumber MinPayoff(int pl = 0) const { return m_support->MinPayoff(pl); }

  // IMPLEMENTATION OF gbtConstNfgRep INTERFACE
  virtual gbtArray<int> NumStrategies(void) const 
  { return m_support->NumStrategies(); }
  virtual int MixedProfileLength(void) const
  { return m_support->MixedProfileLength(); }

  gbtNfgContingency NewContingency(void) const;

  gbtNfgSupport NewNfgSupport(void) const { return m_support->NewNfgSupport(); }
};


#ifndef __BORLANDC__
template <class T> gbtOutput &operator<<(gbtOutput &f, const gbtMixedProfile<T> &);
#endif

#endif    // MIXED_H
