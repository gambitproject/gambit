//
// FILE: mixed.h -- Mixed strategy profile classes
//
// $Id$
//

#ifndef MIXED_H
#define MIXED_H

#include "base/gmisc.h"
#include "nfstrat.h"

#include "gpvector.h"

class Nfg;
class MixedSolution;
class NFOutcome;

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
    const T Payoff(const NFOutcome *o, int pl) const;

  public:
    MixedProfile(const NFSupport &);
    MixedProfile(const MixedProfile<T> &);
    MixedProfile(const BehavProfile<T> &);
    MixedProfile(const MixedSolution &);
    virtual ~MixedProfile();

    MixedProfile<T> &operator=(const MixedProfile<T> &);

    Nfg &Game(void) const  { return const_cast<Nfg &>(*N); }

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


