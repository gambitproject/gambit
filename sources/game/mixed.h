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

#include "base/gmisc.h"
#include "math/gpvector.h"
#include "nfstrat.h"
#include "game/outcome.h"

class Nfg;

template <class T> class gRectArray;
template <class T> class BehavProfile;

template <class T> class MixedProfile : public gPVector<T>  {
  private:
    const Nfg *N;
    NFSupport support;

    // Private Payoff functions

    T PPayoff(int pl, int index, int i) const;
    void PPayoff(int pl, int const_pl, int const_st, int cur_pl, long index,
		 T prob, T&value) const;
    void PPayoff(int pl, int const_pl1, int const_st1, int const_pl2, 
		int const_st2, int cur_pl, long index, T prob, T &value) const;
    void PPayoff(int pl, int const_pl, int cur_pl, long index, T prob,
		gVector<T> &value) const;
    void InitPayoffs(void) const;
    const T Payoff(const gbtNfgOutcome &o, int pl) const;

  public:
    MixedProfile(const NFSupport &);
    MixedProfile(const MixedProfile<T> &);
    MixedProfile(const BehavProfile<T> &);
    virtual ~MixedProfile();

    MixedProfile<T> &operator=(const MixedProfile<T> &);

    Nfg &Game(void) const  { return const_cast<Nfg &>(*N); }
    // For temporary compatibility; Game() is deprecated
    Nfg &GetGame(void) const  { return const_cast<Nfg &>(*N); }

    T LiapValue(void) const;
    void Regret(gPVector<T> &value) const;
    T MaxRegret(void) const;
    bool IsPure(void) const;
    bool IsPure(int pl) const;
    void Centroid(void);

    T Payoff(int pl) const;
    T Payoff(int pl, int player1, int strat1) const;
    T Payoff(int pl, Strategy *) const;
    T Payoff(int pl, int player1, int strat1, int player2, int strat2) const;
    void Payoff(int pl, int const_pl, gVector<T> &payoff) const;

    bool operator==(const MixedProfile<T> &) const;

    const NFSupport &Support(void) const   { return support; }
};

#ifndef __BORLANDC__
template <class T> gOutput &operator<<(gOutput &f, const MixedProfile<T> &);
#endif

#endif    // MIXED_H


